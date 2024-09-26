/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

#include "adw-back-button-private.h"

static void
check_history (AdwBackButton *button,
               int            n_pages,
               ...)
{
  GPtrArray *history = adw_back_button_gather_navigation_history (button);
  va_list args;
  guint i;

  g_assert_cmpint (history->len, ==, n_pages);

  va_start (args, n_pages);

  for (i = 0; i < n_pages; i++) {
    AdwNavigationPage *page = g_ptr_array_index (history, i);
    const char *tag = va_arg (args, const char *);

    g_assert_cmpstr (adw_navigation_page_get_tag (page), ==, tag);
  }

  va_end (args);

  g_ptr_array_unref (history);
}

static inline void
push_page (AdwNavigationView *view,
           const char        *tag,
           GtkWidget         *child)
{
  if (!child)
    child = adw_bin_new ();

  adw_navigation_view_push (view, adw_navigation_page_new_with_tag (child, tag, tag));
}

static inline void
set_sidebar (AdwNavigationSplitView *view,
             const char             *tag,
             GtkWidget              *child)
{
  if (!child)
    child = adw_bin_new ();

  adw_navigation_split_view_set_sidebar (view, adw_navigation_page_new_with_tag (child, tag, tag));
}

static inline void
set_content (AdwNavigationSplitView *view,
             const char             *tag,
             GtkWidget              *child)
{
  if (!child)
    child = adw_bin_new ();

  adw_navigation_split_view_set_content (view, adw_navigation_page_new_with_tag (child, tag, tag));
}

static void
test_adw_back_button_test_simple (void)
{
  AdwNavigationView *view = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  push_page (view, "page1", NULL);
  push_page (view, "page2", NULL);
  push_page (view, "page3", GTK_WIDGET (button));

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (view));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * view
   *   page1
   *   page2
   *   page3: back button
   */

  check_history (button, 2, "page2", "page1");

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_nested (void)
{
  AdwNavigationView *view = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwNavigationView *view1 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwNavigationView *view2 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwNavigationView *view3 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  push_page (view1, "page11", NULL);
  push_page (view1, "page12", NULL);
  push_page (view1, "page13", NULL);

  push_page (view2, "page21", NULL);
  push_page (view2, "page22", NULL);
  push_page (view2, "page23", GTK_WIDGET (view3));

  push_page (view2, "page31", NULL);
  push_page (view2, "page32", NULL);
  push_page (view2, "page33", GTK_WIDGET (button));

  push_page (view, "page1", GTK_WIDGET (view1));
  push_page (view, "page2", GTK_WIDGET (view2));

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (view));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * view
   *   page1: view1
   *     page11
   *     page12
   *     page13
   *   page2: view2
   *     page21
   *     page22
   *     page23: view3
   *       page31
   *       page32
   *       page33: back button
   */

  check_history (button, 7,
                 "page32", "page31",
                 "page22", "page21",
                 "page13", "page12", "page11");

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_split_view_simple (void)
{
  AdwNavigationSplitView *split_view = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  adw_navigation_split_view_set_show_content (split_view, TRUE);

  set_sidebar (split_view, "sidebar", NULL);
  set_content (split_view, "content", GTK_WIDGET (button));

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (split_view));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * split_view
   *   sidebar
   *   ---
   *   content: back button
   */

  check_history (button, 0);

  /*
   * split_view
   *   sidebar
   *   content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  check_history (button, 1, "sidebar");

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_split_view_inverted (void)
{
  AdwNavigationSplitView *split_view = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  adw_navigation_split_view_set_sidebar_position (split_view, GTK_PACK_END);

  set_sidebar (split_view, "sidebar", GTK_WIDGET (button));
  set_content (split_view, "content", NULL);

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (split_view));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * split_view
   *   content
   *   ---
   *   sidebar: back button
   */

  check_history (button, 0);

  /*
   * split_view
   *   content
   *   sidebar: back button
   */

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  check_history (button, 1, "content");

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_split_view_nested_sidebar (void)
{
  AdwNavigationSplitView *split_view1 = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwNavigationSplitView *split_view2 = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  adw_navigation_split_view_set_show_content (split_view1, TRUE);
  adw_navigation_split_view_set_show_content (split_view2, TRUE);

  set_sidebar (split_view1, "outer-sidebar", GTK_WIDGET (split_view2));
  set_content (split_view1, "outer-content", GTK_WIDGET (button));

  set_sidebar (split_view2, "inner-sidebar", NULL);
  set_content (split_view2, "inner-content", NULL);

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (split_view1));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * split_view1
   *   outer-sidebar: split_view2
   *     inner-sidebar
   *     ---
   *     inner-content
   *   ---
   *   outer-content: back button
   */

  check_history (button, 0);

  /*
   * split_view1
   *   outer-sidebar: split_view2
   *     inner-sidebar
   *     ---
   *     inner-content
   *   outer-content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view1, TRUE);
  check_history (button, 1, "outer-sidebar");

  /*
   * split_view1
   *   outer-sidebar: split_view2
   *     inner-sidebar
   *     inner-content
   *   outer-content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view2, TRUE);
  check_history (button, 2, "inner-content", "inner-sidebar");

  /*
   * split_view1
   *   outer-sidebar: split_view2
   *     inner-sidebar
   *     inner-content
   *   ---
   *   outer-content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view1, FALSE);
  check_history (button, 0);

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_split_view_nested_content (void)
{
  AdwNavigationSplitView *split_view1 = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwNavigationSplitView *split_view2 = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  adw_navigation_split_view_set_show_content (split_view1, TRUE);
  adw_navigation_split_view_set_show_content (split_view2, TRUE);

  set_sidebar (split_view1, "outer-sidebar", NULL);
  set_content (split_view1, "outer-content", GTK_WIDGET (split_view2));

  set_sidebar (split_view2, "inner-sidebar", NULL);
  set_content (split_view2, "inner-content", GTK_WIDGET (button));

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (split_view1));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * split_view1
   *   outer-sidebar
   *   ---
   *   outer-content: split_view2
   *     inner-sidebar
   *     ---
   *     inner-content: back button
   */

  check_history (button, 0);

  /*
   * split_view1
   *   outer-sidebar
   *   outer-content: split_view2
   *     inner-sidebar
   *     ---
   *     inner-content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view1, TRUE);
  check_history (button, 1, "outer-sidebar");

  /*
   * split_view1
   *   outer-sidebar
   *   outer-content: split_view2
   *     inner-sidebar
   *     inner-content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view2, TRUE);
  check_history (button, 2, "inner-sidebar", "outer-sidebar");

  /*
   * split_view1
   *   outer-sidebar
   *   ---
   *   outer-content: split_view2
   *     inner-sidebar
   *     inner-content: back button
   */

  adw_navigation_split_view_set_collapsed (split_view1, FALSE);
  check_history (button, 1, "inner-sidebar");

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_split_view_with_nav_views (void)
{
  AdwNavigationSplitView *split_view = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwNavigationView *view1 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwNavigationView *view2 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  adw_navigation_split_view_set_show_content (split_view, TRUE);

  set_sidebar (split_view, "sidebar", GTK_WIDGET (view1));
  set_content (split_view, "content", GTK_WIDGET (view2));

  push_page (view1, "sidebar1", NULL);
  push_page (view1, "sidebar2", NULL);

  push_page (view2, "content1", NULL);
  push_page (view2, "content2", GTK_WIDGET (button));

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (split_view));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * split_view
   *   sidebar: view1
   *     sidebar1
   *     sidebar2
   *   ---
   *   content: view2
   *     content1
   *     content2: back button
   */

  check_history (button, 1, "content1");

  /*
   * split_view
   *   sidebar: view1
   *     sidebar1
   *     sidebar2
   *   content: view2
   *     content1
   *     content2: back button
   */

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  check_history (button, 3, "content1", "sidebar2", "sidebar1");

  g_assert_finalize_object (window);
}

static void
test_adw_back_button_test_split_view_inside_nav_view (void)
{
  AdwNavigationView *view1 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwNavigationSplitView *split_view = ADW_NAVIGATION_SPLIT_VIEW (adw_navigation_split_view_new ());
  AdwNavigationView *view2 = ADW_NAVIGATION_VIEW (adw_navigation_view_new ());
  AdwBackButton *button = ADW_BACK_BUTTON (adw_back_button_new ());
  GtkWidget *window = gtk_window_new ();

  adw_navigation_split_view_set_show_content (split_view, TRUE);

  push_page (view1, "split-view", GTK_WIDGET (split_view));

  set_sidebar (split_view, "sidebar", NULL);
  set_content (split_view, "content", GTK_WIDGET (view2));

  push_page (view2, "content1", GTK_WIDGET (button));

  gtk_window_set_child (GTK_WINDOW (window), GTK_WIDGET (view1));
  gtk_window_present (GTK_WINDOW (window));

  /*
   * view1
   *   split-view: split_view
   *     sidebar
   *     ---
   *     content: view2
   *       content1: back button
   */

  check_history (button, 0);

  /*
   * view1
   *   split-view: split_view
   *     sidebar
   *     content: view2
   *       content1: back button
   */

  adw_navigation_split_view_set_collapsed (split_view, TRUE);
  check_history (button, 1, "sidebar");

  g_assert_finalize_object (window);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/BackButton/simple", test_adw_back_button_test_simple);
  g_test_add_func ("/Adwaita/BackButton/nested", test_adw_back_button_test_nested);
  g_test_add_func ("/Adwaita/BackButton/split_view_simple", test_adw_back_button_test_split_view_simple);
  g_test_add_func ("/Adwaita/BackButton/split_view_inverted", test_adw_back_button_test_split_view_inverted);
  g_test_add_func ("/Adwaita/BackButton/split_view_nested_sidebar", test_adw_back_button_test_split_view_nested_sidebar);
  g_test_add_func ("/Adwaita/BackButton/split_view_nested_content", test_adw_back_button_test_split_view_nested_content);
  g_test_add_func ("/Adwaita/BackButton/split_view_with_nav_views", test_adw_back_button_test_split_view_with_nav_views);
  g_test_add_func ("/Adwaita/BackButton/split_view_inside_nav_view", test_adw_back_button_test_split_view_inside_nav_view);

  return g_test_run ();
}
