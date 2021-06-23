/*
 * Copyright (C) 2019-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-animation-util-private.h"

/**
 * adw_lerp:
 * @a: the start
 * @b: the end
 * @t: the interpolation rate
 *
 * Computes the linear interpolation between @a and @b for @t.
 *
 * Returns: the linear interpolation between @a and @b for @t
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
 * Returns: the ease out for @t
 *
 * Since: 1.0
 */

double
adw_ease_out_cubic (double t)
{
  double p = t - 1;
  return p * p * p + 1;
}

double
adw_ease_in_cubic (gdouble t)
{
  return t * t * t;
}

double
adw_ease_in_out_cubic (double t)
{
  double p = t * 2;

  if (p < 1)
    return 0.5 * p * p * p;

  p -= 2;

  return 0.5 * (p * p * p + 2);
}

/**
 * adw_get_enable_animations:
 * @widget: a `GtkWidget`
 *
 * Checks whether animations are enabled for @widget.
 *
 * This should be used when implementing an animated widget to know whether to
 * animate it or not.
 *
 * Returns: whether animations are enabled for @widget
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
