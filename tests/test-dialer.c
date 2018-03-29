/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

gint notified;


static void
notify_cb(GtkWidget *widget, gpointer data)
{
  notified++;
}


static void
test_hdy_dialer_setnumber(void)
{
  GtkWidget *dialer;

  dialer = hdy_dialer_new();
  g_signal_connect (dialer, "notify::number", G_CALLBACK (notify_cb), NULL);

  g_assert_cmpstr("", ==, hdy_dialer_get_number(HDY_DIALER (dialer)));

  hdy_dialer_set_number(HDY_DIALER (dialer), "#1234");
  g_assert_cmpstr("#1234", ==, hdy_dialer_get_number(HDY_DIALER (dialer)));
  g_assert_cmpint(1, ==, notified);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);

  g_test_add_func("/Handy/Dialer/setnumber", test_hdy_dialer_setnumber);
  return g_test_run();
}
