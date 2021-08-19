/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_tab_button_view (void)
{
  AdwTabButton *button = g_object_ref_sink (ADW_TAB_BUTTON (adw_tab_button_new ()));
  AdwTabView *view;

  g_assert_nonnull (button);

  notified = 0;
  g_signal_connect (button, "notify::view", G_CALLBACK (notify_cb), NULL);

  g_object_get (button, "view", &view, NULL);
  g_assert_null (view);

  adw_tab_button_set_view (button, NULL);
  g_assert_cmpint (notified, ==, 0);

  view = g_object_ref_sink (ADW_TAB_VIEW (adw_tab_view_new ()));
  adw_tab_button_set_view (button, view);
  g_assert_true (adw_tab_button_get_view (button) == view);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (button, "view", NULL, NULL);
  g_assert_null (adw_tab_button_get_view (button));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (button);
  g_assert_finalize_object (view);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/TabButton/view", test_adw_tab_button_view);

  return g_test_run ();
}
