/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_toast_overlay_child (void)
{
  AdwToastOverlay *toast_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));
  GtkWidget *widget = NULL;

  g_assert_nonnull (toast_overlay);

  notified = 0;
  g_signal_connect (toast_overlay, "notify::child", G_CALLBACK (notify_cb), NULL);

  g_object_get (toast_overlay, "child", &widget, NULL);
  g_assert_null (widget);

  adw_toast_overlay_set_child (toast_overlay, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_toast_overlay_set_child (toast_overlay, widget);
  g_assert_true (adw_toast_overlay_get_child (toast_overlay) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast_overlay, "child", NULL, NULL);
  g_assert_null (adw_toast_overlay_get_child (toast_overlay));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast_overlay);
}

static void
test_adw_toast_overlay_add_toast (void)
{
  AdwToastOverlay *toast_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));
  AdwToast *toast = adw_toast_new ("Test Notification");

  g_assert_nonnull (toast_overlay);
  g_assert_nonnull (toast);

  adw_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));

  g_assert_finalize_object (toast_overlay);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/ToastOverlay/child", test_adw_toast_overlay_child);
  g_test_add_func ("/Adwaita/ToastOverlay/add_toast", test_adw_toast_overlay_add_toast);

  return g_test_run ();
}
