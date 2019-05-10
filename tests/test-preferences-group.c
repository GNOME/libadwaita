/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


static void
test_hdy_preferences_group_add (void)
{
  g_autoptr (HdyPreferencesGroup) group = NULL;
  HdyPreferencesRow *row;
  GtkWidget *widget;

  group = g_object_ref_sink (HDY_PREFERENCES_GROUP (hdy_preferences_group_new ()));
  g_assert_nonnull (group);

  row = hdy_preferences_row_new ();
  g_assert_nonnull (row);
  gtk_container_add (GTK_CONTAINER (group), GTK_WIDGET (row));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  gtk_container_add (GTK_CONTAINER (group), widget);

  g_assert (G_TYPE_CHECK_INSTANCE_TYPE (gtk_widget_get_parent (GTK_WIDGET (row)), GTK_TYPE_LIST_BOX));
  g_assert (G_TYPE_CHECK_INSTANCE_TYPE (gtk_widget_get_parent (widget), GTK_TYPE_BOX));
}


static void
test_hdy_preferences_group_title (void)
{
  g_autoptr (HdyPreferencesGroup) group = NULL;

  group = g_object_ref_sink (HDY_PREFERENCES_GROUP (hdy_preferences_group_new ()));
  g_assert_nonnull (group);

  g_assert_cmpstr (hdy_preferences_group_get_title (group), ==, "");

  hdy_preferences_group_set_title (group, "Dummy title");
  g_assert_cmpstr (hdy_preferences_group_get_title (group), ==, "Dummy title");

  hdy_preferences_group_set_title (group, NULL);
  g_assert_cmpstr (hdy_preferences_group_get_title (group), ==, "");
}


static void
test_hdy_preferences_group_description (void)
{
  g_autoptr (HdyPreferencesGroup) group = NULL;

  group = g_object_ref_sink (HDY_PREFERENCES_GROUP (hdy_preferences_group_new ()));
  g_assert_nonnull (group);

  g_assert_cmpstr (hdy_preferences_group_get_description (group), ==, "");

  hdy_preferences_group_set_description (group, "Dummy description");
  g_assert_cmpstr (hdy_preferences_group_get_description (group), ==, "Dummy description");

  hdy_preferences_group_set_description (group, NULL);
  g_assert_cmpstr (hdy_preferences_group_get_description (group), ==, "");
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/PreferencesGroup/add", test_hdy_preferences_group_add);
  g_test_add_func("/Handy/PreferencesGroup/title", test_hdy_preferences_group_title);
  g_test_add_func("/Handy/PreferencesGroup/description", test_hdy_preferences_group_description);

  return g_test_run();
}
