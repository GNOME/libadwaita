/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_entry_row_add_remove (void)
{
  AdwEntryRow *row = g_object_ref_sink (ADW_ENTRY_ROW (adw_entry_row_new ()));
  GtkWidget *prefix, *suffix;

  g_assert_nonnull (row);

  prefix = gtk_check_button_new ();
  g_assert_nonnull (prefix);

  suffix = gtk_check_button_new ();
  g_assert_nonnull (suffix);

  adw_entry_row_add_prefix (row, prefix);
  adw_entry_row_add_suffix (row, suffix);

  adw_entry_row_remove (row, prefix);
  adw_entry_row_remove (row, suffix);

  g_assert_finalize_object (row);
}

static void
test_adw_entry_row_show_apply_button (void)
{
  AdwEntryRow *row = g_object_ref_sink (ADW_ENTRY_ROW (adw_entry_row_new ()));
  gboolean show_apply_button;
  int notified = 0;

  g_assert_nonnull (row);

  g_signal_connect_swapped (row, "notify::show-apply-button", G_CALLBACK (increment), &notified);

  g_object_get (row, "show-apply-button", &show_apply_button, NULL);
  g_assert_false (show_apply_button);

  adw_entry_row_set_show_apply_button (row, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_entry_row_set_show_apply_button (row, TRUE);
  g_assert_true (adw_entry_row_get_show_apply_button (row));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (row, "show-apply-button", FALSE, NULL);
  g_assert_false (adw_entry_row_get_show_apply_button (row));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (row);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/EntryRow/add_remove", test_adw_entry_row_add_remove);
  g_test_add_func("/Adwaita/EntryRow/show_apply_button", test_adw_entry_row_show_apply_button);

  return g_test_run();
}
