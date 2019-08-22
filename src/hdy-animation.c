/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-animation-private.h"

/**
 * PRIVATE:hdy-animation
 * @short_description: Animation helpers
 * @title: Animation Helpers
 * @stability: Private
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

gdouble
hdy_lerp (gdouble a, gdouble b, gdouble t)
{
  return a + (b - a) * (1.0 - t);
}

/* From clutter-easing.c, based on Robert Penner's
 * infamous easing equations, MIT license.
 */
gdouble
hdy_ease_out_cubic (gdouble t)
{
  gdouble p = t - 1;
  return p * p * p + 1;
}
