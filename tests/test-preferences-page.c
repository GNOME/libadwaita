/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <adwaita.h>


static void
test_adw_preferences_page_add_remove (void)
{
  g_autoptr (AdwPreferencesPage) page = NULL;
  AdwPreferencesGroup *group;

  page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));
  g_assert_nonnull (page);

  group = ADW_PREFERENCES_GROUP (adw_preferences_group_new ());
  g_assert_nonnull (group);
  adw_preferences_page_add (page, group);

  adw_preferences_page_remove (page, group);
}


static void
test_adw_preferences_page_title (void)
{
  g_autoptr (AdwPreferencesPage) page = NULL;

  page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));
  g_assert_nonnull (page);

  g_assert_null (adw_preferences_page_get_title (page));

  adw_preferences_page_set_title (page, "Dummy title");
  g_assert_cmpstr (adw_preferences_page_get_title (page), ==, "Dummy title");

  adw_preferences_page_set_title (page, NULL);
  g_assert_null (adw_preferences_page_get_title (page));
}


static void
test_adw_preferences_page_icon_name (void)
{
  g_autoptr (AdwPreferencesPage) page = NULL;

  page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));
  g_assert_nonnull (page);

  g_assert_null (adw_preferences_page_get_icon_name (page));

  adw_preferences_page_set_icon_name (page, "dummy-icon-name");
  g_assert_cmpstr (adw_preferences_page_get_icon_name (page), ==, "dummy-icon-name");

  adw_preferences_page_set_icon_name (page, NULL);
  g_assert_null (adw_preferences_page_get_icon_name (page));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesPage/add_remove", test_adw_preferences_page_add_remove);
  g_test_add_func("/Adwaita/PreferencesPage/title", test_adw_preferences_page_title);
  g_test_add_func("/Adwaita/PreferencesPage/icon_name", test_adw_preferences_page_icon_name);

  return g_test_run();
}
