/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

static void
test_adw_button_row_start_icon_name (void)
{
  AdwButtonRow *row = g_object_ref_sink (ADW_BUTTON_ROW (adw_button_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (adw_button_row_get_start_icon_name (row), ==, "");

  adw_button_row_set_start_icon_name (row, "list-add-symbolic");
  g_assert_cmpstr (adw_button_row_get_start_icon_name (row), ==, "list-add-symbolic");

  g_assert_finalize_object (row);
}

static void
test_adw_button_row_end_icon_name (void)
{
  AdwButtonRow *row = g_object_ref_sink (ADW_BUTTON_ROW (adw_button_row_new ()));
  g_assert_nonnull (row);

  g_assert_cmpstr (adw_button_row_get_end_icon_name (row), ==, "");

  adw_button_row_set_end_icon_name (row, "list-add-symbolic");
  g_assert_cmpstr (adw_button_row_get_end_icon_name (row), ==, "list-add-symbolic");

  g_assert_finalize_object (row);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ButtonRow/start_icon_name", test_adw_button_row_start_icon_name);
  g_test_add_func("/Adwaita/ButtonRow/end_icon_name", test_adw_button_row_end_icon_name);

  return g_test_run();
}
