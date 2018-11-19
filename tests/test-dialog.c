/*
 * Copyright © 2018 Zander Brown <zbrown@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

gint win_width = 0;
gint win_height = 0;
gint dlg_width = 0;
gint dlg_height = 0;

static void
win_size_cb (GtkWidget    *widget,
             GdkRectangle *allocation,
             gpointer      user_data)
{
  gtk_window_get_size (GTK_WINDOW (widget), &win_width, &win_height);
}

static void
dlg_size_cb (GtkWidget    *widget,
             GdkRectangle *allocation,
             gpointer      user_data)
{
  gtk_window_get_size (GTK_WINDOW (widget), &dlg_width, &dlg_height);
}

static void
test_hdy_dialog_is_small (void)
{
  GtkWidget *window;
  GtkWidget *dialog;

  win_width = 0;
  win_height = 0;
  dlg_width = 0;
  dlg_height = 0;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_resize (GTK_WINDOW (window), 400, 400);
  g_signal_connect (window, "size-allocate", G_CALLBACK (win_size_cb), NULL);
  gtk_widget_show (window);

  dialog = hdy_dialog_new (GTK_WINDOW (window));
  g_signal_connect (dialog, "size-allocate", G_CALLBACK (dlg_size_cb), NULL);
  gtk_widget_show (dialog);

  g_assert_cmpint (win_height, ==, dlg_height);
  g_assert_cmpint (win_width, ==, dlg_width);
}

static void
test_hdy_dialog_normal (void)
{
  GtkWidget *window;
  GtkWidget *dialog;

  win_width = 0;
  win_height = 0;
  dlg_width = 0;
  dlg_height = 0;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_resize (GTK_WINDOW (window), 800, 800);
  g_signal_connect (window, "size-allocate", G_CALLBACK (win_size_cb), NULL);
  gtk_widget_show (window);

  dialog = hdy_dialog_new (GTK_WINDOW (window));
  g_signal_connect (dialog, "size-allocate", G_CALLBACK (dlg_size_cb), NULL);
  gtk_widget_show (dialog);

  g_assert_cmpint (win_height, !=, dlg_height);
  g_assert_cmpint (win_width, !=, dlg_width);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);

  g_test_add_func("/Handy/Dialog/is_small", test_hdy_dialog_is_small);
  g_test_add_func("/Handy/Dialog/normal", test_hdy_dialog_normal);
  return g_test_run();
}
