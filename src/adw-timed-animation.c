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
  AdwEasing easing;
  guint repeat_count;
  gboolean reverse;
  gboolean alternate;
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
  PROP_EASING,
  PROP_REPEAT_COUNT,
  PROP_REVERSE,
  PROP_ALTERNATE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static guint
adw_timed_animation_estimate_duration (AdwAnimation *animation)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (animation);

  if (self->repeat_count == 0)
    return ADW_DURATION_INFINITE;

  return self->duration * self->repeat_count;
}

static double
adw_timed_animation_calculate_value (AdwAnimation *animation,
                                     guint         t)
{
  AdwTimedAnimation *self = ADW_TIMED_ANIMATION (animation);

  double value;
  double iteration, progress;
  gboolean reverse = false;

  if (self->duration == 0)
    return self->value_to;

  progress = modf (((double) t / self->duration), &iteration);

  if (self->alternate)
    reverse = ((int) iteration % 2);

  if (self->reverse)
    reverse = !reverse;

  /* When the animation ends, return the exact final value, which depends on the
     direction the animation is going at that moment, having into account that at the
     time of this check we're already on the next iteration. */
  if (t >= adw_timed_animation_estimate_duration (animation))
    return self->alternate == reverse ? self->value_to : self->value_from;

  progress = reverse ? (1 - progress) : progress;

  switch (self->easing) {
    case ADW_EASING_EASE_IN_CUBIC:
      value = adw_ease_in_cubic (progress);
      break;
    case ADW_EASING_EASE_OUT_CUBIC:
      value = adw_ease_out_cubic (progress);
      break;
    case ADW_EASING_EASE_IN_OUT_CUBIC:
      value = adw_ease_in_out_cubic (progress);
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

  case PROP_EASING:
    g_value_set_enum (value, adw_timed_animation_get_easing (self));
    break;

  case PROP_REPEAT_COUNT:
    g_value_set_uint (value, adw_timed_animation_get_repeat_count (self));
    break;

  case PROP_REVERSE:
    g_value_set_boolean (value, adw_timed_animation_get_reverse (self));
    break;

  case PROP_ALTERNATE:
    g_value_set_boolean (value, adw_timed_animation_get_alternate (self));
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

  case PROP_EASING:
    adw_timed_animation_set_easing (self, g_value_get_enum (value));
    break;

  case PROP_REPEAT_COUNT:
    adw_timed_animation_set_repeat_count (self, g_value_get_uint (value));
    break;

  case PROP_REVERSE:
    adw_timed_animation_set_reverse (self, g_value_get_boolean (value));
    break;

  case PROP_ALTERNATE:
    adw_timed_animation_set_alternate (self, g_value_get_boolean (value));
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

  props[PROP_EASING] =
    g_param_spec_enum ("easing",
                       "Easing",
                       "Easing function used in the animation",
                       ADW_TYPE_EASING,
                       ADW_EASING_EASE_OUT_CUBIC,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_REPEAT_COUNT] =
    g_param_spec_uint ("repeat-count",
                       "Repeat count",
                       "Number of times the animation should play",
                       0,
                       G_MAXUINT,
                       1,
                       G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_REVERSE] =
    g_param_spec_boolean ("reverse",
                          "Reverse",
                          "Wheter the animation should play backwards",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  props[PROP_ALTERNATE] =
    g_param_spec_boolean ("alternate",
                          "Alternate",
                          "Whether the animation should change direction each time",
                          FALSE,
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

AdwEasing
adw_timed_animation_get_easing (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self),
                        ADW_EASING_EASE_IN_CUBIC);

  return self->easing;
}

void
adw_timed_animation_set_easing (AdwTimedAnimation *self,
                                AdwEasing          easing)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));
  g_return_if_fail (easing <= ADW_EASING_EASE_IN_OUT_CUBIC);

  if (self->easing == easing)
    return;

  self->easing = easing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EASING]);
}

guint
adw_timed_animation_get_repeat_count (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), 0);

  return self->repeat_count;
}

void
adw_timed_animation_set_repeat_count (AdwTimedAnimation *self,
                                      guint              repeat_count)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->repeat_count == repeat_count)
    return;

  self->repeat_count = repeat_count;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REPEAT_COUNT]);
}

gboolean
adw_timed_animation_get_reverse (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), FALSE);

  return self->reverse;
}

void
adw_timed_animation_set_reverse (AdwTimedAnimation *self,
                                 gboolean           reverse)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->reverse == reverse)
    return;

  self->reverse = reverse;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVERSE]);
}

gboolean
adw_timed_animation_get_alternate (AdwTimedAnimation *self)
{
  g_return_val_if_fail (ADW_IS_TIMED_ANIMATION (self), FALSE);

  return self->alternate;
}

void
adw_timed_animation_set_alternate (AdwTimedAnimation *self,
                                   gboolean           alternate)
{
  g_return_if_fail (ADW_IS_TIMED_ANIMATION (self));

  if (self->alternate == alternate)
    return;

  self->alternate = alternate;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALTERNATE]);
}
