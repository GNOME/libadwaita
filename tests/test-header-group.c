/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


static void
test_hdy_header_group_focus (void)
{
  HdyHeaderGroup *hg;
  GtkHeaderBar *bar1, *bar2 = NULL;

  hg = HDY_HEADER_GROUP (hdy_header_group_new ());

  bar1 = hdy_header_group_get_focus (hg);
  g_assert_null (bar1);
  g_object_get (hg, "focus", &bar2, NULL);
  g_assert (bar1 == bar2);

  bar1 = GTK_HEADER_BAR (gtk_header_bar_new ());
  hdy_header_group_add_header_bar (hg, GTK_HEADER_BAR (bar1));
  hdy_header_group_set_focus (hg, GTK_HEADER_BAR (bar1));
  bar2 = hdy_header_group_get_focus (hg);
  g_assert (bar1 == bar2);
  g_object_get (hg, "focus", &bar2, NULL);
  g_assert (bar1 == bar2);

  g_object_unref (hg);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);

  g_test_add_func("/Handy/HeaderGroup/focus", test_hdy_header_group_focus);
  return g_test_run();
}
