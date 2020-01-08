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


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/Deck/adjacent_child", test_hdy_deck_adjacent_child);
  g_test_add_func ("/Handy/Deck/navigate", test_hdy_deck_navigate);

  return g_test_run ();
}
