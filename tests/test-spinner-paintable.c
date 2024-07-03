/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_spinner_paintable_new (void)
{
  AdwSpinnerPaintable *paintable = adw_spinner_paintable_new (NULL);
  g_assert_nonnull (paintable);

  g_assert_finalize_object (paintable);
}

static void
test_adw_spinner_paintable_new_with_widget (void)
{
  GtkWidget *image = g_object_ref_sink (gtk_image_new ());
  AdwSpinnerPaintable *paintable = adw_spinner_paintable_new (image);
  g_assert_nonnull (paintable);

  gtk_image_set_from_paintable (GTK_IMAGE (image), GDK_PAINTABLE (paintable));

  g_assert_finalize_object (image);
  g_assert_finalize_object (paintable);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/SpinnerPaintable/new", test_adw_spinner_paintable_new);
  g_test_add_func("/Adwaita/SpinnerPaintable/new_with_widget", test_adw_spinner_paintable_new_with_widget);

  return g_test_run();
}
