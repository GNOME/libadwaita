/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>
#include <hdy-keypad-button-private.h>

gint notified;


static void
notify_cb (GtkWidget *widget,
           gpointer   data)
{
  notified++;
}


static void
test_hdy_keypad_show_symbols (void)
{
  HdyKeypad *keypad = HDY_KEYPAD (hdy_keypad_new (TRUE, FALSE));
  GList *l;
  GList *list;

  list = gtk_container_get_children (GTK_CONTAINER (keypad));

  for (l = list; l != NULL; l = l->next) {
    if (HDY_IS_KEYPAD_BUTTON(l->data)) {
      gboolean value;
      g_object_get (l->data, "show-symbols", &value, NULL);
      g_assert_false (value);
    }
  }
  g_list_free (list);
}


static void
test_hdy_keypad_set_actions (void)
{
  HdyKeypad *keypad = HDY_KEYPAD (hdy_keypad_new (FALSE, TRUE));
  GtkWidget *button_right = gtk_button_new ();
  GtkWidget *button_left = gtk_button_new ();

  // Right extra button
  g_assert (gtk_grid_get_child_at (GTK_GRID (keypad), 2, 3) != NULL);
  // Left extra button
  g_assert (gtk_grid_get_child_at (GTK_GRID (keypad), 0, 3) != NULL);

  hdy_keypad_set_right_action (keypad, button_right);
  hdy_keypad_set_left_action (keypad, button_left);
  g_assert (button_right == gtk_grid_get_child_at (GTK_GRID (keypad), 2, 3));
  g_assert (button_left == gtk_grid_get_child_at (GTK_GRID (keypad), 0, 3));

  hdy_keypad_set_right_action (keypad, NULL);
  g_assert (gtk_grid_get_child_at (GTK_GRID (keypad), 2, 3) == NULL);

  hdy_keypad_set_left_action (keypad, NULL);
  g_assert (gtk_grid_get_child_at (GTK_GRID (keypad), 0, 3) == NULL);
}


static void
test_hdy_keypad_button_clicked (void)
{
  HdyKeypad *keypad = HDY_KEYPAD (hdy_keypad_new (FALSE, TRUE));
  GtkWidget *entry = gtk_entry_new ();
  GList *l;
  GList *list;

  notified = 0;

  hdy_keypad_set_entry (keypad, GTK_ENTRY (entry));

  g_signal_connect (hdy_keypad_get_entry (keypad), "insert-text", G_CALLBACK (notify_cb), NULL);

  list = gtk_container_get_children (GTK_CONTAINER (keypad));

  for (l = list; l != NULL; l = l->next) {
    if (HDY_IS_KEYPAD_BUTTON(l->data)) {
      GtkButton *btn = GTK_BUTTON (l->data);
      gtk_button_clicked (btn);
    }
  }

  g_assert_cmpint (notified, ==, 10);

  g_list_free (list);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);

  g_test_add_func ("/Handy/Keypad/show_symbols", test_hdy_keypad_show_symbols);
  g_test_add_func ("/Handy/Keypad/set_actions", test_hdy_keypad_set_actions);
  g_test_add_func ("/Handy/Keypad/button_click", test_hdy_keypad_button_clicked);

  return g_test_run ();
}
