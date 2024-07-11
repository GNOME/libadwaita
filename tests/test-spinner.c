/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_spinner_new (void)
{
  AdwSpinner *spinner = g_object_ref_sink (ADW_SPINNER (adw_spinner_new ()));
  g_assert_nonnull (spinner);

  g_assert_finalize_object (spinner);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/Spinner/new", test_adw_spinner_new);

  return g_test_run();
}
