/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
check_toggles (AdwToggleGroup *group,
               guint           active,
               guint           n_toggles,
               ...)
{
  GtkSelectionModel *toggles = adw_toggle_group_get_toggles (group);
  va_list args;
  guint i;

  va_start (args, n_toggles);

  g_assert_cmpint (adw_toggle_group_get_n_toggles (group), ==, n_toggles);
  g_assert_cmpint (g_list_model_get_n_items (G_LIST_MODEL (toggles)), ==, n_toggles);

  for (i = 0; i < n_toggles; i++) {
    AdwToggle *toggle = g_list_model_get_item (G_LIST_MODEL (toggles), i);
    const char *name = va_arg (args, const char *);

    g_assert_cmpstr (adw_toggle_get_name (toggle), ==, name);

    if (i == active)
      g_assert_true (gtk_selection_model_is_selected (toggles, i));
    else
      g_assert_false (gtk_selection_model_is_selected (toggles, i));

    g_object_unref (toggle);
  }



  va_end (args);

  g_assert_finalize_object (toggles);
}

static void
test_adw_toggle_group_add (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2, *toggle3, *toggle4;
  int index_notified = 0;
  int name_notified = 0;
  int n_toggles_notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::active", G_CALLBACK (increment), &index_notified);
  g_signal_connect_swapped (group, "notify::active-name", G_CALLBACK (increment), &name_notified);
  g_signal_connect_swapped (group, "notify::n-toggles", G_CALLBACK (increment), &n_toggles_notified);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 0);

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 0);
  g_assert_cmpint (name_notified, ==, 0);
  g_assert_cmpint (n_toggles_notified, ==, 0);

  toggle1 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle1));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 0);
  g_assert_cmpint (n_toggles_notified, ==, 1);

  check_toggles (group, 0, 1, NULL);

  toggle2 = adw_toggle_new ();
  adw_toggle_set_name (toggle2, "toggle2");

  /* Active doesn't change again */
  adw_toggle_group_add (group, g_object_ref (toggle2));
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 0);
  g_assert_cmpint (n_toggles_notified, ==, 2);

  check_toggles (group, 0, 2, NULL, "toggle2");

  adw_toggle_group_remove_all (group);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 0);
  g_assert_cmpint (n_toggles_notified, ==, 3);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 0);

  /* Toggle is disabled, so will not be selected */
  toggle3 = adw_toggle_new ();
  adw_toggle_set_name (toggle3, "toggle3");
  adw_toggle_set_enabled (toggle3, FALSE);
  adw_toggle_group_add (group, g_object_ref (toggle3));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 0);
  g_assert_cmpint (n_toggles_notified, ==, 4);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 1, "toggle3");

  /* This one is enabled, so we jump to it */
  toggle4 = adw_toggle_new ();
  adw_toggle_set_name (toggle4, "toggle4");
  adw_toggle_group_add (group, g_object_ref (toggle4));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 1);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle4");
  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 1);
  g_assert_cmpint (n_toggles_notified, ==, 5);

  check_toggles (group, 1, 2, "toggle3", "toggle4");

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
  g_assert_finalize_object (toggle3);
  g_assert_finalize_object (toggle4);
}

static void
test_adw_toggle_group_remove (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2, *toggle3, *toggle4;
  int index_notified = 0;
  int name_notified = 0;
  int n_toggles_notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::active", G_CALLBACK (increment), &index_notified);
  g_signal_connect_swapped (group, "notify::active-name", G_CALLBACK (increment), &name_notified);
  g_signal_connect_swapped (group, "notify::n-toggles", G_CALLBACK (increment), &n_toggles_notified);

  toggle1 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle1));
  g_assert_cmpint (n_toggles_notified, ==, 1);

  toggle2 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle2));
  g_assert_cmpint (n_toggles_notified, ==, 2);

  toggle3 = adw_toggle_new ();
  adw_toggle_set_name (toggle3, "toggle3");
  adw_toggle_group_add (group, g_object_ref (toggle3));
  g_assert_cmpint (n_toggles_notified, ==, 3);

  toggle4 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle4));
  g_assert_cmpint (n_toggles_notified, ==, 4);

  check_toggles (group, 0, 4, NULL, NULL, "toggle3", NULL);

  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 0);

  adw_toggle_group_set_active (group, 2);

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 2);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle3");
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 1);

  check_toggles (group, 2, 4, NULL, NULL, "toggle3", NULL);

  adw_toggle_group_remove (group, adw_toggle_group_get_toggle (group, 1));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 1);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle3");
  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 1);
  g_assert_cmpint (n_toggles_notified, ==, 5);

  check_toggles (group, 1, 3, NULL, "toggle3", NULL);

  adw_toggle_group_remove (group, adw_toggle_group_get_toggle (group, 2));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 1);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle3");
  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 1);
  g_assert_cmpint (n_toggles_notified, ==, 6);

  check_toggles (group, 1, 2, NULL, "toggle3");

  adw_toggle_group_remove (group, adw_toggle_group_get_toggle (group, 1));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 4);
  g_assert_cmpint (name_notified, ==, 2);
  g_assert_cmpint (n_toggles_notified, ==, 7);

  check_toggles (group, 1, 1, NULL);

  adw_toggle_group_remove (group, adw_toggle_group_get_toggle (group, 0));

  g_assert_cmpint (index_notified, ==, 4);
  g_assert_cmpint (name_notified, ==, 2);
  g_assert_cmpint (n_toggles_notified, ==, 8);

  check_toggles (group, 1, 0);

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
  g_assert_finalize_object (toggle3);
  g_assert_finalize_object (toggle4);
}

static void
test_adw_toggle_group_remove_all (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2, *toggle3, *toggle4;
  int index_notified = 0;
  int name_notified = 0;
  int n_toggles_notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::active", G_CALLBACK (increment), &index_notified);
  g_signal_connect_swapped (group, "notify::active-name", G_CALLBACK (increment), &name_notified);
  g_signal_connect_swapped (group, "notify::n-toggles", G_CALLBACK (increment), &n_toggles_notified);

  toggle1 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle1));
  g_assert_cmpint (n_toggles_notified, ==, 1);

  toggle2 = adw_toggle_new ();
  adw_toggle_set_name (toggle2, "toggle2");
  adw_toggle_group_add (group, g_object_ref (toggle2));
  g_assert_cmpint (n_toggles_notified, ==, 2);

  check_toggles (group, 0, 2, NULL, "toggle2");

  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 0);

  adw_toggle_group_remove_all (group);

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 0);
  g_assert_cmpint (n_toggles_notified, ==, 3);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 0);

  toggle3 = adw_toggle_new ();
  adw_toggle_set_name (toggle3, "toggle3");
  adw_toggle_group_add (group, g_object_ref (toggle3));
  g_assert_cmpint (n_toggles_notified, ==, 4);

  toggle4 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle4));

  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 1);
  g_assert_cmpint (n_toggles_notified, ==, 5);

  check_toggles (group, 0, 2, "toggle3", NULL);

  adw_toggle_group_remove_all (group);

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 4);
  g_assert_cmpint (name_notified, ==, 2);
  g_assert_cmpint (n_toggles_notified, ==, 6);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 0);

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
  g_assert_finalize_object (toggle3);
  g_assert_finalize_object (toggle4);
}

static void
test_adw_toggle_group_get_toggle (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2;

  toggle1 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle1));

  toggle2 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle2));

  g_assert_true (adw_toggle_group_get_toggle (group, 0) == toggle1);
  g_assert_true (adw_toggle_group_get_toggle (group, 1) == toggle2);
  g_assert_null (adw_toggle_group_get_toggle (group, 2));
  g_assert_null (adw_toggle_group_get_toggle (group, GTK_INVALID_LIST_POSITION));

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
}

static void
test_adw_toggle_group_get_toggle_by_name (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2;

  toggle1 = adw_toggle_new ();
  adw_toggle_set_name (toggle1, "toggle1");
  adw_toggle_group_add (group, g_object_ref (toggle1));

  toggle2 = adw_toggle_new ();
  adw_toggle_set_name (toggle2, "toggle2");
  adw_toggle_group_add (group, g_object_ref (toggle2));

  g_assert_true (adw_toggle_group_get_toggle_by_name (group, "toggle1") == toggle1);
  g_assert_true (adw_toggle_group_get_toggle_by_name (group, "toggle2") == toggle2);
  g_assert_null (adw_toggle_group_get_toggle_by_name (group, "toggle3"));

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
}

static void
test_adw_toggle_group_active (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2, *toggle3;
  int index_notified = 0;
  int name_notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::active", G_CALLBACK (increment), &index_notified);
  g_signal_connect_swapped (group, "notify::active-name", G_CALLBACK (increment), &name_notified);

  toggle1 = adw_toggle_new ();
  adw_toggle_set_name (toggle1, "toggle1");
  adw_toggle_group_add (group, g_object_ref (toggle1));

  toggle2 = adw_toggle_new ();
  adw_toggle_set_name (toggle2, "toggle2");
  adw_toggle_group_add (group, g_object_ref (toggle2));

  toggle3 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle3));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 1);

  check_toggles (group, 0, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active (group, 0);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 1);

  check_toggles (group, 0, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active (group, 1);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 1);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle2");
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 2);

  check_toggles (group, 1, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active (group, 3);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 3);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active (group, GTK_INVALID_LIST_POSITION);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 3);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active (group, 2);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 2);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 4);
  g_assert_cmpint (name_notified, ==, 3);

  check_toggles (group, 2, 3, "toggle1", "toggle2", NULL);

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
  g_assert_finalize_object (toggle3);
}

static void
test_adw_toggle_group_active_name (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2, *toggle3;
  int index_notified = 0;
  int name_notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::active", G_CALLBACK (increment), &index_notified);
  g_signal_connect_swapped (group, "notify::active-name", G_CALLBACK (increment), &name_notified);

  toggle1 = adw_toggle_new ();
  adw_toggle_set_name (toggle1, "toggle1");
  adw_toggle_group_add (group, g_object_ref (toggle1));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 1);

  toggle2 = adw_toggle_new ();
  adw_toggle_set_name (toggle2, "toggle1");
  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*a toggle already exists*");
  adw_toggle_group_add (group, g_object_ref (toggle2));
  g_test_assert_expected_messages ();

  check_toggles (group, 0, 1, "toggle1");

  adw_toggle_set_name (toggle2, "toggle2");
  adw_toggle_group_add (group, g_object_ref (toggle2));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 1);

  toggle3 = adw_toggle_new ();
  adw_toggle_group_add (group, g_object_ref (toggle3));
  adw_toggle_group_set_active (group, 2);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 2);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 2);

  check_toggles (group, 2, 3, "toggle1", "toggle2", NULL);

  g_test_expect_message (ADW_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, "*Duplicate toggle name*");
  adw_toggle_set_name (toggle3, "toggle1");
  g_test_assert_expected_messages ();
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 2);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 2);

  check_toggles (group, 2, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active_name (group, "toggle1");
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 3);
  g_assert_cmpint (name_notified, ==, 3);

  check_toggles (group, 0, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active_name (group, "toggle2");
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 1);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle2");
  g_assert_cmpint (index_notified, ==, 4);
  g_assert_cmpint (name_notified, ==, 4);

  check_toggles (group, 1, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active_name (group, NULL);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 5);
  g_assert_cmpint (name_notified, ==, 5);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 3, "toggle1", "toggle2", NULL);

  adw_toggle_group_set_active (group, 2);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 2);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 6);
  g_assert_cmpint (name_notified, ==, 5);

  check_toggles (group, 2, 3, "toggle1", "toggle2", NULL);

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
  g_assert_finalize_object (toggle3);
}

static void
test_adw_toggle_group_active_enabled (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  AdwToggle *toggle1, *toggle2;
  int index_notified = 0;
  int name_notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::active", G_CALLBACK (increment), &index_notified);
  g_signal_connect_swapped (group, "notify::active-name", G_CALLBACK (increment), &name_notified);

  toggle1 = adw_toggle_new ();
  adw_toggle_set_name (toggle1, "toggle1");
  adw_toggle_group_add (group, g_object_ref (toggle1));

  toggle2 = adw_toggle_new ();
  adw_toggle_set_name (toggle2, "toggle2");
  adw_toggle_group_add (group, g_object_ref (toggle2));

  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 1);

  check_toggles (group, 0, 2, "toggle1", "toggle2");

  /* Nothing happens when disabling inactive toggles */
  adw_toggle_set_enabled (toggle2, FALSE);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, 0);
  g_assert_cmpstr (adw_toggle_group_get_active_name (group), ==, "toggle1");
  g_assert_cmpint (index_notified, ==, 1);
  g_assert_cmpint (name_notified, ==, 1);

  check_toggles (group, 0, 2, "toggle1", "toggle2");

  /* Selection clears when disabling the active one */
  adw_toggle_set_enabled (toggle1, FALSE);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 2);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 2, "toggle1", "toggle2");

  /* Enabling it back does nothing */
  adw_toggle_set_enabled (toggle1, TRUE);
  g_assert_cmpint (adw_toggle_group_get_active (group), ==, GTK_INVALID_LIST_POSITION);
  g_assert_null (adw_toggle_group_get_active_name (group));
  g_assert_cmpint (index_notified, ==, 2);
  g_assert_cmpint (name_notified, ==, 2);

  check_toggles (group, GTK_INVALID_LIST_POSITION, 2, "toggle1", "toggle2");

  g_assert_finalize_object (group);
  g_assert_finalize_object (toggle1);
  g_assert_finalize_object (toggle2);
}

static void
test_adw_toggle_group_homogeneous (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  gboolean homogeneous;
  int notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::homogeneous", G_CALLBACK (increment), &notified);

  g_object_get (group, "homogeneous", &homogeneous, NULL);
  g_assert_false (homogeneous);

  adw_toggle_group_set_homogeneous (group, FALSE);
  g_assert_false (adw_toggle_group_get_homogeneous (group));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (group, "homogeneous", TRUE, NULL);
  g_assert_true (adw_toggle_group_get_homogeneous (group));
  g_assert_cmpint (notified, ==, 1);

  adw_toggle_group_set_homogeneous (group, FALSE);
  g_object_get (group, "homogeneous", &homogeneous, NULL);
  g_assert_false (homogeneous);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (group);
}

static void
test_adw_toggle_group_can_shrink (void)
{
  AdwToggleGroup *group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  gboolean can_shrink;
  int notified = 0;

  g_assert_nonnull (group);

  g_signal_connect_swapped (group, "notify::can-shrink", G_CALLBACK (increment), &notified);

  g_object_get (group, "can-shrink", &can_shrink, NULL);
  g_assert_true (can_shrink);

  adw_toggle_group_set_can_shrink (group, TRUE);
  g_assert_true (adw_toggle_group_get_can_shrink (group));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (group, "can-shrink", FALSE, NULL);
  g_assert_false (adw_toggle_group_get_can_shrink (group));
  g_assert_cmpint (notified, ==, 1);

  adw_toggle_group_set_can_shrink (group, TRUE);
  g_object_get (group, "can-shrink", &can_shrink, NULL);
  g_assert_true (can_shrink);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (group);
}

static void
test_adw_toggle_name (void)
{
  AdwToggle *toggle = adw_toggle_new ();
  char *name;
  int notified = 0;

  g_assert_nonnull (toggle);

  g_signal_connect_swapped (toggle, "notify::name", G_CALLBACK (increment), &notified);

  g_object_get (toggle, "name", &name, NULL);
  g_assert_null (name);

  adw_toggle_set_name (toggle, NULL);
  g_assert_cmpint (notified, ==, 0);

  adw_toggle_set_name (toggle, "toggle");
  g_assert_cmpstr (adw_toggle_get_name (toggle), ==, "toggle");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toggle, "name", NULL, NULL);
  g_assert_null (adw_toggle_get_name (toggle));
  g_assert_cmpint (notified, ==, 2);

  g_free (name);
  g_assert_finalize_object (toggle);
}

static void
test_adw_toggle_label (void)
{
  AdwToggle *toggle = adw_toggle_new ();
  char *label;
  int notified = 0;

  g_assert_nonnull (toggle);

  g_signal_connect_swapped (toggle, "notify::label", G_CALLBACK (increment), &notified);

  g_object_get (toggle, "label", &label, NULL);
  g_assert_null (label);

  adw_toggle_set_label (toggle, NULL);
  g_assert_cmpint (notified, ==, 0);

  adw_toggle_set_label (toggle, "Toggle");
  g_assert_cmpstr (adw_toggle_get_label (toggle), ==, "Toggle");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toggle, "label", NULL, NULL);
  g_assert_null (adw_toggle_get_label (toggle));
  g_assert_cmpint (notified, ==, 2);

  g_free (label);
  g_assert_finalize_object (toggle);
}

static void
test_adw_toggle_use_underline (void)
{
  AdwToggle *toggle = adw_toggle_new ();
  gboolean use_underline;
  int notified = 0;

  g_assert_nonnull (toggle);

  g_signal_connect_swapped (toggle, "notify::use-underline", G_CALLBACK (increment), &notified);

  g_object_get (toggle, "use-underline", &use_underline, NULL);
  g_assert_false (use_underline);

  adw_toggle_set_use_underline (toggle, FALSE);
  g_assert_false (adw_toggle_get_use_underline (toggle));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (toggle, "use-underline", TRUE, NULL);
  g_assert_true (adw_toggle_get_use_underline (toggle));
  g_assert_cmpint (notified, ==, 1);

  adw_toggle_set_use_underline (toggle, FALSE);
  g_object_get (toggle, "use-underline", &use_underline, NULL);
  g_assert_false (use_underline);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toggle);
}

static void
test_adw_toggle_icon_name (void)
{
  AdwToggle *toggle = adw_toggle_new ();
  char *icon_name;
  int notified = 0;

  g_assert_nonnull (toggle);

  g_signal_connect_swapped (toggle, "notify::icon-name", G_CALLBACK (increment), &notified);

  g_object_get (toggle, "icon-name", &icon_name, NULL);
  g_assert_null (icon_name);

  adw_toggle_set_icon_name (toggle, NULL);
  g_assert_cmpint (notified, ==, 0);

  adw_toggle_set_icon_name (toggle, "go-previous-symbolic");
  g_assert_cmpstr (adw_toggle_get_icon_name (toggle), ==, "go-previous-symbolic");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toggle, "icon-name", NULL, NULL);
  g_assert_null (adw_toggle_get_icon_name (toggle));
  g_assert_cmpint (notified, ==, 2);

  g_free (icon_name);
  g_assert_finalize_object (toggle);
}

static void
test_adw_toggle_child (void)
{
  AdwToggle *toggle = adw_toggle_new ();
  GtkWidget *widget = NULL;
  int notified = 0;

  g_assert_nonnull (toggle);

  g_signal_connect_swapped (toggle, "notify::child", G_CALLBACK (increment), &notified);

  g_object_get (toggle, "child", &widget, NULL);
  g_assert_null (widget);

  adw_toggle_set_child (toggle, NULL);
  g_assert_cmpint (notified, ==, 0);

  widget = gtk_button_new ();
  adw_toggle_set_child (toggle, widget);
  g_assert_true (adw_toggle_get_child (toggle) == widget);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toggle, "child", NULL, NULL);
  g_assert_null (adw_toggle_get_child (toggle));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toggle);
}

static void
test_adw_toggle_enabled (void)
{
  AdwToggle *toggle = adw_toggle_new ();
  gboolean enabled;
  int notified = 0;

  g_assert_nonnull (toggle);

  g_signal_connect_swapped (toggle, "notify::enabled", G_CALLBACK (increment), &notified);

  g_object_get (toggle, "enabled", &enabled, NULL);
  g_assert_true (enabled);

  adw_toggle_set_enabled (toggle, TRUE);
  g_assert_true (adw_toggle_get_enabled (toggle));
  g_assert_cmpint (notified, ==, 0);

  g_object_set (toggle, "enabled", FALSE, NULL);
  g_assert_false (adw_toggle_get_enabled (toggle));
  g_assert_cmpint (notified, ==, 1);

  adw_toggle_set_enabled (toggle, TRUE);
  g_object_get (toggle, "enabled", &enabled, NULL);
  g_assert_true (enabled);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toggle);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/ToggleGroup/add", test_adw_toggle_group_add);
  g_test_add_func ("/Adwaita/ToggleGroup/remove", test_adw_toggle_group_remove);
  g_test_add_func ("/Adwaita/ToggleGroup/remove_all", test_adw_toggle_group_remove_all);
  g_test_add_func ("/Adwaita/ToggleGroup/get_toggle", test_adw_toggle_group_get_toggle);
  g_test_add_func ("/Adwaita/ToggleGroup/get_toggle_by_name", test_adw_toggle_group_get_toggle_by_name);
  g_test_add_func ("/Adwaita/ToggleGroup/active", test_adw_toggle_group_active);
  g_test_add_func ("/Adwaita/ToggleGroup/active_name", test_adw_toggle_group_active_name);
  g_test_add_func ("/Adwaita/ToggleGroup/active_enabled", test_adw_toggle_group_active_enabled);
  g_test_add_func ("/Adwaita/ToggleGroup/homogeneous", test_adw_toggle_group_homogeneous);
  g_test_add_func ("/Adwaita/ToggleGroup/can_shrink", test_adw_toggle_group_can_shrink);
  g_test_add_func ("/Adwaita/Toggle/name", test_adw_toggle_name);
  g_test_add_func ("/Adwaita/Toggle/label", test_adw_toggle_label);
  g_test_add_func ("/Adwaita/Toggle/use_underline", test_adw_toggle_use_underline);
  g_test_add_func ("/Adwaita/Toggle/icon_name", test_adw_toggle_icon_name);
  g_test_add_func ("/Adwaita/Toggle/child", test_adw_toggle_child);
  g_test_add_func ("/Adwaita/Toggle/enabled", test_adw_toggle_enabled);

  return g_test_run ();
}
