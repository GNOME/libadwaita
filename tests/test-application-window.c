/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_application_window_new (void)
{
  g_autoptr (GtkWidget) window = NULL;

  window = g_object_ref_sink (adw_application_window_new (NULL));
  g_assert_nonnull (window);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ApplicationWindow/new", test_adw_application_window_new);

  return g_test_run();
}
