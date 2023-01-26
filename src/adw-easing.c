/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-easing.h"

#include <math.h>

/*
 * Copied from:
 *   https://gitlab.gnome.org/GNOME/clutter/-/blob/a236494ea7f31848b4a459dad41330f225137832/clutter/clutter-easing.c
 *   https://gitlab.gnome.org/GNOME/clutter/-/blob/a236494ea7f31848b4a459dad41330f225137832/clutter/clutter-enums.h
 *
 * Copyright (C) 2011  Intel Corporation
 */

/**
 * AdwEasing:
 * @ADW_LINEAR: Linear tweening.
 * @ADW_EASE_IN_QUAD: Quadratic tweening.
 * @ADW_EASE_OUT_QUAD: Quadratic tweening, inverse of `ADW_EASE_IN_QUAD`.
 * @ADW_EASE_IN_OUT_QUAD: Quadratic tweening, combining `ADW_EASE_IN_QUAD` and
 *   `ADW_EASE_OUT_QUAD`.
 * @ADW_EASE_IN_CUBIC: Cubic tweening.
 * @ADW_EASE_OUT_CUBIC: Cubic tweening, inverse of `ADW_EASE_IN_CUBIC`.
 * @ADW_EASE_IN_OUT_CUBIC: Cubic tweening, combining `ADW_EASE_IN_CUBIC` and
 *   `ADW_EASE_OUT_CUBIC`.
 * @ADW_EASE_IN_QUART: Quartic tweening.
 * @ADW_EASE_OUT_QUART: Quartic tweening, inverse of `ADW_EASE_IN_QUART`.
 * @ADW_EASE_IN_OUT_QUART: Quartic tweening, combining `ADW_EASE_IN_QUART` and
 *   `ADW_EASE_OUT_QUART`.
 * @ADW_EASE_IN_QUINT: Quintic tweening.
 * @ADW_EASE_OUT_QUINT: Quintic tweening, inverse of `ADW_EASE_IN_QUINT`.
 * @ADW_EASE_IN_OUT_QUINT: Quintic tweening, combining `ADW_EASE_IN_QUINT` and
 *   `ADW_EASE_OUT_QUINT`.
 * @ADW_EASE_IN_SINE: Sine wave tweening.
 * @ADW_EASE_OUT_SINE: Sine wave tweening, inverse of `ADW_EASE_IN_SINE`.
 * @ADW_EASE_IN_OUT_SINE: Sine wave tweening, combining `ADW_EASE_IN_SINE` and
 *   `ADW_EASE_OUT_SINE`.
 * @ADW_EASE_IN_EXPO: Exponential tweening.
 * @ADW_EASE_OUT_EXPO: Exponential tweening, inverse of `ADW_EASE_IN_EXPO`.
 * @ADW_EASE_IN_OUT_EXPO: Exponential tweening, combining `ADW_EASE_IN_EXPO` and
 *   `ADW_EASE_OUT_EXPO`.
 * @ADW_EASE_IN_CIRC: Circular tweening.
 * @ADW_EASE_OUT_CIRC: Circular tweening, inverse of `ADW_EASE_IN_CIRC`.
 * @ADW_EASE_IN_OUT_CIRC: Circular tweening, combining `ADW_EASE_IN_CIRC` and
 *   `ADW_EASE_OUT_CIRC`.
 * @ADW_EASE_IN_ELASTIC: Elastic tweening, with offshoot on start.
 * @ADW_EASE_OUT_ELASTIC: Elastic tweening, with offshoot on end, inverse of
 *   `ADW_EASE_IN_ELASTIC`.
 * @ADW_EASE_IN_OUT_ELASTIC: Elastic tweening, with offshoot on both ends,
 *   combining `ADW_EASE_IN_ELASTIC` and `ADW_EASE_OUT_ELASTIC`.
 * @ADW_EASE_IN_BACK: Overshooting cubic tweening, with backtracking on start.
 * @ADW_EASE_OUT_BACK: Overshooting cubic tweening, with backtracking on end,
 *   inverse of `ADW_EASE_IN_BACK`.
 * @ADW_EASE_IN_OUT_BACK: Overshooting cubic tweening, with backtracking on both
 *   ends, combining `ADW_EASE_IN_BACK` and `ADW_EASE_OUT_BACK`.
 * @ADW_EASE_IN_BOUNCE: Exponentially decaying parabolic (bounce) tweening,
 *   on start.
 * @ADW_EASE_OUT_BOUNCE: Exponentially decaying parabolic (bounce) tweening,
 *   with bounce on end, inverse of `ADW_EASE_IN_BOUNCE`.
 * @ADW_EASE_IN_OUT_BOUNCE: Exponentially decaying parabolic (bounce) tweening,
 *   with bounce on both ends, combining `ADW_EASE_IN_BOUNCE` and
 *   `ADW_EASE_OUT_BOUNCE`.
 *
 * Describes the available easing functions for use with
 * [class@TimedAnimation].
 *
 * New values may be added to this enumeration over time.
 */

static inline double
linear (double t,
        double d)
{
  return t / d;
}

static inline double
ease_in_quad (double t,
              double d)
{
  double p = t / d;

  return p * p;
}

static inline double
ease_out_quad (double t,
               double d)
{
  double p = t / d;

  return -1.0 * p * (p - 2);
}

static inline double
ease_in_out_quad (double t,
                  double d)
{
  double p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p;

  p -= 1;

  return -0.5 * (p * (p - 2) - 1);
}

static inline double
ease_in_cubic (double t,
               double d)
{
  double p = t / d;

  return p * p * p;
}

static inline double
ease_out_cubic (double t,
                double d)
{
  double p = t / d - 1;

  return p * p * p + 1;
}

static inline double
ease_in_out_cubic (double t,
                   double d)
{
  double p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p * p;

  p -= 2;

  return 0.5 * (p * p * p + 2);
}

static inline double
ease_in_quart (double t,
               double d)
{
  double p = t / d;

  return p * p * p * p;
}

static inline double
ease_out_quart (double t,
                double d)
{
  double p = t / d - 1;

  return -1.0 * (p * p * p * p - 1);
}

static inline double
ease_in_out_quart (double t,
                   double d)
{
  double p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p * p * p;

  p -= 2;

  return -0.5 * (p * p * p * p - 2);
}

static inline double
ease_in_quint (double t,
               double d)
{
  double p = t / d;

  return p * p * p * p * p;
}

static inline double
ease_out_quint (double t,
                double d)
{
  double p = t / d - 1;

  return p * p * p * p * p + 1;
}

static inline double
ease_in_out_quint (double t,
                   double d)
{
  double p = t / (d / 2);

  if (p < 1)
    return 0.5 * p * p * p * p * p;

  p -= 2;

  return 0.5 * (p * p * p * p * p + 2);
}

static inline double
ease_in_sine (double t,
              double d)
{
  return -1.0 * cos (t / d * G_PI_2) + 1.0;
}

static inline double
ease_out_sine (double t,
               double d)
{
  return sin (t / d * G_PI_2);
}

static inline double
ease_in_out_sine (double t,
                  double d)
{
  return -0.5 * (cos (G_PI * t / d) - 1);
}

static inline double
ease_in_expo (double t,
              double d)
{
  return G_APPROX_VALUE (t, 0, DBL_EPSILON) ? 0.0 : pow (2, 10 * (t / d - 1));
}

static double
ease_out_expo (double t,
               double d)
{
  return G_APPROX_VALUE (t, d, DBL_EPSILON) ? 1.0 : -pow (2, -10 * t / d) + 1;
}

static inline double
ease_in_out_expo (double t,
                  double d)
{
  double p;

  if (G_APPROX_VALUE (t, 0, DBL_EPSILON))
    return 0.0;

  if (G_APPROX_VALUE (t, d, DBL_EPSILON))
    return 1.0;

  p = t / (d / 2);

  if (p < 1)
    return 0.5 * pow (2, 10 * (p - 1));

  p -= 1;

  return 0.5 * (-pow (2, -10 * p) + 2);
}

static inline double
ease_in_circ (double t,
              double d)
{
  double p = t / d;

  return -1.0 * (sqrt (1 - p * p) - 1);
}

static inline double
ease_out_circ (double t,
               double d)
{
  double p = t / d - 1;

  return sqrt (1 - p * p);
}

static inline double
ease_in_out_circ (double t,
                  double d)
{
  double p = t / (d / 2);

  if (p < 1)
    return -0.5 * (sqrt (1 - p * p) - 1);

  p -= 2;

  return 0.5 * (sqrt (1 - p * p) + 1);
}

static inline double
ease_in_elastic (double t,
                 double d)
{
  double p = d * .3;
  double s = p / 4;
  double q = t / d;

  if (G_APPROX_VALUE (q, 1, DBL_EPSILON))
    return 1.0;

  q -= 1;

  return -(pow (2, 10 * q) * sin ((q * d - s) * (2 * G_PI) / p));
}

static inline double
ease_out_elastic (double t,
                  double d)
{
  double p = d * .3;
  double s = p / 4;
  double q = t / d;

  if (G_APPROX_VALUE (q, 1, DBL_EPSILON))
    return 1.0;

  return pow (2, -10 * q) * sin ((q * d - s) * (2 * G_PI) / p) + 1.0;
}

static inline double
ease_in_out_elastic (double t,
                     double d)
{
  double p = d * (.3 * 1.5);
  double s = p / 4;
  double q = t / (d / 2);

  if (G_APPROX_VALUE (q, 2, DBL_EPSILON))
    return 1.0;

  if (q < 1) {
    q -= 1;

    return -.5 * (pow (2, 10 * q) * sin ((q * d - s) * (2 * G_PI) / p));
  } else {
    q -= 1;

    return pow (2, -10 * q)
         * sin ((q * d - s) * (2 * G_PI) / p)
         * .5 + 1.0;
  }
}

static inline double
ease_in_back (double t,
              double d)
{
  double p = t / d;

  return p * p * ((1.70158 + 1) * p - 1.70158);
}

static inline double
ease_out_back (double t,
               double d)
{
  double p = t / d - 1;

  return p * p * ((1.70158 + 1) * p + 1.70158) + 1;
}

static inline double
ease_in_out_back (double t,
                  double d)
{
  double p = t / (d / 2);
  double s = 1.70158 * 1.525;

  if (p < 1)
    return 0.5 * (p * p * ((s + 1) * p - s));

  p -= 2;

  return 0.5 * (p * p * ((s + 1) * p + s) + 2);
}

static inline double
ease_out_bounce (double t,
                 double d)
{
  double p = t / d;

  if (p < (1 / 2.75)) {
    return 7.5625 * p * p;
  } else if (p < (2 / 2.75)) {
    p -= (1.5 / 2.75);

    return 7.5625 * p * p + .75;
  } else if (p < (2.5 / 2.75)) {
    p -= (2.25 / 2.75);

    return 7.5625 * p * p + .9375;
  } else {
    p -= (2.625 / 2.75);

    return 7.5625 * p * p + .984375;
  }
}

static inline double
ease_in_bounce (double t,
                double d)
{
  return 1.0 - ease_out_bounce (d - t, d);
}

static inline double
ease_in_out_bounce (double t,
                    double d)
{
  if (t < d / 2)
    return ease_in_bounce (t * 2, d) * 0.5;
  else
    return ease_out_bounce (t * 2 - d, d) * 0.5 + 1.0 * 0.5;
}

/**
 * adw_easing_ease:
 * @self: an easing value
 * @value: a value to ease
 *
 * Computes easing with @easing for @value.
 *
 * @value should generally be in the [0, 1] range.
 *
 * Returns: the easing for @value
 */
double
adw_easing_ease (AdwEasing self,
                 double    value)
{
  switch (self) {
    case ADW_LINEAR:
      return linear (value, 1);
    case ADW_EASE_IN_QUAD:
      return ease_in_quad (value, 1);
    case ADW_EASE_OUT_QUAD:
      return ease_out_quad (value, 1);
    case ADW_EASE_IN_OUT_QUAD:
      return ease_in_out_quad (value, 1);
    case ADW_EASE_IN_CUBIC:
      return ease_in_cubic (value, 1);
    case ADW_EASE_OUT_CUBIC:
      return ease_out_cubic (value, 1);
    case ADW_EASE_IN_OUT_CUBIC:
      return ease_in_out_cubic (value, 1);
    case ADW_EASE_IN_QUART:
      return ease_in_quart (value, 1);
    case ADW_EASE_OUT_QUART:
      return ease_out_quart (value, 1);
    case ADW_EASE_IN_OUT_QUART:
      return ease_in_out_quart (value, 1);
    case ADW_EASE_IN_QUINT:
      return ease_in_quint (value, 1);
    case ADW_EASE_OUT_QUINT:
      return ease_out_quint (value, 1);
    case ADW_EASE_IN_OUT_QUINT:
      return ease_in_out_quint (value, 1);
    case ADW_EASE_IN_SINE:
      return ease_in_sine (value, 1);
    case ADW_EASE_OUT_SINE:
      return ease_out_sine (value, 1);
    case ADW_EASE_IN_OUT_SINE:
      return ease_in_out_sine (value, 1);
    case ADW_EASE_IN_EXPO:
      return ease_in_expo (value, 1);
    case ADW_EASE_OUT_EXPO:
      return ease_out_expo (value, 1);
    case ADW_EASE_IN_OUT_EXPO:
      return ease_in_out_expo (value, 1);
    case ADW_EASE_IN_CIRC:
      return ease_in_circ (value, 1);
    case ADW_EASE_OUT_CIRC:
      return ease_out_circ (value, 1);
    case ADW_EASE_IN_OUT_CIRC:
      return ease_in_out_circ (value, 1);
    case ADW_EASE_IN_ELASTIC:
      return ease_in_elastic (value, 1);
    case ADW_EASE_OUT_ELASTIC:
      return ease_out_elastic (value, 1);
    case ADW_EASE_IN_OUT_ELASTIC:
      return ease_in_out_elastic (value, 1);
    case ADW_EASE_IN_BACK:
      return ease_in_back (value, 1);
    case ADW_EASE_OUT_BACK:
      return ease_out_back (value, 1);
    case ADW_EASE_IN_OUT_BACK:
      return ease_in_out_back (value, 1);
    case ADW_EASE_IN_BOUNCE:
      return ease_in_bounce (value, 1);
    case ADW_EASE_OUT_BOUNCE:
      return ease_out_bounce (value, 1);
    case ADW_EASE_IN_OUT_BOUNCE:
      return ease_in_out_bounce (value, 1);
    default:
      g_assert_not_reached ();
  }
}
