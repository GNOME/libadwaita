/*
 * Copyright (C) 2019-2020 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-animation-util-private.h"
#include "adw-animation-private.h"


typedef struct
{
  GtkWidget *widget;

  double value;

  double value_from;
  double value_to;
  gint64 duration; /* ms */

  gint64 start_time; /* ms */
  guint tick_cb_id;
  gulong unmap_cb_id;

  AdwAnimationInterpolator interpolator;
  AdwAnimationTargetFunc value_cb;
  AdwAnimationTarget *target;
  gpointer user_data;

  AdwAnimationStatus status;
} AdwAnimationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwAnimation, adw_animation, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_VALUE,
  PROP_WIDGET,
  PROP_VALUE_FROM,
  PROP_VALUE_TO,
  PROP_DURATION,
  PROP_INTERPOLATOR,
  PROP_TARGET,
  PROP_STATUS,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_DONE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

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

  case PROP_VALUE_FROM:
    g_value_set_double (value, adw_animation_get_value_from (self));
    break;

  case PROP_VALUE_TO:
    g_value_set_double (value, adw_animation_get_value_to (self));
    break;

  case PROP_DURATION:
    g_value_set_int64 (value, adw_animation_get_duration (self));
    break;

  case PROP_INTERPOLATOR:
    g_value_set_enum (value, adw_animation_get_interpolator (self));
    break;

  case PROP_TARGET:
    g_value_set_object (value, adw_animation_get_target (self));
    break;

  case PROP_STATUS:
    g_value_set_enum (value, adw_animation_get_status (self));
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
    g_set_object (&priv->widget, g_value_get_object (value));
    break;

  case PROP_VALUE_FROM:
    adw_animation_set_value_from (self, g_value_get_double (value));
    break;

  case PROP_VALUE_TO:
    adw_animation_set_value_to (self, g_value_get_double (value));
    break;

  case PROP_DURATION:
    adw_animation_set_duration (self, g_value_get_int64 (value));
    break;

  case PROP_INTERPOLATOR:
    adw_animation_set_interpolator (self, g_value_get_enum (value));
    break;

  case PROP_TARGET:
    g_set_object (&priv->target, g_value_get_object (value));
    break;

  case PROP_STATUS:
    adw_animation_set_status (self, g_value_get_enum (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
set_value (AdwAnimation *self,
           double        value)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->value = value;
  adw_animation_target_set_value (priv->target, value);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

static void
adw_animation_constructed (GObject *object)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  priv->value = priv->value_from;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);

  G_OBJECT_CLASS (adw_animation_parent_class)->constructed (object);
}

static void
adw_animation_dispose (GObject *object)
{
  AdwAnimation *self = ADW_ANIMATION (object);
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  adw_animation_stop (self);

  g_clear_object (&priv->target);
  g_clear_object (&priv->widget);

  G_OBJECT_CLASS (adw_animation_parent_class)->dispose (object);
}

static void
adw_animation_class_init (AdwAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_animation_constructed;
  object_class->dispose = adw_animation_dispose;
  object_class->set_property = adw_animation_set_property;
  object_class->get_property = adw_animation_get_property;

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

  props[PROP_VALUE_FROM] =
    g_param_spec_double ("value-from",
                         "Initial value",
                         "Initial value of the animation",
                         -G_MAXDOUBLE,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_VALUE_TO] =
    g_param_spec_double ("value-to",
                         "Final value",
                         "Final value of the animation",
                         -G_MAXDOUBLE,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_DURATION] =
    g_param_spec_int64 ("duration",
                        "Duration",
                        "Duration of the animation",
                        0,
                        G_MAXINT64,
                        0,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_INTERPOLATOR] =
    g_param_spec_enum ("interpolator",
                       "Interpolator",
                       "Easing function used in the animation",
                       ADW_TYPE_ANIMATION_INTERPOLATOR,
                       ADW_ANIMATION_INTERPOLATOR_EASE_OUT,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_TARGET] =
    g_param_spec_object ("target",
                         "Target",
                         "Target",
                         ADW_TYPE_ANIMATION_TARGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_STATUS] =
    g_param_spec_enum ("status",
                       "Status",
                       "Status of the animation",
                       ADW_TYPE_ANIMATION_STATUS,
                       ADW_ANIMATION_STATUS_NONE,
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
}

static void
done (AdwAnimation *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  if (priv->status == ADW_ANIMATION_STATUS_COMPLETED)
    return;

  priv->status = ADW_ANIMATION_STATUS_COMPLETED;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATUS]);

  g_signal_emit (self, signals[SIGNAL_DONE], 0);
}

static gboolean
tick_cb (GtkWidget     *widget,
         GdkFrameClock *frame_clock,
         AdwAnimation  *self)
{
  AdwAnimationPrivate *priv = adw_animation_get_instance_private (self);

  gint64 frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000; /* ms */
  double t = (double) (frame_time - priv->start_time) / priv->duration;
  double value;

  if (t >= 1) {
    priv->tick_cb_id = 0;

    set_value (self, priv->value_to);

    if (priv->unmap_cb_id) {
      g_signal_handler_disconnect (priv->widget, priv->unmap_cb_id);
      priv->unmap_cb_id = 0;
    }

    done (self);

    return G_SOURCE_REMOVE;
  }

  switch (priv->interpolator) {
    case ADW_ANIMATION_INTERPOLATOR_EASE_IN:
      value = adw_ease_in_cubic (t);
      break;
    case ADW_ANIMATION_INTERPOLATOR_EASE_OUT:
      value = adw_ease_out_cubic (t);
      break;
    case ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT:
      value = adw_ease_in_out_cubic (t);
      break;
    default:
      g_assert_not_reached ();
  }

  set_value (self, adw_lerp (priv->value_from, priv->value_to, value));

  return G_SOURCE_CONTINUE;
}

AdwAnimation *
adw_animation_new (GtkWidget                 *widget,
                   double                     from,
                   double                     to,
                   gint64                     duration,
                   AdwAnimationTargetFunc     target_func,
                   gpointer                   user_data)
{
  AdwAnimationTarget *target;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (target_func != NULL, NULL);

  target = adw_animation_target_new (target_func, user_data);

  return g_object_new (ADW_TYPE_ANIMATION,
                       "widget", widget,
                       "value-from", from,
                       "value-to", to,
                       "duration", duration,
                       "target", target,
                       NULL);
}


void
adw_animation_start (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (!adw_get_enable_animations (priv->widget) ||
      !gtk_widget_get_mapped (priv->widget) ||
      priv->duration <= 0) {
    set_value (self, priv->value_to);

    done (self);

    return;
  }

  priv->start_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (priv->widget)) / 1000;

  if (priv->tick_cb_id)
    return;

  priv->unmap_cb_id =
    g_signal_connect_swapped (priv->widget, "unmap",
                              G_CALLBACK (adw_animation_stop), self);
  priv->tick_cb_id = gtk_widget_add_tick_callback (priv->widget, (GtkTickCallback) tick_cb, self, NULL);
}

void
adw_animation_stop (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->tick_cb_id) {
    gtk_widget_remove_tick_callback (priv->widget, priv->tick_cb_id);
    priv->tick_cb_id = 0;
  }

  if (priv->unmap_cb_id) {
    g_signal_handler_disconnect (priv->widget, priv->unmap_cb_id);
    priv->unmap_cb_id = 0;
  }

  done (self);
}

double
adw_animation_get_value (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), 0.0);

  priv = adw_animation_get_instance_private (self);

  return priv->value;
}

GtkWidget *
adw_animation_get_widget (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), NULL);

  priv = adw_animation_get_instance_private (self);

  return priv->widget;
}

double
adw_animation_get_value_from (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), 0.0);

  priv = adw_animation_get_instance_private (self);

  return priv->value_from;
}

void
adw_animation_set_value_from (AdwAnimation *self,
                              double        value)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->value_from == value)
    return;

  priv->value_from = value;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE_FROM]);
}

double
adw_animation_get_value_to (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), 0.0);

  priv = adw_animation_get_instance_private (self);

  return priv->value_to;
}

void
adw_animation_set_value_to (AdwAnimation *self,
                            double        value)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->value_to == value)
    return;

  priv->value_to = value;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE_TO]);
}

gint64
adw_animation_get_duration (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), 0);

  priv = adw_animation_get_instance_private (self);

  return priv->duration;
}

void
adw_animation_set_duration (AdwAnimation *self,
                            gint64        duration)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));

  priv = adw_animation_get_instance_private (self);

  if (priv->duration == duration)
    return;

  priv->duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DURATION]);
}

AdwAnimationInterpolator
adw_animation_get_interpolator (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), ADW_ANIMATION_INTERPOLATOR_EASE_OUT);

  priv = adw_animation_get_instance_private (self);

  return priv->interpolator;
}

void
adw_animation_set_interpolator (AdwAnimation             *self,
                                AdwAnimationInterpolator  interpolator)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));
  g_return_if_fail (interpolator <= ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT);

  priv = adw_animation_get_instance_private (self);

  if (priv->interpolator == interpolator)
    return;

  priv->interpolator = interpolator;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATOR]);
}

AdwAnimationTarget *
adw_animation_get_target (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), NULL);

  priv = adw_animation_get_instance_private (self);

  return priv->target;
}

AdwAnimationStatus
adw_animation_get_status (AdwAnimation *self)
{
  AdwAnimationPrivate *priv;

  g_return_val_if_fail (ADW_IS_ANIMATION (self), ADW_ANIMATION_STATUS_NONE);

  priv = adw_animation_get_instance_private (self);

  return priv->status;
}

void
adw_animation_set_status (AdwAnimation       *self,
                          AdwAnimationStatus  status)
{
  AdwAnimationPrivate *priv;

  g_return_if_fail (ADW_IS_ANIMATION (self));
  g_return_if_fail (status <= ADW_ANIMATION_STATUS_CANCELED);

  priv = adw_animation_get_instance_private (self);

  if (priv->status == status)
    return;

  priv->status = status;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STATUS]);
}

struct _AdwAnimationTarget
{
  GObject parent_instance;

  AdwAnimationTargetFunc callback;
  gpointer user_data;
};

G_DEFINE_FINAL_TYPE (AdwAnimationTarget, adw_animation_target, G_TYPE_OBJECT);

static void
adw_animation_target_class_init (AdwAnimationTargetClass *klass)
{
}

static void
adw_animation_target_init (AdwAnimationTarget *self)
{
}

AdwAnimationTarget *
adw_animation_target_new (AdwAnimationTargetFunc callback,
                          gpointer               user_data)
{
  AdwAnimationTarget *self;

  g_return_val_if_fail (callback != NULL, NULL);

  self = g_object_new (ADW_TYPE_ANIMATION_TARGET, NULL);

  self->callback = callback;
  self->user_data = user_data;

  return self;
}

void
adw_animation_target_set_value (AdwAnimationTarget *self,
                                double              value)
{
  g_return_if_fail (ADW_IS_ANIMATION_TARGET (self));

  self->callback (value, self->user_data);
}
