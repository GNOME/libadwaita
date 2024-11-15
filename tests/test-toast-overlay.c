/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
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
test_adw_toast_overlay_child (void)
{
  AdwToastOverlay *toast_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (toast_overlay);

  g_signal_connect_swapped (toast_overlay, "notify::child", G_CALLBACK (increment), &notified);

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
  adw_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));
  adw_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));

  g_assert_finalize_object (toast_overlay);
  g_assert_finalize_object (toast);
}

static void
test_adw_toast_overlay_dismiss_all (void)
{
  AdwToastOverlay *toast_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));
  AdwToast *toast = adw_toast_new ("Test Notification");

  g_assert_nonnull (toast_overlay);
  g_assert_nonnull (toast);

  adw_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));

  adw_toast_overlay_dismiss_all (toast_overlay);

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
  g_test_add_func ("/Adwaita/ToastOverlay/dismiss_all", test_adw_toast_overlay_dismiss_all);

  return g_test_run ();
}
