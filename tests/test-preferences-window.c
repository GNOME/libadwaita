/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_preferences_window_add_remove (void)
{
  g_autoptr (HdyPreferencesWindow) window = NULL;
  HdyPreferencesPage *page;

  window = g_object_ref_sink (HDY_PREFERENCES_WINDOW (hdy_preferences_window_new ()));
  g_assert_nonnull (window);

  page = HDY_PREFERENCES_PAGE (hdy_preferences_page_new ());
  g_assert_nonnull (page);
  hdy_preferences_window_add (window, page);

  hdy_preferences_window_remove (window, page);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/PreferencesWindow/add_remove", test_hdy_preferences_window_add_remove);

  return g_test_run();
}
