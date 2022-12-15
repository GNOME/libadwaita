/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
test_adw_squeezer_homogeneous (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_true (adw_squeezer_get_homogeneous (squeezer));

  adw_squeezer_set_homogeneous (squeezer, FALSE);
  g_assert_false (adw_squeezer_get_homogeneous (squeezer));

  adw_squeezer_set_homogeneous (squeezer, TRUE);
  g_assert_true (adw_squeezer_get_homogeneous (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_allow_none (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_false (adw_squeezer_get_allow_none (squeezer));

  adw_squeezer_set_allow_none (squeezer, TRUE);
  g_assert_true (adw_squeezer_get_allow_none (squeezer));

  adw_squeezer_set_allow_none (squeezer, FALSE);
  g_assert_false (adw_squeezer_get_allow_none (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_transition_duration (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_cmpuint (adw_squeezer_get_transition_duration (squeezer), ==, 200);

  adw_squeezer_set_transition_duration (squeezer, 400);
  g_assert_cmpuint (adw_squeezer_get_transition_duration (squeezer), ==, 400);

  adw_squeezer_set_transition_duration (squeezer, -1);
  g_assert_cmpuint (adw_squeezer_get_transition_duration (squeezer), ==, G_MAXUINT);

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_transition_type (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_cmpuint (adw_squeezer_get_transition_type (squeezer), ==, ADW_SQUEEZER_TRANSITION_TYPE_NONE);

  adw_squeezer_set_transition_type (squeezer, ADW_SQUEEZER_TRANSITION_TYPE_CROSSFADE);
  g_assert_cmpuint (adw_squeezer_get_transition_type (squeezer), ==, ADW_SQUEEZER_TRANSITION_TYPE_CROSSFADE);

  adw_squeezer_set_transition_type (squeezer, ADW_SQUEEZER_TRANSITION_TYPE_NONE);
  g_assert_cmpuint (adw_squeezer_get_transition_type (squeezer), ==, ADW_SQUEEZER_TRANSITION_TYPE_NONE);

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_transition_running (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_false (adw_squeezer_get_transition_running (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_show_hide_child (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));
  GtkWidget *child;

  g_assert_nonnull (squeezer);

  g_assert_null (adw_squeezer_get_visible_child (squeezer));

  child = gtk_label_new ("");
  adw_squeezer_add (squeezer, child);
  g_assert (adw_squeezer_get_visible_child (squeezer) == child);

  gtk_widget_set_visible (child, FALSE);
  g_assert_null (adw_squeezer_get_visible_child (squeezer));

  gtk_widget_set_visible (child, TRUE);
  g_assert (adw_squeezer_get_visible_child (squeezer) == child);

  adw_squeezer_remove (squeezer, child);
  g_assert_null (adw_squeezer_get_visible_child (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_interpolate_size (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));

  g_assert_nonnull (squeezer);

  g_assert_false (adw_squeezer_get_interpolate_size (squeezer));

  adw_squeezer_set_interpolate_size (squeezer, TRUE);
  g_assert_true (adw_squeezer_get_interpolate_size (squeezer));

  adw_squeezer_set_interpolate_size (squeezer, FALSE);
  g_assert_false (adw_squeezer_get_interpolate_size (squeezer));

  g_assert_finalize_object (squeezer);
}


static void
test_adw_squeezer_page_enabled (void)
{
  AdwSqueezer *squeezer = g_object_ref_sink (ADW_SQUEEZER (adw_squeezer_new ()));
  GtkWidget *child;
  AdwSqueezerPage *page;

  g_assert_nonnull (squeezer);

  child = gtk_label_new ("");
  page = adw_squeezer_add (squeezer, child);
  g_assert_true (adw_squeezer_page_get_enabled (page));

  adw_squeezer_page_set_enabled (page, FALSE);
  g_assert_false (adw_squeezer_page_get_enabled (page));

  adw_squeezer_page_set_enabled (page, TRUE);
  g_assert_true (adw_squeezer_page_get_enabled (page));

  g_assert_finalize_object (squeezer);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ViewSwitcher/homogeneous", test_adw_squeezer_homogeneous);
  g_test_add_func("/Adwaita/ViewSwitcher/allow_none", test_adw_squeezer_allow_none);
  g_test_add_func("/Adwaita/ViewSwitcher/transition_duration", test_adw_squeezer_transition_duration);
  g_test_add_func("/Adwaita/ViewSwitcher/transition_type", test_adw_squeezer_transition_type);
  g_test_add_func("/Adwaita/ViewSwitcher/transition_running", test_adw_squeezer_transition_running);
  g_test_add_func("/Adwaita/ViewSwitcher/show_hide_child", test_adw_squeezer_show_hide_child);
  g_test_add_func("/Adwaita/ViewSwitcher/interpolate_size", test_adw_squeezer_interpolate_size);
  g_test_add_func("/Adwaita/ViewSwitcher/page_enabled", test_adw_squeezer_page_enabled);

  return g_test_run();
}
