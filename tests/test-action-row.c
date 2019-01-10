/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


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
test_hdy_action_row_add_action (void)
{
  g_autoptr (HdyActionRow) row = NULL;
  GtkWidget *sw;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  sw = gtk_switch_new ();
  g_assert_nonnull (sw);

  hdy_action_row_add_action (row, sw);
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
test_hdy_action_row_title (void)
{
  g_autoptr (HdyActionRow) row = NULL;

  row = g_object_ref_sink (HDY_ACTION_ROW (hdy_action_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (hdy_action_row_get_title (row), ==, "");

  hdy_action_row_set_title (row, "Dummy title");
  g_assert_cmpstr (hdy_action_row_get_title (row), ==, "Dummy title");
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
test_hdy_action_row_use_undeline (void)
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


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/ActionRow/add", test_hdy_action_row_add);
  g_test_add_func("/Handy/ActionRow/add_action", test_hdy_action_row_add_action);
  g_test_add_func("/Handy/ActionRow/add_prefix", test_hdy_action_row_add_prefix);
  g_test_add_func("/Handy/ActionRow/title", test_hdy_action_row_title);
  g_test_add_func("/Handy/ActionRow/subtitle", test_hdy_action_row_subtitle);
  g_test_add_func("/Handy/ActionRow/icon_name", test_hdy_action_row_icon_name);
  g_test_add_func("/Handy/ActionRow/use_underline", test_hdy_action_row_use_undeline);

  return g_test_run();
}
