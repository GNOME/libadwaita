/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
test_adw_expander_row_add_remove (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  GtkWidget *child;

  g_assert_nonnull (row);

  child = gtk_list_box_row_new ();
  g_assert_nonnull (child);

  adw_expander_row_add_row (row, child);
  adw_expander_row_remove (row, child);

  g_assert_finalize_object (row);
}


static void
test_adw_expander_row_subtitle (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_cmpstr (adw_expander_row_get_subtitle (row), ==, "");

  adw_expander_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (adw_expander_row_get_subtitle (row), ==, "Dummy subtitle");

  adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);
  adw_expander_row_set_subtitle (row, "Invalid <b>markup");
  g_assert_cmpstr (adw_expander_row_get_subtitle (row), ==, "Invalid <b>markup");

  g_assert_finalize_object (row);
}


static void
test_adw_expander_row_expanded (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_false (adw_expander_row_get_expanded (row));

  adw_expander_row_set_expanded (row, TRUE);
  g_assert_true (adw_expander_row_get_expanded (row));

  adw_expander_row_set_expanded (row, FALSE);
  g_assert_false (adw_expander_row_get_expanded (row));

  g_assert_finalize_object (row);
}


static void
test_adw_expander_row_enable_expansion (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_true (adw_expander_row_get_enable_expansion (row));
  g_assert_false (adw_expander_row_get_expanded (row));

  adw_expander_row_set_expanded (row, TRUE);
  g_assert_true (adw_expander_row_get_expanded (row));

  adw_expander_row_set_enable_expansion (row, FALSE);
  g_assert_false (adw_expander_row_get_enable_expansion (row));
  g_assert_false (adw_expander_row_get_expanded (row));

  adw_expander_row_set_expanded (row, TRUE);
  g_assert_false (adw_expander_row_get_expanded (row));

  adw_expander_row_set_enable_expansion (row, TRUE);
  g_assert_true (adw_expander_row_get_enable_expansion (row));
  g_assert_true (adw_expander_row_get_expanded (row));

  g_assert_finalize_object (row);
}


static void
test_adw_expander_row_show_enable_switch (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));

  g_assert_nonnull (row);

  g_assert_false (adw_expander_row_get_show_enable_switch (row));

  adw_expander_row_set_show_enable_switch (row, TRUE);
  g_assert_true (adw_expander_row_get_show_enable_switch (row));

  adw_expander_row_set_show_enable_switch (row, FALSE);
  g_assert_false (adw_expander_row_get_show_enable_switch (row));

  g_assert_finalize_object (row);
}

static void
test_adw_expander_row_title_lines (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adw_expander_row_get_title_lines (row), ==, 0);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adw_action_row_set_title_lines: assertion 'title_lines >= 0' failed");
  adw_expander_row_set_title_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adw_expander_row_get_title_lines (row), ==, 0);

  adw_expander_row_set_title_lines (row, 1);
  g_assert_cmpint (adw_expander_row_get_title_lines (row), ==, 1);

  g_assert_finalize_object (row);
}


static void
test_adw_expander_row_subtitle_lines (void)
{
  AdwExpanderRow *row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adw_expander_row_get_subtitle_lines (row), ==, 0);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adw_action_row_set_subtitle_lines: assertion 'subtitle_lines >= 0' failed");
  adw_expander_row_set_subtitle_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adw_expander_row_get_subtitle_lines (row), ==, 0);

  adw_expander_row_set_subtitle_lines (row, 1);
  g_assert_cmpint (adw_expander_row_get_subtitle_lines (row), ==, 1);

  g_assert_finalize_object (row);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ExpanderRow/add_remove", test_adw_expander_row_add_remove);
  g_test_add_func("/Adwaita/ExpanderRow/subtitle", test_adw_expander_row_subtitle);
  g_test_add_func("/Adwaita/ExpanderRow/expanded", test_adw_expander_row_expanded);
  g_test_add_func("/Adwaita/ExpanderRow/enable_expansion", test_adw_expander_row_enable_expansion);
  g_test_add_func("/Adwaita/ExpanderRow/show_enable_switch", test_adw_expander_row_show_enable_switch);
  g_test_add_func("/Adwaita/ExpanderRow/title_lines", test_adw_expander_row_title_lines);
  g_test_add_func("/Adwaita/ExpanderRow/subtitle_lines", test_adw_expander_row_subtitle_lines);

  return g_test_run();
}
