/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_view_switcher_bar_policy (void)
{
  g_autoptr (AdwViewSwitcherBar) bar = NULL;

  bar = g_object_ref_sink (ADW_VIEW_SWITCHER_BAR (adw_view_switcher_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_cmpint (adw_view_switcher_bar_get_policy (bar), ==, ADW_VIEW_SWITCHER_POLICY_NARROW);

  adw_view_switcher_bar_set_policy (bar, ADW_VIEW_SWITCHER_POLICY_AUTO);
  g_assert_cmpint (adw_view_switcher_bar_get_policy (bar), ==, ADW_VIEW_SWITCHER_POLICY_AUTO);

  adw_view_switcher_bar_set_policy (bar, ADW_VIEW_SWITCHER_POLICY_WIDE);
  g_assert_cmpint (adw_view_switcher_bar_get_policy (bar), ==, ADW_VIEW_SWITCHER_POLICY_WIDE);

  adw_view_switcher_bar_set_policy (bar, ADW_VIEW_SWITCHER_POLICY_NARROW);
  g_assert_cmpint (adw_view_switcher_bar_get_policy (bar), ==, ADW_VIEW_SWITCHER_POLICY_NARROW);
}


static void
test_adw_view_switcher_bar_stack (void)
{
  g_autoptr (AdwViewSwitcherBar) bar = NULL;
  GtkStack *stack;

  bar = g_object_ref_sink (ADW_VIEW_SWITCHER_BAR (adw_view_switcher_bar_new ()));
  g_assert_nonnull (bar);

  stack = GTK_STACK (gtk_stack_new ());
  g_assert_nonnull (stack);

  g_assert_null (adw_view_switcher_bar_get_stack (bar));

  adw_view_switcher_bar_set_stack (bar, stack);
  g_assert (adw_view_switcher_bar_get_stack (bar) == stack);

  adw_view_switcher_bar_set_stack (bar, NULL);
  g_assert_null (adw_view_switcher_bar_get_stack (bar));
}


static void
test_adw_view_switcher_bar_reveal (void)
{
  g_autoptr (AdwViewSwitcherBar) bar = NULL;

  bar = g_object_ref_sink (ADW_VIEW_SWITCHER_BAR (adw_view_switcher_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_false (adw_view_switcher_bar_get_reveal (bar));

  adw_view_switcher_bar_set_reveal (bar, TRUE);
  g_assert_true (adw_view_switcher_bar_get_reveal (bar));

  adw_view_switcher_bar_set_reveal (bar, FALSE);
  g_assert_false (adw_view_switcher_bar_get_reveal (bar));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ViewSwitcherBar/policy", test_adw_view_switcher_bar_policy);
  g_test_add_func("/Adwaita/ViewSwitcherBar/stack", test_adw_view_switcher_bar_stack);
  g_test_add_func("/Adwaita/ViewSwitcherBar/reveal", test_adw_view_switcher_bar_reveal);

  return g_test_run();
}
