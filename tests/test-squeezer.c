/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


static void
test_hdy_squeezer_homogeneous (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  g_assert_true (hdy_squeezer_get_homogeneous (squeezer));

  hdy_squeezer_set_homogeneous (squeezer, FALSE);
  g_assert_false (hdy_squeezer_get_homogeneous (squeezer));

  hdy_squeezer_set_homogeneous (squeezer, TRUE);
  g_assert_true (hdy_squeezer_get_homogeneous (squeezer));
}


static void
test_hdy_squeezer_transition_duration (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  g_assert_cmpuint (hdy_squeezer_get_transition_duration (squeezer), ==, 200);

  hdy_squeezer_set_transition_duration (squeezer, 400);
  g_assert_cmpuint (hdy_squeezer_get_transition_duration (squeezer), ==, 400);

  hdy_squeezer_set_transition_duration (squeezer, -1);
  g_assert_cmpuint (hdy_squeezer_get_transition_duration (squeezer), ==, G_MAXUINT);
}


static void
test_hdy_squeezer_transition_type (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  g_assert_cmpuint (hdy_squeezer_get_transition_type (squeezer), ==, HDY_SQUEEZER_TRANSITION_TYPE_NONE);

  hdy_squeezer_set_transition_type (squeezer, HDY_SQUEEZER_TRANSITION_TYPE_CROSSFADE);
  g_assert_cmpuint (hdy_squeezer_get_transition_type (squeezer), ==, HDY_SQUEEZER_TRANSITION_TYPE_CROSSFADE);

  hdy_squeezer_set_transition_type (squeezer, HDY_SQUEEZER_TRANSITION_TYPE_NONE);
  g_assert_cmpuint (hdy_squeezer_get_transition_type (squeezer), ==, HDY_SQUEEZER_TRANSITION_TYPE_NONE);
}


static void
test_hdy_squeezer_transition_running (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  g_assert_false (hdy_squeezer_get_transition_running (squeezer));
}


static void
test_hdy_squeezer_show_hide_child (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;
  GtkWidget *child;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  g_assert_null (hdy_squeezer_get_visible_child (squeezer));

  child = gtk_label_new ("");
  gtk_container_add (GTK_CONTAINER (squeezer), child);
  g_assert_null (hdy_squeezer_get_visible_child (squeezer));

  gtk_widget_show (child);
  g_assert (hdy_squeezer_get_visible_child (squeezer) == child);

  gtk_widget_hide (child);
  g_assert_null (hdy_squeezer_get_visible_child (squeezer));

  gtk_widget_show (child);
  g_assert (hdy_squeezer_get_visible_child (squeezer) == child);

  gtk_container_remove (GTK_CONTAINER (squeezer), child);
  g_assert_null (hdy_squeezer_get_visible_child (squeezer));
}


static void
test_hdy_squeezer_interpolate_size (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  g_assert_false (hdy_squeezer_get_interpolate_size (squeezer));

  hdy_squeezer_set_interpolate_size (squeezer, TRUE);
  g_assert_true (hdy_squeezer_get_interpolate_size (squeezer));

  hdy_squeezer_set_interpolate_size (squeezer, FALSE);
  g_assert_false (hdy_squeezer_get_interpolate_size (squeezer));
}


static void
test_hdy_squeezer_child_enabled (void)
{
  g_autoptr (HdySqueezer) squeezer = NULL;
  GtkWidget *child;

  squeezer = g_object_ref_sink (hdy_squeezer_new ());
  g_assert_nonnull (squeezer);

  child = gtk_label_new ("");
  gtk_widget_show (child);
  gtk_container_add (GTK_CONTAINER (squeezer), child);
  g_assert_true (hdy_squeezer_get_child_enabled (squeezer, child));

  hdy_squeezer_set_child_enabled (squeezer, child, FALSE);
  g_assert_false (hdy_squeezer_get_child_enabled (squeezer, child));

  hdy_squeezer_set_child_enabled (squeezer, child, TRUE);
  g_assert_true (hdy_squeezer_get_child_enabled (squeezer, child));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/ViewSwitcher/homogeneous", test_hdy_squeezer_homogeneous);
  g_test_add_func("/Handy/ViewSwitcher/transition_duration", test_hdy_squeezer_transition_duration);
  g_test_add_func("/Handy/ViewSwitcher/transition_type", test_hdy_squeezer_transition_type);
  g_test_add_func("/Handy/ViewSwitcher/transition_running", test_hdy_squeezer_transition_running);
  g_test_add_func("/Handy/ViewSwitcher/show_hide_child", test_hdy_squeezer_show_hide_child);
  g_test_add_func("/Handy/ViewSwitcher/interpolate_size", test_hdy_squeezer_interpolate_size);
  g_test_add_func("/Handy/ViewSwitcher/child_enabled", test_hdy_squeezer_child_enabled);

  return g_test_run();
}
