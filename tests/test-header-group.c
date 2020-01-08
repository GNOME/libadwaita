/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_header_group_decorate_all (void)
{
  g_autoptr (HdyHeaderGroup) hg = HDY_HEADER_GROUP (hdy_header_group_new ());
  gboolean decorate_all = FALSE;

  g_assert_false (hdy_header_group_get_decorate_all (hg));
  g_object_get (hg, "decorate-all", &decorate_all, NULL);
  g_assert_false (decorate_all);

  hdy_header_group_set_decorate_all (hg, TRUE);

  g_assert_true (hdy_header_group_get_decorate_all (hg));
  g_object_get (hg, "decorate-all", &decorate_all, NULL);
  g_assert_true (decorate_all);

  g_object_set (hg, "decorate-all", FALSE, NULL);

  g_assert_false (hdy_header_group_get_decorate_all (hg));
  g_object_get (hg, "decorate-all", &decorate_all, NULL);
  g_assert_false (decorate_all);
}


static void
test_hdy_header_group_add_remove (void)
{
  g_autoptr (HdyHeaderGroup) hg = HDY_HEADER_GROUP (hdy_header_group_new ());
  g_autoptr (HdyHeaderBar) bar1 = HDY_HEADER_BAR (g_object_ref_sink (hdy_header_bar_new ()));
  g_autoptr (GtkHeaderBar) bar2 = GTK_HEADER_BAR (g_object_ref_sink (gtk_header_bar_new ()));

  g_assert_cmpint (g_slist_length (hdy_header_group_get_children (hg)), ==, 0);

  hdy_header_group_add_header_bar (hg, bar1);
  g_assert_cmpint (g_slist_length (hdy_header_group_get_children (hg)), ==, 1);

  hdy_header_group_add_gtk_header_bar (hg, bar2);
  g_assert_cmpint (g_slist_length (hdy_header_group_get_children (hg)), ==, 2);

  hdy_header_group_remove_gtk_header_bar (hg, bar2);
  g_assert_cmpint (g_slist_length (hdy_header_group_get_children (hg)), ==, 1);

  hdy_header_group_remove_header_bar (hg, bar1);

  g_assert_cmpint (g_slist_length (hdy_header_group_get_children (hg)), ==, 0);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/HeaderGroup/decorate_all", test_hdy_header_group_decorate_all);
  g_test_add_func("/Handy/HeaderGroup/add_remove", test_hdy_header_group_add_remove);
  return g_test_run();
}
