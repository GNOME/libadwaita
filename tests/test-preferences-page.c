/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_preferences_page_add_remove (void)
{
  g_autoptr (HdyPreferencesPage) page = NULL;
  HdyPreferencesGroup *group;

  page = g_object_ref_sink (HDY_PREFERENCES_PAGE (hdy_preferences_page_new ()));
  g_assert_nonnull (page);

  group = HDY_PREFERENCES_GROUP (hdy_preferences_group_new ());
  g_assert_nonnull (group);
  hdy_preferences_page_add (page, group);

  hdy_preferences_page_remove (page, group);
}


static void
test_hdy_preferences_page_title (void)
{
  g_autoptr (HdyPreferencesPage) page = NULL;

  page = g_object_ref_sink (HDY_PREFERENCES_PAGE (hdy_preferences_page_new ()));
  g_assert_nonnull (page);

  g_assert_null (hdy_preferences_page_get_title (page));

  hdy_preferences_page_set_title (page, "Dummy title");
  g_assert_cmpstr (hdy_preferences_page_get_title (page), ==, "Dummy title");

  hdy_preferences_page_set_title (page, NULL);
  g_assert_null (hdy_preferences_page_get_title (page));
}


static void
test_hdy_preferences_page_icon_name (void)
{
  g_autoptr (HdyPreferencesPage) page = NULL;

  page = g_object_ref_sink (HDY_PREFERENCES_PAGE (hdy_preferences_page_new ()));
  g_assert_nonnull (page);

  g_assert_null (hdy_preferences_page_get_icon_name (page));

  hdy_preferences_page_set_icon_name (page, "dummy-icon-name");
  g_assert_cmpstr (hdy_preferences_page_get_icon_name (page), ==, "dummy-icon-name");

  hdy_preferences_page_set_icon_name (page, NULL);
  g_assert_null (hdy_preferences_page_get_icon_name (page));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/PreferencesPage/add_remove", test_hdy_preferences_page_add_remove);
  g_test_add_func("/Handy/PreferencesPage/title", test_hdy_preferences_page_title);
  g_test_add_func("/Handy/PreferencesPage/icon_name", test_hdy_preferences_page_icon_name);

  return g_test_run();
}
