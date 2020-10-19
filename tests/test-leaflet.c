/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_leaflet_adjacent_child (void)
{
  g_autoptr (HdyLeaflet) leaflet = NULL;
  GtkWidget *children[3];
  gint i;
  GtkWidget *result;

  leaflet = HDY_LEAFLET (hdy_leaflet_new ());
  g_assert_nonnull (leaflet);

  for (i = 0; i < 3; i++) {
    children[i] = gtk_label_new ("");
    g_assert_nonnull (children[i]);

    gtk_container_add (GTK_CONTAINER (leaflet), children[i]);
  }

  gtk_container_child_set (GTK_CONTAINER (leaflet), children[1],
                           "navigatable", FALSE,
                           NULL);

  hdy_leaflet_set_visible_child (leaflet, children[0]);

  result = hdy_leaflet_get_adjacent_child (leaflet, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_null (result);

  result = hdy_leaflet_get_adjacent_child (leaflet, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result == children[2]);

  hdy_leaflet_set_visible_child (leaflet, children[1]);

  result = hdy_leaflet_get_adjacent_child (leaflet, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result == children[0]);

  result = hdy_leaflet_get_adjacent_child (leaflet, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result == children[2]);

  hdy_leaflet_set_visible_child (leaflet, children[2]);

  result = hdy_leaflet_get_adjacent_child (leaflet, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result == children[0]);

  result = hdy_leaflet_get_adjacent_child (leaflet, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_null (result);
}


static void
test_hdy_leaflet_navigate (void)
{
  g_autoptr (HdyLeaflet) leaflet = NULL;
  GtkWidget *children[3];
  gint i;
  gboolean result;

  leaflet = HDY_LEAFLET (hdy_leaflet_new ());
  g_assert_nonnull (leaflet);

  for (i = 0; i < 3; i++) {
    children[i] = gtk_label_new ("");
    g_assert_nonnull (children[i]);

    gtk_container_add (GTK_CONTAINER (leaflet), children[i]);
  }

  gtk_container_child_set (GTK_CONTAINER (leaflet), children[1],
                           "navigatable", FALSE,
                           NULL);

  hdy_leaflet_set_visible_child (leaflet, children[0]);

  result = hdy_leaflet_navigate (leaflet, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_false (result);

  result = hdy_leaflet_navigate (leaflet, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result);
  g_assert_true (hdy_leaflet_get_visible_child (leaflet) == children[2]);

  result = hdy_leaflet_navigate (leaflet, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_false (result);

  result = hdy_leaflet_navigate (leaflet, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result);
  g_assert_true (hdy_leaflet_get_visible_child (leaflet) == children[0]);
}


static void
test_hdy_leaflet_prepend (void)
{
  g_autoptr (HdyLeaflet) leaflet = NULL;
  GtkWidget *label_A = gtk_label_new("");
  GtkWidget *label_B = gtk_label_new("");
  GList *children = NULL;

  leaflet = HDY_LEAFLET (hdy_leaflet_new ());
  g_assert_nonnull (leaflet);

  hdy_leaflet_prepend (leaflet, label_B);
  children = gtk_container_get_children (GTK_CONTAINER (leaflet));
  g_assert_true (g_list_index (children, label_B) == 0);
  g_list_free (children);

  hdy_leaflet_prepend (leaflet, label_A);
  children = gtk_container_get_children (GTK_CONTAINER (leaflet));
  g_assert_true (g_list_index (children, label_A) == 0);
  g_assert_true (g_list_index (children, label_B) == 1);
  g_list_free (children);
}


static void
test_hdy_leaflet_insert_child_after (void)
{
  g_autoptr (HdyLeaflet) leaflet = NULL;
  GtkWidget *label_A = gtk_label_new("");
  GtkWidget *label_B = gtk_label_new("");
  GtkWidget *label_C = gtk_label_new("");
  GList *children = NULL;

  leaflet = HDY_LEAFLET (hdy_leaflet_new ());
  g_assert_nonnull (leaflet);

  gtk_container_add (GTK_CONTAINER (leaflet), label_B);

  hdy_leaflet_insert_child_after (leaflet, label_A, NULL);
  children = gtk_container_get_children (GTK_CONTAINER (leaflet));
  g_assert_true (g_list_index (children, label_A) == 0);
  g_list_free (children);

  hdy_leaflet_insert_child_after (leaflet, label_C, label_B);
  children = gtk_container_get_children (GTK_CONTAINER (leaflet));
  g_assert_true (g_list_index (children, label_C) == 2);
  g_list_free (children);
}


static void
test_hdy_leaflet_reorder_child_after (void)
{
  g_autoptr (HdyLeaflet) leaflet = NULL;
  GtkWidget *label_A = gtk_label_new("");
  GtkWidget *label_B = gtk_label_new("");
  GtkWidget *label_C = gtk_label_new("");
  GList *children = NULL;

  leaflet = HDY_LEAFLET (hdy_leaflet_new ());
  g_assert_nonnull (leaflet);

  gtk_container_add (GTK_CONTAINER (leaflet), label_A);
  gtk_container_add (GTK_CONTAINER (leaflet), label_B);
  gtk_container_add (GTK_CONTAINER (leaflet), label_C);

  hdy_leaflet_reorder_child_after (leaflet, label_C, NULL);
  children = gtk_container_get_children (GTK_CONTAINER (leaflet));
  g_assert_true (g_list_index (children, label_C) == 0);
  g_list_free (children);

  hdy_leaflet_reorder_child_after (leaflet, label_A, label_B);
  children = gtk_container_get_children (GTK_CONTAINER (leaflet));
  g_assert_true (g_list_index (children, label_A) == 2);
  g_list_free (children);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/Leaflet/adjacent_child", test_hdy_leaflet_adjacent_child);
  g_test_add_func ("/Handy/Leaflet/navigate", test_hdy_leaflet_navigate);
  g_test_add_func ("/Handy/Leaflet/prepend", test_hdy_leaflet_prepend);
  g_test_add_func ("/Handy/Leaflet/insert_child_after", test_hdy_leaflet_insert_child_after);
  g_test_add_func ("/Handy/Leaflet/reorder_child_after", test_hdy_leaflet_reorder_child_after);

  return g_test_run ();
}
