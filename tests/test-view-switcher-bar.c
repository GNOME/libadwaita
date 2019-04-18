/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


static void
test_hdy_view_switcher_bar_policy (void)
{
  g_autoptr (HdyViewSwitcherBar) bar = NULL;

  bar = g_object_ref_sink (hdy_view_switcher_bar_new ());
  g_assert_nonnull (bar);

  g_assert_cmpint (hdy_view_switcher_bar_get_policy (bar), ==, HDY_VIEW_SWITCHER_POLICY_NARROW);

  hdy_view_switcher_bar_set_policy (bar, HDY_VIEW_SWITCHER_POLICY_AUTO);
  g_assert_cmpint (hdy_view_switcher_bar_get_policy (bar), ==, HDY_VIEW_SWITCHER_POLICY_AUTO);

  hdy_view_switcher_bar_set_policy (bar, HDY_VIEW_SWITCHER_POLICY_WIDE);
  g_assert_cmpint (hdy_view_switcher_bar_get_policy (bar), ==, HDY_VIEW_SWITCHER_POLICY_WIDE);

  hdy_view_switcher_bar_set_policy (bar, HDY_VIEW_SWITCHER_POLICY_NARROW);
  g_assert_cmpint (hdy_view_switcher_bar_get_policy (bar), ==, HDY_VIEW_SWITCHER_POLICY_NARROW);
}


static void
test_hdy_view_switcher_bar_icon_size (void)
{
  g_autoptr (HdyViewSwitcherBar) bar = NULL;

  bar = g_object_ref_sink (hdy_view_switcher_bar_new ());
  g_assert_nonnull (bar);

  g_assert_cmpint (hdy_view_switcher_bar_get_icon_size (bar), ==, GTK_ICON_SIZE_BUTTON);

  hdy_view_switcher_bar_set_icon_size (bar, GTK_ICON_SIZE_MENU);
  g_assert_cmpint (hdy_view_switcher_bar_get_icon_size (bar), ==, GTK_ICON_SIZE_MENU);

  hdy_view_switcher_bar_set_icon_size (bar, GTK_ICON_SIZE_BUTTON);
  g_assert_cmpint (hdy_view_switcher_bar_get_icon_size (bar), ==, GTK_ICON_SIZE_BUTTON);
}


static void
test_hdy_view_switcher_bar_stack (void)
{
  g_autoptr (HdyViewSwitcherBar) bar = NULL;
  GtkStack *stack;

  bar = g_object_ref_sink (hdy_view_switcher_bar_new ());
  g_assert_nonnull (bar);

  stack = GTK_STACK (gtk_stack_new ());
  g_assert_nonnull (stack);

  g_assert_null (hdy_view_switcher_bar_get_stack (bar));

  hdy_view_switcher_bar_set_stack (bar, stack);
  g_assert (hdy_view_switcher_bar_get_stack (bar) == stack);

  hdy_view_switcher_bar_set_stack (bar, NULL);
  g_assert_null (hdy_view_switcher_bar_get_stack (bar));
}


static void
test_hdy_view_switcher_bar_reveal (void)
{
  g_autoptr (HdyViewSwitcherBar) bar = NULL;

  bar = g_object_ref_sink (hdy_view_switcher_bar_new ());
  g_assert_nonnull (bar);

  g_assert_false (hdy_view_switcher_bar_get_reveal (bar));

  hdy_view_switcher_bar_set_reveal (bar, TRUE);
  g_assert_true (hdy_view_switcher_bar_get_reveal (bar));

  hdy_view_switcher_bar_set_reveal (bar, FALSE);
  g_assert_false (hdy_view_switcher_bar_get_reveal (bar));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/ViewSwitcherBar/policy", test_hdy_view_switcher_bar_policy);
  g_test_add_func("/Handy/ViewSwitcherBar/icon_size", test_hdy_view_switcher_bar_icon_size);
  g_test_add_func("/Handy/ViewSwitcherBar/stack", test_hdy_view_switcher_bar_stack);
  g_test_add_func("/Handy/ViewSwitcherBar/reveal", test_hdy_view_switcher_bar_reveal);

  return g_test_run();
}
