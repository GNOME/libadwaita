/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_action_row_add_remove (void)
{
  AdwActionRow *row = g_object_ref_sink (ADW_ACTION_ROW (adw_action_row_new ()));
  GtkWidget *prefix, *suffix;

  g_assert_nonnull (row);

  prefix = gtk_check_button_new ();
  g_assert_nonnull (prefix);

  suffix = gtk_check_button_new ();
  g_assert_nonnull (suffix);

  adw_action_row_add_prefix (row, prefix);
  adw_action_row_add_suffix (row, suffix);

  adw_action_row_remove (row, prefix);
  adw_action_row_remove (row, suffix);

  g_assert_finalize_object (row);
}


static void
test_adw_action_row_subtitle (void)
{
  AdwActionRow *row = g_object_ref_sink (ADW_ACTION_ROW (adw_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (adw_action_row_get_subtitle (row), ==, "");

  adw_action_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (adw_action_row_get_subtitle (row), ==, "Dummy subtitle");

  adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);
  adw_action_row_set_subtitle (row, "Invalid <b>markup");
  g_assert_cmpstr (adw_action_row_get_subtitle (row), ==, "Invalid <b>markup");

  g_assert_finalize_object (row);
}


static void
test_adw_action_row_title_lines (void)
{
  AdwActionRow *row = g_object_ref_sink (ADW_ACTION_ROW (adw_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adw_action_row_get_title_lines (row), ==, 0);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adw_action_row_set_title_lines: assertion 'title_lines >= 0' failed");
  adw_action_row_set_title_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adw_action_row_get_title_lines (row), ==, 0);

  adw_action_row_set_title_lines (row, 1);
  g_assert_cmpint (adw_action_row_get_title_lines (row), ==, 1);

  g_assert_finalize_object (row);
}


static void
test_adw_action_row_subtitle_lines (void)
{
  AdwActionRow *row = g_object_ref_sink (ADW_ACTION_ROW (adw_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (adw_action_row_get_subtitle_lines (row), ==, 0);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "adw_action_row_set_subtitle_lines: assertion 'subtitle_lines >= 0' failed");
  adw_action_row_set_subtitle_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (adw_action_row_get_subtitle_lines (row), ==, 0);

  adw_action_row_set_subtitle_lines (row, 1);
  g_assert_cmpint (adw_action_row_get_subtitle_lines (row), ==, 1);

  g_assert_finalize_object (row);
}

static void
test_adw_action_row_subtitle_selectable (void)
{
  AdwActionRow *row = g_object_ref_sink (ADW_ACTION_ROW (adw_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (adw_action_row_get_subtitle_selectable (row));

  adw_action_row_set_subtitle_selectable (row, TRUE);
  g_assert_true (adw_action_row_get_subtitle_selectable (row));

  adw_action_row_set_subtitle_selectable (row, FALSE);
  g_assert_false (adw_action_row_get_subtitle_selectable (row));

  g_assert_finalize_object (row);
}


static void
test_adw_action_row_activate (void)
{
  AdwActionRow *row = g_object_ref_sink (ADW_ACTION_ROW (adw_action_row_new ()));
  int activated = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "activated", G_CALLBACK (increment), &activated);

  adw_action_row_activate (row);
  g_assert_cmpint (activated, ==, 1);

  g_assert_finalize_object (row);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ActionRow/add_remove", test_adw_action_row_add_remove);
  g_test_add_func("/Adwaita/ActionRow/subtitle", test_adw_action_row_subtitle);
  g_test_add_func("/Adwaita/ActionRow/title_lines", test_adw_action_row_title_lines);
  g_test_add_func("/Adwaita/ActionRow/subtitle_lines", test_adw_action_row_subtitle_lines);
  g_test_add_func("/Adwaita/ActionRow/subtitle_selectable", test_adw_action_row_subtitle_selectable);
  g_test_add_func("/Adwaita/ActionRow/activate", test_adw_action_row_activate);

  return g_test_run();
}
