/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_breakpoint_bin_child (void)
{
  AdwBreakpointBin *bin = g_object_ref_sink (ADW_BREAKPOINT_BIN (adw_breakpoint_bin_new ()));
  GtkWidget *widget = NULL;

  g_assert_nonnull (bin);

  notified = 0;
  g_signal_connect (bin, "notify::child", G_CALLBACK (notify_cb), NULL);

  g_object_get (bin, "child", &widget, NULL);
  g_assert_null (widget);

  adw_breakpoint_bin_set_child (bin, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_breakpoint_bin_set_child (bin, widget);
  g_assert_true (adw_breakpoint_bin_get_child (bin) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bin, "child", NULL, NULL);
  g_assert_null (adw_breakpoint_bin_get_child (bin));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (bin);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/BreakpointBin/child", test_adw_breakpoint_bin_child);

  return g_test_run ();
}
