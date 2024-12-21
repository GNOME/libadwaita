/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-adaptive-preview-private.h"

#include "adw-bin.h"
#include "adw-combo-row.h"
#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"
#include <math.h>

#define MIN_SCALE 0.25
#define MAX_SCALE 1.0

typedef enum {
  ROTATION_0DEG,
  ROTATION_90DEG,
  ROTATION_180DEG,
  ROTATION_270DEG,
} ScreenRotation;

typedef struct {
  const char *name;
  int top_bar;
  int bottom_bar;
} ShellPreset;

typedef struct {
  const char *name;
  int width;
  int height;
} DevicePreset;

static const ShellPreset shell_presets[] = {
  { N_("Desktop Shell"), 32, 0  },
  { N_("Mobile Shell"),  26, 18 },
  { N_("Phosh"),         32, 15 },
  { N_("Custom"),        -1, -1 },
};

static const DevicePreset device_presets[] = {
  { N_("Small Phone"), 360,  720 },
  { N_("Large Phone"), 360,  760 },
  { N_("Tablet"),      1280, 800 },
  { N_("Custom"),      -1,   -1  },
};

// Mobile shell
#define DEFAULT_SHELL_PRESET 1

// L5, PP
#define DEFAULT_DEVICE_PRESET 0

struct _AdwAdaptivePreview
{
  GtkWidget parent_instance;

  GtkWidget *content;
  GtkWidget *scale_bin;
  GtkWidget *screen_view;
  GtkWidget *child_bin;
  GtkWidget *top_bar;
  GtkWidget *bottom_bar;

  AdwComboRow *device_preset_row;
  GtkWidget *width_row;
  GtkAdjustment *width_adj;
  GtkWidget *height_row;
  GtkAdjustment *height_adj;

  AdwComboRow *shell_preset_row;
  GtkWidget *top_bar_row;
  GtkAdjustment *top_bar_adj;
  GtkWidget *bottom_bar_row;
  GtkAdjustment *bottom_bar_adj;

  int screen_width;
  int screen_height;
  int top_bar_height;
  int bottom_bar_height;
  ScreenRotation rotation;
  gboolean scale_to_fit;

  gboolean changing_screen_size;
  gboolean changing_shell;

  gboolean window_controls;
};

G_DEFINE_FINAL_TYPE (AdwAdaptivePreview, adw_adaptive_preview, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_CHILD,
  PROP_WINDOW_CONTROLS,
  PROP_SCALE_TO_FIT,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_EXIT,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static int
get_screen_width (AdwAdaptivePreview *self)
{
  gboolean rotated = self->rotation == ROTATION_90DEG ||
                     self->rotation == ROTATION_270DEG;

  return rotated ? self->screen_height : self->screen_width;
}

static int
get_screen_height (AdwAdaptivePreview *self)
{
  gboolean rotated = self->rotation == ROTATION_90DEG ||
                     self->rotation == ROTATION_270DEG;

  return rotated ? self->screen_width : self->screen_height;
}

static void
screen_size_changed_cb (AdwAdaptivePreview *self)
{
  if (self->changing_screen_size)
    return;

  int new_width = gtk_adjustment_get_value (self->width_adj);
  int new_height = gtk_adjustment_get_value (self->height_adj);

  if (self->screen_width == new_width && self->screen_height == new_height)
    return;

  self->screen_width = new_width;
  self->screen_height = new_height;

  gtk_widget_queue_resize (self->screen_view);
}

static void
device_preset_cb (AdwAdaptivePreview *self)
{
  int selected = adw_combo_row_get_selected (self->device_preset_row);
  const DevicePreset *preset;

  g_assert (selected < G_N_ELEMENTS (device_presets));

  preset = &device_presets[selected];

  gtk_widget_set_sensitive (self->width_row, preset->width < 0);
  gtk_widget_set_sensitive (self->height_row, preset->height < 0);

  if (preset->width < 0 && preset->height < 0)
    return;

  self->changing_screen_size = TRUE;
  if (preset->width >= 0)
    gtk_adjustment_set_value (self->width_adj, preset->width);
  if (preset->height >= 0)
    gtk_adjustment_set_value (self->height_adj, preset->height);
  self->changing_screen_size = FALSE;

  screen_size_changed_cb (self);
}

static void
shell_changed_cb (AdwAdaptivePreview *self)
{
  if (self->changing_shell)
    return;

  int top_bar = gtk_adjustment_get_value (self->top_bar_adj);
  int bottom_bar = gtk_adjustment_get_value (self->bottom_bar_adj);

  if (self->top_bar_height == top_bar && self->bottom_bar_height == bottom_bar)
    return;

  self->top_bar_height = top_bar;
  self->bottom_bar_height = bottom_bar;

  gtk_widget_set_size_request (self->top_bar, -1, top_bar);
  gtk_widget_set_size_request (self->bottom_bar, -1, bottom_bar);

  gtk_widget_set_visible (self->top_bar, top_bar > 0);
  gtk_widget_set_visible (self->bottom_bar, bottom_bar > 0);

  gtk_widget_queue_resize (self->screen_view);
}

static void
shell_preset_cb (AdwAdaptivePreview *self)
{
  int selected = adw_combo_row_get_selected (self->shell_preset_row);
  const ShellPreset *preset;

  g_assert (selected < G_N_ELEMENTS (shell_presets));

  preset = &shell_presets[selected];

  gtk_widget_set_sensitive (self->top_bar_row, preset->top_bar < 0);
  gtk_widget_set_sensitive (self->bottom_bar_row, preset->bottom_bar < 0);

  if (preset->top_bar < 0 && preset->bottom_bar < 0)
    return;

  self->changing_shell = TRUE;
  if (preset->top_bar >= 0)
    gtk_adjustment_set_value (self->top_bar_adj, preset->top_bar);
  if (preset->bottom_bar >= 0)
    gtk_adjustment_set_value (self->bottom_bar_adj, preset->bottom_bar);
  self->changing_shell = FALSE;

  shell_changed_cb (self);
}

static void
rotate_left_cb (AdwAdaptivePreview *self)
{
  if (self->rotation == ROTATION_270DEG)
    self->rotation = ROTATION_0DEG;
  else
    self->rotation++;

  gtk_widget_queue_resize (self->screen_view);
}

static void
rotate_right_cb (AdwAdaptivePreview *self)
{
  if (self->rotation == ROTATION_0DEG)
    self->rotation = ROTATION_270DEG;
  else
    self->rotation--;

  gtk_widget_queue_resize (self->screen_view);
}

static void
exit_clicked_cb (AdwAdaptivePreview *self)
{
  g_signal_emit (self, signals[SIGNAL_EXIT], 0);
}

static void
setup_presets (AdwAdaptivePreview *self)
{
  GtkStringList *shells = gtk_string_list_new (NULL);
  GtkStringList *devices = gtk_string_list_new (NULL);
  int i;

  for (i = 0; i < G_N_ELEMENTS (shell_presets); i++) {
    const ShellPreset *preset = &shell_presets[i];
    gtk_string_list_append (shells, _(preset->name));
  }

  for (i = 0; i < G_N_ELEMENTS (device_presets); i++) {
    const DevicePreset *preset = &device_presets[i];
    gtk_string_list_append (devices, _(preset->name));
  }

  adw_combo_row_set_model (self->shell_preset_row, G_LIST_MODEL (shells));
  adw_combo_row_set_selected (self->shell_preset_row, DEFAULT_SHELL_PRESET);

  adw_combo_row_set_model (self->device_preset_row, G_LIST_MODEL (devices));
  adw_combo_row_set_selected (self->device_preset_row, DEFAULT_DEVICE_PRESET);
}

static void
measure_screen_view (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  AdwAdaptivePreview *self =
    ADW_ADAPTIVE_PREVIEW (gtk_widget_get_ancestor (widget, ADW_TYPE_ADAPTIVE_PREVIEW));
  int min, nat, top_min, bottom_min;

  if (gtk_widget_should_layout (self->top_bar)) {
    gtk_widget_measure (self->top_bar, orientation, for_size,
                        &top_min, NULL, NULL, NULL);
  } else {
    top_min = 0;
  }

  if (gtk_widget_should_layout (self->bottom_bar)) {
    gtk_widget_measure (self->bottom_bar, orientation, for_size,
                        &bottom_min, NULL, NULL, NULL);
  } else {
    bottom_min = 0;
  }

  if (orientation == GTK_ORIENTATION_VERTICAL)
    min = top_min + bottom_min;
  else
    min = MAX (top_min, bottom_min);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    nat = get_screen_width (self);
  else
    nat = get_screen_height (self);

  min = MAX (min, nat);

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_screen_view (GtkWidget *widget,
                      int        width,
                      int        height,
                      int        baseline)
{
  AdwAdaptivePreview *self =
    ADW_ADAPTIVE_PREVIEW (gtk_widget_get_ancestor (widget, ADW_TYPE_ADAPTIVE_PREVIEW));
  int top_bar_height = 0, bottom_bar_height = 0;

  if (gtk_widget_should_layout (self->top_bar)) {
    gtk_widget_measure (self->top_bar, GTK_ORIENTATION_VERTICAL, -1,
                        &top_bar_height, NULL, NULL, NULL);
    gtk_widget_allocate (self->top_bar, width, top_bar_height, -1, NULL);
  }

  if (gtk_widget_should_layout (self->bottom_bar)) {
    int bottom_bar_y;
    GskTransform *transform;

    gtk_widget_measure (self->bottom_bar, GTK_ORIENTATION_VERTICAL, -1,
                        &bottom_bar_height, NULL, NULL, NULL);

    bottom_bar_y = height - bottom_bar_height;

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, bottom_bar_y));

    gtk_widget_allocate (self->bottom_bar, width, bottom_bar_height, -1, transform);
  }

  if (gtk_widget_should_layout (self->child_bin)) {
    int child_width, child_height, available_height;
    GskTransform *transform;

    gtk_widget_measure (self->child_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                        &child_width, NULL, NULL, NULL);
    gtk_widget_measure (self->child_bin, GTK_ORIENTATION_VERTICAL, -1,
                        &child_height, NULL, NULL, NULL);

    available_height = height - top_bar_height - bottom_bar_height;

    if (child_width > width || child_height > available_height) {
      g_warning ("Window contents don't fit: provided %d×%d, available %d×%d",
                 child_width, child_height, width, available_height);
    }

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, self->top_bar_height));

    gtk_widget_allocate (self->child_bin, width, available_height, -1, transform);
  }
}

static void
measure_scale_bin (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  AdwAdaptivePreview *self =
    ADW_ADAPTIVE_PREVIEW (gtk_widget_get_ancestor (widget, ADW_TYPE_ADAPTIVE_PREVIEW));
  GtkWidget *child = gtk_widget_get_first_child (widget);
  int nat;

  if (gtk_widget_should_layout (child))
    gtk_widget_measure (child, orientation, for_size, NULL, &nat, NULL, NULL);
  else
    nat = 0;

  if (minimum)
    *minimum = (int) round (nat * (self->scale_to_fit ? MIN_SCALE : 1.0));
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_scale_bin (GtkWidget *widget,
                    int        width,
                    int        height,
                    int        baseline)
{
  AdwAdaptivePreview *self =
    ADW_ADAPTIVE_PREVIEW (gtk_widget_get_ancestor (widget, ADW_TYPE_ADAPTIVE_PREVIEW));
  GtkWidget *child = gtk_widget_get_first_child (widget);
  int child_width, child_height;
  GskTransform *transform = NULL;
  float scale;

  if (!gtk_widget_should_layout (child))
    return;

  child_width = get_screen_width (self);
  child_height = get_screen_height (self);

  if (self->scale_to_fit) {
    scale = MIN ((float) width / (float) child_width,
                 (float) height / (float) child_height);
    scale = CLAMP (scale, MIN_SCALE, MAX_SCALE);
  } else {
    scale = 1.0;
  }

  transform = gsk_transform_translate (transform,
                                       &GRAPHENE_POINT_INIT (roundf (width / 2.0f),
                                                             roundf (height / 2.0f)));
  transform = gsk_transform_scale (transform, scale, scale);
  transform = gsk_transform_translate (transform,
                                       &GRAPHENE_POINT_INIT (-roundf (child_width / 2.0f),
                                                             -roundf (child_height / 2.0f)));

  gtk_widget_allocate (child, child_width, child_height, -1, transform);
}

static void
adw_adaptive_preview_dispose (GObject *object)
{
  AdwAdaptivePreview *self = ADW_ADAPTIVE_PREVIEW (object);

  gtk_widget_dispose_template (GTK_WIDGET (self), ADW_TYPE_ADAPTIVE_PREVIEW);

  G_OBJECT_CLASS (adw_adaptive_preview_parent_class)->dispose (object);
}

static void
adw_adaptive_preview_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  AdwAdaptivePreview *self = ADW_ADAPTIVE_PREVIEW (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_adaptive_preview_get_child (self));
    break;
  case PROP_WINDOW_CONTROLS:
    g_value_set_boolean (value, adw_adaptive_preview_get_window_controls (self));
    break;
  case PROP_SCALE_TO_FIT:
    g_value_set_boolean (value, adw_adaptive_preview_get_scale_to_fit (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_adaptive_preview_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  AdwAdaptivePreview *self = ADW_ADAPTIVE_PREVIEW (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_adaptive_preview_set_child (self, g_value_get_object (value));
    break;
  case PROP_WINDOW_CONTROLS:
    adw_adaptive_preview_set_window_controls (self, g_value_get_boolean (value));
    break;
  case PROP_SCALE_TO_FIT:
    adw_adaptive_preview_set_scale_to_fit (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_adaptive_preview_class_init (AdwAdaptivePreviewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_adaptive_preview_dispose;
  object_class->get_property = adw_adaptive_preview_get_property;
  object_class->set_property = adw_adaptive_preview_set_property;

  widget_class->compute_expand = adw_widget_compute_expand;

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);

  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_WINDOW_CONTROLS] =
    g_param_spec_boolean ("window-controls", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_SCALE_TO_FIT] =
    g_param_spec_boolean ("scale-to-fit", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
  gtk_widget_class_set_css_name (widget_class, "adaptive-preview");

  signals[SIGNAL_EXIT] =
    g_signal_new ("exit",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_EXIT],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-adaptive-preview.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, content);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, scale_bin);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, screen_view);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, child_bin);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, top_bar);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, bottom_bar);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, device_preset_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, width_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, width_adj);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, height_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, height_adj);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, shell_preset_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, top_bar_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, top_bar_adj);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, bottom_bar_row);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, bottom_bar_adj);

  gtk_widget_class_bind_template_callback (widget_class, screen_size_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, device_preset_cb);
  gtk_widget_class_bind_template_callback (widget_class, shell_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, shell_preset_cb);
  gtk_widget_class_bind_template_callback (widget_class, rotate_left_cb);
  gtk_widget_class_bind_template_callback (widget_class, rotate_right_cb);
  gtk_widget_class_bind_template_callback (widget_class, exit_clicked_cb);
}

static void
adw_adaptive_preview_init (AdwAdaptivePreview *self)
{
  self->window_controls = TRUE;
  self->rotation = ROTATION_0DEG;
  self->scale_to_fit = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_set_layout_manager (self->screen_view,
                                 gtk_custom_layout_new (NULL,
                                                        measure_screen_view,
                                                        allocate_screen_view));

  gtk_widget_set_layout_manager (self->scale_bin,
                                 gtk_custom_layout_new (NULL,
                                                        measure_scale_bin,
                                                        allocate_scale_bin));

  setup_presets (self);

  gtk_adjustment_set_value (self->width_adj, 360);
  gtk_adjustment_set_value (self->height_adj, 720);
}

GtkWidget *
adw_adaptive_preview_new (void)
{
  return g_object_new (ADW_TYPE_ADAPTIVE_PREVIEW, NULL);
}

GtkWidget *
adw_adaptive_preview_get_child (AdwAdaptivePreview *self)
{
  g_return_val_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self), NULL);

  return adw_bin_get_child (ADW_BIN (self->child_bin));
}

void
adw_adaptive_preview_set_child (AdwAdaptivePreview *self,
                                GtkWidget          *child)
{
  g_return_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child == adw_adaptive_preview_get_child (self))
    return;

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  adw_bin_set_child (ADW_BIN (self->child_bin), child);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

gboolean
adw_adaptive_preview_get_window_controls (AdwAdaptivePreview *self)
{
  g_return_val_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self), FALSE);

  return self->window_controls;
}

void
adw_adaptive_preview_set_window_controls (AdwAdaptivePreview *self,
                                          gboolean            window_controls)
{
  g_return_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self));

  window_controls = !!window_controls;

  if (window_controls == self->window_controls)
    return;

  self->window_controls = window_controls;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WINDOW_CONTROLS]);
}

gboolean
adw_adaptive_preview_get_scale_to_fit (AdwAdaptivePreview *self)
{
  g_return_val_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self), FALSE);

  return self->scale_to_fit;
}

void
adw_adaptive_preview_set_scale_to_fit (AdwAdaptivePreview *self,
                                       gboolean            scale_to_fit)
{
  g_return_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self));

  scale_to_fit = !!scale_to_fit;

  if (scale_to_fit == self->scale_to_fit)
    return;

  self->scale_to_fit = scale_to_fit;

  gtk_widget_queue_resize (self->scale_bin);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SCALE_TO_FIT]);
}

GtkWidget *
adw_adaptive_preview_get_screen (AdwAdaptivePreview *self)
{
  return self->screen_view;
}
