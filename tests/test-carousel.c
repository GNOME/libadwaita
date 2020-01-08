/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
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
test_hdy_carousel_add_remove (void)
{
  HdyCarousel *carousel;
  GtkWidget *child1, *child2, *child3;

  carousel = HDY_CAROUSEL (hdy_carousel_new ());

  child1 = gtk_label_new ("");
  child2 = gtk_label_new ("");
  child3 = gtk_label_new ("");

  notified = 0;
  g_signal_connect (carousel, "notify::n-pages", G_CALLBACK (notify_cb), NULL);

  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 0);

  gtk_container_add (GTK_CONTAINER (carousel), child1);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 1);
  g_assert_cmpint (notified, ==, 1);

  hdy_carousel_prepend (carousel, child2);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 2);
  g_assert_cmpint (notified, ==, 2);

  hdy_carousel_insert (carousel, child3, 1);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 3);
  g_assert_cmpint (notified, ==, 3);

  hdy_carousel_reorder (carousel, child3, 0);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 3);
  g_assert_cmpint (notified, ==, 3);

  gtk_container_remove (GTK_CONTAINER (carousel), child2);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 2);
  g_assert_cmpint (notified, ==, 4);

  gtk_container_remove (GTK_CONTAINER (carousel), child1);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 1);
  g_assert_cmpint (notified, ==, 5);

  gtk_container_remove (GTK_CONTAINER (carousel), child3);
  g_assert_cmpuint (hdy_carousel_get_n_pages (carousel), ==, 0);
  g_assert_cmpint (notified, ==, 6);

  g_object_unref (carousel);
}

static void
test_hdy_carousel_scroll_to (void)
{
  HdyCarousel *carousel;
  GtkWidget *child1, *child2, *child3;

  carousel = HDY_CAROUSEL (hdy_carousel_new ());

  child1 = gtk_label_new ("");
  child2 = gtk_label_new ("");
  child3 = gtk_label_new ("");

  notified = 0;
  g_signal_connect (carousel, "notify::position", G_CALLBACK (notify_cb), NULL);

  gtk_container_add (GTK_CONTAINER (carousel), child1);
  gtk_container_add (GTK_CONTAINER (carousel), child2);
  gtk_container_add (GTK_CONTAINER (carousel), child3);

  /* Since tests are done synchronously, avoid animations */
  hdy_carousel_set_animation_duration (carousel, 0);

  g_assert_cmpfloat(hdy_carousel_get_position (carousel), ==, 0);
  g_assert_cmpint (notified, ==, 0);

  hdy_carousel_scroll_to (carousel, child3);
  g_assert_cmpfloat(hdy_carousel_get_position (carousel), ==, 2);
  g_assert_cmpint (notified, ==, 1);

  hdy_carousel_scroll_to (carousel, child2);
  g_assert_cmpfloat(hdy_carousel_get_position (carousel), ==, 1);
  g_assert_cmpint (notified, ==, 2);

  g_object_unref (carousel);
}

static void
test_hdy_carousel_interactive (void)
{
  HdyCarousel *carousel = HDY_CAROUSEL (hdy_carousel_new ());
  gboolean interactive;

  notified = 0;
  g_signal_connect (carousel, "notify::interactive", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_true (hdy_carousel_get_interactive (carousel));
  hdy_carousel_set_interactive (carousel, FALSE);
  g_assert_false (hdy_carousel_get_interactive (carousel));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "interactive", TRUE, NULL);
  g_object_get (carousel, "interactive", &interactive, NULL);
  g_assert_true (interactive);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_carousel_set_interactive (carousel, TRUE);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_carousel_spacing (void)
{
  HdyCarousel *carousel = HDY_CAROUSEL (hdy_carousel_new ());
  guint spacing;

  notified = 0;
  g_signal_connect (carousel, "notify::spacing", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpuint (hdy_carousel_get_spacing (carousel), ==, 0);
  hdy_carousel_set_spacing (carousel, 12);
  g_assert_cmpuint (hdy_carousel_get_spacing (carousel), ==, 12);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "spacing", 6, NULL);
  g_object_get (carousel, "spacing", &spacing, NULL);
  g_assert_cmpuint (spacing, ==, 6);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_carousel_set_spacing (carousel, 6);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_carousel_animation_duration (void)
{
  HdyCarousel *carousel = HDY_CAROUSEL (hdy_carousel_new ());
  guint duration;

  notified = 0;
  g_signal_connect (carousel, "notify::animation-duration", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpuint (hdy_carousel_get_animation_duration (carousel), ==, 250);
  hdy_carousel_set_animation_duration (carousel, 200);
  g_assert_cmpuint (hdy_carousel_get_animation_duration (carousel), ==, 200);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "animation-duration", 500, NULL);
  g_object_get (carousel, "animation-duration", &duration, NULL);
  g_assert_cmpuint (duration, ==, 500);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_carousel_set_animation_duration (carousel, 500);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_carousel_allow_mouse_drag (void)
{
  HdyCarousel *carousel = HDY_CAROUSEL (hdy_carousel_new ());
  gboolean allow_mouse_drag;

  notified = 0;
  g_signal_connect (carousel, "notify::allow-mouse-drag", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_true (hdy_carousel_get_allow_mouse_drag (carousel));
  hdy_carousel_set_allow_mouse_drag (carousel, FALSE);
  g_assert_false (hdy_carousel_get_allow_mouse_drag (carousel));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "allow-mouse-drag", TRUE, NULL);
  g_object_get (carousel, "allow-mouse-drag", &allow_mouse_drag, NULL);
  g_assert_true (allow_mouse_drag);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_carousel_set_allow_mouse_drag (carousel, TRUE);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_carousel_reveal_duration (void)
{
  HdyCarousel *carousel = HDY_CAROUSEL (hdy_carousel_new ());
  guint duration;

  notified = 0;
  g_signal_connect (carousel, "notify::reveal-duration", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpuint (hdy_carousel_get_reveal_duration (carousel), ==, 0);
  hdy_carousel_set_reveal_duration (carousel, 200);
  g_assert_cmpuint (hdy_carousel_get_reveal_duration (carousel), ==, 200);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (carousel, "reveal-duration", 500, NULL);
  g_object_get (carousel, "reveal-duration", &duration, NULL);
  g_assert_cmpuint (duration, ==, 500);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_carousel_set_reveal_duration (carousel, 500);
  g_assert_cmpint (notified, ==, 2);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/Carousel/add_remove", test_hdy_carousel_add_remove);
  g_test_add_func("/Handy/Carousel/scroll_to", test_hdy_carousel_scroll_to);
  g_test_add_func("/Handy/Carousel/interactive", test_hdy_carousel_interactive);
  g_test_add_func("/Handy/Carousel/spacing", test_hdy_carousel_spacing);
  g_test_add_func("/Handy/Carousel/animation_duration", test_hdy_carousel_animation_duration);
  g_test_add_func("/Handy/Carousel/allow_mouse_drag", test_hdy_carousel_allow_mouse_drag);
  g_test_add_func("/Handy/Carousel/reveal_duration", test_hdy_carousel_reveal_duration);
  return g_test_run();
}
