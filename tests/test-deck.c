/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_deck_adjacent_child (void)
{
  g_autoptr (HdyDeck) deck = NULL;
  GtkWidget *children[2];
  gint i;
  GtkWidget *result;

  deck = HDY_DECK (hdy_deck_new ());
  g_assert_nonnull (deck);

  for (i = 0; i < 2; i++) {
    children[i] = gtk_label_new ("");
    g_assert_nonnull (children[i]);

    gtk_container_add (GTK_CONTAINER (deck), children[i]);
  }

  hdy_deck_set_visible_child (deck, children[0]);

  result = hdy_deck_get_adjacent_child (deck, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_null (result);

  result = hdy_deck_get_adjacent_child (deck, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result == children[1]);

  hdy_deck_set_visible_child (deck, children[1]);

  result = hdy_deck_get_adjacent_child (deck, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result == children[0]);

  result = hdy_deck_get_adjacent_child (deck, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_null (result);
}


static void
test_hdy_deck_navigate (void)
{
  g_autoptr (HdyDeck) deck = NULL;
  GtkWidget *children[2];
  gint i;
  gboolean result;

  deck = HDY_DECK (hdy_deck_new ());
  g_assert_nonnull (deck);

  for (i = 0; i < 2; i++) {
    children[i] = gtk_label_new ("");
    g_assert_nonnull (children[i]);

    gtk_container_add (GTK_CONTAINER (deck), children[i]);
  }

  hdy_deck_set_visible_child (deck, children[0]);

  result = hdy_deck_navigate (deck, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_false (result);

  result = hdy_deck_navigate (deck, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_true (result);
  g_assert_true (hdy_deck_get_visible_child (deck) == children[1]);

  result = hdy_deck_navigate (deck, HDY_NAVIGATION_DIRECTION_FORWARD);
  g_assert_false (result);

  result = hdy_deck_navigate (deck, HDY_NAVIGATION_DIRECTION_BACK);
  g_assert_true (result);
  g_assert_true (hdy_deck_get_visible_child (deck) == children[0]);
}


static void
test_hdy_deck_prepend (void)
{
  g_autoptr (HdyDeck) deck = NULL;
  GtkWidget *labels[2];
  gint i;
  GList *children = NULL;

  deck = HDY_DECK (hdy_deck_new ());
  g_assert_nonnull (deck);

  for (i = 0; i < 2; i++) {
    labels[i] = gtk_label_new ("");
    g_assert_nonnull (labels[i]);
  }

  hdy_deck_prepend (deck, labels[1]);
  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[1]), ==, 0);
  g_list_free (children);

  hdy_deck_prepend (deck, labels[0]);
  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[0]), ==, 0);
  g_assert_cmpint (g_list_index (children, labels[1]), ==, 1);
  g_list_free (children);
}


static void
test_hdy_deck_insert_child_after (void)
{
  g_autoptr (HdyDeck) deck = NULL;
  GtkWidget *labels[3];
  gint i;
  GList *children = NULL;

  deck = HDY_DECK (hdy_deck_new ());
  g_assert_nonnull (deck);

  for (i = 0; i < 3; i++) {
    labels[i] = gtk_label_new ("");
    g_assert_nonnull (labels[i]);
  }

  gtk_container_add (GTK_CONTAINER (deck), labels[2]);

  hdy_deck_insert_child_after (deck, labels[0], NULL);
  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[0]), ==, 0);
  g_assert_cmpint (g_list_index (children, labels[2]), ==, 1);
  g_list_free (children);

  hdy_deck_insert_child_after (deck, labels[1], labels[0]);
  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[0]), ==, 0);
  g_assert_cmpint (g_list_index (children, labels[1]), ==, 1);
  g_assert_cmpint (g_list_index (children, labels[2]), ==, 2);
  g_list_free (children);
}


static void
test_hdy_deck_reorder_child_after (void)
{
  g_autoptr (HdyDeck) deck = NULL;
  GtkWidget *labels[3];
  gint i;
  GList *children = NULL;

  deck = HDY_DECK (hdy_deck_new ());
  g_assert_nonnull (deck);

  for (i = 0; i < 3; i++) {
    labels[i] = gtk_label_new ("");
    g_assert_nonnull (labels[i]);

    gtk_container_add (GTK_CONTAINER (deck), labels[i]);
  }

  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[0]), ==, 0);
  g_assert_cmpint (g_list_index (children, labels[1]), ==, 1);
  g_assert_cmpint (g_list_index (children, labels[2]), ==, 2);
  g_list_free (children);

  hdy_deck_reorder_child_after (deck, labels[2], NULL);
  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[2]), ==, 0);
  g_assert_cmpint (g_list_index (children, labels[0]), ==, 1);
  g_assert_cmpint (g_list_index (children, labels[1]), ==, 2);
  g_list_free (children);

  hdy_deck_reorder_child_after (deck, labels[0], labels[1]);
  children = gtk_container_get_children (GTK_CONTAINER (deck));
  g_assert_cmpint (g_list_index (children, labels[2]), ==, 0);
  g_assert_cmpint (g_list_index (children, labels[1]), ==, 1);
  g_assert_cmpint (g_list_index (children, labels[0]), ==, 2);
  g_list_free (children);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/Deck/adjacent_child", test_hdy_deck_adjacent_child);
  g_test_add_func ("/Handy/Deck/navigate", test_hdy_deck_navigate);
  g_test_add_func ("/Handy/Deck/prepend", test_hdy_deck_prepend);
  g_test_add_func ("/Handy/Deck/insert_child_after", test_hdy_deck_insert_child_after);
  g_test_add_func ("/Handy/Deck/reorder_child_after", test_hdy_deck_reorder_child_after);

  return g_test_run ();
}
