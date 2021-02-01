/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_carousel_indicator_lines_carousel (void)
{
  g_autoptr (AdwCarouselIndicatorLines) lines = NULL;
  AdwCarousel *carousel;

  lines = g_object_ref_sink (ADW_CAROUSEL_INDICATOR_LINES (adw_carousel_indicator_lines_new ()));
  g_assert_nonnull (lines);

  notified = 0;
  g_signal_connect (lines, "notify::carousel", G_CALLBACK (notify_cb), NULL);

  carousel = ADW_CAROUSEL (adw_carousel_new ());
  g_assert_nonnull (carousel);

  g_assert_null (adw_carousel_indicator_lines_get_carousel (lines));
  g_assert_cmpint (notified, ==, 0);

  adw_carousel_indicator_lines_set_carousel (lines, carousel);
  g_assert (adw_carousel_indicator_lines_get_carousel (lines) == carousel);
  g_assert_cmpint (notified, ==, 1);

  adw_carousel_indicator_lines_set_carousel (lines, NULL);
  g_assert_null (adw_carousel_indicator_lines_get_carousel (lines));
  g_assert_cmpint (notified, ==, 2);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/CarouselInidicatorLines/carousel", test_adw_carousel_indicator_lines_carousel);
  return g_test_run();
}
