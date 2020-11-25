/*
 * Copyright (C) 2019-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-animation-private.h"

/**
 * SECTION:hdy-animation
 * @short_description: Animation helpers
 * @title: Animation Helpers
 *
 * Animation helpers.
 *
 * Since: 0.0.11
 */

G_DEFINE_BOXED_TYPE (HdyAnimation, hdy_animation, hdy_animation_ref, hdy_animation_unref)

struct _HdyAnimation
{
  gatomicrefcount ref_count;

  GtkWidget *widget;

  gdouble value;

  gdouble value_from;
  gdouble value_to;
  gint64 duration; /* ms */

  gint64 start_time; /* ms */
  guint tick_cb_id;

  HdyAnimationEasingFunc easing_func;
  HdyAnimationValueCallback value_cb;
  HdyAnimationDoneCallback done_cb;
  gpointer user_data;
};

static void
set_value (HdyAnimation *self,
           gdouble       value)
{
  self->value = value;
  self->value_cb (value, self->user_data);
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         HdyAnimation  *self)
{
  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000; /* ms */
  gdouble t = (gdouble) (frame_time - self->start_time) / self->duration;

  if (t >= 1) {
    self->tick_cb_id = 0;

    set_value (self, self->value_to);

    g_signal_handlers_disconnect_by_func (self->widget, hdy_animation_stop, self);

    self->done_cb (self->user_data);

    return G_SOURCE_REMOVE;
  }

  set_value (self, hdy_lerp (self->value_from, self->value_to, self->easing_func (t)));

  return G_SOURCE_CONTINUE;
}

static void
hdy_animation_free (HdyAnimation *self)
{
  hdy_animation_stop (self);

  g_slice_free (HdyAnimation, self);
}

HdyAnimation *
hdy_animation_new (GtkWidget                 *widget,
                   gdouble                    from,
                   gdouble                    to,
                   gint64                     duration,
                   HdyAnimationEasingFunc     easing_func,
                   HdyAnimationValueCallback  value_cb,
                   HdyAnimationDoneCallback   done_cb,
                   gpointer                   user_data)
{
  HdyAnimation *self;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (easing_func != NULL, NULL);
  g_return_val_if_fail (value_cb != NULL, NULL);
  g_return_val_if_fail (done_cb != NULL, NULL);

  self = g_slice_new0 (HdyAnimation);

  g_atomic_ref_count_init (&self->ref_count);

  self->widget = widget;
  self->value_from = from;
  self->value_to = to;
  self->duration = duration;
  self->easing_func = easing_func;
  self->value_cb = value_cb;
  self->done_cb = done_cb;
  self->user_data = user_data;

  self->value = from;

  return self;
}

HdyAnimation *
hdy_animation_ref (HdyAnimation *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  g_atomic_ref_count_inc (&self->ref_count);

  return self;
}

void
hdy_animation_unref (HdyAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (g_atomic_ref_count_dec (&self->ref_count))
    hdy_animation_free (self);
}

void
hdy_animation_start (HdyAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (!hdy_get_enable_animations (self->widget) ||
      !gtk_widget_get_mapped (self->widget) ||
      self->duration <= 0) {
    set_value (self, self->value_to);

    self->done_cb (self->user_data);

    return;
  }

  self->start_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (self->widget)) / 1000;

  if (self->tick_cb_id)
    return;

  g_signal_connect_swapped (self->widget, "unmap",
                            G_CALLBACK (hdy_animation_stop), self);
  self->tick_cb_id = gtk_widget_add_tick_callback (self->widget, (GtkTickCallback) tick_cb, self, NULL);
}

void
hdy_animation_stop (HdyAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (!self->tick_cb_id)
    return;

  gtk_widget_remove_tick_callback (self->widget, self->tick_cb_id);
  self->tick_cb_id = 0;

  g_signal_handlers_disconnect_by_func (self->widget, hdy_animation_stop, self);

  self->done_cb (self->user_data);
}

gdouble
hdy_animation_get_value (HdyAnimation *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->value;
}

/**
 * hdy_get_enable_animations:
 * @widget: a #GtkWidget
 *
 * Returns whether animations are enabled for that widget. This should be used
 * when implementing an animated widget to know whether to animate it or not.
 *
 * Returns: %TRUE if animations are enabled for @widget.
 *
 * Since: 0.0.11
 */
gboolean
hdy_get_enable_animations (GtkWidget *widget)
{
  gboolean enable_animations = TRUE;

  g_assert (GTK_IS_WIDGET (widget));

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-enable-animations", &enable_animations,
                NULL);

  return enable_animations;
}

/**
 * hdy_lerp: (skip)
 * @a: the start
 * @b: the end
 * @t: the interpolation rate
 *
 * Computes the linear interpolation between @a and @b for @t.
 *
 * Returns: the linear interpolation between @a and @b for @t.
 *
 * Since: 0.0.11
 */
gdouble
hdy_lerp (gdouble a, gdouble b, gdouble t)
{
  return a * (1.0 - t) + b * t;
}

/* From clutter-easing.c, based on Robert Penner's
 * infamous easing equations, MIT license.
 */

/**
 * hdy_ease_out_cubic:
 * @t: the term
 *
 * Computes the ease out for @t.
 *
 * Returns: the ease out for @t.
 *
 * Since: 0.0.11
 */
gdouble
hdy_ease_out_cubic (gdouble t)
{
  gdouble p = t - 1;
  return p * p * p + 1;
}
