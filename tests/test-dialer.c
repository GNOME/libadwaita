/* hdy-dialer.c
 *
 * Copyright (C) 2017 Guido Günther <agx@sigxcpu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
  g_test_init (&argc, &argv, NULL);
  gtk_init(&argc, &argv);

  g_test_add_func("/Handy/Dialer/setnumber", test_hdy_dialer_setnumber);
  return g_test_run();
}
