/*
 * Copyright (C) 2019-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-animation-private.h"

/**
 * SECTION:adw-animation
 * @short_description: Animation helpers
 * @title: Animation Helpers
 *
 * Animation helpers.
 *
 * Since: 1.0
 */

G_DEFINE_BOXED_TYPE (AdwAnimation, adw_animation, adw_animation_ref, adw_animation_unref)

struct _AdwAnimation
{
  gatomicrefcount ref_count;

  GtkWidget *widget;

  double value;

  double value_from;
  double value_to;
  gint64 duration; /* ms */

  gint64 start_time; /* ms */
  guint tick_cb_id;

  AdwAnimationEasingFunc easing_func;
  AdwAnimationValueCallback value_cb;
  AdwAnimationDoneCallback done_cb;
  gpointer user_data;

  gboolean is_done;
};

static void
set_value (AdwAnimation *self,
           double        value)
{
  self->value = value;
  self->value_cb (value, self->user_data);
}

static void
done (AdwAnimation *self)
{
  if (self->is_done)
    return;

  self->is_done = TRUE;
  self->done_cb (self->user_data);
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         AdwAnimation  *self)
{
  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000; /* ms */
  double t = (double) (frame_time - self->start_time) / self->duration;

  if (t >= 1) {
    self->tick_cb_id = 0;

    set_value (self, self->value_to);

    g_signal_handlers_disconnect_by_func (self->widget, adw_animation_stop, self);

    done (self);

    return G_SOURCE_REMOVE;
  }

  set_value (self, adw_lerp (self->value_from, self->value_to, self->easing_func (t)));

  return G_SOURCE_CONTINUE;
}

static void
adw_animation_free (AdwAnimation *self)
{
  adw_animation_stop (self);

  g_slice_free (AdwAnimation, self);
}

AdwAnimation *
adw_animation_new (GtkWidget                 *widget,
                   double                     from,
                   double                     to,
                   gint64                     duration,
                   AdwAnimationEasingFunc     easing_func,
                   AdwAnimationValueCallback  value_cb,
                   AdwAnimationDoneCallback   done_cb,
                   gpointer                   user_data)
{
  AdwAnimation *self;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (easing_func != NULL, NULL);
  g_return_val_if_fail (value_cb != NULL, NULL);
  g_return_val_if_fail (done_cb != NULL, NULL);

  self = g_slice_new0 (AdwAnimation);

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
  self->is_done = FALSE;

  return self;
}

AdwAnimation *
adw_animation_ref (AdwAnimation *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  g_atomic_ref_count_inc (&self->ref_count);

  return self;
}

void
adw_animation_unref (AdwAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (g_atomic_ref_count_dec (&self->ref_count))
    adw_animation_free (self);
}

void
adw_animation_start (AdwAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (!adw_get_enable_animations (self->widget) ||
      !gtk_widget_get_mapped (self->widget) ||
      self->duration <= 0) {
    set_value (self, self->value_to);

    done (self);

    return;
  }

  self->start_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (self->widget)) / 1000;

  if (self->tick_cb_id)
    return;

  g_signal_connect_swapped (self->widget, "unmap",
                            G_CALLBACK (adw_animation_stop), self);
  self->tick_cb_id = gtk_widget_add_tick_callback (self->widget, (GtkTickCallback) tick_cb, self, NULL);
}

void
adw_animation_stop (AdwAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (self->tick_cb_id) {
    gtk_widget_remove_tick_callback (self->widget, self->tick_cb_id);
    self->tick_cb_id = 0;

    g_signal_handlers_disconnect_by_func (self->widget, adw_animation_stop, self);
  }

  done (self);
}

double
adw_animation_get_value (AdwAnimation *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->value;
}

/**
 * adw_get_enable_animations:
 * @widget: a #GtkWidget
 *
 * Returns whether animations are enabled for that widget. This should be used
 * when implementing an animated widget to know whether to animate it or not.
 *
 * Returns: %TRUE if animations are enabled for @widget.
 *
 * Since: 1.0
 */
gboolean
adw_get_enable_animations (GtkWidget *widget)
{
  gboolean enable_animations = TRUE;

  g_assert (GTK_IS_WIDGET (widget));

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-enable-animations", &enable_animations,
                NULL);

  return enable_animations;
}

/**
 * adw_lerp: (skip)
 * @a: the start
 * @b: the end
 * @t: the interpolation rate
 *
 * Computes the linear interpolation between @a and @b for @t.
 *
 * Returns: the linear interpolation between @a and @b for @t.
 *
 * Since: 1.0
 */
double
adw_lerp (double a, double b, double t)
{
  return a * (1.0 - t) + b * t;
}

/* From clutter-easing.c, based on Robert Penner's
 * infamous easing equations, MIT license.
 */

/**
 * adw_ease_out_cubic:
 * @t: the term
 *
 * Computes the ease out for @t.
 *
 * Returns: the ease out for @t.
 *
 * Since: 1.0
 */
double
adw_ease_out_cubic (double t)
{
  double p = t - 1;
  return p * p * p + 1;
}
