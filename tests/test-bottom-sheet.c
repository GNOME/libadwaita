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
test_adw_bottom_sheet_content (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::content", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "content", &widget, NULL);
  g_assert_null (widget);

  adw_bottom_sheet_set_content (sheet, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_bottom_sheet_set_content (sheet, widget);
  g_assert_true (adw_bottom_sheet_get_content (sheet) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "content", NULL, NULL);
  g_assert_null (adw_bottom_sheet_get_content (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_sheet (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::sheet", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "sheet", &widget, NULL);
  g_assert_null (widget);

  adw_bottom_sheet_set_sheet (sheet, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_bottom_sheet_set_sheet (sheet, widget);
  g_assert_true (adw_bottom_sheet_get_sheet (sheet) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "sheet", NULL, NULL);
  g_assert_null (adw_bottom_sheet_get_sheet (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_bottom_bar (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::bottom-bar", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "bottom-bar", &widget, NULL);
  g_assert_null (widget);

  adw_bottom_sheet_set_bottom_bar (sheet, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_bottom_sheet_set_bottom_bar (sheet, widget);
  g_assert_true (adw_bottom_sheet_get_bottom_bar (sheet) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "bottom-bar", NULL, NULL);
  g_assert_null (adw_bottom_sheet_get_bottom_bar (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_open (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean open;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::open", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "open", &open, NULL);
  g_assert_false (open);

  adw_bottom_sheet_set_open (sheet, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_open (sheet, TRUE);
  g_assert_true (adw_bottom_sheet_get_open (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "open", FALSE, NULL);
  g_assert_false (adw_bottom_sheet_get_open (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_align (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  float align;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::align", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "align", &align, NULL);
  g_assert_cmpfloat_with_epsilon (align, 0.5, 0.005);

  adw_bottom_sheet_set_align (sheet, 0.5);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_align (sheet, 1);
  g_assert_cmpfloat_with_epsilon (adw_bottom_sheet_get_align (sheet), 1, 0.005);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "align", 0.0f, NULL);
  g_assert_cmpfloat_with_epsilon (adw_bottom_sheet_get_align (sheet), 0, 0.005);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_full_width (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean full_width;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::full-width", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "full-width", &full_width, NULL);
  g_assert_true (full_width);

  adw_bottom_sheet_set_full_width (sheet, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_full_width (sheet, FALSE);
  g_assert_false (adw_bottom_sheet_get_full_width (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "full-width", TRUE, NULL);
  g_assert_true (adw_bottom_sheet_get_full_width (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_show_drag_handle (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean show_drag_handle;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::show-drag-handle", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "show-drag-handle", &show_drag_handle, NULL);
  g_assert_true (show_drag_handle);

  adw_bottom_sheet_set_show_drag_handle (sheet, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_show_drag_handle (sheet, FALSE);
  g_assert_false (adw_bottom_sheet_get_show_drag_handle (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "show-drag-handle", TRUE, NULL);
  g_assert_true (adw_bottom_sheet_get_show_drag_handle (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_modal (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean modal;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::modal", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "modal", &modal, NULL);
  g_assert_true (modal);

  adw_bottom_sheet_set_modal (sheet, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_modal (sheet, FALSE);
  g_assert_false (adw_bottom_sheet_get_modal (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "modal", TRUE, NULL);
  g_assert_true (adw_bottom_sheet_get_modal (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_can_open (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean can_open;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::can-open", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "can-open", &can_open, NULL);
  g_assert_true (can_open);

  adw_bottom_sheet_set_can_open (sheet, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_can_open (sheet, FALSE);
  g_assert_false (adw_bottom_sheet_get_can_open (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "can-open", TRUE, NULL);
  g_assert_true (adw_bottom_sheet_get_can_open (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_can_close (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean can_close;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::can-close", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "can-close", &can_close, NULL);
  g_assert_true (can_close);

  adw_bottom_sheet_set_can_close (sheet, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_can_close (sheet, FALSE);
  g_assert_false (adw_bottom_sheet_get_can_close (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "can-close", TRUE, NULL);
  g_assert_true (adw_bottom_sheet_get_can_close (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

static void
test_adw_bottom_sheet_reveal_bottom_bar (void)
{
  AdwBottomSheet *sheet = ADW_BOTTOM_SHEET (g_object_ref_sink (adw_bottom_sheet_new ()));
  gboolean reveal;
  int notified = 0;

  g_assert_nonnull (sheet);

  g_signal_connect_swapped (sheet, "notify::reveal-bottom-bar", G_CALLBACK (increment), &notified);

  g_object_get (sheet, "reveal-bottom-bar", &reveal, NULL);
  g_assert_true (reveal);

  adw_bottom_sheet_set_reveal_bottom_bar (sheet, TRUE);
  g_assert_cmpint (notified, ==, 0);

  adw_bottom_sheet_set_reveal_bottom_bar (sheet, FALSE);
  g_assert_false (adw_bottom_sheet_get_reveal_bottom_bar (sheet));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sheet, "reveal-bottom-bar", TRUE, NULL);
  g_assert_true (adw_bottom_sheet_get_reveal_bottom_bar (sheet));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sheet);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/BottomSheet/content", test_adw_bottom_sheet_content);
  g_test_add_func ("/Adwaita/BottomSheet/sheet", test_adw_bottom_sheet_sheet);
  g_test_add_func ("/Adwaita/BottomSheet/bottom_bar", test_adw_bottom_sheet_bottom_bar);
  g_test_add_func ("/Adwaita/BottomSheet/open", test_adw_bottom_sheet_open);
  g_test_add_func ("/Adwaita/BottomSheet/align", test_adw_bottom_sheet_align);
  g_test_add_func ("/Adwaita/BottomSheet/full_width", test_adw_bottom_sheet_full_width);
  g_test_add_func ("/Adwaita/BottomSheet/show_drag_handle", test_adw_bottom_sheet_show_drag_handle);
  g_test_add_func ("/Adwaita/BottomSheet/modal", test_adw_bottom_sheet_modal);
  g_test_add_func ("/Adwaita/BottomSheet/can_open", test_adw_bottom_sheet_can_open);
  g_test_add_func ("/Adwaita/BottomSheet/can_close", test_adw_bottom_sheet_can_close);
  g_test_add_func ("/Adwaita/BottomSheet/reveal_bottom_bar", test_adw_bottom_sheet_reveal_bottom_bar);

  return g_test_run ();
}
