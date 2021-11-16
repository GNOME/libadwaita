/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-timed-animation-private.h"

#include "adw-animation-private.h"
#include "adw-animation-util-private.h"

struct _AdwTimedAnimation
{
  AdwAnimation parent_instance;

  double value_from;
  double value_to;
  guint duration; /* ms */

  AdwAnimationInterpolator interpolator;
};

struct _AdwTimedAnimationClass
{
  AdwAnimationClass parent_class;
};

G_DEFINE_TYPE (AdwTimedAnimation, adw_timed_animation, ADW_TYPE_ANIMATION)

enum {
  PROP_0,
  PROP_VALUE_FROM,
  PROP_VALUE_TO,
  PROP_DURATION,
  PROP_INTERPOLATOR,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static guint
adw_timed_animation_estimate_duration (AdwAnimation *animation)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (animation);

  return self->duration;
}

static double
adw_timed_animation_calculate_value (AdwAnimation *animation,
                                     guint         t)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (animation);
  double value;

  if (self->duration == 0)
    return self->value_to;

  switch (self->interpolator) {
    case ADW_ANIMATION_INTERPOLATOR_EASE_IN:
      value = adw_ease_in_cubic ((double) t / self->duration);
      break;
    case ADW_ANIMATION_INTERPOLATOR_EASE_OUT:
      value = adw_ease_out_cubic ((double) t / self->duration);
      break;
    case ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT:
      value = adw_ease_in_out_cubic ((double) t / self->duration);
      break;
    default:
      g_assert_not_reached ();
  }

  return adw_lerp (self->value_from, self->value_to, value);
}

static void
adw_timed_animation_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (object);

  switch (prop_id) {
  case PROP_VALUE_FROM:
    g_value_set_double (value, adw_timed_animation_get_value_from (self));
    break;

  case PROP_VALUE_TO:
    g_value_set_double (value, adw_timed_animation_get_value_to (self));
    break;

  case PROP_DURATION:
    g_value_set_uint (value, adw_timed_animation_get_duration (self));
    break;

  case PROP_INTERPOLATOR:
    g_value_set_enum (value, adw_timed_animation_get_interpolator (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_timed_animation_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (object);

  switch (prop_id) {
  case PROP_VALUE_FROM:
    adw_timed_animation_set_value_from (self, g_value_get_double (value));
    break;

  case PROP_VALUE_TO:
    adw_timed_animation_set_value_to (self, g_value_get_double (value));
    break;

  case PROP_DURATION:
    adw_timed_animation_set_duration (self, g_value_get_uint (value));
    break;

  case PROP_INTERPOLATOR:
    adw_timed_animation_set_interpolator (self, g_value_get_enum (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_timed_animation_class_init (AdwTimedAnimationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AdwAnimationClass *animation_class = ADW_ANIMATION_CLASS (klass);

  object_class->set_property = adw_timed_animation_set_property;
  object_class->get_property = adw_timed_animation_get_property;

  animation_class->estimate_duration = adw_timed_animation_estimate_duration;
  animation_class->calculate_value = adw_timed_animation_calculate_value;

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
    g_param_spec_uint ("duration",
                       "Duration",
                       "Duration of the animation in ms",
                       0,
                       ADW_DURATION_INFINITE,
                       0,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_INTERPOLATOR] =
    g_param_spec_enum ("interpolator",
                       "Interpolator",
                       "Easing function used in the animation",
                       ADW_TYPE_ANIMATION_INTERPOLATOR,
                       ADW_ANIMATION_INTERPOLATOR_EASE_OUT,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_timed_animation_init (AdwTimedAnimation *self)
{
}

AdwAnimation *
adw_timed_animation_new (GtkWidget          *widget,
                         double              from,
                         double              to,
                         guint               duration,
                         AdwAnimationTarget *target)
{
  AdwAnimation *animation;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);
  g_return_val_if_fail (ADW_IS_ANIMATION_TARGET (target), NULL);

  animation = g_object_new (ADW_TYPE_TIMED_ANIMATION,
                            "widget", widget,
                            "value-from", from,
                            "value-to", to,
                            "duration", duration,
                            "target", target,
                            NULL);

  g_object_unref (target);

  return animation;
}

double
adw_timed_animation_get_value_from (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0.0);

  return self->value_from;
}

void
adw_timed_animation_set_value_from (AdwTimedAnimation *self,
                                    double             value)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->value_from == value)
    return;

  self->value_from = value;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE_FROM]);
}

double
adw_timed_animation_get_value_to (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0.0);

  return self->value_to;
}

void
adw_timed_animation_set_value_to (AdwTimedAnimation *self,
                                  double             value)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->value_to == value)
    return;

  self->value_to = value;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE_TO]);
}

guint
adw_timed_animation_get_duration (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0);

  return self->duration;
}

void
adw_timed_animation_set_duration (AdwTimedAnimation *self,
                                  guint              duration)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->duration == duration)
    return;

  self->duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DURATION]);
}

AdwAnimationInterpolator
adw_timed_animation_get_interpolator (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self),
                        ADW_ANIMATION_INTERPOLATOR_EASE_IN);

  return self->interpolator;
}

void
adw_timed_animation_set_interpolator (AdwTimedAnimation        *self,
                                      AdwAnimationInterpolator  interpolator)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));
  g_return_if_fail (interpolator <= ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT);

  if (self->interpolator == interpolator)
    return;

  self->interpolator = interpolator;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATOR]);
}
