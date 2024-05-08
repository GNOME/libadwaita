/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_navigation_split_view_sidebar (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  AdwNavigationPage *widget = NULL;
  int notified = 0, showing = 0, hiding = 0, shown = 0, hidden = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar", &widget, NULL);
  g_assert_null (widget);

  adw_navigation_split_view_set_sidebar (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Sidebar"));
  g_signal_connect_swapped (widget, "showing", G_CALLBACK (increment), &showing);
  g_signal_connect_swapped (widget, "hiding", G_CALLBACK (increment), &hiding);
  g_signal_connect_swapped (widget, "shown", G_CALLBACK (increment), &shown);
  g_signal_connect_swapped (widget, "hidden", G_CALLBACK (increment), &hidden);

  adw_navigation_split_view_set_sidebar (split_view, widget);
  g_assert_true (adw_navigation_split_view_get_sidebar (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  g_object_set (split_view, "sidebar", NULL, NULL);
  g_assert_null (adw_navigation_split_view_get_sidebar (split_view));
  g_assert_cmpint (notified, ==, 2);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  g_assert_finalize_object (widget);
  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_content (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  AdwNavigationPage *widget = NULL;
  int notified = 0, showing = 0, hiding = 0, shown = 0, hidden = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::content", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "content", &widget, NULL);
  g_assert_null (widget);

  adw_navigation_split_view_set_content (split_view, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Content"));
  g_signal_connect_swapped (widget, "showing", G_CALLBACK (increment), &showing);
  g_signal_connect_swapped (widget, "hiding", G_CALLBACK (increment), &hiding);
  g_signal_connect_swapped (widget, "shown", G_CALLBACK (increment), &shown);
  g_signal_connect_swapped (widget, "hidden", G_CALLBACK (increment), &hidden);

  adw_navigation_split_view_set_content (split_view, widget);
  g_assert_true (adw_navigation_split_view_get_content (split_view) == widget);
  g_assert_cmpint (notified, ==, 1);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 0);
  g_assert_cmpint (hidden, ==, 0);

  g_object_set (split_view, "content", NULL, NULL);
  g_assert_null (adw_navigation_split_view_get_content (split_view));
  g_assert_cmpint (notified, ==, 2);
  g_assert_cmpint (showing, ==, 1);
  g_assert_cmpint (shown, ==, 1);
  g_assert_cmpint (hiding, ==, 1);
  g_assert_cmpint (hidden, ==, 1);

  g_assert_finalize_object (widget);
  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_collapsed (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  gboolean collapsed;
  int notified = 0;
  int sidebar_showing = 0, sidebar_hiding = 0, sidebar_shown = 0, sidebar_hidden = 0;
  int content_showing = 0, content_hiding = 0, content_shown = 0, content_hidden = 0;
  AdwNavigationPage *sidebar, *content;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::collapsed", G_CALLBACK (increment), &notified);

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

  sidebar = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Sidebar"));
  g_signal_connect_swapped (sidebar, "showing", G_CALLBACK (increment), &sidebar_showing);
  g_signal_connect_swapped (sidebar, "hiding", G_CALLBACK (increment), &sidebar_hiding);
  g_signal_connect_swapped (sidebar, "shown", G_CALLBACK (increment), &sidebar_shown);
  g_signal_connect_swapped (sidebar, "hidden", G_CALLBACK (increment), &sidebar_hidden);

  content = g_object_ref_sink (adw_navigation_page_new (gtk_button_new (), "Content"));
  g_signal_connect_swapped (content, "showing", G_CALLBACK (increment), &content_showing);
  g_signal_connect_swapped (content, "hiding", G_CALLBACK (increment), &content_hiding);
  g_signal_connect_swapped (content, "shown", G_CALLBACK (increment), &content_shown);
  g_signal_connect_swapped (content, "hidden", G_CALLBACK (increment), &content_hidden);

  adw_navigation_split_view_set_sidebar (split_view, sidebar);
  g_assert_cmpint (sidebar_showing, ==, 1);
  g_assert_cmpint (sidebar_shown, ==, 1);
  g_assert_cmpint (sidebar_hiding, ==, 0);
  g_assert_cmpint (sidebar_hidden, ==, 0);

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 3);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 4);

  adw_navigation_split_view_set_sidebar (split_view, NULL);
  adw_navigation_split_view_set_content (split_view, content);
  g_assert_cmpint (sidebar_showing, ==, 1);
  g_assert_cmpint (sidebar_shown, ==, 1);
  g_assert_cmpint (sidebar_hiding, ==, 1);
  g_assert_cmpint (sidebar_hidden, ==, 1);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 0);
  g_assert_cmpint (content_hidden, ==, 0);

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 5);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 6);

  adw_navigation_split_view_set_sidebar (split_view, sidebar);
  g_assert_cmpint (sidebar_showing, ==, 2);
  g_assert_cmpint (sidebar_shown, ==, 2);
  g_assert_cmpint (sidebar_hiding, ==, 1);
  g_assert_cmpint (sidebar_hidden, ==, 1);

  adw_navigation_split_view_set_show_content (split_view, TRUE);

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 7);
  g_assert_cmpint (sidebar_showing, ==, 2);
  g_assert_cmpint (sidebar_shown, ==, 2);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 0);
  g_assert_cmpint (content_hidden, ==, 0);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 8);
  g_assert_cmpint (sidebar_showing, ==, 3);
  g_assert_cmpint (sidebar_shown, ==, 3);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 0);
  g_assert_cmpint (content_hidden, ==, 0);

  adw_navigation_split_view_set_show_content (split_view, FALSE);

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  g_assert_true (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 9);
  g_assert_cmpint (sidebar_showing, ==, 3);
  g_assert_cmpint (sidebar_shown, ==, 3);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 1);
  g_assert_cmpint (content_shown, ==, 1);
  g_assert_cmpint (content_hiding, ==, 1);
  g_assert_cmpint (content_hidden, ==, 1);

  adw_navigation_split_view_set_collapsed (split_view, FALSE);
  g_assert_false (adw_navigation_split_view_get_collapsed (split_view));
  g_assert_cmpint (notified, ==, 10);
  g_assert_cmpint (sidebar_showing, ==, 3);
  g_assert_cmpint (sidebar_shown, ==, 3);
  g_assert_cmpint (sidebar_hiding, ==, 2);
  g_assert_cmpint (sidebar_hidden, ==, 2);
  g_assert_cmpint (content_showing, ==, 2);
  g_assert_cmpint (content_shown, ==, 2);
  g_assert_cmpint (content_hiding, ==, 1);
  g_assert_cmpint (content_hidden, ==, 1);

  g_assert_finalize_object (split_view);
  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (content);
}

static void
test_adw_navigation_split_view_show_content (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  gboolean show_content;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::show-content", G_CALLBACK (increment), &notified);

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
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::min-sidebar-width", G_CALLBACK (increment), &notified);

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
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::max-sidebar-width", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "max-sidebar-width", &width, NULL);
  g_assert_true (G_APPROX_VALUE (width, 280, DBL_EPSILON));

  adw_navigation_split_view_set_max_sidebar_width (split_view, 280);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_max_sidebar_width (split_view, 200);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_max_sidebar_width (split_view), 200, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "max-sidebar-width", 280.0, NULL);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_max_sidebar_width (split_view), 280, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_sidebar_width_fraction (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  double fraction;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-width-fraction", G_CALLBACK (increment), &notified);

  g_object_get (split_view, "sidebar-width-fraction", &fraction, NULL);
  g_assert_true (G_APPROX_VALUE (fraction, 0.25, DBL_EPSILON));

  adw_navigation_split_view_set_sidebar_width_fraction (split_view, 0.25);
  g_assert_cmpint (notified, ==, 0);

  adw_navigation_split_view_set_sidebar_width_fraction (split_view, 0.2);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_sidebar_width_fraction (split_view), 0.2, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (split_view, "sidebar-width-fraction", 0.25, NULL);
  g_assert_true (G_APPROX_VALUE (adw_navigation_split_view_get_sidebar_width_fraction (split_view), 0.25, DBL_EPSILON));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_sidebar_width_unit (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  AdwLengthUnit unit;
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::sidebar-width-unit", G_CALLBACK (increment), &notified);

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

  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

  adw_navigation_page_set_tag (adw_navigation_split_view_get_sidebar (split_view), "sidebar");

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_duplicate_tag (void)
{
  if (g_test_subprocess ()) {
    AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

    adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));

    adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "sidebar"));

    g_assert_finalize_object (split_view);
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*already has the same tag*");
}

static void
test_adw_navigation_split_view_retag_sidebar (void)
{
  if (g_test_subprocess ()) {
    AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

    adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
    adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

    adw_navigation_page_set_tag (adw_navigation_split_view_get_sidebar (split_view), "content");

    g_assert_finalize_object (split_view);
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*already has the same tag*");
}

static void
test_adw_navigation_split_view_retag_content (void)
{
  if (g_test_subprocess ()) {
    AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

    adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
    adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

    adw_navigation_page_set_tag (adw_navigation_split_view_get_content (split_view), "sidebar");

    g_assert_finalize_object (split_view);
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*already has the same tag*");
}

static void
test_adw_navigation_split_view_actions (void)
{
  AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));
  int notified = 0;

  g_assert_nonnull (split_view);

  g_signal_connect_swapped (split_view, "notify::show-content", G_CALLBACK (increment), &notified);

  adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
  adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 0);

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");

  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");
  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");

  g_assert_true (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 1);

  gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.pop", NULL);

  g_assert_false (adw_navigation_split_view_get_show_content (split_view));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (split_view);
}

static void
test_adw_navigation_split_view_push_exiting (void)
{
  if (g_test_subprocess ()) {
    AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

    adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
    adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

    gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "sidebar");

    g_assert_finalize_object (split_view);
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*already in the navigation stack*");
}

static void
test_adw_navigation_split_view_push_invalid (void)
{
  if (g_test_subprocess ()) {
    AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

    adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
    adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

    gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "something");

    g_assert_finalize_object (split_view);
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*No page with the tag*");
}

static void
test_adw_navigation_split_view_push_duplicate (void)
{
  if (g_test_subprocess ()) {
    AdwNavigationSplitView *split_view = g_object_ref_sink (ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ()));

    adw_navigation_split_view_set_sidebar (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Sidebar", "sidebar"));
    adw_navigation_split_view_set_content (split_view, adw_navigation_page_new_with_tag (gtk_button_new (), "Content", "content"));

    gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");
    gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "content");

    gtk_widget_activate_action (GTK_WIDGET (split_view), "navigation.push", "s", "sidebar");

    g_assert_finalize_object (split_view);
  }

  g_test_trap_subprocess (NULL, 0, 0);
  g_test_trap_assert_failed ();
  g_test_trap_assert_stderr ("*already in the navigation stack*");
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
  g_test_add_func ("/Adwaita/NavigationSplitView/duplicate_tag", test_adw_navigation_split_view_duplicate_tag);
  g_test_add_func ("/Adwaita/NavigationSplitView/retag_sidebar", test_adw_navigation_split_view_retag_sidebar);
  g_test_add_func ("/Adwaita/NavigationSplitView/retag_content", test_adw_navigation_split_view_retag_content);
  g_test_add_func ("/Adwaita/NavigationSplitView/actions", test_adw_navigation_split_view_actions);
  g_test_add_func ("/Adwaita/NavigationSplitView/push_exiting", test_adw_navigation_split_view_push_exiting);
  g_test_add_func ("/Adwaita/NavigationSplitView/push_invalid", test_adw_navigation_split_view_push_invalid);
  g_test_add_func ("/Adwaita/NavigationSplitView/push_duplicate", test_adw_navigation_split_view_push_duplicate);

  return g_test_run ();
}
