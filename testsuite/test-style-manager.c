/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>
#include "adw-settings-private.h"

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

  g_signal_handlers_disconnect_by_func (manager, G_CALLBACK (notify_cb), NULL);
  adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_DEFAULT);
}

static int
test_dark (AdwStyleManager *manager,
           gboolean         initial_dark,
           int              expected_notified,
           ...)
{
  va_list args;
  AdwColorScheme scheme;
  gboolean last_dark = initial_dark;

  va_start (args, expected_notified);

  for (scheme = ADW_COLOR_SCHEME_DEFAULT; scheme <= ADW_COLOR_SCHEME_FORCE_DARK; scheme++) {
    gboolean dark = va_arg (args, gboolean);

    adw_style_manager_set_color_scheme (manager, scheme);

    if (dark)
      g_assert_true (adw_style_manager_get_dark (manager));
    else
      g_assert_false (adw_style_manager_get_dark (manager));

    if (dark != last_dark)
      expected_notified++;

    g_assert_cmpint (notified, ==, expected_notified);

    last_dark = dark;
  }

  va_end (args);

  return expected_notified;
}

static void
test_adw_style_manager_dark (void)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();
  AdwSettings *settings = adw_settings_get_default ();
  int expected_notified = 0;

  adw_settings_start_override (settings);
  adw_settings_override_system_supports_color_schemes (settings, TRUE);
  adw_settings_override_color_scheme (settings, ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT);

  notified = 0;
  g_signal_connect (manager, "notify::dark", G_CALLBACK (notify_cb), NULL);

  expected_notified = test_dark (manager, FALSE, expected_notified,
                                 FALSE, FALSE, FALSE, FALSE, TRUE);

  adw_settings_override_color_scheme (settings, ADW_SYSTEM_COLOR_SCHEME_DEFAULT);
  expected_notified = test_dark (manager, TRUE, expected_notified,
                                 FALSE, FALSE, FALSE, TRUE, TRUE);

  adw_settings_override_color_scheme (settings, ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK);
  expected_notified = test_dark (manager, TRUE, expected_notified,
                                 TRUE, FALSE, TRUE, TRUE, TRUE);

  adw_settings_end_override (settings);

  g_signal_handlers_disconnect_by_func (manager, G_CALLBACK (notify_cb), NULL);
  adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_DEFAULT);
}

static void
test_adw_style_manager_high_contrast (void)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();
  AdwSettings *settings = adw_settings_get_default ();

  adw_settings_start_override (settings);
  adw_settings_override_high_contrast (settings, FALSE);

  notified = 0;
  g_signal_connect (manager, "notify::high-contrast", G_CALLBACK (notify_cb), NULL);

  g_assert_false (adw_style_manager_get_high_contrast (manager));

  adw_settings_override_high_contrast (settings, FALSE);
  g_assert_false (adw_style_manager_get_high_contrast (manager));
  g_assert_cmpint (notified, ==, 0);

  adw_settings_override_high_contrast (settings, TRUE);
  g_assert_true (adw_style_manager_get_high_contrast (manager));
  g_assert_cmpint (notified, ==, 1);

  adw_settings_end_override (settings);

  g_signal_handlers_disconnect_by_func (manager, G_CALLBACK (notify_cb), NULL);
}

static void
test_adw_style_manager_system_supports_color_schemes (void)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();
  AdwSettings *settings = adw_settings_get_default ();

  adw_settings_start_override (settings);
  adw_settings_override_system_supports_color_schemes (settings, FALSE);

  notified = 0;
  g_signal_connect (manager, "notify::system-supports-color-schemes", G_CALLBACK (notify_cb), NULL);

  g_assert_false (adw_style_manager_get_system_supports_color_schemes (manager));

  adw_settings_override_system_supports_color_schemes (settings, FALSE);
  g_assert_false (adw_style_manager_get_system_supports_color_schemes (manager));
  g_assert_cmpint (notified, ==, 0);

  adw_settings_override_system_supports_color_schemes (settings, TRUE);
  g_assert_true (adw_style_manager_get_system_supports_color_schemes (manager));
  g_assert_cmpint (notified, ==, 1);

  adw_settings_end_override (settings);

  g_signal_handlers_disconnect_by_func (manager, G_CALLBACK (notify_cb), NULL);
}

static void
test_adw_style_manager_inheritance (void)
{
  AdwStyleManager *default_manager = adw_style_manager_get_default ();
  AdwStyleManager *display_manager = adw_style_manager_get_for_display (gdk_display_get_default ());
  AdwSettings *settings = adw_settings_get_default ();

  adw_settings_start_override (settings);
  adw_settings_override_system_supports_color_schemes (settings, TRUE);
  adw_settings_override_color_scheme (settings, ADW_SYSTEM_COLOR_SCHEME_DEFAULT);

  g_assert_cmpint (adw_style_manager_get_color_scheme (default_manager), ==, ADW_COLOR_SCHEME_DEFAULT);
  g_assert_cmpint (adw_style_manager_get_color_scheme (display_manager), ==, ADW_COLOR_SCHEME_DEFAULT);
  g_assert_false (adw_style_manager_get_dark (default_manager));
  g_assert_false (adw_style_manager_get_dark (display_manager));

  adw_style_manager_set_color_scheme (default_manager, ADW_COLOR_SCHEME_PREFER_DARK);

  g_assert_cmpint (adw_style_manager_get_color_scheme (display_manager), ==, ADW_COLOR_SCHEME_DEFAULT);
  g_assert_true (adw_style_manager_get_dark (default_manager));
  g_assert_true (adw_style_manager_get_dark (display_manager));

  adw_style_manager_set_color_scheme (display_manager, ADW_COLOR_SCHEME_PREFER_LIGHT);
  g_assert_true (adw_style_manager_get_dark (default_manager));
  g_assert_false (adw_style_manager_get_dark (display_manager));

  adw_settings_override_color_scheme (settings, ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK);
  g_assert_true (adw_style_manager_get_dark (default_manager));
  g_assert_true (adw_style_manager_get_dark (display_manager));

  adw_style_manager_set_color_scheme (default_manager, ADW_COLOR_SCHEME_FORCE_LIGHT);
  g_assert_false (adw_style_manager_get_dark (default_manager));
  g_assert_true (adw_style_manager_get_dark (display_manager));

  adw_style_manager_set_color_scheme (display_manager, ADW_COLOR_SCHEME_DEFAULT);
  g_assert_false (adw_style_manager_get_dark (default_manager));
  g_assert_false (adw_style_manager_get_dark (display_manager));

  adw_settings_end_override (settings);
  adw_style_manager_set_color_scheme (default_manager, ADW_COLOR_SCHEME_DEFAULT);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/StyleManager/color_scheme", test_adw_style_manager_color_scheme);
  g_test_add_func("/Adwaita/StyleManager/dark", test_adw_style_manager_dark);
  g_test_add_func("/Adwaita/StyleManager/high_contrast", test_adw_style_manager_high_contrast);
  g_test_add_func("/Adwaita/StyleManager/system_supports_color_schemes", test_adw_style_manager_system_supports_color_schemes);
  g_test_add_func("/Adwaita/StyleManager/inheritance", test_adw_style_manager_inheritance);

  return g_test_run();
}
