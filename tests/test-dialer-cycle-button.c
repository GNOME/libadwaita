/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-3.0+
 */

#include <handy.h>

gint notified;


static void
cycle_end_cb(GtkWidget *widget, gpointer data)
{
  notified++;
}


static void
test_hdy_dialer_cycle_button_cycle_end(void)
{
  GtkWidget *btn;

  btn = hdy_dialer_cycle_button_new ("abc");
  g_signal_connect (btn, "cycle-end", G_CALLBACK (cycle_end_cb), NULL);

  hdy_dialer_cycle_button_stop_cycle (HDY_DIALER_CYCLE_BUTTON (btn));
  g_assert_cmpint (1, ==, notified);
  notified = 0;
}


static void
test_hdy_dialer_cycle_button_cycle_timeout(void)
{
  HdyDialerCycleButton *btn;

  btn = HDY_DIALER_CYCLE_BUTTON (hdy_dialer_cycle_button_new ("abc"));
  g_assert_cmpint (1000, ==, hdy_dialer_cycle_button_get_cycle_timeout (btn));
  hdy_dialer_cycle_button_set_cycle_timeout (btn, 10);
  g_assert_cmpint (10, ==, hdy_dialer_cycle_button_get_cycle_timeout (btn));
}


gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  gtk_init(&argc, &argv);

  g_test_add_func("/Handy/DialerCycleButton/cycle_end", test_hdy_dialer_cycle_button_cycle_end);
  g_test_add_func("/Handy/DialerCycleButton/cycle_timeout", test_hdy_dialer_cycle_button_cycle_timeout);
  return g_test_run();
}
