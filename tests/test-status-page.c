/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>

gint notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_hdy_status_page_icon_name (void)
{
  g_autoptr (HdyStatusPage) status_page = NULL;
  const gchar *icon_name = NULL;

  status_page = HDY_STATUS_PAGE (g_object_ref_sink (hdy_status_page_new ()));
  g_assert_nonnull (status_page);

  notified = 0;
  g_signal_connect (status_page, "notify::icon-name", G_CALLBACK (notify_cb), NULL);

  g_object_get (status_page, "icon-name", &icon_name, NULL);
  g_assert_cmpstr (icon_name, ==, NULL);

  hdy_status_page_set_icon_name (status_page, NULL);
  g_assert_cmpint (notified, ==, 0);

  hdy_status_page_set_icon_name (status_page, "some-icon-symbolic");
  g_assert_cmpstr (hdy_status_page_get_icon_name (status_page), ==, "some-icon-symbolic");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "icon-name", "other-icon-symbolic", NULL);
  g_assert_cmpstr (hdy_status_page_get_icon_name (status_page), ==, "other-icon-symbolic");
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_status_page_title (void)
{
  g_autoptr (HdyStatusPage) status_page = NULL;
  const gchar *title = NULL;

  status_page = HDY_STATUS_PAGE (g_object_ref_sink (hdy_status_page_new ()));
  g_assert_nonnull (status_page);

  notified = 0;
  g_signal_connect (status_page, "notify::title", G_CALLBACK (notify_cb), NULL);

  g_object_get (status_page, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");

  hdy_status_page_set_title (status_page, "");
  g_assert_cmpint (notified, ==, 0);

  hdy_status_page_set_title (status_page, "Some Title");
  g_assert_cmpstr (hdy_status_page_get_title (status_page), ==, "Some Title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "title", "Other Title", NULL);
  g_assert_cmpstr (hdy_status_page_get_title (status_page), ==, "Other Title");
  g_assert_cmpint (notified, ==, 2);
}

static void
test_hdy_status_page_description (void)
{
  g_autoptr (HdyStatusPage) status_page = NULL;
  const gchar *description = NULL;

  status_page = HDY_STATUS_PAGE (g_object_ref_sink (hdy_status_page_new ()));
  g_assert_nonnull (status_page);

  notified = 0;
  g_signal_connect (status_page, "notify::description", G_CALLBACK (notify_cb), NULL);

  g_object_get (status_page, "description", &description, NULL);
  g_assert_cmpstr (description, ==, "");

  hdy_status_page_set_description (status_page, "");
  g_assert_cmpint (notified, ==, 0);

  hdy_status_page_set_description (status_page, "Some description");
  g_assert_cmpstr (hdy_status_page_get_description (status_page), ==, "Some description");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "description", "Other description", NULL);
  g_assert_cmpstr (hdy_status_page_get_description (status_page), ==, "Other description");
  g_assert_cmpint (notified, ==, 2);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/StatusPage/icon_name", test_hdy_status_page_icon_name);
  g_test_add_func ("/Handy/StatusPage/title", test_hdy_status_page_title);
  g_test_add_func ("/Handy/StatusPage/description", test_hdy_status_page_description);

  return g_test_run ();
}
