/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

gint notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_bin_child (void)
{
  g_autoptr (AdwBin) bin = NULL;
  GtkWidget *widget = NULL;

  bin = g_object_ref_sink (ADW_BIN (adw_bin_new ()));
  g_assert_nonnull (bin);

  notified = 0;
  g_signal_connect (bin, "notify::child", G_CALLBACK (notify_cb), NULL);

  g_object_get (bin, "child", &widget, NULL);
  g_assert_null (widget);

  adw_bin_set_child (bin, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_bin_set_child (bin, widget);
  g_assert_true (adw_bin_get_child (bin) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (bin, "child", NULL, NULL);
  g_assert_null (adw_bin_get_child (bin));
  g_assert_cmpint (notified, ==, 2);
}

gint
main (gint argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Bin/child", test_adw_bin_child);

  return g_test_run ();
}
