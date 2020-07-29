/*
 * Copyright (C) 2019 Purism SPC
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
