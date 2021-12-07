/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_preferences_window_add_remove (void)
{
  AdwPreferencesWindow *window = ADW_PREFERENCES_WINDOW (adw_preferences_window_new ());
  AdwPreferencesPage *page;

  g_assert_nonnull (window);

  page = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
  g_assert_nonnull (page);
  adw_preferences_window_add (window, page);

  adw_preferences_window_remove (window, page);

  g_assert_finalize_object (window);
}

static void
test_adw_preferences_window_add_toast (void)
{
  AdwPreferencesWindow *window = ADW_PREFERENCES_WINDOW (adw_preferences_window_new ());
  AdwToast *toast = adw_toast_new ("Test Notification");

  g_assert_nonnull (window);
  g_assert_nonnull (toast);

  adw_preferences_window_add_toast (window, g_object_ref (toast));

  g_assert_finalize_object (window);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesWindow/add_remove", test_adw_preferences_window_add_remove);
  g_test_add_func("/Adwaita/PreferencesWindow/add_toast", test_adw_preferences_window_add_toast);

  return g_test_run();
}
