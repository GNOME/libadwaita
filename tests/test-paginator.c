/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

gint notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_hdy_paginator_add_remove (void)
{
  HdyPaginator *paginator;
  GtkWidget *child1, *child2, *child3;

  paginator = HDY_PAGINATOR (hdy_paginator_new ());

  child1 = gtk_label_new ("");
  child2 = gtk_label_new ("");
  child3 = gtk_label_new ("");

  notified = 0;
  g_signal_connect (paginator, "notify::n-pages", G_CALLBACK (notify_cb), NULL);

  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 0);

  gtk_container_add (GTK_CONTAINER (paginator), child1);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 1);
  g_assert_cmpint (notified, ==, 1);

  hdy_paginator_prepend (paginator, child2);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 2);
  g_assert_cmpint (notified, ==, 2);

  hdy_paginator_insert (paginator, child3, 1);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 3);
  g_assert_cmpint (notified, ==, 3);

  hdy_paginator_reorder (paginator, child3, 0);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 3);
  g_assert_cmpint (notified, ==, 3);

  gtk_container_remove (GTK_CONTAINER (paginator), child2);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 2);
  g_assert_cmpint (notified, ==, 4);

  gtk_container_remove (GTK_CONTAINER (paginator), child1);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 1);
  g_assert_cmpint (notified, ==, 5);

  gtk_container_remove (GTK_CONTAINER (paginator), child3);
  g_assert_cmpuint (hdy_paginator_get_n_pages (paginator), ==, 0);
  g_assert_cmpint (notified, ==, 6);

  g_object_unref (paginator);
}

static void
test_hdy_paginator_scroll_to (void)
{
  HdyPaginator *paginator;
  GtkWidget *child1, *child2, *child3;

  paginator = HDY_PAGINATOR (hdy_paginator_new ());

  child1 = gtk_label_new ("");
  child2 = gtk_label_new ("");
  child3 = gtk_label_new ("");

  notified = 0;
  g_signal_connect (paginator, "notify::position", G_CALLBACK (notify_cb), NULL);

  gtk_container_add (GTK_CONTAINER (paginator), child1);
  gtk_container_add (GTK_CONTAINER (paginator), child2);
  gtk_container_add (GTK_CONTAINER (paginator), child3);

  /* Since tests are done synchronously, avoid animations */
  hdy_paginator_set_animation_duration (paginator, 0);

  g_assert_cmpfloat(hdy_paginator_get_position (paginator), ==, 0);
  g_assert_cmpint (notified, ==, 0);

  hdy_paginator_scroll_to (paginator, child3);
  g_assert_cmpfloat(hdy_paginator_get_position (paginator), ==, 2);
  g_assert_cmpint (notified, ==, 1);

  hdy_paginator_scroll_to (paginator, child2);
  g_assert_cmpfloat(hdy_paginator_get_position (paginator), ==, 1);
  g_assert_cmpint (notified, ==, 2);

  g_object_unref (paginator);
}

static void
test_hdy_paginator_interactive (void)
{
  HdyPaginator *paginator = HDY_PAGINATOR (hdy_paginator_new ());
  gboolean interactive;

  notified = 0;
  g_signal_connect (paginator, "notify::interactive", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_true (hdy_paginator_get_interactive (paginator));
  hdy_paginator_set_interactive (paginator, FALSE);
  g_assert_false (hdy_paginator_get_interactive (paginator));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (paginator, "interactive", TRUE, NULL);
  g_object_get (paginator, "interactive", &interactive, NULL);
  g_assert_true (interactive);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_paginator_set_interactive (paginator, TRUE);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_paginator_indicator_style (void)
{
  HdyPaginator *paginator = HDY_PAGINATOR (hdy_paginator_new ());
  HdyPaginatorIndicatorStyle indicator_style;

  notified = 0;
  g_signal_connect (paginator, "notify::indicator-style", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpint (hdy_paginator_get_indicator_style (paginator), ==, HDY_PAGINATOR_INDICATOR_STYLE_NONE);
  hdy_paginator_set_indicator_style (paginator, HDY_PAGINATOR_INDICATOR_STYLE_DOTS);
  g_assert_cmpint (hdy_paginator_get_indicator_style (paginator), ==, HDY_PAGINATOR_INDICATOR_STYLE_DOTS);
  g_assert_cmpint (notified, ==, 1);
  hdy_paginator_set_indicator_style (paginator, HDY_PAGINATOR_INDICATOR_STYLE_LINES);
  g_assert_cmpint (hdy_paginator_get_indicator_style (paginator), ==, HDY_PAGINATOR_INDICATOR_STYLE_LINES);
  g_assert_cmpint (notified, ==, 2);

  /* Property */
  g_object_set (paginator, "indicator-style", HDY_PAGINATOR_INDICATOR_STYLE_DOTS, NULL);
  g_object_get (paginator, "indicator-style", &indicator_style, NULL);
  g_assert_cmpint (indicator_style, ==, HDY_PAGINATOR_INDICATOR_STYLE_DOTS);
  g_assert_cmpint (notified, ==, 3);

  /* Setting the same value should not notify */
  hdy_paginator_set_indicator_style (paginator, HDY_PAGINATOR_INDICATOR_STYLE_DOTS);
  g_assert_cmpint (notified, ==, 3);
}

static void
test_hdy_paginator_indicator_spacing (void)
{
  HdyPaginator *paginator = HDY_PAGINATOR (hdy_paginator_new ());
  uint spacing;

  notified = 0;
  g_signal_connect (paginator, "notify::indicator-spacing", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpuint (hdy_paginator_get_indicator_spacing (paginator), ==, 0);
  hdy_paginator_set_indicator_spacing (paginator, 12);
  g_assert_cmpuint (hdy_paginator_get_indicator_spacing (paginator), ==, 12);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (paginator, "indicator-spacing", 6, NULL);
  g_object_get (paginator, "indicator-spacing", &spacing, NULL);
  g_assert_cmpuint (spacing, ==, 6);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_paginator_set_indicator_spacing (paginator, 6);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_paginator_center_content (void)
{
  HdyPaginator *paginator = HDY_PAGINATOR (hdy_paginator_new ());
  gboolean center_content;

  notified = 0;
  g_signal_connect (paginator, "notify::center-content", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_false (hdy_paginator_get_center_content (paginator));
  hdy_paginator_set_center_content (paginator, TRUE);
  g_assert_true (hdy_paginator_get_center_content (paginator));
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (paginator, "center-content", FALSE, NULL);
  g_object_get (paginator, "center-content", &center_content, NULL);
  g_assert_false (center_content);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_paginator_set_center_content (paginator, FALSE);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_paginator_spacing (void)
{
  HdyPaginator *paginator = HDY_PAGINATOR (hdy_paginator_new ());
  uint spacing;

  notified = 0;
  g_signal_connect (paginator, "notify::spacing", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpuint (hdy_paginator_get_spacing (paginator), ==, 0);
  hdy_paginator_set_spacing (paginator, 12);
  g_assert_cmpuint (hdy_paginator_get_spacing (paginator), ==, 12);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (paginator, "spacing", 6, NULL);
  g_object_get (paginator, "spacing", &spacing, NULL);
  g_assert_cmpuint (spacing, ==, 6);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_paginator_set_spacing (paginator, 6);
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_paginator_animation_duration (void)
{
  HdyPaginator *paginator = HDY_PAGINATOR (hdy_paginator_new ());
  uint duration;

  notified = 0;
  g_signal_connect (paginator, "notify::animation-duration", G_CALLBACK (notify_cb), NULL);

  /* Accessors */
  g_assert_cmpuint (hdy_paginator_get_animation_duration (paginator), ==, 250);
  hdy_paginator_set_animation_duration (paginator, 200);
  g_assert_cmpuint (hdy_paginator_get_animation_duration (paginator), ==, 200);
  g_assert_cmpint (notified, ==, 1);

  /* Property */
  g_object_set (paginator, "animation-duration", 500, NULL);
  g_object_get (paginator, "animation-duration", &duration, NULL);
  g_assert_cmpuint (duration, ==, 500);
  g_assert_cmpint (notified, ==, 2);

  /* Setting the same value should not notify */
  hdy_paginator_set_animation_duration (paginator, 500);
  g_assert_cmpint (notified, ==, 2);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/Paginator/add_remove", test_hdy_paginator_add_remove);
  g_test_add_func("/Handy/Paginator/scroll_to", test_hdy_paginator_scroll_to);
  g_test_add_func("/Handy/Paginator/interactive", test_hdy_paginator_interactive);
  g_test_add_func("/Handy/Paginator/indicator_style", test_hdy_paginator_indicator_style);
  g_test_add_func("/Handy/Paginator/indicator_spacing", test_hdy_paginator_indicator_spacing);
  g_test_add_func("/Handy/Paginator/center_content", test_hdy_paginator_center_content);
  g_test_add_func("/Handy/Paginator/spacing", test_hdy_paginator_spacing);
  g_test_add_func("/Handy/Paginator/animation_duration", test_hdy_paginator_animation_duration);
  return g_test_run();
}
