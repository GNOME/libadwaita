/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_expander_row_add_remove (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;
  GtkWidget *child;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  child = gtk_list_box_row_new ();
  g_assert_nonnull (child);

  adw_expander_row_add (row, child);
  adw_expander_row_remove (row, child);
}


static void
test_adw_expander_row_subtitle (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (adw_expander_row_get_subtitle (row), ==, "");

  adw_expander_row_set_subtitle (row, "Dummy subtitle");
  g_assert_cmpstr (adw_expander_row_get_subtitle (row), ==, "Dummy subtitle");
}


static void
test_adw_expander_row_icon_name (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (adw_expander_row_get_icon_name (row));

  adw_expander_row_set_icon_name (row, "dummy-icon-name");
  g_assert_cmpstr (adw_expander_row_get_icon_name (row), ==, "dummy-icon-name");
}


static void
test_adw_expander_row_use_undeline (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (adw_expander_row_get_use_underline (row));

  adw_expander_row_set_use_underline (row, TRUE);
  g_assert_true (adw_expander_row_get_use_underline (row));

  adw_expander_row_set_use_underline (row, FALSE);
  g_assert_false (adw_expander_row_get_use_underline (row));
}


static void
test_adw_expander_row_expanded (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (adw_expander_row_get_expanded (row));

  adw_expander_row_set_expanded (row, TRUE);
  g_assert_true (adw_expander_row_get_expanded (row));

  adw_expander_row_set_expanded (row, FALSE);
  g_assert_false (adw_expander_row_get_expanded (row));
}


static void
test_adw_expander_row_enable_expansion (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
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
}


static void
test_adw_expander_row_show_enable_switch (void)
{
  g_autoptr (AdwExpanderRow) row = NULL;

  row = g_object_ref_sink (ADW_EXPANDER_ROW (adw_expander_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (adw_expander_row_get_show_enable_switch (row));

  adw_expander_row_set_show_enable_switch (row, TRUE);
  g_assert_true (adw_expander_row_get_show_enable_switch (row));

  adw_expander_row_set_show_enable_switch (row, FALSE);
  g_assert_false (adw_expander_row_get_show_enable_switch (row));
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ExpanderRow/add_remove", test_adw_expander_row_add_remove);
  g_test_add_func("/Adwaita/ExpanderRow/subtitle", test_adw_expander_row_subtitle);
  g_test_add_func("/Adwaita/ExpanderRow/icon_name", test_adw_expander_row_icon_name);
  g_test_add_func("/Adwaita/ExpanderRow/use_underline", test_adw_expander_row_use_undeline);
  g_test_add_func("/Adwaita/ExpanderRow/expanded", test_adw_expander_row_expanded);
  g_test_add_func("/Adwaita/ExpanderRow/enable_expansion", test_adw_expander_row_enable_expansion);
  g_test_add_func("/Adwaita/ExpanderRow/show_enable_switch", test_adw_expander_row_show_enable_switch);

  return g_test_run();
}
