/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>


static void
test_adw_preferences_page_add_remove (void)
{
  AdwPreferencesPage *page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));
  AdwPreferencesGroup *group;

  g_assert_nonnull (page);

  group = ADW_PREFERENCES_GROUP (adw_preferences_group_new ());
  g_assert_nonnull (group);
  adw_preferences_page_add (page, group);

  adw_preferences_page_remove (page, group);

  g_assert_finalize_object (page);
}


static void
test_adw_preferences_page_icon_name (void)
{
  AdwPreferencesPage *page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_null (adw_preferences_page_get_icon_name (page));

  adw_preferences_page_set_icon_name (page, "dummy-icon-name");
  g_assert_cmpstr (adw_preferences_page_get_icon_name (page), ==, "dummy-icon-name");

  adw_preferences_page_set_icon_name (page, NULL);
  g_assert_null (adw_preferences_page_get_icon_name (page));

  g_assert_finalize_object (page);
}


static void
test_adw_preferences_page_title (void)
{
  AdwPreferencesPage *page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_cmpstr (adw_preferences_page_get_title (page), ==, "");

  adw_preferences_page_set_title (page, "Dummy title");
  g_assert_cmpstr (adw_preferences_page_get_title (page), ==, "Dummy title");

  adw_preferences_page_set_title (page, NULL);
  g_assert_cmpstr (adw_preferences_page_get_title (page), ==, "");

  g_assert_finalize_object (page);
}


static void
test_adw_preferences_page_description (void)
{
  AdwPreferencesPage *page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_cmpstr (adw_preferences_page_get_description (page), ==, "");

  adw_preferences_page_set_description (page, "Dummy description");
  g_assert_cmpstr (adw_preferences_page_get_description (page), ==, "Dummy description");

  adw_preferences_page_set_description (page, NULL);
  g_assert_cmpstr (adw_preferences_page_get_description (page), ==, "");

  g_assert_finalize_object (page);
}


static void
test_adw_preferences_page_use_underline (void)
{
  AdwPreferencesPage *page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_false (adw_preferences_page_get_use_underline (page));

  adw_preferences_page_set_use_underline (page, TRUE);
  g_assert_true (adw_preferences_page_get_use_underline (page));

  adw_preferences_page_set_use_underline (page, FALSE);
  g_assert_false (adw_preferences_page_get_use_underline (page));

  g_assert_finalize_object (page);
}

static void
test_adw_preferences_page_description_center (void)
{
  AdwPreferencesPage *page = g_object_ref_sink (ADW_PREFERENCES_PAGE (adw_preferences_page_new ()));

  g_assert_nonnull (page);

  g_assert_false (adw_preferences_page_get_description_centered (page));

  adw_preferences_page_set_description_centered (page, TRUE);
  g_assert_true (adw_preferences_page_get_description_centered (page));

  g_assert_finalize_object (page);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesPage/add_remove", test_adw_preferences_page_add_remove);
  g_test_add_func("/Adwaita/PreferencesPage/icon_name", test_adw_preferences_page_icon_name);
  g_test_add_func("/Adwaita/PreferencesPage/title", test_adw_preferences_page_title);
  g_test_add_func("/Adwaita/PreferencesPage/description", test_adw_preferences_page_description);
  g_test_add_func("/Adwaita/PreferencesPage/use_underline", test_adw_preferences_page_use_underline);
  g_test_add_func("/Adwaita/PreferencesPage/description_center", test_adw_preferences_page_description_center);

  return g_test_run();
}
