/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_window_new (void)
{
  g_autoptr (GtkWidget) window = NULL;

  window = g_object_ref_sink (adw_window_new ());
  g_assert_nonnull (window);
}


gint
main (gint argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/Window/new", test_adw_window_new);

  return g_test_run();
}
