/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_preferences_row_title (void)
{
  g_autoptr (AdwPreferencesRow) row = NULL;

  row = g_object_ref_sink (ADW_PREFERENCES_ROW (adw_preferences_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (adw_preferences_row_get_title (row));

  adw_preferences_row_set_title (row, "Dummy title");
  g_assert_cmpstr (adw_preferences_row_get_title (row), ==, "Dummy title");

  adw_preferences_row_set_title (row, NULL);
  g_assert_null (adw_preferences_row_get_title (row));
}


static void
test_adw_preferences_row_use_undeline (void)
{
  g_autoptr (AdwPreferencesRow) row = NULL;

  row = g_object_ref_sink (ADW_PREFERENCES_ROW (adw_preferences_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (adw_preferences_row_get_use_underline (row));

  adw_preferences_row_set_use_underline (row, TRUE);
  g_assert_true (adw_preferences_row_get_use_underline (row));

  adw_preferences_row_set_use_underline (row, FALSE);
  g_assert_false (adw_preferences_row_get_use_underline (row));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesRow/title", test_adw_preferences_row_title);
  g_test_add_func("/Adwaita/PreferencesRow/use_underline", test_adw_preferences_row_use_undeline);

  return g_test_run();
}
