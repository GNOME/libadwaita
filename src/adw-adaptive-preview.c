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
#include "adw-adaptive-preview-presets-private.h"

#include "adw-animation.h"
#include "adw-animation-util.h"
#include "adw-bin.h"
#include "adw-combo-row.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-spring-animation.h"
#include "adw-toast.h"
#include "adw-toast-overlay.h"
#include "adw-widget-utils-private.h"
#include "adw-window-title.h"
#include <math.h>

#define MIN_SCALE 0.25
#define MAX_SCALE 1.0

#define DEG_TO_RAD(x) ((x) * G_PI / 180)

typedef enum {
  ROTATION_0DEG,
  ROTATION_90DEG,
  ROTATION_180DEG,
  ROTATION_270DEG,
} ScreenRotation;

struct _AdwAdaptivePreview
{
  GtkWidget parent_instance;

  AdwToastOverlay *toast_overlay;
  GtkWidget *scale_bin;
  GtkWidget *device_container;
  GtkWidget *device_view;
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

  AdwWindowTitle *content_title;

  int screen_width;
  int screen_height;
  int top_bar_height;
  int bottom_bar_height;
  ScreenRotation rotation;
  gboolean scale_to_fit;
  float screen_scale;
  const char *notches;

  gboolean highlight_bezel;

  gboolean changing_screen_size;
  gboolean changing_shell;

  gboolean window_controls;

  int last_device_preset;
  AdwAnimation *rotate_animation;

  GdkPaintable *device_paintable;
};

static GtkCssProvider *css_provider = NULL;

G_DEFINE_FINAL_TYPE (AdwAdaptivePreview, adw_adaptive_preview, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_CHILD,
  PROP_WINDOW_CONTROLS,
  PROP_SCALE_TO_FIT,
  PROP_HIGHLIGHT_BEZEL,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_EXIT,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static inline float
get_dpi (const DevicePreset *preset)
{
  return sqrtf (preset->width * preset->width +
                preset->height * preset->height) / preset->screen_diagonal;
}

static inline void
append_variable (GString    *string,
                 const char *name,
                 float       value)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  g_ascii_dtostr (buf, sizeof (buf), value);

  g_string_append_printf (string, "  %s: %spx;\n", name, buf);
}

static void
generate_device_css (void)
{
  GString *string = g_string_new (NULL);
  int i;

  for (i = 0; i < G_N_ELEMENTS (device_presets); i++) {
    const DevicePreset *preset = &device_presets[i];
    float dpi = get_dpi (preset);

    g_string_append_printf (string, "adaptive-preview .device-view.%s {\n",
                            preset->id);

    append_variable (string, "--top-screen-corner-radius",
                     preset->top_screen_corners / preset->scale_factor);
    append_variable (string, "--bottom-screen-corner-radius",
                     preset->bottom_screen_corners / preset->scale_factor);

    append_variable (string, "--top-device-corner-radius",
                     preset->top_device_corners * dpi / preset->scale_factor);
    append_variable (string, "--bottom-device-corner-radius",
                     preset->bottom_device_corners * dpi / preset->scale_factor);

    append_variable (string, "--top-bezel",
                     round (preset->top_bezel * dpi / preset->scale_factor));
    append_variable (string, "--side-bezel",
                     round (preset->side_bezel * dpi / preset->scale_factor));
    append_variable (string, "--bottom-bezel",
                     round (preset->bottom_bezel * dpi / preset->scale_factor));

    g_string_append (string, "}\n");
  }

  gtk_css_provider_load_from_string (css_provider, string->str);

  g_string_free (string, TRUE);
}

static GskTransform *
transform_for_angle (AdwAdaptivePreview *self,
                     GskTransform       *transform,
                     gboolean            inverted)
{
  switch (self->rotation) {
  case ROTATION_0DEG:
    break;
  case ROTATION_90DEG:
    if (inverted) {
      transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (0.0, self->screen_height));
      transform = gsk_transform_rotate (transform, -90);
    } else {
      transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (self->screen_width, 0.0));
      transform = gsk_transform_rotate (transform, 90);
    }
    break;
  case ROTATION_180DEG:
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (self->screen_width,
                                                                          self->screen_height));
    transform = gsk_transform_rotate (transform, 180);
    break;
  case ROTATION_270DEG:
    if (inverted) {
      transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (self->screen_width, 0.0));
      transform = gsk_transform_rotate (transform, -270);
    } else {
      transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (0.0, self->screen_height));
      transform = gsk_transform_rotate (transform, 270);
    }
    break;
  default:
    g_assert_not_reached ();
  }

  return transform;
}

static float
get_rotation_angle (ScreenRotation rotation)
{
  switch (rotation) {
  case ROTATION_0DEG:
    return 0;
  case ROTATION_90DEG:
    return 90;
  case ROTATION_180DEG:
    return 180;
  case ROTATION_270DEG:
    return 270;
  default:
    g_assert_not_reached ();
  }
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

  adw_window_title_set_title (self->content_title, _(preset->name));

  if (self->last_device_preset >= 0) {
    const DevicePreset *last_preset;

    g_assert (self->last_device_preset < G_N_ELEMENTS (device_presets));

    last_preset = &device_presets[self->last_device_preset];

    gtk_widget_remove_css_class (self->device_view, last_preset->id);
  }
  self->changing_screen_size = TRUE;

  if (preset->width >= 0)
    gtk_adjustment_set_value (self->width_adj, preset->width / preset->scale_factor);
  if (preset->height >= 0)
    gtk_adjustment_set_value (self->height_adj, preset->height / preset->scale_factor);
  self->screen_scale = preset->scale_factor;
  self->notches = preset->notches;

  gtk_widget_add_css_class (self->device_view, preset->id);
  self->last_device_preset = selected;

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
rotate_to (AdwAdaptivePreview *self,
           ScreenRotation      rotation)
{
  double angle = adw_animation_get_value (self->rotate_animation);
  double new_angle = get_rotation_angle (rotation);
  self->rotation = rotation;

  while (angle < 0)
    angle += 360;
  while (angle >= 360)
    angle -= 360;

  if (ABS (angle - new_angle) > (360 - ABS (angle - new_angle))) {
      if (angle < new_angle)
        new_angle -= 360;
      else
        new_angle += 360;
  }

  adw_animation_pause (self->rotate_animation);
  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->rotate_animation), angle);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->rotate_animation), new_angle);
  adw_animation_play (self->rotate_animation);
}

static void
rotate_left_cb (AdwAdaptivePreview *self)
{
  if (self->rotation == ROTATION_0DEG)
    rotate_to (self, ROTATION_270DEG);
  else
    rotate_to (self, self->rotation - 1);
}

static void
rotate_right_cb (AdwAdaptivePreview *self)
{
  if (self->rotation == ROTATION_270DEG)
    rotate_to (self, ROTATION_0DEG);
  else
    rotate_to (self, self->rotation + 1);
}

static void
exit_clicked_cb (AdwAdaptivePreview *self)
{
  g_signal_emit (self, signals[SIGNAL_EXIT], 0);
}

static void
copy_texture (AdwAdaptivePreview *self,
              GdkTexture         *texture)
{
  GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (self));
  GdkClipboard *clipboard = gdk_display_get_clipboard (display);
  AdwToast *toast = adw_toast_new (_("Screenshot Copied to Clipboard"));

  gdk_clipboard_set_texture (clipboard, texture);

  adw_toast_overlay_add_toast (self->toast_overlay, toast);
}

static void
screenshot_clicked_cb (AdwAdaptivePreview *self)
{
  int width = gtk_widget_get_width (self->device_container);
  int height = gtk_widget_get_height (self->device_container);
  GskTransform *transform = transform_for_angle (self, NULL, FALSE);
  GtkSnapshot *snapshot = gtk_snapshot_new ();
  GskRenderNode *node;
  GskRenderer *renderer;
  GdkTexture *texture;
  graphene_rect_t bounds;

  gtk_snapshot_transform (snapshot, transform);

  gdk_paintable_snapshot (self->device_paintable, snapshot, width, height);

  node = gtk_snapshot_free_to_node (snapshot);

  gsk_render_node_get_bounds (node, &bounds);

  renderer = gtk_native_get_renderer (gtk_widget_get_native (GTK_WIDGET (self)));
  texture = gsk_renderer_render_texture (renderer, node, &bounds);

  copy_texture (self, texture);

  gsk_transform_unref (transform);
  gsk_render_node_unref (node);
  g_object_unref (texture);
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

  self->last_device_preset = DEFAULT_DEVICE_PRESET;
}

static void
snapshot_screen_view (AdwGizmo    *gizmo,
                      GtkSnapshot *snapshot)
{
  AdwAdaptivePreview *self =
    ADW_ADAPTIVE_PREVIEW (gtk_widget_get_ancestor (GTK_WIDGET (gizmo), ADW_TYPE_ADAPTIVE_PREVIEW));
  GskPathBuilder *builder;
  GskPath *notch_path, *path;
  graphene_rect_t bounds;
  GdkRGBA rgba;

  if (!self->notches) {
    gtk_widget_snapshot_child (GTK_WIDGET (gizmo), self->top_bar, snapshot);
    gtk_widget_snapshot_child (GTK_WIDGET (gizmo), self->child_bin, snapshot);
    gtk_widget_snapshot_child (GTK_WIDGET (gizmo), self->bottom_bar, snapshot);
    return;
  }

  builder = gsk_path_builder_new ();

  notch_path = gsk_path_parse (self->notches);
  g_assert (notch_path != NULL);

  graphene_rect_init (&bounds, 0, 0,
                      self->screen_width * self->screen_scale,
                      self->screen_height * self->screen_scale);

  gsk_path_builder_add_rect (builder, &bounds);
  gsk_path_builder_add_path (builder, notch_path);

  path = gsk_path_builder_free_to_path (builder);

  gtk_snapshot_push_mask (snapshot, GSK_MASK_MODE_ALPHA);
  gtk_snapshot_scale (snapshot, 1.0f / self->screen_scale, 1.0f / self->screen_scale);

  rgba.red = 0.0;
  rgba.green = 0.0;
  rgba.blue = 0.0;
  rgba.alpha = 1.0;

  gtk_snapshot_append_fill (snapshot, path, GSK_FILL_RULE_EVEN_ODD, &rgba);
  gtk_snapshot_pop (snapshot);

  gtk_widget_snapshot_child (GTK_WIDGET (gizmo), self->top_bar, snapshot);
  gtk_widget_snapshot_child (GTK_WIDGET (gizmo), self->child_bin, snapshot);
  gtk_widget_snapshot_child (GTK_WIDGET (gizmo), self->bottom_bar, snapshot);
  gtk_snapshot_pop (snapshot);

  gsk_path_unref (notch_path);
  gsk_path_unref (path);
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
    nat = self->screen_width;
  else
    nat = self->screen_height;

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
  gboolean rotated = self->rotation == ROTATION_90DEG ||
                     self->rotation == ROTATION_270DEG;

  if (rotated) {
    int tmp = width;
    width = height;
    height = tmp;
  }

  if (gtk_widget_should_layout (self->top_bar)) {
    gtk_widget_measure (self->top_bar, GTK_ORIENTATION_VERTICAL, -1,
                        &top_bar_height, NULL, NULL, NULL);
    gtk_widget_allocate (self->top_bar, width, top_bar_height, -1,
                         transform_for_angle (self, NULL, TRUE));
  }

  if (gtk_widget_should_layout (self->bottom_bar)) {
    int bottom_bar_y;
    GskTransform *transform = NULL;

    gtk_widget_measure (self->bottom_bar, GTK_ORIENTATION_VERTICAL, -1,
                        &bottom_bar_height, NULL, NULL, NULL);

    bottom_bar_y = height - bottom_bar_height;

    transform = transform_for_angle (self, transform, TRUE);
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (0, bottom_bar_y));

    gtk_widget_allocate (self->bottom_bar, width, bottom_bar_height, -1, transform);
  }

  if (gtk_widget_should_layout (self->child_bin)) {
    int child_width, child_height, available_height;
    GskTransform *transform = NULL;

    available_height = height - top_bar_height - bottom_bar_height;

    if (gtk_widget_get_request_mode (self->child_bin) == GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH) {
      gtk_widget_measure (self->child_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                          &child_width, NULL, NULL, NULL);
      gtk_widget_measure (self->child_bin, GTK_ORIENTATION_VERTICAL, width,
                          &child_height, NULL, NULL, NULL);
    } else {
      gtk_widget_measure (self->child_bin, GTK_ORIENTATION_HORIZONTAL, available_height,
                          &child_width, NULL, NULL, NULL);
      gtk_widget_measure (self->child_bin, GTK_ORIENTATION_VERTICAL, -1,
                          &child_height, NULL, NULL, NULL);
    }

    if (child_width > width || child_height > available_height) {
      g_warning ("Window contents don't fit: provided %d×%d, available %d×%d",
                 child_width, child_height, width, available_height);
    }

    transform = transform_for_angle (self, transform, TRUE);
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (0, self->top_bar_height));

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

  gtk_widget_measure (child, GTK_ORIENTATION_HORIZONTAL, -1, NULL, &child_width, NULL, NULL);
  gtk_widget_measure (child, GTK_ORIENTATION_VERTICAL, -1, NULL, &child_height, NULL, NULL);

  if (self->scale_to_fit) {
    double a = adw_animation_get_value (self->rotate_animation);
    double t = 2 * ABS ((a / 180 - floor (a / 180 + 0.5))); /* Triangle wave */

    double scale1 = MIN ((float) width / (float) child_width,
                         (float) height / (float) child_height);
    double scale2 = MIN ((float) width / (float) child_height,
                         (float) height / (float) child_width);

    scale1 = CLAMP (scale1, MIN_SCALE, MAX_SCALE);
    scale2 = CLAMP (scale2, MIN_SCALE, MAX_SCALE);

    scale = adw_lerp (scale1, scale2, t);
  } else {
    scale = 1.0;
  }

  transform = gsk_transform_translate (transform,
                                       &GRAPHENE_POINT_INIT (roundf (width / 2.0f),
                                                             roundf (height / 2.0f)));
  transform = gsk_transform_scale (transform, scale, scale);
  transform = gsk_transform_rotate (transform, adw_animation_get_value (self->rotate_animation));
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

  g_clear_object (&self->rotate_animation);
  g_clear_object (&self->device_paintable);

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
  case PROP_HIGHLIGHT_BEZEL:
    g_value_set_boolean (value, adw_adaptive_preview_get_highlight_bezel (self));
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
  case PROP_HIGHLIGHT_BEZEL:
    adw_adaptive_preview_set_highlight_bezel (self, g_value_get_boolean (value));
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

  props[PROP_HIGHLIGHT_BEZEL] =
    g_param_spec_boolean ("highlight-bezel", NULL, NULL,
                          FALSE,
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
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, toast_overlay);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, scale_bin);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, device_container);
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, device_view);
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
  gtk_widget_class_bind_template_child (widget_class, AdwAdaptivePreview, content_title);

  gtk_widget_class_bind_template_callback (widget_class, screen_size_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, device_preset_cb);
  gtk_widget_class_bind_template_callback (widget_class, shell_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, shell_preset_cb);
  gtk_widget_class_bind_template_callback (widget_class, rotate_left_cb);
  gtk_widget_class_bind_template_callback (widget_class, rotate_right_cb);
  gtk_widget_class_bind_template_callback (widget_class, exit_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, screenshot_clicked_cb);
}

static void
rotate_animation_cb (double              value,
                     AdwAdaptivePreview *self)
{
  gtk_widget_queue_resize (self->screen_view);
}

static void
adw_adaptive_preview_init (AdwAdaptivePreview *self)
{
  AdwAnimationTarget *target;

  self->window_controls = TRUE;
  self->rotation = ROTATION_0DEG;
  self->scale_to_fit = TRUE;
  self->last_device_preset = -1;

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_set_layout_manager (self->screen_view,
                                 gtk_custom_layout_new (NULL,
                                                        measure_screen_view,
                                                        allocate_screen_view));

  gtk_widget_set_layout_manager (self->scale_bin,
                                 gtk_custom_layout_new (NULL,
                                                        measure_scale_bin,
                                                        allocate_scale_bin));

  adw_gizmo_set_snapshot_func (ADW_GIZMO (self->screen_view), snapshot_screen_view);

  setup_presets (self);

  gtk_adjustment_set_value (self->width_adj, 360);
  gtk_adjustment_set_value (self->height_adj, 720);

  if (!css_provider) {
    css_provider = gtk_css_provider_new ();
    gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                GTK_STYLE_PROVIDER (css_provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_THEME + 1);
    generate_device_css ();
  }

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) rotate_animation_cb,
                                              self,
                                              NULL);

  self->rotate_animation =
    adw_spring_animation_new (GTK_WIDGET (self),
                              0, 1,
                              adw_spring_params_new (1, 1, 800),
                              target);

  self->device_paintable = gtk_widget_paintable_new (self->device_container);
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

gboolean
adw_adaptive_preview_get_highlight_bezel (AdwAdaptivePreview *self)
{
  g_return_val_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self), FALSE);

  return self->highlight_bezel;
}

void
adw_adaptive_preview_set_highlight_bezel (AdwAdaptivePreview *self,
                                          gboolean            highlight_bezel)
{
  g_return_if_fail (ADW_IS_ADAPTIVE_PREVIEW (self));

  highlight_bezel = !!highlight_bezel;

  if (highlight_bezel == self->highlight_bezel)
    return;

  self->highlight_bezel = highlight_bezel;

  if (highlight_bezel)
    gtk_widget_add_css_class (self->device_view, "highlight");
  else
    gtk_widget_remove_css_class (self->device_view, "highlight");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGHLIGHT_BEZEL]);
}

GtkWidget *
adw_adaptive_preview_get_screen (AdwAdaptivePreview *self)
{
  return self->screen_view;
}
