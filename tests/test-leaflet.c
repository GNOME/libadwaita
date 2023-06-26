/*
 * Copyright (C) 2020 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
assert_page_position (GtkSelectionModel *pages,
                      GtkWidget         *widget,
                      int                position)
{
  AdwLeafletPage *page = g_list_model_get_item (G_LIST_MODEL (pages), position);

  g_assert_true (widget == adw_leaflet_page_get_child (page));

  g_object_unref (page);
}


static void
test_adw_leaflet_adjacent_child (void)
{
  AdwLeaflet *leaflet = g_object_ref_sink (ADW_LEAFLET (adw_leaflet_new ()));
  GtkWidget *children[3];
  int i;
  GtkWidget *result;

  g_assert_nonnull (leaflet);

  for (i = 0; i < 3; i++) {
    AdwLeafletPage *page;

    children[i] = gtk_label_new ("");
    g_assert_nonnull (children[i]);

    page = adw_leaflet_append (leaflet, children[i]);

    if (i == 1)
      adw_leaflet_page_set_navigatable (page, FALSE);
  }

  adw_leaflet_set_visible_child (leaflet, children[0]);

  result = adw_leaflet_get_adjacent_child (leaflet, ADW_NAVIGATION_DIRECTION_BACK);
  g_assert_null (result);

  result = adw_leaflet_get_adjacent_child (leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result == children[2]);

  adw_leaflet_set_visible_child (leaflet, children[1]);

  result = adw_leaflet_get_adjacent_child (leaflet, ADW_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result == children[0]);

  result = adw_leaflet_get_adjacent_child (leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result == children[2]);

  adw_leaflet_set_visible_child (leaflet, children[2]);

  result = adw_leaflet_get_adjacent_child (leaflet, ADW_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result == children[0]);

  result = adw_leaflet_get_adjacent_child (leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
  g_assert_null (result);

  g_assert_finalize_object (leaflet);
}


static void
test_adw_leaflet_navigate (void)
{
  AdwLeaflet *leaflet = g_object_ref_sink (ADW_LEAFLET (adw_leaflet_new ()));
  GtkWidget *children[3];
  int i;
  gboolean result;

  g_assert_nonnull (leaflet);

  result = adw_leaflet_navigate (leaflet, ADW_NAVIGATION_DIRECTION_BACK);
  g_assert_false (result);

  result = adw_leaflet_navigate (leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
  g_assert_false (result);

  for (i = 0; i < 3; i++) {
    AdwLeafletPage *page;

    children[i] = gtk_label_new ("");
    g_assert_nonnull (children[i]);

    page = adw_leaflet_append (leaflet, children[i]);

    if (i == 1)
      adw_leaflet_page_set_navigatable (page, FALSE);
  }

  adw_leaflet_set_visible_child (leaflet, children[0]);

  result = adw_leaflet_navigate (leaflet, ADW_NAVIGATION_DIRECTION_BACK);
  g_assert_false (result);

  result = adw_leaflet_navigate (leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result);
  g_assert_true (adw_leaflet_get_visible_child (leaflet) == children[2]);

  result = adw_leaflet_navigate (leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
  g_assert_false (result);

  result = adw_leaflet_navigate (leaflet, ADW_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result);
  g_assert_true (adw_leaflet_get_visible_child (leaflet) == children[0]);

  g_assert_finalize_object (leaflet);
}


static void
test_adw_leaflet_prepend (void)
{
  AdwLeaflet *leaflet = g_object_ref_sink (ADW_LEAFLET (adw_leaflet_new ()));
  GtkWidget *labels[2];
  int i;
  GtkSelectionModel *pages;

  g_assert_nonnull (leaflet);

  for (i = 0; i < 2; i++) {
    labels[i] = gtk_label_new ("");
    g_assert_nonnull (labels[i]);
  }

  pages = adw_leaflet_get_pages (leaflet);

  adw_leaflet_prepend (leaflet, labels[1]);
  assert_page_position (pages, labels[1], 0);

  adw_leaflet_prepend (leaflet, labels[0]);
  assert_page_position (pages, labels[0], 0);
  assert_page_position (pages, labels[1], 1);

  g_assert_finalize_object (leaflet);
  g_assert_finalize_object (pages);
}


static void
test_adw_leaflet_insert_child_after (void)
{
  AdwLeaflet *leaflet = g_object_ref_sink (ADW_LEAFLET (adw_leaflet_new ()));
  GtkWidget *labels[3];
  int i;
  GtkSelectionModel *pages;

  g_assert_nonnull (leaflet);

  for (i = 0; i < 3; i++) {
    labels[i] = gtk_label_new ("");
    g_assert_nonnull (labels[i]);
  }

  pages = adw_leaflet_get_pages (leaflet);

  adw_leaflet_append (leaflet, labels[2]);

  assert_page_position (pages, labels[2], 0);

  adw_leaflet_insert_child_after (leaflet, labels[0], NULL);
  assert_page_position (pages, labels[0], 0);
  assert_page_position (pages, labels[2], 1);

  adw_leaflet_insert_child_after (leaflet, labels[1], labels[0]);
  assert_page_position (pages, labels[0], 0);
  assert_page_position (pages, labels[1], 1);
  assert_page_position (pages, labels[2], 2);

  g_assert_finalize_object (leaflet);
  g_assert_finalize_object (pages);
}


static void
test_adw_leaflet_reorder_child_after (void)
{
  AdwLeaflet *leaflet = g_object_ref_sink (ADW_LEAFLET (adw_leaflet_new ()));
  GtkWidget *labels[3];
  int i;
  GtkSelectionModel *pages;

  g_assert_nonnull (leaflet);

  for (i = 0; i < 3; i++) {
    labels[i] = gtk_label_new ("");
    g_assert_nonnull (labels[i]);

    adw_leaflet_append (leaflet, labels[i]);
  }

  pages = adw_leaflet_get_pages (leaflet);

  assert_page_position (pages, labels[0], 0);
  assert_page_position (pages, labels[1], 1);
  assert_page_position (pages, labels[2], 2);

  adw_leaflet_reorder_child_after (leaflet, labels[2], NULL);
  assert_page_position (pages, labels[2], 0);
  assert_page_position (pages, labels[0], 1);
  assert_page_position (pages, labels[1], 2);

  adw_leaflet_reorder_child_after (leaflet, labels[0], labels[1]);
  assert_page_position (pages, labels[2], 0);
  assert_page_position (pages, labels[1], 1);
  assert_page_position (pages, labels[0], 2);

  g_assert_finalize_object (leaflet);
  g_assert_finalize_object (pages);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Leaflet/adjacent_child", test_adw_leaflet_adjacent_child);
  g_test_add_func ("/Adwaita/Leaflet/navigate", test_adw_leaflet_navigate);
  g_test_add_func ("/Adwaita/Leaflet/prepend", test_adw_leaflet_prepend);
  g_test_add_func ("/Adwaita/Leaflet/insert_child_after", test_adw_leaflet_insert_child_after);
  g_test_add_func ("/Adwaita/Leaflet/reorder_child_after", test_adw_leaflet_reorder_child_after);

  return g_test_run ();
}
