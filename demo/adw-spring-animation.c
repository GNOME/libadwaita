/*
 * Copyright (C) 2021 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "adw-spring-animation-private.h"

#include <adwaita.h>
#include <math.h>

#define DELTA 0.001

G_DEFINE_BOXED_TYPE (AdwSpringAnimation, adw_spring_animation, adw_spring_animation_ref, adw_spring_animation_unref)

struct _AdwSpringAnimation
{
  gatomicrefcount ref_count;

  GtkWidget *widget;

  gdouble value;

  gdouble value_from;
  gdouble value_to;

  gdouble velocity;
  gdouble damping;
  gdouble mass;
  gdouble stiffness;
  gdouble epsilon;

  gdouble estimated_duration;

  gint64 start_time; /* ms */
  guint tick_cb_id;

  AdwAnimationValueCallback value_cb;
  AdwAnimationDoneCallback done_cb;
  gpointer user_data;
};

static void
set_value (AdwSpringAnimation *self,
           gdouble             value)
{
  self->value = value;
  self->value_cb (value, self->user_data);
}

/* Based on RBBSpringAnimation from RBBAnimation, MIT license.
 * https://github.com/robb/RBBAnimation/blob/master/RBBAnimation/RBBSpringAnimation.m
 */
static gdouble
oscillate (AdwSpringAnimation *self,
           gdouble             t)
{
  gdouble b = self->damping;
  gdouble m = self->mass;
  gdouble k = self->stiffness;
  gdouble v0 = self->velocity;

  gdouble beta = b / (2 * m);
  gdouble omega0 = sqrt (k / m);

  gdouble x0 = -1;

  gdouble envelope = exp (-beta * t);

  /*
   * Solutions of the form C1*e^(lambda1*x) + C2*e^(lambda2*x)
   * for the differential equation m*ẍ+b*ẋ+kx = 0
   */

  if (beta < omega0) { /* Underdamped */
    gdouble omega1 = sqrt ((omega0 * omega0) - (beta * beta));

    return -x0 + envelope * (x0 * cos (omega1 * t) + ((beta * x0 + v0) / omega1) * sin (omega1 * t));
  }

  if (beta > omega0) { /* Overdamped */
    gdouble omega2 = sqrt ((beta * beta) - (omega0 * omega0));

    return -x0 + envelope * (x0 * cosh (omega2 * t) + ((beta * x0 + v0) / omega2) * sinh (omega2 * t));
  }

  /* Critically damped */
  return -x0 + envelope * (x0 + (beta * x0 + v0) * t);
}

static gdouble
estimate_duration (AdwSpringAnimation *self)
{
  gdouble beta = self->damping / (2 * self->mass);
  gdouble omega0;
  gdouble x0, y0;
  gdouble x1, y1;
  gdouble m;

  if (beta <= 0)
    return INFINITY;

  omega0 = sqrt (self->stiffness / self->mass);

  /*
   * As first ansatz for the overdamped solution,
   * and general estimation for the oscillating ones
   * we took the value of the envelope when its < epsilon
   */
  x0 = -log (self->epsilon) / beta;

  if (beta <= omega0)
    return x0;

  /*
   * Since the overdamped solution decays way slower than the envelope
   * we need to use the value of the oscillation itself.
   * Newton's root finding method is a good candidate in this particular case:
   * https://en.wikipedia.org/wiki/Newton%27s_method
   */
  y0 = oscillate (self, x0);
  m = (oscillate (self, x0 + DELTA) - y0) / DELTA;

  x1 = (1 - y0 + m * x0) / m;
  y1 = oscillate (self, x1);

  while (ABS (1 - y1) > self->epsilon) {
    x0 = x1;
    y0 = y1;

    m = (oscillate (self, x0 + DELTA) - y0) / DELTA;

    x1 = (1 - y0 + m * x0) / m;
    y1 = oscillate (self, x1);
  }

  return x1;
}

static inline gdouble
lerp (gdouble a, gdouble b, gdouble t)
{
  return a * (1.0 - t) + b * t;
}

static gboolean
tick_cb (GtkWidget          *widget,
         GdkFrameClock      *frame_clock,
         AdwSpringAnimation *self)
{
  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;
  gdouble t = (gdouble) (frame_time - self->start_time) / 1000;

  if (t >= self->estimated_duration) {
    self->tick_cb_id = 0;

    set_value (self, self->value_to);

    g_signal_handlers_disconnect_by_func (self->widget, adw_spring_animation_stop, self);

    self->done_cb (self->user_data);

    return G_SOURCE_REMOVE;
  }

  set_value (self, lerp (self->value_from, self->value_to, oscillate (self, t)));

  return G_SOURCE_CONTINUE;
}

static void
adw_spring_animation_free (AdwSpringAnimation *self)
{
  adw_spring_animation_stop (self);

  g_slice_free (AdwSpringAnimation, self);
}

AdwSpringAnimation *
adw_spring_animation_new (GtkWidget                 *widget,
                          gdouble                    from,
                          gdouble                    to,
                          gdouble                    velocity,
                          gdouble                    damping,
                          gdouble                    mass,
                          gdouble                    stiffness,
                          gdouble                    epsilon,
                          AdwAnimationValueCallback  value_cb,
                          AdwAnimationDoneCallback   done_cb,
                          gpointer                   user_data)
{
  AdwSpringAnimation *self;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (damping > 0, NULL);
  g_return_val_if_fail (mass > 0, NULL);
  g_return_val_if_fail (stiffness > 0, NULL);
  g_return_val_if_fail (value_cb != NULL, NULL);
  g_return_val_if_fail (done_cb != NULL, NULL);

  self = g_slice_new0 (AdwSpringAnimation);

  g_atomic_ref_count_init (&self->ref_count);

  self->widget = widget;
  self->value_from = from;
  self->value_to = to;
  self->velocity = velocity / (to - from);
  self->damping = damping;
  self->mass = mass;
  self->stiffness = stiffness;
  self->epsilon = epsilon;

  self->value_cb = value_cb;
  self->done_cb = done_cb;
  self->user_data = user_data;

  self->value = from;

  self->estimated_duration = estimate_duration (self);

  return self;
}

AdwSpringAnimation *
adw_spring_animation_new_with_damping_ratio (GtkWidget                 *widget,
                                             gdouble                    from,
                                             gdouble                    to,
                                             gdouble                    velocity,
                                             gdouble                    damping_ratio,
                                             gdouble                    mass,
                                             gdouble                    stiffness,
                                             gdouble                    epsilon,
                                             AdwAnimationValueCallback  value_cb,
                                             AdwAnimationDoneCallback   done_cb,
                                             gpointer                   user_data)
{
  gdouble critical_damping = 2 * sqrt (mass * stiffness);
  gdouble damping = damping_ratio * critical_damping;

  return adw_spring_animation_new (widget, from, to, velocity, damping, mass,
                                   stiffness, epsilon, value_cb, done_cb, user_data);
}

AdwSpringAnimation *
adw_spring_animation_ref (AdwSpringAnimation *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  g_atomic_ref_count_inc (&self->ref_count);

  return self;
}

void
adw_spring_animation_unref (AdwSpringAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (g_atomic_ref_count_dec (&self->ref_count))
    adw_spring_animation_free (self);
}

void
adw_spring_animation_start (AdwSpringAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (!adw_get_enable_animations (self->widget) ||
      !gtk_widget_get_mapped (self->widget) ||
      ABS (self->value_from - self->value_to) < self->epsilon) {
    set_value (self, self->value_to);

    self->done_cb (self->user_data);

    return;
  }

  self->start_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (self->widget)) / 1000;

  if (self->tick_cb_id)
    return;

  g_signal_connect_swapped (self->widget, "unmap",
                            G_CALLBACK (adw_spring_animation_stop), self);
  self->tick_cb_id = gtk_widget_add_tick_callback (self->widget, (GtkTickCallback) tick_cb, self, NULL);
}

void
adw_spring_animation_stop (AdwSpringAnimation *self)
{
  g_return_if_fail (self != NULL);

  if (!self->tick_cb_id)
    return;

  gtk_widget_remove_tick_callback (self->widget, self->tick_cb_id);
  self->tick_cb_id = 0;

  g_signal_handlers_disconnect_by_func (self->widget, adw_spring_animation_stop, self);

  self->done_cb (self->user_data);
}

gdouble
adw_spring_animation_get_value (AdwSpringAnimation *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->value;
}

gdouble
adw_spring_animation_get_estimated_duration (AdwSpringAnimation *self)
{
  g_return_val_if_fail (self != NULL, 0.0);

  return self->estimated_duration;
}
