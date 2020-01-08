/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_window_handle_new (void)
{
  g_autoptr (GtkWidget) handle = NULL;

  handle = g_object_ref_sink (hdy_window_handle_new ());
  g_assert_nonnull (handle);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/WindowHandle/new", test_hdy_window_handle_new);

  return g_test_run();
}
