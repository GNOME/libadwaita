/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_status_page_icon_name (void)
{
  AdwStatusPage *status_page = ADW_STATUS_PAGE (g_object_ref_sink (adw_status_page_new ()));
  const char *icon_name = NULL;

  g_assert_nonnull (status_page);

  notified = 0;
  g_signal_connect (status_page, "notify::icon-name", G_CALLBACK (notify_cb), NULL);

  g_object_get (status_page, "icon-name", &icon_name, NULL);
  g_assert_cmpstr (icon_name, ==, NULL);

  adw_status_page_set_icon_name (status_page, NULL);
  g_assert_cmpint (notified, ==, 0);

  adw_status_page_set_icon_name (status_page, "some-icon-symbolic");
  g_assert_cmpstr (adw_status_page_get_icon_name (status_page), ==, "some-icon-symbolic");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "icon-name", "other-icon-symbolic", NULL);
  g_assert_cmpstr (adw_status_page_get_icon_name (status_page), ==, "other-icon-symbolic");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (status_page);
}

static void
test_adw_status_page_title (void)
{
  AdwStatusPage *status_page = ADW_STATUS_PAGE (g_object_ref_sink (adw_status_page_new ()));
  char *title;

  g_assert_nonnull (status_page);

  notified = 0;
  g_signal_connect (status_page, "notify::title", G_CALLBACK (notify_cb), NULL);

  g_object_get (status_page, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");

  adw_status_page_set_title (status_page, "");
  g_assert_cmpint (notified, ==, 0);

  adw_status_page_set_title (status_page, "Some Title");
  g_assert_cmpstr (adw_status_page_get_title (status_page), ==, "Some Title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "title", "Other Title", NULL);
  g_assert_cmpstr (adw_status_page_get_title (status_page), ==, "Other Title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (status_page);
}

static void
test_adw_status_page_description (void)
{
  AdwStatusPage *status_page = ADW_STATUS_PAGE (g_object_ref_sink (adw_status_page_new ()));
  char *description;

  g_assert_nonnull (status_page);

  notified = 0;
  g_signal_connect (status_page, "notify::description", G_CALLBACK (notify_cb), NULL);

  g_object_get (status_page, "description", &description, NULL);
  g_assert_cmpstr (description, ==, "");

  adw_status_page_set_description (status_page, "");
  g_assert_cmpint (notified, ==, 0);

  adw_status_page_set_description (status_page, "Some description");
  g_assert_cmpstr (adw_status_page_get_description (status_page), ==, "Some description");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (status_page, "description", "Other description", NULL);
  g_assert_cmpstr (adw_status_page_get_description (status_page), ==, "Other description");
  g_assert_cmpint (notified, ==, 2);

  g_free (description);
  g_assert_finalize_object (status_page);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/StatusPage/icon_name", test_adw_status_page_icon_name);
  g_test_add_func ("/Adwaita/StatusPage/title", test_adw_status_page_title);
  g_test_add_func ("/Adwaita/StatusPage/description", test_adw_status_page_description);

  return g_test_run ();
}
