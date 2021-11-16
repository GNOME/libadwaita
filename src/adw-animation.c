/*
 * Copyright (C) 2019-2020 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-animation-private.h"
#include "adw-animation-target-private.h"
#include "adw-animation-util-private.h"

typedef struct
{
  GtkWidget *widget;

  double value;

  gint64 start_time; /* ms */
  gint64 paused_time;
  guint tick_cb_id;
  gulong unmap_cb_id;

  AdwAnimationTarget *target;
  gpointer user_data;

  AdwAnimationState state;
} AdwAnimationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwAnimation, adw_animation, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_VALUE,
  PROP_WIDGET,
  PROP_TARGET,
  PROP_STATE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_DONE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
widget_notify_cb (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->widget = NULL;
}

static void
set_widget (AdwAnimation *self,
            GtkWidget    *widget)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->widget == widget)
    return;

  if (priv->widget)
    g_object_weak_unref (G_OBJECT (priv->widget),
                         (GWeakNotify) widget_notify_cb,
                         self);

  priv->widget = widget;

  if (priv->widget)
    g_object_weak_ref (G_OBJECT (priv->widget),
                       (GWeakNotify) widget_notify_cb,
                       self);
}

static void
set_value (AdwAnimation *self,
           guint         t)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->value = ADW_ANIMATION_GET_CLASS (self)->calculate_value (self, t);

  adw_animation_target_set_value (priv->target, priv->value);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

static void
stop_animation (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->tick_cb_id) {
    gtk_widget_remove_tick_callback (priv->widget, priv->tick_cb_id);
    priv->tick_cb_id = 0;
  }

  if (priv->unmap_cb_id) {
    g_signal_handler_disconnect (priv->widget, priv->unmap_cb_id);
    priv->unmap_cb_id = 0;
  }
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         AdwAnimation  *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000; /* ms */
  guint duration = ADW_ANIMATION_GET_CLASS (self)->estimate_duration (self);
  guint t = (guint) (frame_time - priv->start_time);

  if (t >= duration && duration != ADW_DURATION_INFINITE) {
    adw_animation_skip (self);

    return G_SOURCE_REMOVE;
  }

  set_value (self, t);

  return G_SOURCE_CONTINUE;
}

static guint
adw_animation_estimate_duration (AdwAnimation *animation)
{
  g_assert_not_reached ();
}

static double
adw_animation_calculate_value (AdwAnimation *animation,
                               guint         t)
{
  g_assert_not_reached ();
}

static void
play (AdwAnimation *self)
{

  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_PLAYING) {
    g_critical ("Trying to play animation %p, but it's already playing", self);

    return;
  }

  priv->state = ADW_ANIMATION_PLAYING;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  if (!adw_get_enable_animations (priv->widget) ||
      !gtk_widget_get_mapped (priv->widget)) {
    adw_animation_skip (g_object_ref (self));

    return;
  }

  priv->start_time += gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (priv->widget)) / 1000;
  priv->start_time -= priv->paused_time;

  if (priv->tick_cb_id)
    return;

  priv->unmap_cb_id =
    g_signal_connect_swapped (priv->widget, "unmap",
                              G_CALLBACK (adw_animation_skip), self);
  priv->tick_cb_id = gtk_widget_add_tick_callback (priv->widget, (GtkTickCallback) tick_cb, self, NULL);

  g_object_ref (self);
}

static void
adw_animation_constructed (GObject *object)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  G_OBJECT_CLASS (adw_animation_parent_class)->constructed (object);

  priv->value = ADW_ANIMATION_GET_CLASS (self)->calculate_value (self, 0);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

static void
adw_animation_dispose (GObject *object)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_PLAYING)
    adw_animation_skip (self);

  g_clear_object (&priv->target);

  set_widget (self, NULL);

  G_OBJECT_CLASS (adw_animation_parent_class)->dispose (object);
}

static void
adw_animation_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  AdwAnimation *self = ADW_ANIMATION (object);

  switch (prop_id) {
  case PROP_VALUE:
    g_value_set_double (value, adw_animation_get_value (self));
    break;

  case PROP_WIDGET:
    g_value_set_object (value, adw_animation_get_widget (self));
    break;

  case PROP_TARGET:
    g_value_set_object (value, adw_animation_get_target (self));
    break;

  case PROP_STATE:
    g_value_set_enum (value, adw_animation_get_state (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_animation_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  switch (prop_id) {
  case PROP_WIDGET:
    set_widget (self, g_value_get_object (value));
    break;

  case PROP_TARGET:
    g_set_object (&priv->target, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_animation_class_init (AdwAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_animation_constructed;
  object_class->dispose = adw_animation_dispose;
  object_class->set_property = adw_animation_set_property;
  object_class->get_property = adw_animation_get_property;

  klass->estimate_duration = adw_animation_estimate_duration;
  klass->calculate_value = adw_animation_calculate_value;

  props[PROP_VALUE] =
    g_param_spec_double ("value",
                         "Value",
                         "The current value of the animation",
                         -G_MAXDOUBLE,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READABLE);

  props[PROP_WIDGET] =
    g_param_spec_object ("widget",
                         "Widget",
                         "The target widget whose property will be animated",
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  props[PROP_TARGET] =
    g_param_spec_object ("target",
                         "Target",
                         "Target",
                         ADW_TYPE_ANIMATION_TARGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_STATE] =
    g_param_spec_enum ("state",
                       "State",
                       "State of the animation",
                       ADW_TYPE_ANIMATION_STATE,
                       ADW_ANIMATION_IDLE,
                       G_PARAM_READABLE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  signals[SIGNAL_DONE] =
    g_signal_new ("done",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);
}

static void
adw_animation_init (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->state = ADW_ANIMATION_IDLE;
}

GtkWidget *
adw_animation_get_widget (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), NULL);

  priv = adw_animation_get_instance_private (self);

  return priv->widget;
}

AdwAnimationTarget *
adw_animation_get_target (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), NULL);

  priv = adw_animation_get_instance_private (self);

  return priv->target;
}

double
adw_animation_get_value (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), 0.0);

  priv = adw_animation_get_instance_private (self);

  return priv->value;
}

AdwAnimationState
adw_animation_get_state (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), ADW_ANIMATION_IDLE);

  priv = adw_animation_get_instance_private (self);

  return priv->state;
}

void
adw_animation_play (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state != ADW_ANIMATION_IDLE) {
    priv->state = ADW_ANIMATION_IDLE;
    priv->start_time = 0;
    priv->paused_time = 0;
  }

  play (self);
}

void
adw_animation_pause (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state != ADW_ANIMATION_PLAYING)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  priv->state = ADW_ANIMATION_PAUSED;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  stop_animation (self);

  priv->paused_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (priv->widget)) / 1000;

  g_object_thaw_notify (G_OBJECT (self));

  g_object_unref (self);
}

void
adw_animation_resume (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state != ADW_ANIMATION_PAUSED) {
    g_critical ("Trying to resume animation %p, but it's not paused", self);

    return;
  }

  play (self);
}

void
adw_animation_skip (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;
  gboolean was_playing;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_FINISHED)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  was_playing = priv->state == ADW_ANIMATION_PLAYING;

  priv->state = ADW_ANIMATION_FINISHED;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  stop_animation (self);

  set_value (self, ADW_ANIMATION_GET_CLASS (self)->estimate_duration (self));

  priv->start_time = 0;
  priv->paused_time = 0;

  g_object_thaw_notify (G_OBJECT (self));

  g_signal_emit (self, signals[SIGNAL_DONE], 0);

  if (was_playing)
    g_object_unref (self);
}

void
adw_animation_reset (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;
  gboolean was_playing;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->state == ADW_ANIMATION_IDLE)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  was_playing = priv->state == ADW_ANIMATION_PLAYING;

  priv->state = ADW_ANIMATION_IDLE;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATE]);

  stop_animation (self);

  set_value (self, 0);
  priv->start_time = 0;
  priv->paused_time = 0;

  g_object_thaw_notify (G_OBJECT (self));

  if (was_playing)
    g_object_unref (self);
}
