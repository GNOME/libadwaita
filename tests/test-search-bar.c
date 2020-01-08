/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_search_bar_add (void)
{
  g_autoptr (HdySearchBar) bar = NULL;
  GtkWidget *entry;

  bar = g_object_ref_sink (HDY_SEARCH_BAR (hdy_search_bar_new ()));
  g_assert_nonnull (bar);

  entry = gtk_entry_new ();
  g_assert_nonnull (entry);

  gtk_container_add (GTK_CONTAINER (bar), entry);
}


static void
test_hdy_search_bar_connect_entry (void)
{
  g_autoptr (HdySearchBar) bar = NULL;
  GtkWidget *box, *entry;

  bar = g_object_ref_sink (HDY_SEARCH_BAR (hdy_search_bar_new ()));
  g_assert_nonnull (bar);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  g_assert_nonnull (box);

  entry = gtk_entry_new ();
  g_assert_nonnull (entry);

  gtk_container_add (GTK_CONTAINER (box), entry);
  gtk_container_add (GTK_CONTAINER (bar), box);
  hdy_search_bar_connect_entry (bar, GTK_ENTRY (entry));
}


static void
test_hdy_search_bar_search_mode (void)
{
  g_autoptr (HdySearchBar) bar = NULL;

  bar = g_object_ref_sink (HDY_SEARCH_BAR (hdy_search_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_false (hdy_search_bar_get_search_mode (bar));

  hdy_search_bar_set_search_mode (bar, TRUE);
  g_assert_true (hdy_search_bar_get_search_mode (bar));

  hdy_search_bar_set_search_mode (bar, FALSE);
  g_assert_false (hdy_search_bar_get_search_mode (bar));
}


static void
test_hdy_search_bar_show_close_button (void)
{
  g_autoptr (HdySearchBar) bar = NULL;

  bar = g_object_ref_sink (HDY_SEARCH_BAR (hdy_search_bar_new ()));
  g_assert_nonnull (bar);

  g_assert_false (hdy_search_bar_get_show_close_button (bar));

  hdy_search_bar_set_show_close_button (bar, TRUE);
  g_assert_true (hdy_search_bar_get_show_close_button (bar));

  hdy_search_bar_set_show_close_button (bar, FALSE);
  g_assert_false (hdy_search_bar_get_show_close_button (bar));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/SearchBar/add", test_hdy_search_bar_add);
  g_test_add_func("/Handy/SearchBar/connect_entry", test_hdy_search_bar_connect_entry);
  g_test_add_func("/Handy/SearchBar/search_mode", test_hdy_search_bar_search_mode);
  g_test_add_func("/Handy/SearchBar/show_close_button", test_hdy_search_bar_show_close_button);

  return g_test_run();
}
