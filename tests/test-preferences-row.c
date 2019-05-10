/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


static void
test_hdy_preferences_row_title (void)
{
  g_autoptr (HdyPreferencesRow) row = NULL;

  row = g_object_ref_sink (HDY_PREFERENCES_ROW (hdy_preferences_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (hdy_preferences_row_get_title (row));

  hdy_preferences_row_set_title (row, "Dummy title");
  g_assert_cmpstr (hdy_preferences_row_get_title (row), ==, "Dummy title");

  hdy_preferences_row_set_title (row, NULL);
  g_assert_null (hdy_preferences_row_get_title (row));
}


static void
test_hdy_preferences_row_use_undeline (void)
{
  g_autoptr (HdyPreferencesRow) row = NULL;

  row = g_object_ref_sink (HDY_PREFERENCES_ROW (hdy_preferences_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (hdy_preferences_row_get_use_underline (row));

  hdy_preferences_row_set_use_underline (row, TRUE);
  g_assert_true (hdy_preferences_row_get_use_underline (row));

  hdy_preferences_row_set_use_underline (row, FALSE);
  g_assert_false (hdy_preferences_row_get_use_underline (row));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/PreferencesRow/title", test_hdy_preferences_row_title);
  g_test_add_func("/Handy/PreferencesRow/use_underline", test_hdy_preferences_row_use_undeline);

  return g_test_run();
}
