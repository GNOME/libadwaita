/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

static void
test_hdy_swipe_group_add_remove (void)
{
  g_autoptr (HdySwipeGroup) group = NULL;
  g_autoptr (HdySwipeable) swipeable1 = NULL;
  g_autoptr (HdySwipeable) swipeable2 = NULL;

  group = hdy_swipe_group_new ();

  swipeable1 = HDY_SWIPEABLE (hdy_paginator_new ());
  swipeable2 = HDY_SWIPEABLE (hdy_paginator_new ());

  g_assert_cmpint (g_slist_length (hdy_swipe_group_get_swipeables (group)), ==, 0);

  hdy_swipe_group_add_swipeable (group, swipeable1);
  g_assert_cmpint (g_slist_length (hdy_swipe_group_get_swipeables (group)), ==, 1);

  hdy_swipe_group_add_swipeable (group, swipeable2);
  g_assert_cmpint (g_slist_length (hdy_swipe_group_get_swipeables (group)), ==, 2);

  hdy_swipe_group_remove_swipeable (group, swipeable2);
  g_assert_cmpint (g_slist_length (hdy_swipe_group_get_swipeables (group)), ==, 1);

  hdy_swipe_group_remove_swipeable (group, swipeable1);
  g_assert_cmpint (g_slist_length (hdy_swipe_group_get_swipeables (group)), ==, 0);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);

  g_test_add_func("/Handy/SwipeGroup/add_remove", test_hdy_swipe_group_add_remove);
  return g_test_run();
}
