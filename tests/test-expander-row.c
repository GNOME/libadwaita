/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_expander_row_add (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;
  GtkWidget *sw;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  sw = gtk_switch_new ();
  g_assert_nonnull (sw);

  gtk_container_add (GTK_CONTAINER (row), sw);
}


static void
test_hdy_expander_row_subtitle (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (hdy_expander_row_get_subtitle (row), ==, "");

  hdy_expander_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (hdy_expander_row_get_subtitle (row), ==, "Dummy subtitle");
}


static void
test_hdy_expander_row_icon_name (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (hdy_expander_row_get_icon_name (row));

  hdy_expander_row_set_icon_name (row, "dummy-icon-name");
  g_assert_cmpstr (hdy_expander_row_get_icon_name (row), ==, "dummy-icon-name");
}


static void
test_hdy_expander_row_use_undeline (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (hdy_expander_row_get_use_underline (row));

  hdy_expander_row_set_use_underline (row, TRUE);
  g_assert_true (hdy_expander_row_get_use_underline (row));

  hdy_expander_row_set_use_underline (row, FALSE);
  g_assert_false (hdy_expander_row_get_use_underline (row));
}


static void
test_hdy_expander_row_expanded (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (hdy_expander_row_get_expanded (row));

  hdy_expander_row_set_expanded (row, TRUE);
  g_assert_true (hdy_expander_row_get_expanded (row));

  hdy_expander_row_set_expanded (row, FALSE);
  g_assert_false (hdy_expander_row_get_expanded (row));
}


static void
test_hdy_expander_row_enable_expansion (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_true (hdy_expander_row_get_enable_expansion (row));
  g_assert_false (hdy_expander_row_get_expanded (row));

  hdy_expander_row_set_expanded (row, TRUE);
  g_assert_true (hdy_expander_row_get_expanded (row));

  hdy_expander_row_set_enable_expansion (row, FALSE);
  g_assert_false (hdy_expander_row_get_enable_expansion (row));
  g_assert_false (hdy_expander_row_get_expanded (row));

  hdy_expander_row_set_expanded (row, TRUE);
  g_assert_false (hdy_expander_row_get_expanded (row));

  hdy_expander_row_set_enable_expansion (row, TRUE);
  g_assert_true (hdy_expander_row_get_enable_expansion (row));
  g_assert_true (hdy_expander_row_get_expanded (row));
}


static void
test_hdy_expander_row_show_enable_switch (void)
{
  g_autoptr (HdyExpanderRow) row = NULL;

  row = g_object_ref_sink (HDY_EXPANDER_ROW (hdy_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (hdy_expander_row_get_show_enable_switch (row));

  hdy_expander_row_set_show_enable_switch (row, TRUE);
  g_assert_true (hdy_expander_row_get_show_enable_switch (row));

  hdy_expander_row_set_show_enable_switch (row, FALSE);
  g_assert_false (hdy_expander_row_get_show_enable_switch (row));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/ExpanderRow/add", test_hdy_expander_row_add);
  g_test_add_func("/Handy/ExpanderRow/subtitle", test_hdy_expander_row_subtitle);
  g_test_add_func("/Handy/ExpanderRow/icon_name", test_hdy_expander_row_icon_name);
  g_test_add_func("/Handy/ExpanderRow/use_underline", test_hdy_expander_row_use_undeline);
  g_test_add_func("/Handy/ExpanderRow/expanded", test_hdy_expander_row_expanded);
  g_test_add_func("/Handy/ExpanderRow/enable_expansion", test_hdy_expander_row_enable_expansion);
  g_test_add_func("/Handy/ExpanderRow/show_enable_switch", test_hdy_expander_row_show_enable_switch);

  return g_test_run();
}
