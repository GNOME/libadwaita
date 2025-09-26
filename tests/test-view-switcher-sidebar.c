/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_view_switcher_sidebar_stack (void)
{
  AdwViewSwitcherSidebar *sidebar = g_object_ref_sink (ADW_VIEW_SWITCHER_SIDEBAR (adw_view_switcher_sidebar_new ()));
  AdwViewStack *stack = g_object_ref_sink (ADW_VIEW_STACK (adw_view_stack_new ()));
  int notified = 0;

  g_assert_nonnull (sidebar);
  g_assert_nonnull (stack);

  g_signal_connect_swapped (sidebar, "notify::stack", G_CALLBACK (increment), &notified);

  g_assert_null (adw_view_switcher_sidebar_get_stack (sidebar));

  adw_view_switcher_sidebar_set_stack (sidebar, stack);
  g_assert (adw_view_switcher_sidebar_get_stack (sidebar) == stack);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "stack", NULL, NULL);
  g_assert_null (adw_view_switcher_sidebar_get_stack (sidebar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (stack);
}

static void
test_adw_view_switcher_sidebar_mode (void)
{
  AdwViewSwitcherSidebar *sidebar = g_object_ref_sink (ADW_VIEW_SWITCHER_SIDEBAR (adw_view_switcher_sidebar_new ()));
  AdwSidebarMode mode;
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::mode", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "mode", &mode, NULL);
  g_assert_cmpint (mode, ==, ADW_SIDEBAR_MODE_SIDEBAR);

  adw_view_switcher_sidebar_set_mode (sidebar, ADW_SIDEBAR_MODE_PAGE);
  g_assert_cmpint (adw_view_switcher_sidebar_get_mode (sidebar), ==, ADW_SIDEBAR_MODE_PAGE);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "mode", ADW_SIDEBAR_MODE_SIDEBAR, NULL);
  g_assert_cmpint (adw_view_switcher_sidebar_get_mode (sidebar), ==, ADW_SIDEBAR_MODE_SIDEBAR);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
}

static void
test_adw_view_switcher_sidebar_filter (void)
{
  AdwViewSwitcherSidebar *sidebar = g_object_ref_sink (ADW_VIEW_SWITCHER_SIDEBAR (adw_view_switcher_sidebar_new ()));
  GtkFilter *filter = NULL;
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::filter", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "filter", &filter, NULL);
  g_assert_null (filter);

  adw_view_switcher_sidebar_set_filter (sidebar, NULL);
  g_assert_cmpint (notified, ==, 0);

  filter = GTK_FILTER (gtk_bool_filter_new (NULL));
  adw_view_switcher_sidebar_set_filter (sidebar, filter);
  g_assert_true (adw_view_switcher_sidebar_get_filter (sidebar) == filter);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "filter", NULL, NULL);
  g_assert_null (adw_view_switcher_sidebar_get_filter (sidebar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (filter);
}

static void
test_adw_view_switcher_sidebar_placeholder (void)
{
  AdwViewSwitcherSidebar *sidebar = g_object_ref_sink (ADW_VIEW_SWITCHER_SIDEBAR (adw_view_switcher_sidebar_new ()));
  GtkWidget *placeholder = NULL;
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::placeholder", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "placeholder", &placeholder, NULL);
  g_assert_null (placeholder);

  adw_view_switcher_sidebar_set_placeholder (sidebar, NULL);
  g_assert_cmpint (notified, ==, 0);

  placeholder = gtk_button_new ();
  adw_view_switcher_sidebar_set_placeholder (sidebar, placeholder);
  g_assert_true (adw_view_switcher_sidebar_get_placeholder (sidebar) == placeholder);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "placeholder", NULL, NULL);
  g_assert_null (adw_view_switcher_sidebar_get_placeholder (sidebar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ViewSwitcherSidebar/stack", test_adw_view_switcher_sidebar_stack);
  g_test_add_func("/Adwaita/ViewSwitcherSidebar/mode", test_adw_view_switcher_sidebar_mode);
  g_test_add_func("/Adwaita/ViewSwitcherSidebar/filter", test_adw_view_switcher_sidebar_filter);
  g_test_add_func("/Adwaita/ViewSwitcherSidebar/placeholder", test_adw_view_switcher_sidebar_placeholder);

  return g_test_run();
}
