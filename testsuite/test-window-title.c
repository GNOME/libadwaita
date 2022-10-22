/*
 * Copyright (C) 2021 Purism SPC
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
test_adw_window_title_title (void)
{
  AdwWindowTitle *window_title = g_object_ref_sink (ADW_WINDOW_TITLE (adw_window_title_new ("Some title", NULL)));
  char *title;

  g_assert_nonnull (window_title);

  notified = 0;
  g_signal_connect (window_title, "notify::title", G_CALLBACK (notify_cb), NULL);

  g_object_get (window_title, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Some title");

  adw_window_title_set_title (window_title, "Some title");
  g_assert_cmpint (notified, ==, 0);

  adw_window_title_set_title (window_title, "Another title");
  g_assert_cmpstr (adw_window_title_get_title (window_title), ==, "Another title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (window_title, "title", "Yet another title", NULL);
  g_assert_cmpstr (adw_window_title_get_title (window_title), ==, "Yet another title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (window_title);
}

static void
test_adw_window_title_subtitle (void)
{
  AdwWindowTitle *window_title = g_object_ref_sink (ADW_WINDOW_TITLE (adw_window_title_new (NULL, "Some subtitle")));
  char *subtitle;

  g_assert_nonnull (window_title);

  notified = 0;
  g_signal_connect (window_title, "notify::subtitle", G_CALLBACK (notify_cb), NULL);

  g_object_get (window_title, "subtitle", &subtitle, NULL);
  g_assert_cmpstr (subtitle, ==, "Some subtitle");

  adw_window_title_set_subtitle (window_title, "Some subtitle");
  g_assert_cmpint (notified, ==, 0);

  adw_window_title_set_subtitle (window_title, "Another subtitle");
  g_assert_cmpstr (adw_window_title_get_subtitle (window_title), ==, "Another subtitle");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (window_title, "subtitle", "Yet another subtitle", NULL);
  g_assert_cmpstr (adw_window_title_get_subtitle (window_title), ==, "Yet another subtitle");
  g_assert_cmpint (notified, ==, 2);

  g_free (subtitle);
  g_assert_finalize_object (window_title);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/WindowTitle/title", test_adw_window_title_title);
  g_test_add_func ("/Adwaita/WindowTitle/subtitle", test_adw_window_title_subtitle);

  return g_test_run ();
}
