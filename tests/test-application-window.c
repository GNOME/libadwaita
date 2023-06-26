/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
test_adw_application_window_new (void)
{
  GtkWidget *window = adw_application_window_new (NULL);
  g_assert_nonnull (window);

  g_assert_finalize_object (window);
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
