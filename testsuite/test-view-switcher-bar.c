/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
test_adw_view_switcher_bar_stack (void)
{
  AdwViewSwitcherBar *bar = g_object_ref_sink (ADW_VIEW_SWITCHER_BAR (adw_view_switcher_bar_new ()));
  AdwViewStack *stack = g_object_ref_sink (ADW_VIEW_STACK (adw_view_stack_new ()));

  g_assert_nonnull (bar);
  g_assert_nonnull (stack);

  g_assert_null (adw_view_switcher_bar_get_stack (bar));

  adw_view_switcher_bar_set_stack (bar, stack);
  g_assert (adw_view_switcher_bar_get_stack (bar) == stack);

  adw_view_switcher_bar_set_stack (bar, NULL);
  g_assert_null (adw_view_switcher_bar_get_stack (bar));

  g_assert_finalize_object (bar);
  g_assert_finalize_object (stack);
}


static void
test_adw_view_switcher_bar_reveal (void)
{
  AdwViewSwitcherBar *bar = g_object_ref_sink (ADW_VIEW_SWITCHER_BAR (adw_view_switcher_bar_new ()));

  g_assert_nonnull (bar);

  g_assert_false (adw_view_switcher_bar_get_reveal (bar));

  adw_view_switcher_bar_set_reveal (bar, TRUE);
  g_assert_true (adw_view_switcher_bar_get_reveal (bar));

  adw_view_switcher_bar_set_reveal (bar, FALSE);
  g_assert_false (adw_view_switcher_bar_get_reveal (bar));

  g_assert_finalize_object (bar);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ViewSwitcherBar/stack", test_adw_view_switcher_bar_stack);
  g_test_add_func("/Adwaita/ViewSwitcherBar/reveal", test_adw_view_switcher_bar_reveal);

  return g_test_run();
}
