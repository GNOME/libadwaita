/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_view_switcher_policy (void)
{
  g_autoptr (AdwViewSwitcher) view_switcher = NULL;

  view_switcher = g_object_ref_sink (ADW_VIEW_SWITCHER (adw_view_switcher_new ()));
  g_assert_nonnull (view_switcher);

  g_assert_cmpint (adw_view_switcher_get_policy (view_switcher), ==, ADW_VIEW_SWITCHER_POLICY_AUTO);

  adw_view_switcher_set_policy (view_switcher, ADW_VIEW_SWITCHER_POLICY_NARROW);
  g_assert_cmpint (adw_view_switcher_get_policy (view_switcher), ==, ADW_VIEW_SWITCHER_POLICY_NARROW);

  adw_view_switcher_set_policy (view_switcher, ADW_VIEW_SWITCHER_POLICY_WIDE);
  g_assert_cmpint (adw_view_switcher_get_policy (view_switcher), ==, ADW_VIEW_SWITCHER_POLICY_WIDE);

  adw_view_switcher_set_policy (view_switcher, ADW_VIEW_SWITCHER_POLICY_AUTO);
  g_assert_cmpint (adw_view_switcher_get_policy (view_switcher), ==, ADW_VIEW_SWITCHER_POLICY_AUTO);
}


static void
test_adw_view_switcher_narrow_ellipsize (void)
{
  g_autoptr (AdwViewSwitcher) view_switcher = NULL;

  view_switcher = g_object_ref_sink (ADW_VIEW_SWITCHER (adw_view_switcher_new ()));
  g_assert_nonnull (view_switcher);

  g_assert_cmpint (adw_view_switcher_get_narrow_ellipsize (view_switcher), ==, PANGO_ELLIPSIZE_NONE);

  adw_view_switcher_set_narrow_ellipsize (view_switcher, PANGO_ELLIPSIZE_END);
  g_assert_cmpint (adw_view_switcher_get_narrow_ellipsize (view_switcher), ==, PANGO_ELLIPSIZE_END);

  adw_view_switcher_set_narrow_ellipsize (view_switcher, PANGO_ELLIPSIZE_NONE);
  g_assert_cmpint (adw_view_switcher_get_narrow_ellipsize (view_switcher), ==, PANGO_ELLIPSIZE_NONE);
}


static void
test_adw_view_switcher_stack (void)
{
  g_autoptr (AdwViewSwitcher) view_switcher = NULL;
  AdwViewStack *stack;

  view_switcher = g_object_ref_sink (ADW_VIEW_SWITCHER (adw_view_switcher_new ()));
  g_assert_nonnull (view_switcher);

  stack = ADW_VIEW_STACK (adw_view_stack_new ());
  g_assert_nonnull (stack);

  g_assert_null (adw_view_switcher_get_stack (view_switcher));

  adw_view_switcher_set_stack (view_switcher, stack);
  g_assert (adw_view_switcher_get_stack (view_switcher) == stack);

  adw_view_switcher_set_stack (view_switcher, NULL);
  g_assert_null (adw_view_switcher_get_stack (view_switcher));
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ViewSwitcher/policy", test_adw_view_switcher_policy);
  g_test_add_func("/Adwaita/ViewSwitcher/narrow_ellipsize", test_adw_view_switcher_narrow_ellipsize);
  g_test_add_func("/Adwaita/ViewSwitcher/stack", test_adw_view_switcher_stack);

  return g_test_run();
}
