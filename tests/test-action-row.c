/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>

gint activated;

static void
activated_cb (GtkWidget *widget, gpointer data)
{
  activated++;
}


static void
test_hdy_action_row_add (void)
{
  g_autoptr (HdyActionRow) row = NULL;
  GtkWidget *sw;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  sw = gtk_switch_new ();
  g_assert_nonnull (sw);

  gtk_container_add (GTK_CONTAINER (row), sw);
}


static void
test_hdy_action_row_add_prefix (void)
{
  g_autoptr (HdyActionRow) row = NULL;
  GtkWidget *radio;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  radio = gtk_radio_button_new (NULL);
  g_assert_nonnull (radio);

  hdy_action_row_add_prefix (row, radio);
}


static void
test_hdy_action_row_subtitle (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (hdy_action_row_get_subtitle (row), ==, "");

  hdy_action_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (hdy_action_row_get_subtitle (row), ==, "Dummy subtitle");
}


static void
test_hdy_action_row_icon_name (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (hdy_action_row_get_icon_name (row));

  hdy_action_row_set_icon_name (row, "dummy-icon-name");
  g_assert_cmpstr (hdy_action_row_get_icon_name (row), ==, "dummy-icon-name");
}


static void
test_hdy_action_row_use_underline (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (hdy_action_row_get_use_underline (row));

  hdy_action_row_set_use_underline (row, TRUE);
  g_assert_true (hdy_action_row_get_use_underline (row));

  hdy_action_row_set_use_underline (row, FALSE);
  g_assert_false (hdy_action_row_get_use_underline (row));
}


static void
test_hdy_action_row_title_lines (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (hdy_action_row_get_title_lines (row), ==, 1);

  g_test_expect_message (HDY_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "hdy_action_row_set_title_lines: assertion 'title_lines >= 0' failed");
  hdy_action_row_set_title_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (hdy_action_row_get_title_lines (row), ==, 1);

  hdy_action_row_set_title_lines (row, 0);
  g_assert_cmpint (hdy_action_row_get_title_lines (row), ==, 0);
}


static void
test_hdy_action_row_subtitle_lines (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpint (hdy_action_row_get_subtitle_lines (row), ==, 1);

  g_test_expect_message (HDY_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "hdy_action_row_set_subtitle_lines: assertion 'subtitle_lines >= 0' failed");
  hdy_action_row_set_subtitle_lines (row, -1);
  g_test_assert_expected_messages ();

  g_assert_cmpint (hdy_action_row_get_subtitle_lines (row), ==, 1);

  hdy_action_row_set_subtitle_lines (row, 0);
  g_assert_cmpint (hdy_action_row_get_subtitle_lines (row), ==, 0);
}


static void
test_hdy_action_row_activate (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  activated = 0;
  g_signal_connect (row, "activated", G_CALLBACK (activated_cb), NULL);

  hdy_action_row_activate (row);
  g_assert_cmpint (activated, ==, 1);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/ActionRow/add", test_hdy_action_row_add);
  g_test_add_func("/Handy/ActionRow/add_prefix", test_hdy_action_row_add_prefix);
  g_test_add_func("/Handy/ActionRow/subtitle", test_hdy_action_row_subtitle);
  g_test_add_func("/Handy/ActionRow/icon_name", test_hdy_action_row_icon_name);
  g_test_add_func("/Handy/ActionRow/use_underline", test_hdy_action_row_use_underline);
  g_test_add_func("/Handy/ActionRow/title_lines", test_hdy_action_row_title_lines);
  g_test_add_func("/Handy/ActionRow/subtitle_lines", test_hdy_action_row_subtitle_lines);
  g_test_add_func("/Handy/ActionRow/activate", test_hdy_action_row_activate);

  return g_test_run();
}
