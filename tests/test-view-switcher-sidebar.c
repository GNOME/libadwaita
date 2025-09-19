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

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ViewSwitcherSidebar/stack", test_adw_view_switcher_sidebar_stack);
  g_test_add_func("/Adwaita/ViewSwitcherSidebar/mode", test_adw_view_switcher_sidebar_mode);

  return g_test_run();
}
