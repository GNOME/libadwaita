/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-indicator-bin-private.h"

#include "adw-gizmo-private.h"

/**
 * AdwIndicatorBin:
 *
 * A helper object for [class@Adw.ViewSwitcherButton]
 *
 * The `AdwIndicatorBin` widget shows an unread indicator over the child widget
 * masking it if they overlap.
 *
 * Since: 1.0
 */

struct _AdwIndicatorBin
{
  GtkWidget parent_instance;

  GtkWidget *child;
  gboolean show_indicator;
  gboolean contained;

  GtkWidget *mask;
  GtkWidget *indicator;

  GskGLShader *shader;
  gboolean shader_compiled;
};

static void adw_indicator_bin_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwIndicatorBin, adw_indicator_bin, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_indicator_bin_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  PROP_SHOW_INDICATOR,
  PROP_CONTAINED,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];


static void
ensure_shader (AdwIndicatorBin *self)
{
  GtkNative *native;
  GskRenderer *renderer;
  g_autoptr (GError) error = NULL;

  if (self->shader)
    return;

  self->shader = gsk_gl_shader_new_from_resource ("/org/gnome/Adwaita/glsl/mask.glsl");

  native = gtk_widget_get_native (GTK_WIDGET (self));
  renderer = gtk_native_get_renderer (native);

  self->shader_compiled = gsk_gl_shader_compile (self->shader, renderer, &error);

  if (error) {
    /* If shaders aren't supported, the error doesn't matter and we just
     * silently fall back */
    if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
      g_critical ("Couldn't compile shader: %s\n", error->message);
  }
}

static void
adw_indicator_bin_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           int             for_size,
                           int            *min,
                           int            *nat,
                           int            *min_baseline,
                           int            *nat_baseline)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (widget);

  if (!self->child) {
    if (min)
      *min = 0;
    if (nat)
      *nat = 0;
    if (min_baseline)
      *min_baseline = -1;
    if (nat_baseline)
      *nat_baseline = -1;

    return;
  }

  gtk_widget_measure (self->child, orientation, for_size,
                      min, nat, min_baseline, nat_baseline);
}

static void
adw_indicator_bin_size_allocate (GtkWidget *widget,
                                 int        width,
                                 int        height,
                                 int        baseline)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (widget);
  GtkRequisition size;
  float x, y;

  if (self->child)
    gtk_widget_allocate (self->child, width, height, baseline, NULL);

  gtk_widget_get_preferred_size (self->indicator, NULL, &size);

  if (self->contained) {
    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
      x = 0;
    else
      x = width - size.width;

    y = 0;
  } else {
    if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
      x = -size.width / 2.0f;
    else
      x = width - size.width / 2.0f;

    y = -size.height / 2.0f;
  }

  gtk_widget_allocate (self->mask, size.width, size.height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
  gtk_widget_allocate (self->indicator, size.width, size.height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
}

static void
adw_indicator_bin_snapshot (GtkWidget   *widget,
                            GtkSnapshot *snapshot)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (widget);

  if (!self->show_indicator) {
    if (self->child)
      gtk_widget_snapshot_child (widget, self->child, snapshot);

    return;
  }

  if (self->child) {
    GtkSnapshot *child_snapshot;
    g_autoptr (GskRenderNode) child_node = NULL;

    child_snapshot = gtk_snapshot_new ();
    gtk_widget_snapshot_child (widget, self->child, child_snapshot);
    child_node = gtk_snapshot_free_to_node (child_snapshot);

    ensure_shader (self);

    if (self->shader_compiled) {
      graphene_rect_t bounds;

      gsk_render_node_get_bounds (child_node, &bounds);
      gtk_snapshot_push_gl_shader (snapshot, self->shader, &bounds,
                                   gsk_gl_shader_format_args (self->shader, NULL));
    }

    gtk_snapshot_append_node (snapshot, child_node);

    if (self->shader_compiled) {
      gtk_snapshot_gl_shader_pop_texture (snapshot);

      gtk_widget_snapshot_child (widget, self->mask, snapshot);
      gtk_snapshot_gl_shader_pop_texture (snapshot);

      gtk_snapshot_pop (snapshot);
    }
  }

  gtk_widget_snapshot_child (widget, self->indicator, snapshot);
}

static void
adw_indicator_bin_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_indicator_bin_get_child (self));
    break;

  case PROP_SHOW_INDICATOR:
    g_value_set_boolean (value, adw_indicator_bin_get_show_indicator (self));
    break;

  case PROP_CONTAINED:
    g_value_set_boolean (value, adw_indicator_bin_get_contained (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_indicator_bin_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_indicator_bin_set_child (self, g_value_get_object (value));
    break;

  case PROP_SHOW_INDICATOR:
    adw_indicator_bin_set_show_indicator (self, g_value_get_boolean (value));
    break;

  case PROP_CONTAINED:
    adw_indicator_bin_set_contained (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_indicator_bin_dispose (GObject *object)
{
  AdwIndicatorBin *self = ADW_INDICATOR_BIN (object);

  g_clear_object (&self->shader);
  g_clear_pointer (&self->child, gtk_widget_unparent);
  g_clear_pointer (&self->mask, gtk_widget_unparent);
  g_clear_pointer (&self->indicator, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_indicator_bin_parent_class)->dispose (object);
}
static void
adw_indicator_bin_class_init (AdwIndicatorBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_indicator_bin_get_property;
  object_class->set_property = adw_indicator_bin_set_property;
  object_class->dispose = adw_indicator_bin_dispose;

  widget_class->measure = adw_indicator_bin_measure;
  widget_class->size_allocate = adw_indicator_bin_size_allocate;
  widget_class->snapshot = adw_indicator_bin_snapshot;

  /**
   * AdwIndicatorBin:child:
   *
   * The child widget.
   *
   * Since: 1.0
   */
  props[PROP_CHILD] =
      g_param_spec_object ("child",
                           "Child",
                           "The child widget",
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIndicatorBin:show-indicator:
   *
   * Whether to show the indicator.
   *
   * Since: 1.0
   */
  props[PROP_SHOW_INDICATOR] =
    g_param_spec_boolean ("show-indicator",
                          "Show Indicator",
                          "Whether to show the indicator",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIndicatorBin:contained:
   *
   * Whether the indicator is centered on the top end corner of the widget or is
   * at the top end corner but contained in the widget bounds.
   *
   * Since: 1.0
   */
  props[PROP_CONTAINED] =
    g_param_spec_boolean ("contained",
                          "Contained",
                          "Whether the indicator is contained in the widget bounds",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "indicatorbin");
}

static void
adw_indicator_bin_init (AdwIndicatorBin *self)
{
  self->mask = adw_gizmo_new ("mask", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_can_target (self->mask, FALSE);
  gtk_widget_set_parent (self->mask, GTK_WIDGET (self));

  self->indicator = adw_gizmo_new ("indicator", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_can_target (self->indicator, FALSE);
  gtk_widget_set_parent (self->indicator, GTK_WIDGET (self));
}

static void
adw_indicator_bin_buildable_add_child (GtkBuildable *buildable,
                                       GtkBuilder   *builder,
                                       GObject      *child,
                                       const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_indicator_bin_set_child (ADW_INDICATOR_BIN (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_indicator_bin_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_indicator_bin_buildable_add_child;
}

/**
 * adw_indicator_bin_new:
 *
 * Creates a new `AdwIndicatorBin`.
 *
 * Returns: the newly created `AdwIndicatorBin`
 *
 * Since: 1.0
 */
GtkWidget *
adw_indicator_bin_new (void)
{
  return g_object_new (ADW_TYPE_INDICATOR_BIN, NULL);
}

/**
 * adw_indicator_bin_get_child:
 * @self: an `AdwIndicatorBin`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.0
 */
GtkWidget *
adw_indicator_bin_get_child (AdwIndicatorBin *self)
{
  g_return_val_if_fail (ADW_IS_INDICATOR_BIN (self), NULL);

  return self->child;
}

/**
 * adw_indicator_bin_set_child:
 * @self: an `AdwIndicatorBin`
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.0
 */
void
adw_indicator_bin_set_child (AdwIndicatorBin *self,
                             GtkWidget       *child)
{
  g_return_if_fail (ADW_IS_INDICATOR_BIN (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (self->child == child)
    return;

  if (self->child)
    gtk_widget_unparent (self->child);

  self->child = child;

  if (self->child)
    gtk_widget_set_parent (self->child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

gboolean
adw_indicator_bin_get_show_indicator (AdwIndicatorBin *self)
{
  g_return_val_if_fail (ADW_IS_INDICATOR_BIN (self), FALSE);

  return self->show_indicator;
}

void
adw_indicator_bin_set_show_indicator (AdwIndicatorBin *self,
                                      gboolean         show_indicator)
{
  g_return_if_fail (ADW_IS_INDICATOR_BIN (self));

  show_indicator = !!show_indicator;

  if (self->show_indicator == show_indicator)
    return;

  self->show_indicator = show_indicator;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_INDICATOR]);
}

gboolean
adw_indicator_bin_get_contained (AdwIndicatorBin *self)
{
  g_return_val_if_fail (ADW_IS_INDICATOR_BIN (self), FALSE);

  return self->contained;
}

void
adw_indicator_bin_set_contained (AdwIndicatorBin *self,
                                 gboolean         contained)
{
  g_return_if_fail (ADW_IS_INDICATOR_BIN (self));

  contained = !!contained;

  if (self->contained == contained)
    return;

  self->contained = contained;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTAINED]);
}
