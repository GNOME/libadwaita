/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


static void
test_hdy_arrows_setters (void)
{
  HdyArrows *arrows;

  arrows = HDY_ARROWS (hdy_arrows_new());

  /* Check the getters and setters */
  hdy_arrows_set_duration (arrows, 10);
  g_assert_cmpint (hdy_arrows_get_duration (arrows), ==, 10);

  hdy_arrows_set_direction (arrows, HDY_ARROWS_DIRECTION_LEFT);
  g_assert_cmpint (hdy_arrows_get_direction (arrows), ==, HDY_ARROWS_DIRECTION_LEFT);

  hdy_arrows_set_count(arrows, 5);
  g_assert_cmpint (hdy_arrows_get_count (arrows), ==, 5);

  hdy_arrows_animate (arrows);

  gtk_widget_destroy (GTK_WIDGET (arrows));
}


static void
test_hdy_arrows_gobject (void)
{
  HdyArrows *arrows;

  arrows = g_object_new (HDY_TYPE_ARROWS,
                         "duration", 10,
                         "direction", HDY_ARROWS_DIRECTION_LEFT,
                         "count", 5,
                         NULL);


  g_assert_cmpint (hdy_arrows_get_duration (arrows), ==, 10);
  g_assert_cmpint (hdy_arrows_get_direction (arrows), ==, HDY_ARROWS_DIRECTION_LEFT);
  g_assert_cmpint (hdy_arrows_get_count (arrows), ==, 5);

  hdy_arrows_animate (arrows);

  gtk_widget_destroy (GTK_WIDGET(arrows));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/Arrows/methods", test_hdy_arrows_setters);
  g_test_add_func("/Handy/Arrows/gobject", test_hdy_arrows_gobject);
  return g_test_run();
}
