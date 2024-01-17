/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_dialog_child (void)
{
  AdwDialog *dialog = g_object_ref_sink (adw_dialog_new ());
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "child", &widget, NULL);
  g_assert_null (widget);

  adw_dialog_set_child (dialog, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_dialog_set_child (dialog, widget);
  g_assert_true (adw_dialog_get_child (dialog) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "child", NULL, NULL);
  g_assert_null (adw_dialog_get_child (dialog));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

static void
test_adw_dialog_title (void)
{
  AdwDialog *dialog = g_object_ref_sink (adw_dialog_new ());
  char *title;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");

  adw_dialog_set_title (dialog, "Title");
  g_assert_cmpstr (adw_dialog_get_title (dialog), ==, "Title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "title", "Title 2", NULL);
  g_assert_cmpstr (adw_dialog_get_title (dialog), ==, "Title 2");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (dialog);
}

static void
test_adw_dialog_can_close (void)
{
  AdwDialog *dialog = g_object_ref_sink (adw_dialog_new ());
  gboolean can_close;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::can-close", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "can-close", &can_close, NULL);
  g_assert_true (can_close);

  adw_dialog_set_can_close (dialog, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_dialog_set_can_close (dialog, FALSE);
  g_assert_false (adw_dialog_get_can_close (dialog));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "can-close", TRUE, NULL);
  g_assert_true (adw_dialog_get_can_close (dialog));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

static void
test_adw_dialog_follows_content_size (void)
{
  AdwDialog *dialog = g_object_ref_sink (adw_dialog_new ());
  gboolean follows_content_size;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::follows-content-size", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "follows-content-size", &follows_content_size, NULL);
  g_assert_false (follows_content_size);

  adw_dialog_set_follows_content_size (dialog, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_dialog_set_follows_content_size (dialog, TRUE);
  g_assert_true (adw_dialog_get_follows_content_size (dialog));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "follows-content-size", FALSE, NULL);
  g_assert_false (adw_dialog_get_follows_content_size (dialog));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

static void
test_adw_dialog_presentation_mode (void)
{
  AdwDialog *dialog = g_object_ref_sink (adw_dialog_new ());
  AdwDialogPresentationMode presentation_mode;
  int notified = 0;

  g_assert_nonnull (dialog);

  g_signal_connect_swapped (dialog, "notify::presentation-mode", G_CALLBACK (increment), &notified);

  g_object_get (dialog, "presentation-mode", &presentation_mode, NULL);
  g_assert_cmpint (presentation_mode, ==, ADW_DIALOG_AUTO);

  adw_dialog_set_presentation_mode (dialog, ADW_DIALOG_AUTO);
  g_assert_cmpint (notified, ==, 0);

  adw_dialog_set_presentation_mode (dialog, ADW_DIALOG_FLOATING);
  g_assert_cmpint (adw_dialog_get_presentation_mode (dialog), ==, ADW_DIALOG_FLOATING);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (dialog, "presentation-mode", ADW_DIALOG_BOTTOM_SHEET, NULL);
  g_assert_cmpint (adw_dialog_get_presentation_mode (dialog), ==, ADW_DIALOG_BOTTOM_SHEET);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (dialog);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Dialog/child", test_adw_dialog_child);
  g_test_add_func ("/Adwaita/Dialog/title", test_adw_dialog_title);
  g_test_add_func ("/Adwaita/Dialog/can-close", test_adw_dialog_can_close);
  g_test_add_func ("/Adwaita/Dialog/follows-content-size", test_adw_dialog_follows_content_size);
  g_test_add_func ("/Adwaita/Dialog/presentation-mode", test_adw_dialog_presentation_mode);

  return g_test_run ();
}
