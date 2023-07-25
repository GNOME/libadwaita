/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_inline_view_switcher_stack (void)
{
  AdwInlineViewSwitcher *switcher = g_object_ref_sink (ADW_INLINE_VIEW_SWITCHER (adw_inline_view_switcher_new ()));
  AdwViewStack *stack = g_object_ref_sink (ADW_VIEW_STACK (adw_view_stack_new ()));
  int notified = 0;

  g_assert_nonnull (switcher);

  g_signal_connect_swapped (switcher, "notify::stack", G_CALLBACK (increment), &notified);

  adw_view_stack_add_titled (stack, adw_bin_new (), "first", "First");
  adw_view_stack_add_titled (stack, adw_bin_new (), "second", "Second");
  adw_view_stack_add_titled (stack, adw_bin_new (), "third", "Third");

  adw_view_stack_set_visible_child_name (stack, "first");

  adw_inline_view_switcher_set_stack (switcher, NULL);
  g_assert_null (adw_inline_view_switcher_get_stack (switcher));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (switcher, "stack", stack, NULL);
  g_assert_true (adw_inline_view_switcher_get_stack (switcher) == stack);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (switcher, "stack", NULL, NULL);
  g_assert_null (adw_inline_view_switcher_get_stack (switcher));
  g_assert_cmpint (notified, ==, 2);

  adw_inline_view_switcher_set_stack (switcher, stack);
  g_assert_true (adw_inline_view_switcher_get_stack (switcher) == stack);
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (switcher);
  g_assert_finalize_object (stack);
}

static void
test_adw_inline_view_switcher_display_mode (void)
{
  AdwInlineViewSwitcher *switcher = g_object_ref_sink (ADW_INLINE_VIEW_SWITCHER (adw_inline_view_switcher_new ()));
  AdwInlineViewSwitcherDisplayMode mode;
  int notified = 0;

  g_assert_nonnull (switcher);

  g_signal_connect_swapped (switcher, "notify::display-mode", G_CALLBACK (increment), &notified);

  g_object_get (switcher, "display-mode", &mode, NULL);
  g_assert_cmpint (mode, ==, ADW_INLINE_VIEW_SWITCHER_LABELS);

  adw_inline_view_switcher_set_display_mode (switcher, ADW_INLINE_VIEW_SWITCHER_LABELS);
  g_assert_cmpint (adw_inline_view_switcher_get_display_mode (switcher), ==, ADW_INLINE_VIEW_SWITCHER_LABELS);
  g_assert_cmpint (notified, ==, 0);

  g_object_set (switcher, "display-mode", ADW_INLINE_VIEW_SWITCHER_ICONS, NULL);
  g_assert_cmpint (adw_inline_view_switcher_get_display_mode (switcher), ==, ADW_INLINE_VIEW_SWITCHER_ICONS);
  g_assert_cmpint (notified, ==, 1);

  adw_inline_view_switcher_set_display_mode (switcher, ADW_INLINE_VIEW_SWITCHER_BOTH);
  g_object_get (switcher, "display-mode", &mode, NULL);
  g_assert_cmpint (mode, ==, ADW_INLINE_VIEW_SWITCHER_BOTH);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (switcher);
}

static void
test_adw_inline_view_switcher_homogeneous (void)
{
  AdwInlineViewSwitcher *switcher = g_object_ref_sink (ADW_INLINE_VIEW_SWITCHER (adw_inline_view_switcher_new ()));
  gboolean homogeneous;
  int notified = 0;

  g_assert_nonnull (switcher);

  g_signal_connect_swapped (switcher, "notify::homogeneous", G_CALLBACK (increment), &notified);

  g_object_get (switcher, "homogeneous", &homogeneous, NULL);
  g_assert_false (homogeneous);

  adw_inline_view_switcher_set_homogeneous (switcher, FALSE);
  g_assert_false (adw_inline_view_switcher_get_homogeneous (switcher));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (switcher, "homogeneous", TRUE, NULL);
  g_assert_true (adw_inline_view_switcher_get_homogeneous (switcher));
  g_assert_cmpint (notified, ==, 1);

  adw_inline_view_switcher_set_homogeneous (switcher, FALSE);
  g_object_get (switcher, "homogeneous", &homogeneous, NULL);
  g_assert_false (homogeneous);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (switcher);
}

static void
test_adw_inline_view_switcher_can_shrink (void)
{
  AdwInlineViewSwitcher *switcher = g_object_ref_sink (ADW_INLINE_VIEW_SWITCHER (adw_inline_view_switcher_new ()));
  gboolean can_shrink;
  int notified = 0;

  g_assert_nonnull (switcher);

  g_signal_connect_swapped (switcher, "notify::can-shrink", G_CALLBACK (increment), &notified);

  g_object_get (switcher, "can-shrink", &can_shrink, NULL);
  g_assert_true (can_shrink);

  adw_inline_view_switcher_set_can_shrink (switcher, TRUE);
  g_assert_true (adw_inline_view_switcher_get_can_shrink (switcher));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (switcher, "can-shrink", FALSE, NULL);
  g_assert_false (adw_inline_view_switcher_get_can_shrink (switcher));
  g_assert_cmpint (notified, ==, 1);

  adw_inline_view_switcher_set_can_shrink (switcher, TRUE);
  g_object_get (switcher, "can-shrink", &can_shrink, NULL);
  g_assert_true (can_shrink);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (switcher);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/InlineViewSwitcher/stack", test_adw_inline_view_switcher_stack);
  g_test_add_func ("/Adwaita/InlineViewSwitcher/display_mode", test_adw_inline_view_switcher_display_mode);
  g_test_add_func ("/Adwaita/InlineViewSwitcher/homogeneous", test_adw_inline_view_switcher_homogeneous);
  g_test_add_func ("/Adwaita/InlineViewSwitcher/can_shrink", test_adw_inline_view_switcher_can_shrink);

  return g_test_run ();
}
