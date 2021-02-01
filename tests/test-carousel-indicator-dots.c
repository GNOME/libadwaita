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
test_adw_carousel_indicator_dots_carousel (void)
{
  g_autoptr (AdwCarouselIndicatorDots) dots = NULL;
  AdwCarousel *carousel;

  dots = g_object_ref_sink (ADW_CAROUSEL_INDICATOR_DOTS (adw_carousel_indicator_dots_new ()));
  g_assert_nonnull (dots);

  notified = 0;
  g_signal_connect (dots, "notify::carousel", G_CALLBACK (notify_cb), NULL);

  carousel = ADW_CAROUSEL (adw_carousel_new ());
  g_assert_nonnull (carousel);

  g_assert_null (adw_carousel_indicator_dots_get_carousel (dots));
  g_assert_cmpint (notified, ==, 0);

  adw_carousel_indicator_dots_set_carousel (dots, carousel);
  g_assert (adw_carousel_indicator_dots_get_carousel (dots) == carousel);
  g_assert_cmpint (notified, ==, 1);

  adw_carousel_indicator_dots_set_carousel (dots, NULL);
  g_assert_null (adw_carousel_indicator_dots_get_carousel (dots));
  g_assert_cmpint (notified, ==, 2);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/CarouselIndicatorDots/carousel", test_adw_carousel_indicator_dots_carousel);
  return g_test_run();
}
