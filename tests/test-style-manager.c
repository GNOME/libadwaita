/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_style_manager_color_scheme (void)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();
  AdwColorScheme color_scheme;

  notified = 0;
  g_signal_connect (manager, "notify::color-scheme", G_CALLBACK (notify_cb), NULL);

  g_object_get (manager, "color-scheme", &color_scheme, NULL);
  g_assert_cmpint (color_scheme, ==, ADW_COLOR_SCHEME_DEFAULT);
  g_assert_cmpint (notified, ==, 0);

  adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_DEFAULT);
  g_assert_cmpint (notified, ==, 0);

  adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_PREFER_DARK);
  g_object_get (manager, "color-scheme", &color_scheme, NULL);
  g_assert_cmpint (color_scheme, ==, ADW_COLOR_SCHEME_PREFER_DARK);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (manager, "color-scheme", ADW_COLOR_SCHEME_PREFER_LIGHT, NULL);
  g_assert_cmpint (adw_style_manager_get_color_scheme (manager), ==, ADW_COLOR_SCHEME_PREFER_LIGHT);
  g_assert_cmpint (notified, ==, 2);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/StyleManager/color_scheme", test_adw_style_manager_color_scheme);

  return g_test_run();
}
