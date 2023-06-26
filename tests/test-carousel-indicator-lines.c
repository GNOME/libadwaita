/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_carousel_indicator_lines_carousel (void)
{
  AdwCarouselIndicatorLines *lines = g_object_ref_sink (ADW_CAROUSEL_INDICATOR_LINES (adw_carousel_indicator_lines_new ()));
  AdwCarousel *carousel;
  int notified = 0;

  g_assert_nonnull (lines);

  g_signal_connect_swapped (lines, "notify::carousel", G_CALLBACK (increment), &notified);

  carousel = g_object_ref_sink (ADW_CAROUSEL (adw_carousel_new ()));
  g_assert_nonnull (carousel);

  g_assert_null (adw_carousel_indicator_lines_get_carousel (lines));
  g_assert_cmpint (notified, ==, 0);

  adw_carousel_indicator_lines_set_carousel (lines, carousel);
  g_assert (adw_carousel_indicator_lines_get_carousel (lines) == carousel);
  g_assert_cmpint (notified, ==, 1);

  adw_carousel_indicator_lines_set_carousel (lines, NULL);
  g_assert_null (adw_carousel_indicator_lines_get_carousel (lines));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (lines);
  g_assert_finalize_object (carousel);
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
