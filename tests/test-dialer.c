/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

gint notified;


static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}


static void
test_hdy_dialer_setnumber (void)
{
  GtkWidget *dialer;

  notified = 0;
  dialer = hdy_dialer_new ();
  g_signal_connect (dialer, "notify::number", G_CALLBACK (notify_cb), NULL);

  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), ==, "");

  hdy_dialer_set_number (HDY_DIALER (dialer), "#1234");
  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), ==, "#1234");
  g_assert_cmpint (notified, ==, 1);

  /* Check that we're assigning to the string and not overwriting */
  hdy_dialer_set_number (HDY_DIALER (dialer), "#123");
  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), !=, "#1234");
  g_assert_cmpint (notified, ==, 2);

  /* Do the same using the GObject property */
  g_object_set (G_OBJECT (dialer), "number", "#12", NULL);
  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), !=, "#123");
  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), ==, "#12");
  g_assert_cmpint (notified, ==, 3);
}


static void
test_hdy_dialer_clear_number (void)
{
  GtkWidget *dialer;

  notified = 0;
  dialer = hdy_dialer_new ();
  g_signal_connect (dialer, "notify::number", G_CALLBACK (notify_cb), NULL);

  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), ==, "");
  hdy_dialer_clear_number (HDY_DIALER (dialer));
  g_assert_cmpint (notified, ==, 0);

  hdy_dialer_set_number (HDY_DIALER (dialer), "#1234");
  g_assert_cmpstr (hdy_dialer_get_number (HDY_DIALER (dialer)), ==, "#1234");
  g_assert_cmpint (notified, ==, 1);
  hdy_dialer_clear_number (HDY_DIALER (dialer));
  g_assert_cmpint (notified, ==, 2);
  hdy_dialer_clear_number (HDY_DIALER (dialer));
  g_assert_cmpint (notified, ==, 2);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func ("/Handy/Dialer/setnumber", test_hdy_dialer_setnumber);
  g_test_add_func ("/Handy/Dialer/clear_number", test_hdy_dialer_clear_number);
  return g_test_run ();
}
