/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_preferences_group_add_remove (void)
{
  AdwPreferencesGroup *group = g_object_ref_sink (ADW_PREFERENCES_GROUP (adw_preferences_group_new ()));
  AdwPreferencesRow *row;
  GtkWidget *widget;

  g_assert_nonnull (group);

  row = ADW_PREFERENCES_ROW (adw_preferences_row_new ());
  g_assert_nonnull (row);
  adw_preferences_group_add (group, GTK_WIDGET (row));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  adw_preferences_group_add (group, widget);

  g_assert (G_TYPE_CHECK_INSTANCE_TYPE (gtk_widget_get_parent (GTK_WIDGET (row)), GTK_TYPE_LIST_BOX));
  g_assert (G_TYPE_CHECK_INSTANCE_TYPE (gtk_widget_get_parent (widget), GTK_TYPE_BOX));

  adw_preferences_group_remove (group, GTK_WIDGET (row));
  adw_preferences_group_remove (group, widget);

  g_assert_finalize_object (group);
}

static void
test_adw_preferences_group_title (void)
{
  AdwPreferencesGroup *group = g_object_ref_sink (ADW_PREFERENCES_GROUP (adw_preferences_group_new ()));

  g_assert_nonnull (group);

  g_assert_cmpstr (adw_preferences_group_get_title (group), ==, "");

  adw_preferences_group_set_title (group, "Dummy title");
  g_assert_cmpstr (adw_preferences_group_get_title (group), ==, "Dummy title");

  adw_preferences_group_set_title (group, NULL);
  g_assert_cmpstr (adw_preferences_group_get_title (group), ==, "");

  g_assert_finalize_object (group);
}

static void
test_adw_preferences_group_description (void)
{
  AdwPreferencesGroup *group = g_object_ref_sink (ADW_PREFERENCES_GROUP (adw_preferences_group_new ()));

  g_assert_nonnull (group);

  g_assert_cmpstr (adw_preferences_group_get_description (group), ==, "");

  adw_preferences_group_set_description (group, "Dummy description");
  g_assert_cmpstr (adw_preferences_group_get_description (group), ==, "Dummy description");

  adw_preferences_group_set_description (group, NULL);
  g_assert_cmpstr (adw_preferences_group_get_description (group), ==, "");

  g_assert_finalize_object (group);
}

static void
test_adw_preferences_group_separate_rows (void)
{
  AdwPreferencesGroup *group = g_object_ref_sink (ADW_PREFERENCES_GROUP (adw_preferences_group_new ()));

  g_assert_nonnull (group);

  g_assert_false (adw_preferences_group_get_separate_rows (group));

  adw_preferences_group_set_separate_rows (group, TRUE);
  g_assert_true (adw_preferences_group_get_separate_rows (group));

  g_assert_finalize_object (group);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesGroup/add_remove", test_adw_preferences_group_add_remove);
  g_test_add_func("/Adwaita/PreferencesGroup/title", test_adw_preferences_group_title);
  g_test_add_func("/Adwaita/PreferencesGroup/description", test_adw_preferences_group_description);
  g_test_add_func("/Adwaita/PreferencesGroup/separate_rows", test_adw_preferences_group_separate_rows);

  return g_test_run();
}
