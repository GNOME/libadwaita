/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_view_switcher_policy (void)
{
  g_autoptr (HdyViewSwitcher) view_switcher = NULL;

  view_switcher = g_object_ref_sink (HDY_VIEW_SWITCHER (hdy_view_switcher_new ()));
  g_assert_nonnull (view_switcher);

  g_assert_cmpint (hdy_view_switcher_get_policy (view_switcher), ==, HDY_VIEW_SWITCHER_POLICY_AUTO);

  hdy_view_switcher_set_policy (view_switcher, HDY_VIEW_SWITCHER_POLICY_NARROW);
  g_assert_cmpint (hdy_view_switcher_get_policy (view_switcher), ==, HDY_VIEW_SWITCHER_POLICY_NARROW);

  hdy_view_switcher_set_policy (view_switcher, HDY_VIEW_SWITCHER_POLICY_WIDE);
  g_assert_cmpint (hdy_view_switcher_get_policy (view_switcher), ==, HDY_VIEW_SWITCHER_POLICY_WIDE);

  hdy_view_switcher_set_policy (view_switcher, HDY_VIEW_SWITCHER_POLICY_AUTO);
  g_assert_cmpint (hdy_view_switcher_get_policy (view_switcher), ==, HDY_VIEW_SWITCHER_POLICY_AUTO);
}


static void
test_hdy_view_switcher_narrow_ellipsize (void)
{
  g_autoptr (HdyViewSwitcher) view_switcher = NULL;

  view_switcher = g_object_ref_sink (HDY_VIEW_SWITCHER (hdy_view_switcher_new ()));
  g_assert_nonnull (view_switcher);

  g_assert_cmpint (hdy_view_switcher_get_narrow_ellipsize (view_switcher), ==, PANGO_ELLIPSIZE_NONE);

  hdy_view_switcher_set_narrow_ellipsize (view_switcher, PANGO_ELLIPSIZE_END);
  g_assert_cmpint (hdy_view_switcher_get_narrow_ellipsize (view_switcher), ==, PANGO_ELLIPSIZE_END);

  hdy_view_switcher_set_narrow_ellipsize (view_switcher, PANGO_ELLIPSIZE_NONE);
  g_assert_cmpint (hdy_view_switcher_get_narrow_ellipsize (view_switcher), ==, PANGO_ELLIPSIZE_NONE);
}


static void
test_hdy_view_switcher_stack (void)
{
  g_autoptr (HdyViewSwitcher) view_switcher = NULL;
  GtkStack *stack;

  view_switcher = g_object_ref_sink (HDY_VIEW_SWITCHER (hdy_view_switcher_new ()));
  g_assert_nonnull (view_switcher);

  stack = GTK_STACK (gtk_stack_new ());
  g_assert_nonnull (stack);

  g_assert_null (hdy_view_switcher_get_stack (view_switcher));

  hdy_view_switcher_set_stack (view_switcher, stack);
  g_assert (hdy_view_switcher_get_stack (view_switcher) == stack);

  hdy_view_switcher_set_stack (view_switcher, NULL);
  g_assert_null (hdy_view_switcher_get_stack (view_switcher));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/ViewSwitcher/policy", test_hdy_view_switcher_policy);
  g_test_add_func("/Handy/ViewSwitcher/narrow_ellipsize", test_hdy_view_switcher_narrow_ellipsize);
  g_test_add_func("/Handy/ViewSwitcher/stack", test_hdy_view_switcher_stack);

  return g_test_run();
}
