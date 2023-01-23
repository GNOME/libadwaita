/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-fading-label-private.h"

#include <glib/gi18n-lib.h>
#include "adw-bidi-private.h"

#define FADE_WIDTH 18.0f

struct _AdwFadingLabel
{
  GtkWidget parent_instance;

  GtkWidget *label;
  float align;

  GskGLShader *shader;
  gboolean shader_compiled;
};

G_DEFINE_FINAL_TYPE (AdwFadingLabel, adw_fading_label, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_LABEL,
  PROP_ALIGN,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static gboolean
is_rtl (AdwFadingLabel *self)
{
  PangoDirection pango_direction = PANGO_DIRECTION_NEUTRAL;
  const char *label = adw_fading_label_get_label (self);

  if (label)
    pango_direction = adw_find_base_dir (label, -1);

  if (pango_direction == PANGO_DIRECTION_RTL)
    return TRUE;

  if (pango_direction == PANGO_DIRECTION_LTR)
    return FALSE;

  return gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
}

static void
ensure_shader (AdwFadingLabel *self)
{
  GtkNative *native;
  GskRenderer *renderer;
  GError *error = NULL;

  if (self->shader)
    return;

  self->shader = gsk_gl_shader_new_from_resource ("/org/gnome/Adwaita/glsl/fade.glsl");

  native = gtk_widget_get_native (GTK_WIDGET (self));
  renderer = gtk_native_get_renderer (native);

  self->shader_compiled = gsk_gl_shader_compile (self->shader, renderer, &error);

  if (error) {
    /* If shaders aren't supported, the error doesn't matter and we just
     * silently fall back */
    if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
      g_warning ("Couldn't compile shader: %s\n", error->message);
  }

  g_clear_error (&error);
}

static void
adw_fading_label_measure (GtkWidget      *widget,
                          GtkOrientation  orientation,
                          int             for_size,
                          int             *min,
                          int             *nat,
                          int             *min_baseline,
                          int             *nat_baseline)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (widget);

  gtk_widget_measure (self->label, orientation, for_size,
                      min, nat, min_baseline, nat_baseline);

  if (orientation == GTK_ORIENTATION_HORIZONTAL && min)
    *min = 0;
}

static void
adw_fading_label_size_allocate (GtkWidget *widget,
                                int        width,
                                int        height,
                                int        baseline)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (widget);
  float align = is_rtl (self) ? 1 - self->align : self->align;
  int child_width;
  float offset;
  GskTransform *transform;

  gtk_widget_measure (self->label, GTK_ORIENTATION_HORIZONTAL, height,
                      NULL, &child_width, NULL, NULL);

  offset = (width - child_width) * align;
  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (offset, 0));

  gtk_widget_allocate (self->label, child_width, height, baseline, transform);
}

static void
adw_fading_label_snapshot (GtkWidget   *widget,
                           GtkSnapshot *snapshot)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (widget);
  float align = is_rtl (self) ? 1 - self->align : self->align;
  int width = gtk_widget_get_width (widget);
  int clipped_size;
  GtkSnapshot *child_snapshot;
  GskRenderNode *node;
  graphene_rect_t bounds;

  if (width <= 0)
    return;

  clipped_size = gtk_widget_get_allocated_width (self->label) - width;

  if (clipped_size <= 0) {
    gtk_widget_snapshot_child (widget, self->label, snapshot);

    return;
  }

  child_snapshot = gtk_snapshot_new ();
  gtk_widget_snapshot_child (widget, self->label, child_snapshot);
  node = gtk_snapshot_free_to_node (child_snapshot);

  gsk_render_node_get_bounds (node, &bounds);
  bounds.origin.x = 0;
  bounds.origin.y = floor (bounds.origin.y);
  bounds.size.width = width;
  bounds.size.height = ceil (bounds.size.height);

  ensure_shader (self);

  if (self->shader_compiled) {
    gtk_snapshot_push_gl_shader (snapshot, self->shader, &bounds,
                                 gsk_gl_shader_format_args (self->shader,
                                                            "offsetLeft", 0.0f,
                                                            "offsetRight", 0.0f,
                                                            "strengthLeft", align > 0 ? 1.0f : 0.0f,
                                                            "strengthRight", align < 1 ? 1.0f : 0.0f,
                                                            "widthLeft", FADE_WIDTH,
                                                            "widthRight", FADE_WIDTH,
                                                            NULL));
  } else {
    gtk_snapshot_push_clip (snapshot, &bounds);
  }

  gtk_snapshot_append_node (snapshot, node);

  if (self->shader_compiled)
    gtk_snapshot_gl_shader_pop_texture (snapshot);

  gtk_snapshot_pop (snapshot);

  gsk_render_node_unref (node);
}

static void
adw_fading_label_unrealize (GtkWidget *widget)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (widget);

  GTK_WIDGET_CLASS (adw_fading_label_parent_class)->unrealize (widget);

  g_clear_object (&self->shader);
}

static void
adw_fading_label_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (object);

  switch (prop_id) {
  case PROP_LABEL:
    g_value_set_string (value, adw_fading_label_get_label (self));
    break;

  case PROP_ALIGN:
    g_value_set_float (value, adw_fading_label_get_align (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_fading_label_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (object);

  switch (prop_id) {
  case PROP_LABEL:
    adw_fading_label_set_label (self, g_value_get_string (value));
    break;

  case PROP_ALIGN:
    adw_fading_label_set_align (self, g_value_get_float (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_fading_label_dispose (GObject *object)
{
  AdwFadingLabel *self = ADW_FADING_LABEL (object);

  g_clear_pointer (&self->label, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_fading_label_parent_class)->dispose (object);
}

static void
adw_fading_label_class_init (AdwFadingLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_fading_label_get_property;
  object_class->set_property = adw_fading_label_set_property;
  object_class->dispose = adw_fading_label_dispose;

  widget_class->measure = adw_fading_label_measure;
  widget_class->size_allocate = adw_fading_label_size_allocate;
  widget_class->snapshot = adw_fading_label_snapshot;
  widget_class->unrealize = adw_fading_label_unrealize;

  props[PROP_LABEL] =
    g_param_spec_string ("label", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ALIGN] =
    g_param_spec_float ("align", NULL, NULL,
                        0.0, 1.0, 0.0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_fading_label_init (AdwFadingLabel *self)
{
  self->label = gtk_label_new (NULL);
  gtk_label_set_single_line_mode (GTK_LABEL (self->label), TRUE);

  gtk_widget_set_parent (self->label, GTK_WIDGET (self));
}

const char *
adw_fading_label_get_label (AdwFadingLabel *self)
{
  g_return_val_if_fail (ADW_IS_FADING_LABEL (self), NULL);

  return gtk_label_get_label (GTK_LABEL (self->label));
}

void
adw_fading_label_set_label (AdwFadingLabel *self,
                            const char     *label)
{
  g_return_if_fail (ADW_IS_FADING_LABEL (self));

  if (!g_strcmp0 (label, adw_fading_label_get_label (self)))
    return;

  gtk_label_set_label (GTK_LABEL (self->label), label);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LABEL]);
}

float
adw_fading_label_get_align (AdwFadingLabel *self)
{
  g_return_val_if_fail (ADW_IS_FADING_LABEL (self), 0.0f);

  return self->align;
}

void
adw_fading_label_set_align (AdwFadingLabel *self,
                            float           align)
{
  g_return_if_fail (ADW_IS_FADING_LABEL (self));

  align = CLAMP (align, 0.0, 1.0);

  if (G_APPROX_VALUE (self->align, align, FLT_EPSILON))
    return;

  self->align = align;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALIGN]);
}
