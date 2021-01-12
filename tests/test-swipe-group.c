/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>

static void
test_adw_swipe_group_add_remove (void)
{
  g_autoptr (AdwSwipeGroup) group = NULL;
  g_autoptr (AdwSwipeable) swipeable1 = NULL;
  g_autoptr (AdwSwipeable) swipeable2 = NULL;

  group = adw_swipe_group_new ();

  swipeable1 = ADW_SWIPEABLE (adw_carousel_new ());
  swipeable2 = ADW_SWIPEABLE (adw_carousel_new ());

  g_assert_cmpint (g_slist_length (adw_swipe_group_get_swipeables (group)), ==, 0);

  adw_swipe_group_add_swipeable (group, swipeable1);
  g_assert_cmpint (g_slist_length (adw_swipe_group_get_swipeables (group)), ==, 1);

  adw_swipe_group_add_swipeable (group, swipeable2);
  g_assert_cmpint (g_slist_length (adw_swipe_group_get_swipeables (group)), ==, 2);

  adw_swipe_group_remove_swipeable (group, swipeable2);
  g_assert_cmpint (g_slist_length (adw_swipe_group_get_swipeables (group)), ==, 1);

  adw_swipe_group_remove_swipeable (group, swipeable1);
  g_assert_cmpint (g_slist_length (adw_swipe_group_get_swipeables (group)), ==, 0);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/SwipeGroup/add_remove", test_adw_swipe_group_add_remove);
  return g_test_run();
}
