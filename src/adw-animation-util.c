/*
 * Copyright (C) 2019-2020 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
 * Returns: the computed value
 */
double
adw_lerp (double a, double b, double t)
{
  return a * (1.0 - t) + b * t;
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
 */
gboolean
adw_get_enable_animations (GtkWidget *widget)
{
  gboolean enable_animations = TRUE;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-enable-animations", &enable_animations,
                NULL);

  return enable_animations;
}

gboolean
adw_get_reduce_motion (GtkWidget *widget)
{
  GtkReducedMotion reduced_motion = GTK_REDUCED_MOTION_NO_PREFERENCE;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-interface-reduced-motion", &reduced_motion,
                NULL);

  return reduced_motion == GTK_REDUCED_MOTION_REDUCE;
}
