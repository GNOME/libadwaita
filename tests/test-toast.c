/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_toast_title (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  g_autofree char *title = NULL;

  g_assert_nonnull (toast);

  notified = 0;
  g_signal_connect (toast, "notify::title", G_CALLBACK (notify_cb), NULL);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");

  adw_toast_set_title (toast, "Another title");
  g_assert_cmpstr (adw_toast_get_title (toast), ==, "Another title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "title", "Title", NULL);
  g_assert_cmpstr (adw_toast_get_title (toast), ==, "Title");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_button_label (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  char *button_label;

  g_assert_nonnull (toast);

  notified = 0;
  g_signal_connect (toast, "notify::button-label", G_CALLBACK (notify_cb), NULL);

  g_object_get (toast, "button-label", &button_label, NULL);
  g_assert_null (button_label);

  adw_toast_set_button_label (toast, "Button");
  g_assert_cmpstr (adw_toast_get_button_label (toast), ==, "Button");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "button-label", "Button 2", NULL);
  g_assert_cmpstr (adw_toast_get_button_label (toast), ==, "Button 2");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_action_name (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  char *action_name;

  g_assert_nonnull (toast);

  notified = 0;
  g_signal_connect (toast, "notify::action-name", G_CALLBACK (notify_cb), NULL);

  g_object_get (toast, "action-name", &action_name, NULL);
  g_assert_null (action_name);

  adw_toast_set_action_name (toast, "win.something");
  g_assert_cmpstr (adw_toast_get_action_name (toast), ==, "win.something");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "action-name", "win.something-else", NULL);
  g_assert_cmpstr (adw_toast_get_action_name (toast), ==, "win.something-else");
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_action_target (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  GVariant *action_target;
  g_autoptr (GVariant) variant1 = g_variant_ref_sink (g_variant_new_int32 (1));
  g_autoptr (GVariant) variant2 = g_variant_ref_sink (g_variant_new_int32 (2));
  g_autoptr (GVariant) variant3 = g_variant_ref_sink (g_variant_new_int32 (3));

  g_assert_nonnull (toast);

  notified = 0;
  g_signal_connect (toast, "notify::action-target", G_CALLBACK (notify_cb), NULL);

  g_object_get (toast, "action-target", &action_target, NULL);
  g_assert_null (action_target);

  adw_toast_set_action_target_value (toast, g_variant_new_int32 (1));
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "action-target", g_variant_new_int32 (2), NULL);
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant2);
  g_assert_cmpint (notified, ==, 2);

  adw_toast_set_action_target (toast, "i", 3);
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant3);
  g_assert_cmpint (notified, ==, 3);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_detailed_action_name (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  g_autoptr (GVariant) variant = g_variant_ref_sink (g_variant_new_int32 (2));

  g_assert_nonnull (toast);

  g_assert_null (adw_toast_get_action_name (toast));
  g_assert_null (adw_toast_get_action_target_value (toast));

  adw_toast_set_detailed_action_name (toast, "win.something");
  g_assert_cmpstr (adw_toast_get_action_name (toast), ==, "win.something");
  g_assert_null (adw_toast_get_action_target_value (toast));

  adw_toast_set_detailed_action_name (toast, "win.something(2)");
  g_assert_cmpstr (adw_toast_get_action_name (toast), ==, "win.something");
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_priority (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  AdwToastPriority priority;

  g_assert_nonnull (toast);

  notified = 0;
  g_signal_connect (toast, "notify::priority", G_CALLBACK (notify_cb), NULL);

  g_object_get (toast, "priority", &priority, NULL);
  g_assert_cmpint (priority, ==, ADW_TOAST_PRIORITY_NORMAL);

  adw_toast_set_priority (toast, ADW_TOAST_PRIORITY_HIGH);
  g_assert_cmpint (adw_toast_get_priority (toast), ==, ADW_TOAST_PRIORITY_HIGH);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "priority", ADW_TOAST_PRIORITY_NORMAL, NULL);
  g_assert_cmpint (adw_toast_get_priority (toast), ==, ADW_TOAST_PRIORITY_NORMAL);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_dismiss (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  AdwToastOverlay *overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));

  g_assert_nonnull (overlay);
  g_assert_nonnull (toast);

  adw_toast_overlay_add_toast (overlay, g_object_ref (toast));
  adw_toast_dismiss (toast);

  g_assert_finalize_object (overlay);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Toast/title", test_adw_toast_title);
  g_test_add_func ("/Adwaita/Toast/button_label", test_adw_toast_button_label);
  g_test_add_func ("/Adwaita/Toast/action_name", test_adw_toast_action_name);
  g_test_add_func ("/Adwaita/Toast/action_target", test_adw_toast_action_target);
  g_test_add_func ("/Adwaita/Toast/detailed_action_name", test_adw_toast_detailed_action_name);
  g_test_add_func ("/Adwaita/Toast/priority", test_adw_toast_priority);
  g_test_add_func ("/Adwaita/Toast/dismiss", test_adw_toast_dismiss);

  return g_test_run ();
}
