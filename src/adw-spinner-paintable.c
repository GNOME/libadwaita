/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n.h>

#include "adw-spinner-paintable.h"

#include "adw-animation-util.h"
#include "adw-easing.h"
#include "adw-timed-animation.h"

#include <math.h>

#define MIN_RADIUS 8
#define MAX_RADIUS 32
#define SMALL_WIDTH 2.5
#define LARGE_WIDTH 7
#define SPIN_DURATION_MS 1200
#define START_ANGLE (G_PI * 0.35)
#define CIRCLE_OPACITY 0.15
/* GSK will fail to draw the arc entirely if the distance is too small */
#define MIN_ARC_LENGTH (G_PI * 0.015)
#define MAX_ARC_LENGTH (G_PI * 0.9)
#define IDLE_DISTANCE (G_PI * 0.9)
#define OVERLAP_DISTANCE (G_PI * 0.7)
#define EXTEND_DISTANCE (G_PI * 1.1)
#define CONTRACT_DISTANCE (G_PI * 1.35)
/* How many full cycles it takes for the spinner to loop. Should be:
 * (IDLE_DISTANCE + EXTEND_DISTANCE + CONTRACT_DISTANCE - OVERLAP_DISTANCE) * k,
 * where k is an integer */
#define N_CYCLES 53

/**
 * AdwSpinnerPaintable:
 *
 * A paintable showing a loading spinner.
 *
 * <picture>
 *   <source srcset="spinner-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="spinner.png" alt="spinner">
 * </picture>
 *
 * `AdwSpinnerPaintable` size varies depending on the available space, but is
 * capped at 64×64 pixels.
 *
 * To be able to animate, `AdwSpinnerPaintable` needs a widget. It will be
 * animated according to that widget's frame clock, and only if that widget is
 * mapped. Ideally it should be the same widget the paintable is displayed in,
 * but that's not a requirement.
 *
 * Most applications should be using [class@Spinner] instead.
 * `AdwSpinnerPaintable` is provided for the cases where using a widget is
 * impractical or impossible, such as [property@StatusPage:paintable]:
 *
 * ```xml
 * <object class="AdwStatusPage" id="status_page">
 *   <property name="paintable">
 *     <object class="AdwSpinnerPaintable">
 *       <property name="widget">status_page</property>
 *     </object>
 *   </property>
 *   <!-- ... -->
 * </object>
 * ```
 *
 * Since: 1.6
 */

struct _AdwSpinnerPaintable
{
  GObject parent_instance;

  AdwAnimation *animation;
  GtkWidget *widget;
  guint invalidate_source_id;
};

static void adw_spinner_paintable_iface_init (GdkPaintableInterface *iface);
static void adw_spinner_symbolic_paintable_iface_init (GtkSymbolicPaintableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSpinnerPaintable, adw_spinner_paintable, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GDK_TYPE_PAINTABLE,
                                                      adw_spinner_paintable_iface_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SYMBOLIC_PAINTABLE,
                                                      adw_spinner_symbolic_paintable_iface_init))

enum {
  PROP_0,
  PROP_WIDGET,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static inline double
inverse_lerp (double a,
              double b,
              double t)
{
  return (t - a) / (b - a);
}

static double
normalize_angle (double angle)
{
  while (angle < 0)
    angle += G_PI * 2;

  while (angle > G_PI * 2)
    angle -= G_PI * 2;

  return angle;
}

static float
get_arc_start (float angle)
{
  float l = IDLE_DISTANCE + EXTEND_DISTANCE + CONTRACT_DISTANCE - OVERLAP_DISTANCE;
  float t;

  angle = fmod (angle, l);

  if (angle > EXTEND_DISTANCE) {
    t = 1;
  } else {
    t = angle / EXTEND_DISTANCE;
    t = adw_easing_ease (ADW_EASE_IN_OUT_SINE, t);
  }

  return adw_lerp (MIN_ARC_LENGTH, MAX_ARC_LENGTH, t) - angle * MAX_ARC_LENGTH / l;
}

static float
get_arc_end (float angle)
{
  float l = IDLE_DISTANCE + EXTEND_DISTANCE + CONTRACT_DISTANCE - OVERLAP_DISTANCE;
  float t;

  angle = fmod (angle, l);

  if (angle < EXTEND_DISTANCE - OVERLAP_DISTANCE) {
    t = 0;
  } else if (angle > l - IDLE_DISTANCE) {
    t = 1;
  } else {
    t = (angle - EXTEND_DISTANCE + OVERLAP_DISTANCE) / CONTRACT_DISTANCE;
    t = adw_easing_ease (ADW_EASE_IN_OUT_SINE, t);
  }

  return adw_lerp (0, MAX_ARC_LENGTH - MIN_ARC_LENGTH, t) - angle * MAX_ARC_LENGTH / l;
}

static void
animation_cb (double               value,
              AdwSpinnerPaintable *self)
{
  gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
}

static void
invalidate_cb (AdwSpinnerPaintable *self)
{
  g_clear_object (&self->animation);
  gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WIDGET]);
  self->invalidate_source_id = 0;
}

static void
widget_notify_cb (AdwSpinnerPaintable *self)
{
  self->widget = NULL;

  /* FIXME: This idle callback is a workaround for https://gitlab.gnome.org/GNOME/glib/-/issues/3434
   *
   * 1. self->widget (an AdwSpinner) is destroyed
   * 2. In this weak notify, AdwSpinnerPaintable destroys the AdwAnimation self->animation
   * 3. AdwAnimation attempts to unregister its weak pointer on the same AdwSpinner but fails due to the GLib bug
   * 4. AdwAnimation's weak notify executes unexpectedly after AdwAnimation is destroyed and corrupts heap memory
   *
   * Using an idle avoids the problem by deferring destruction of AdwAnimation to the next main context iteration.
   * This workaround can be removed once the GLib bug is fixed.
   */
  self->invalidate_source_id = g_idle_add_once ((GSourceOnceFunc) invalidate_cb, self);
}

static void
widget_map_cb (AdwSpinnerPaintable *self)
{
  adw_animation_play (self->animation);
}

static void
adw_spinner_paintable_dispose (GObject *object)
{
  AdwSpinnerPaintable *self = ADW_SPINNER_PAINTABLE (object);

  if (self->animation)
    adw_animation_reset (self->animation);

  g_clear_object (&self->animation);

  if (self->widget) {
    g_signal_handlers_disconnect_by_func (self->widget, widget_map_cb, self);

    g_object_weak_unref (G_OBJECT (self->widget),
                         (GWeakNotify) widget_notify_cb,
                         self);

    self->widget = NULL;
  }

  g_clear_handle_id (&self->invalidate_source_id, g_source_remove);

  G_OBJECT_CLASS (adw_spinner_paintable_parent_class)->dispose (object);
}

static void
adw_spinner_paintable_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwSpinnerPaintable *self = ADW_SPINNER_PAINTABLE (object);

  switch (prop_id) {
  case PROP_WIDGET:
    g_value_set_object (value, adw_spinner_paintable_get_widget (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_spinner_paintable_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwSpinnerPaintable *self = ADW_SPINNER_PAINTABLE (object);

  switch (prop_id) {
  case PROP_WIDGET:
    adw_spinner_paintable_set_widget (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_spinner_paintable_class_init (AdwSpinnerPaintableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_spinner_paintable_dispose;
  object_class->get_property = adw_spinner_paintable_get_property;
  object_class->set_property = adw_spinner_paintable_set_property;

  /**
   * AdwSpinnerPaintable:widget:
   *
   * The widget the spinner uses for frame clock.
   *
   * Since: 1.6
   */
  props[PROP_WIDGET] =
    g_param_spec_object ("widget", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_spinner_paintable_init (AdwSpinnerPaintable *self)
{
}

static void
adw_spinner_paintable_snapshot_symbolic (GtkSymbolicPaintable *paintable,
                                         GtkSnapshot          *snapshot,
                                         double                width,
                                         double                height,
                                         const GdkRGBA        *colors,
                                         gsize                 n_colors)
{
  AdwSpinnerPaintable *self = ADW_SPINNER_PAINTABLE (paintable);
  float radius, line_width;
  GdkRGBA color;
  GskPathBuilder *builder;
  GskPath *circle_path, *arc_path;
  GskStroke *stroke;
  double progress;
  float base_angle, start_angle, end_angle, length;
  GskPathPoint start_point, end_point;
  GskPathMeasure *measure;

  radius = MIN (floorf (MIN (width, height) / 2), MAX_RADIUS);
  line_width = adw_lerp (SMALL_WIDTH, LARGE_WIDTH,
                         inverse_lerp (MIN_RADIUS, MAX_RADIUS, radius));

  if (radius < line_width / 2)
    return;

  gtk_snapshot_translate (snapshot,
                          &GRAPHENE_POINT_INIT (roundf (width / 2),
                                                roundf (height / 2)));

  stroke = gsk_stroke_new (line_width);
  gsk_stroke_set_line_cap (stroke, GSK_LINE_CAP_ROUND);

  gtk_snapshot_push_mask (snapshot, GSK_MASK_MODE_LUMINANCE);

  /* Circle */

  builder = gsk_path_builder_new ();

  gsk_path_builder_add_circle (builder,
                               &GRAPHENE_POINT_INIT (0, 0),
                               radius - line_width / 2);
  circle_path = gsk_path_builder_free_to_path (builder);

  color.red = color.green = color.blue = CIRCLE_OPACITY;
  color.alpha = 1;
  gtk_snapshot_append_stroke (snapshot, circle_path, stroke, &color);

  /* Moving part */

  if (self->animation)
    progress = adw_animation_get_value (self->animation);
  else
    progress = EXTEND_DISTANCE - OVERLAP_DISTANCE / 2;

  base_angle = (float) progress;
  start_angle = base_angle + get_arc_start (base_angle) + START_ANGLE;
  end_angle = base_angle + get_arc_end (base_angle) + START_ANGLE;

  start_angle = normalize_angle (start_angle);
  end_angle = normalize_angle (end_angle);

  measure = gsk_path_measure_new (circle_path);
  length = gsk_path_measure_get_length (measure);
  g_assert (gsk_path_measure_get_point (measure,
                                        start_angle / (G_PI * 2) * length,
                                        &start_point));
  g_assert (gsk_path_measure_get_point (measure,
                                        end_angle / (G_PI * 2) * length,
                                        &end_point));

  builder = gsk_path_builder_new ();
  gsk_path_builder_add_segment (builder, circle_path, &end_point, &start_point);
  arc_path = gsk_path_builder_free_to_path (builder);

  color.red = color.green = color.blue = 1;
  gtk_snapshot_append_stroke (snapshot, arc_path, stroke, &color);

  /* Mask with the correct color */

  gtk_snapshot_pop (snapshot);

  gtk_snapshot_append_color (snapshot,
                             &colors[0],
                             &GRAPHENE_RECT_INIT (-radius, -radius, radius * 2, radius * 2));
  gtk_snapshot_pop (snapshot);

  gsk_stroke_free (stroke);
  gsk_path_unref (circle_path);
  gsk_path_unref (arc_path);
  gsk_path_measure_unref (measure);
}

static void
adw_spinner_symbolic_paintable_iface_init (GtkSymbolicPaintableInterface *iface)
{
  iface->snapshot_symbolic = adw_spinner_paintable_snapshot_symbolic;
}

static void
adw_spinner_paintable_snapshot (GdkPaintable *paintable,
                                GtkSnapshot  *snapshot,
                                double        width,
                                double        height)
{
  AdwSpinnerPaintable *self = ADW_SPINNER_PAINTABLE (paintable);
  GdkRGBA color;

  gtk_widget_get_color (self->widget, &color);

  gtk_symbolic_paintable_snapshot_symbolic (GTK_SYMBOLIC_PAINTABLE (self),
                                            snapshot, width, height, &color, 1);
}

static GdkPaintableFlags
adw_spinner_paintable_get_flags (GdkPaintable *paintable)
{
  return GDK_PAINTABLE_STATIC_SIZE;
}

static double
adw_spinner_paintable_get_intrinsic_aspect_ratio (GdkPaintable *paintable)
{
  return 1;
}

static void
adw_spinner_paintable_iface_init (GdkPaintableInterface *iface)
{
  iface->snapshot = adw_spinner_paintable_snapshot;
  iface->get_flags = adw_spinner_paintable_get_flags;
  iface->get_intrinsic_aspect_ratio = adw_spinner_paintable_get_intrinsic_aspect_ratio;
}

/**
 * adw_spinner_paintable_new: (constructor)
 * @widget: (nullable): the widget used for frame clock
 *
 * Creates a new `AdwSpinnerPaintable` for @widget.
 *
 * Returns: the newly created `AdwSpinnerPaintable`
 *
 * Since: 1.6
 */
AdwSpinnerPaintable *
adw_spinner_paintable_new (GtkWidget *widget)
{
  g_return_val_if_fail (widget == NULL || GTK_IS_WIDGET (widget), NULL);

  return g_object_new (ADW_TYPE_SPINNER_PAINTABLE,
                       "widget", widget,
                       NULL);
}

/**
 * adw_spinner_paintable_get_widget:
 * @self: a spinner paintable
 *
 * Gets the widget used for frame clock.
 *
 * Returns: (transfer none) (nullable): the widget
 *
 * Since: 1.6
 */
GtkWidget *
adw_spinner_paintable_get_widget (AdwSpinnerPaintable *self)
{
  g_return_val_if_fail (ADW_IS_SPINNER_PAINTABLE (self), NULL);

  return self->widget;
}

/**
 * adw_spinner_paintable_set_widget:
 * @self: a spinner paintable
 * @widget: (nullable): the widget to use for frame clock
 *
 * Sets the widget used for frame clock.
 *
 * Since: 1.6
 */
void
adw_spinner_paintable_set_widget (AdwSpinnerPaintable *self,
                                  GtkWidget           *widget)
{
  g_return_if_fail (ADW_IS_SPINNER_PAINTABLE (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  if (widget == self->widget)
    return;

  if (self->widget) {
    g_clear_object (&self->animation);

    g_signal_handlers_disconnect_by_func (self->widget, widget_map_cb, self);

    g_object_weak_unref (G_OBJECT (self->widget),
                         (GWeakNotify) widget_notify_cb,
                         self);
  }

  self->widget = widget;

  if (self->widget) {
    AdwAnimationTarget *target =
      adw_callback_animation_target_new ((AdwAnimationTargetFunc) animation_cb,
                                         self, NULL);

    self->animation = adw_timed_animation_new (self->widget,
                                               0, N_CYCLES * G_PI * 2,
                                               SPIN_DURATION_MS * N_CYCLES,
                                               target);

    adw_animation_set_follow_enable_animations_setting (self->animation, FALSE);

    adw_timed_animation_set_repeat_count (ADW_TIMED_ANIMATION (self->animation), 0);
    adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->animation), ADW_LINEAR);

    if (gtk_widget_get_mapped (self->widget))
      adw_animation_play (self->animation);

    g_signal_connect_swapped (self->widget, "map", G_CALLBACK (widget_map_cb), self);

    g_object_weak_ref (G_OBJECT (self->widget),
                       (GWeakNotify) widget_notify_cb,
                       self);
  }

  gdk_paintable_invalidate_contents (GDK_PAINTABLE (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WIDGET]);
}
