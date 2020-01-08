/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>

gint notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_hdy_carousel_indicator_dots_carousel (void)
{
  g_autoptr (HdyCarouselIndicatorDots) dots = NULL;
  HdyCarousel *carousel;

  dots = g_object_ref_sink (HDY_CAROUSEL_INDICATOR_DOTS (hdy_carousel_indicator_dots_new ()));
  g_assert_nonnull (dots);

  notified = 0;
  g_signal_connect (dots, "notify::carousel", G_CALLBACK (notify_cb), NULL);

  carousel = HDY_CAROUSEL (hdy_carousel_new ());
  g_assert_nonnull (carousel);

  g_assert_null (hdy_carousel_indicator_dots_get_carousel (dots));
  g_assert_cmpint (notified, ==, 0);

  hdy_carousel_indicator_dots_set_carousel (dots, carousel);
  g_assert (hdy_carousel_indicator_dots_get_carousel (dots) == carousel);
  g_assert_cmpint (notified, ==, 1);

  hdy_carousel_indicator_dots_set_carousel (dots, NULL);
  g_assert_null (hdy_carousel_indicator_dots_get_carousel (dots));
  g_assert_cmpint (notified, ==, 2);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/CarouselIndicatorDots/carousel", test_hdy_carousel_indicator_dots_carousel);
  return g_test_run();
}
