/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_navigation_split_view_sidebar (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  AdwNavigationPage *widget = NULL;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::sidebar", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "sidebar", &widget, NULL);
  g_assert_null (widget);

  adw_navigation_split_view_set_sidebar (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = adw_navigation_page_new (gtk_button_new (), "Sidebar");
  adw_navigation_split_view_set_sidebar (split_view, widget);
  g_assert_true (adw_navigation_split_view_get_sidebar (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar", NULL, NULL);
  g_assert_null (adw_navigation_split_view_get_sidebar (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_content (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  AdwNavigationPage *widget = NULL;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::content", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "content", &widget, NULL);
  g_assert_null (widget);

  adw_navigation_split_view_set_content (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = adw_navigation_page_new (gtk_button_new (), "Content");
  adw_navigation_split_view_set_content (split_view, widget);
  g_assert_true (adw_navigation_split_view_get_content (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "content", NULL, NULL);
  g_assert_null (adw_navigation_split_view_get_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_collapsed (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  gboolean collapsed;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::collapsed", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "collapsed", &collapsed, NULL);
  g_assert_false (collapsed);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "collapsed", FALSE, NULL);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 2);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new (gtk_button_new (), "Sidebar"));

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 3);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 4);

  adw_navigation_split_view_set_sidebar (split_view, NULL);
  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new (gtk_button_new (), "Content"));

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 5);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 6);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new (gtk_button_new (), "Sidebar"));

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 7);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 8);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_show_content (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  gboolean show_content;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::show-content", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "show-content", &show_content, NULL);
  g_assert_false (show_content);

  adw_navigation_split_view_set_show_content (split_view, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new (gtk_button_new (), "Sidebar"));

  adw_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 3);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 4);

  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new (gtk_button_new (), "Content"));
  adw_navigation_split_view_set_sidebar (split_view, NULL);

  adw_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 5);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 6);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new (gtk_button_new (), "Sidebar"));

  adw_navigation_split_view_set_show_content (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 7);

  g_object_set (split_view, "show-content", FALSE, NULL);
  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 8);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_min_sidebar_width (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  double width;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::min-sidebar-width", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "min-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 180, DBL_EPSILON));

  adw_navigation_split_view_set_min_sidebar_width (split_view, 180);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_min_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_min_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "min-sidebar-width", 180.0, NULL);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_min_sidebar_width (split_view), 180, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_max_sidebar_width (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  double width;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::max-sidebar-width", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "max-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 360, DBL_EPSILON));

  adw_navigation_split_view_set_max_sidebar_width (split_view, 360);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_max_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_max_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "max-sidebar-width", 360.0, NULL);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_max_sidebar_width (split_view), 360, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_sidebar_width_fraction (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  double fraction;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::sidebar-width-fraction", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "sidebar-width-fraction", &fraction, NULL);
  g_assert_true (G_APPROX_VALUE (fraction, 0.3, DBL_EPSILON));

  adw_navigation_split_view_set_sidebar_width_fraction (split_view, 0.3);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_sidebar_width_fraction (split_view, 0.2);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_sidebar_width_fraction (split_view), 0.2, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-fraction", 0.3, NULL);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_sidebar_width_fraction (split_view), 0.3, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_sidebar_width_unit (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  AdwLengthUnit unit;

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::sidebar-width-unit", G_CALLBACK (notify_cb), NULL);

  g_object_get (split_view, "sidebar-width-unit", &unit, NULL);
  g_assert_cmpint (unit, ==, ADW_LENGTH_UNIT_SP);

  adw_navigation_split_view_set_sidebar_width_unit (split_view, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_sidebar_width_unit (split_view, ADW_LENGTH_UNIT_PX);
  g_assert_cmpint (adw_navigation_split_view_get_sidebar_width_unit (split_view), ==, ADW_LENGTH_UNIT_PX);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-unit", ADW_LENGTH_UNIT_SP, NULL);
  g_assert_cmpint (adw_navigation_split_view_get_sidebar_width_unit (split_view), ==, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_page_tags (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

  g_assert_nonnull (split_view);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already has the same tag*");
  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "sidebar"));
  g_test_assert_expected_messages ();

  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already has the same tag*");
  adw_navigation_page_set_tag (adw_navigation_split_view_get_sidebar (split_view), "content");
  g_test_assert_expected_messages ();

  adw_navigation_page_set_tag (adw_navigation_split_view_get_sidebar (split_view), "sidebar");

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already has the same tag*");
  adw_navigation_page_set_tag (adw_navigation_split_view_get_content (split_view), "sidebar");
  g_test_assert_expected_messages ();

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_actions (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

  g_assert_nonnull (split_view);

  notified = 0;
  g_signal_connect (split_view, "notify::show-content", G_CALLBACK (notify_cb), NULL);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 0);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in the navigation stack*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "sidebar");
  g_test_assert_expected_messages ();

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*No page with the tag*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "something");
  g_test_assert_expected_messages ();

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");

  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in the navigation stack*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");
  g_test_assert_expected_messages ();

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*already in the navigation stack*");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "sidebar");
  g_test_assert_expected_messages ();

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.pop", NULL);

  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/NavigationSplitView/sidebar", test_adw_navigation_split_view_sidebar);
  g_test_add_func ("/Adwaita/NavigationSplitView/content", test_adw_navigation_split_view_content);
  g_test_add_func ("/Adwaita/NavigationSplitView/collapsed", test_adw_navigation_split_view_collapsed);
  g_test_add_func ("/Adwaita/NavigationSplitView/show_content", test_adw_navigation_split_view_show_content);
  g_test_add_func ("/Adwaita/NavigationSplitView/min_sidebar_width", test_adw_navigation_split_view_min_sidebar_width);
  g_test_add_func ("/Adwaita/NavigationSplitView/max_sidebar_width", test_adw_navigation_split_view_max_sidebar_width);
  g_test_add_func ("/Adwaita/NavigationSplitView/sidebar_width_fraction", test_adw_navigation_split_view_sidebar_width_fraction);
  g_test_add_func ("/Adwaita/NavigationSplitView/sidebar_width_unit", test_adw_navigation_split_view_sidebar_width_unit);
  g_test_add_func ("/Adwaita/NavigationSplitView/page_tags", test_adw_navigation_split_view_page_tags);
  g_test_add_func ("/Adwaita/NavigationSplitView/actions", test_adw_navigation_split_view_actions);

  return g_test_run ();
}
