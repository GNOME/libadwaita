/*
 * Copyright (C) 2026 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-icon-paintable.h"
#include "adw-style-manager-private.h"

/**
 * AdwIconPaintable:
 *
 * TODO
 *
 * Since: 1.9
 */

struct _AdwIconPaintable
{
  GObject parent_instance;

  char *icon_name;
  guint state;
  gboolean animate_in;
  GtkWidget *widget;

  GtkSvg *svg;
  char *path;
};

static void adw_icon_paintable_iface_init (GdkPaintableInterface *iface);
static void adw_icon_symbolic_paintable_iface_init (GtkSymbolicPaintableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwIconPaintable, adw_icon_paintable, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GDK_TYPE_PAINTABLE,
                                                      adw_icon_paintable_iface_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SYMBOLIC_PAINTABLE,
                                                      adw_icon_symbolic_paintable_iface_init))

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_STATE,
  PROP_ANIMATE_IN,
  PROP_WIDGET,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
update_frame_clock (AdwIconPaintable *self)
{
  if (self->widget && gtk_widget_get_mapped (self->widget)) {
    GdkFrameClock *frame_clock = gtk_widget_get_frame_clock (self->widget);

    gtk_svg_set_frame_clock (self->svg, frame_clock);
    gtk_svg_play (self->svg);
  } else {
    gtk_svg_pause (self->svg);
    gtk_svg_set_frame_clock (self->svg, NULL);
  }
}

static void
recreate_svg (AdwIconPaintable *self)
{
  g_clear_object (&self->svg);

  if (self->path && *self->path) {
    self->svg = gtk_svg_new_from_resource (self->path);

    g_signal_connect_swapped (self->svg, "invalidate-contents",
                              G_CALLBACK (gdk_paintable_invalidate_contents), self);
    g_signal_connect_swapped (self->svg, "invalidate-size",
                              G_CALLBACK (gdk_paintable_invalidate_size), self);

    if (self->widget && gtk_widget_get_mapped (self->widget)) {
      GdkFrameClock *frame_clock = gtk_widget_get_frame_clock (self->widget);

      gtk_svg_set_frame_clock (self->svg, frame_clock);
    }

    if (self->animate_in) {
      gtk_svg_set_state (self->svg, -1);
      gtk_svg_play (self->svg);
      gtk_svg_set_state (self->svg, self->state);
    } else {
      gtk_svg_set_state (self->svg, self->state);
      gtk_svg_play (self->svg);
    }
  }

  gdk_paintable_invalidate_size (GDK_PAINTABLE (self));
}

static void
reload_icon (AdwIconPaintable *self)
{
  AdwStyleManager *manager;
  GdkDisplay *display;
  char *path;

  if (!self->icon_name || !*self->icon_name) {
    g_clear_object (&self->svg);
    gdk_paintable_invalidate_size (GDK_PAINTABLE (self));
    return;
  }

  if (self->widget)
    display = gtk_widget_get_display (self->widget);
  else
    display = gdk_display_get_default ();

  manager = adw_style_manager_get_for_display (display);
  path = adw_style_manager_lookup_icon_path (manager, self->icon_name);

  if (!g_strcmp0 (path, self->path)) {
    g_free (path);
    return;
  }

  g_free (self->path);
  self->path = path;

  recreate_svg (self);
}

static void
widget_notify_cb (AdwIconPaintable *self)
{
  self->widget = NULL;

  if (self->svg)
    update_frame_clock (self);

  reload_icon (self);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WIDGET]);
}

static void
widget_map_cb (AdwIconPaintable *self)
{
  g_assert (GTK_IS_WIDGET (self->widget));

  reload_icon (self);
  update_frame_clock (self);
}

static void
widget_unmap_cb (AdwIconPaintable *self)
{
  if (self->svg)
    update_frame_clock (self);
}

static void
adw_icon_paintable_dispose (GObject *object)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (object);

  if (self->widget) {
    g_signal_handlers_disconnect_by_func (self->widget, widget_map_cb, self);
    g_signal_handlers_disconnect_by_func (self->widget, widget_unmap_cb, self);

    g_object_weak_unref (G_OBJECT (self->widget),
                         (GWeakNotify) widget_notify_cb,
                         self);

    self->widget = NULL;
  }

  g_clear_object (&self->svg);

  G_OBJECT_CLASS (adw_icon_paintable_parent_class)->dispose (object);
}

static void
adw_icon_paintable_finalize (GObject *object)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (object);

  g_free (self->icon_name);
  g_free (self->path);

  G_OBJECT_CLASS (adw_icon_paintable_parent_class)->finalize (object);
}

static void
adw_icon_paintable_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_icon_paintable_get_icon_name (self));
    break;
  case PROP_STATE:
    g_value_set_uint (value, adw_icon_paintable_get_state (self));
    break;
  case PROP_ANIMATE_IN:
    g_value_set_boolean (value, adw_icon_paintable_get_animate_in (self));
    break;
  case PROP_WIDGET:
    g_value_set_object (value, adw_icon_paintable_get_widget (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_icon_paintable_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adw_icon_paintable_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_STATE:
    adw_icon_paintable_set_state (self, g_value_get_uint (value));
    break;
  case PROP_ANIMATE_IN:
    adw_icon_paintable_set_animate_in (self, g_value_get_boolean (value));
    break;
  case PROP_WIDGET:
    adw_icon_paintable_set_widget (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_icon_paintable_class_init (AdwIconPaintableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_icon_paintable_dispose;
  object_class->finalize = adw_icon_paintable_finalize;
  object_class->get_property = adw_icon_paintable_get_property;
  object_class->set_property = adw_icon_paintable_set_property;

  /**
   * AdwIconPaintable:icon-name:
   *
   * TODO
   *
   * Since: 1.9
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIconPaintable:state:
   *
   * TODO
   *
   * Since: 1.9
   */
  props[PROP_STATE] =
    g_param_spec_uint ("state", NULL, NULL,
                       0, G_MAXUINT, GTK_SVG_STATE_EMPTY,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIconPaintable:animate-in:
   *
   * TODO
   *
   * Since: 1.9
   */
  props[PROP_ANIMATE_IN] =
    g_param_spec_boolean ("animate-in", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwIconPaintable:widget:
   *
   * The widget the icon uses for frame clock.
   *
   * Since: 1.9
   */
  props[PROP_WIDGET] =
    g_param_spec_object ("widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_icon_paintable_init (AdwIconPaintable *self)
{
  self->icon_name = g_strdup ("");
}

static void
adw_icon_paintable_snapshot_symbolic (GtkSymbolicPaintable *paintable,
                                      GtkSnapshot          *snapshot,
                                      double                width,
                                      double                height,
                                      const GdkRGBA        *colors,
                                      gsize                 n_colors)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (paintable);

  if (!self->svg)
    return;

  gtk_symbolic_paintable_snapshot_symbolic (GTK_SYMBOLIC_PAINTABLE (self->svg),
                                            snapshot, width, height,
                                            colors, n_colors);
}

static void
adw_icon_paintable_snapshot_with_weight (GtkSymbolicPaintable *paintable,
                                         GtkSnapshot          *snapshot,
                                         double                width,
                                         double                height,
                                         const GdkRGBA        *colors,
                                         gsize                 n_colors,
                                         double                weight)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (paintable);

  if (!self->svg)
    return;

  gtk_symbolic_paintable_snapshot_with_weight (GTK_SYMBOLIC_PAINTABLE (self->svg),
                                               snapshot, width, height,
                                               colors, n_colors, weight);
}

static void
adw_icon_symbolic_paintable_iface_init (GtkSymbolicPaintableInterface *iface)
{
  iface->snapshot_symbolic = adw_icon_paintable_snapshot_symbolic;
  iface->snapshot_with_weight = adw_icon_paintable_snapshot_with_weight;
}

static void
adw_icon_paintable_snapshot (GdkPaintable *paintable,
                             GtkSnapshot  *snapshot,
                             double        width,
                             double        height)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (paintable);

  if (!self->svg)
    return;

  gdk_paintable_snapshot (GDK_PAINTABLE (self->svg), snapshot, width, height);
}

static int
adw_icon_paintable_get_intrinsic_width (GdkPaintable *paintable)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (paintable);

  if (!self->svg)
    return 16;

  return gdk_paintable_get_intrinsic_width (GDK_PAINTABLE (self->svg));
}

static int
adw_icon_paintable_get_intrinsic_height (GdkPaintable *paintable)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (paintable);

  if (!self->svg)
    return 16;

  return gdk_paintable_get_intrinsic_height (GDK_PAINTABLE (self->svg));
}

static double
adw_icon_paintable_get_intrinsic_aspect_ratio (GdkPaintable *paintable)
{
  AdwIconPaintable *self = ADW_ICON_PAINTABLE (paintable);

  if (!self->svg)
    return 1.0;

  return gdk_paintable_get_intrinsic_aspect_ratio (GDK_PAINTABLE (self->svg));
}

static void
adw_icon_paintable_iface_init (GdkPaintableInterface *iface)
{
  iface->snapshot = adw_icon_paintable_snapshot;
  iface->get_intrinsic_width = adw_icon_paintable_get_intrinsic_width;
  iface->get_intrinsic_height = adw_icon_paintable_get_intrinsic_height;
  iface->get_intrinsic_aspect_ratio = adw_icon_paintable_get_intrinsic_aspect_ratio;
}

/**
 * adw_icon_paintable_new: (constructor)
 * @icon_name: the icon name to use
 * @widget: (nullable): the widget used for frame clock
 *
 * Creates a new `AdwIconPaintable` for @icon_name and @widget.
 *
 * Returns: the newly created `AdwIconPaintable`
 *
 * Since: 1.9
 */
AdwIconPaintable *
adw_icon_paintable_new (const char *icon_name,
                        GtkWidget  *widget)
{
  g_return_val_if_fail (icon_name != NULL, NULL);
  g_return_val_if_fail (widget == NULL || GTK_IS_WIDGET (widget), NULL);

  return g_object_new (ADW_TYPE_ICON_PAINTABLE,
                       "icon-name", icon_name,
                       "widget", widget,
                       NULL);
}

/**
 * adw_icon_paintable_get_icon_name:
 * @self: an icon paintable
 *
 * TODO
 *
 * Returns: (transfer none): the icon name
 *
 * Since: 1.9
 */
const char *
adw_icon_paintable_get_icon_name (AdwIconPaintable *self)
{
  g_return_val_if_fail (ADW_IS_ICON_PAINTABLE (self), NULL);

  return self->icon_name;
}

/**
 * adw_icon_paintable_set_icon_name:
 * @self: an icon paintable
 * @icon_name: the new icon name
 *
 * TODO
 *
 * Since: 1.9
 */
void
adw_icon_paintable_set_icon_name (AdwIconPaintable *self,
                                  const char       *icon_name)
{
  g_return_if_fail (ADW_IS_ICON_PAINTABLE (self));
  g_return_if_fail (icon_name != NULL);

  if (!g_set_str (&self->icon_name, icon_name))
    return;

  reload_icon (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adw_icon_paintable_get_state:
 * @self: an icon paintable
 *
 * TODO
 *
 * Returns: TODO
 *
 * Since: 1.9
 */
guint
adw_icon_paintable_get_state (AdwIconPaintable *self)
{
  g_return_val_if_fail (ADW_IS_ICON_PAINTABLE (self), 0);

  return self->state;
}

/**
 * adw_icon_paintable_set_state:
 * @self: an icon paintable
 * @state: TODO
 *
 * TODO
 *
 * Since: 1.9
 */
void
adw_icon_paintable_set_state (AdwIconPaintable *self,
                              guint             state)
{
  g_return_if_fail (ADW_IS_ICON_PAINTABLE (self));

  if (state == self->state)
    return;

  self->state = state;

  if (self->svg)
    gtk_svg_set_state (self->svg, state);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);
}

/**
 * adw_icon_paintable_get_animate_in:
 * @self: an icon paintable
 *
 * TODO
 *
 * Returns: TODO
 *
 * Since: 1.9
 */
gboolean
adw_icon_paintable_get_animate_in (AdwIconPaintable *self)
{
  g_return_val_if_fail (ADW_IS_ICON_PAINTABLE (self), FALSE);

  return self->animate_in;
}

/**
 * adw_icon_paintable_set_animate_in:
 * @self: an icon paintable
 * @animate_in: TODO
 *
 * TODO
 *
 * Since: 1.9
 */
void
adw_icon_paintable_set_animate_in (AdwIconPaintable *self,
                                   gboolean          animate_in)
{
  g_return_if_fail (ADW_IS_ICON_PAINTABLE (self));

  animate_in = !!animate_in;

  if (animate_in == self->animate_in)
    return;

  self->animate_in = animate_in;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ANIMATE_IN]);
}

/**
 * adw_icon_paintable_get_widget:
 * @self: an icon paintable
 *
 * Gets the widget used for frame clock.
 *
 * Returns: (transfer none) (nullable): the widget
 *
 * Since: 1.9
 */
GtkWidget *
adw_icon_paintable_get_widget (AdwIconPaintable *self)
{
  g_return_val_if_fail (ADW_IS_ICON_PAINTABLE (self), NULL);

  return self->widget;
}

/**
 * adw_icon_paintable_set_widget:
 * @self: an icon paintable
 * @widget: (nullable): the widget to use for frame clock
 *
 * Sets the widget used for frame clock.
 *
 * Since: 1.9
 */
void
adw_icon_paintable_set_widget (AdwIconPaintable *self,
                               GtkWidget        *widget)
{
  g_return_if_fail (ADW_IS_ICON_PAINTABLE (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  if (widget == self->widget)
    return;

  if (self->widget) {
    g_signal_handlers_disconnect_by_func (self->widget, widget_map_cb, self);
    g_signal_handlers_disconnect_by_func (self->widget, widget_unmap_cb, self);

    if (gtk_widget_get_mapped (self->widget))
      widget_unmap_cb (self);

    g_object_weak_unref (G_OBJECT (self->widget),
                         (GWeakNotify) widget_notify_cb,
                         self);
  }

  self->widget = widget;

  if (self->widget) {
    g_signal_connect_swapped (self->widget, "map", G_CALLBACK (widget_map_cb), self);
    g_signal_connect_swapped (self->widget, "unmap", G_CALLBACK (widget_unmap_cb), self);

    if (gtk_widget_get_mapped (self->widget))
      widget_map_cb (self);

    g_object_weak_ref (G_OBJECT (self->widget),
                       (GWeakNotify) widget_notify_cb,
                       self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WIDGET]);
}
