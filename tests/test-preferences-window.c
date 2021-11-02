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


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesWindow/add_remove", test_adw_preferences_window_add_remove);

  return g_test_run();
}
