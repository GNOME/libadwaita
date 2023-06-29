/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_toast_title (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  char *title;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");

  adw_toast_set_title (toast, "Another title");
  g_assert_cmpstr (adw_toast_get_title (toast), ==, "Another title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "title", "Title", NULL);
  g_assert_cmpstr (adw_toast_get_title (toast), ==, "Title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (toast);
}

static void
test_adw_toast_title_format (void)
{
  AdwToast *toast;
  const int n_value = 42;
  char *title;

  toast = adw_toast_new_format ("Title %d", n_value);

  g_assert_nonnull (toast);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title 42");

  g_free (title);
  g_assert_finalize_object (toast);
}

static void
test_adw_toast_button_label (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  char *button_label;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::button-label", G_CALLBACK (increment), &notified);

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
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::action-name", G_CALLBACK (increment), &notified);

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
  GVariant *action_target, *variant;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::action-target", G_CALLBACK (increment), &notified);

  g_object_get (toast, "action-target", &action_target, NULL);
  g_assert_null (action_target);

  variant = g_variant_ref_sink (g_variant_new_int32 (1));
  adw_toast_set_action_target_value (toast, g_variant_new_int32 (1));
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant);
  g_assert_cmpint (notified, ==, 1);
  g_variant_unref (variant);

  variant = g_variant_ref_sink (g_variant_new_int32 (2));
  g_object_set (toast, "action-target", g_variant_new_int32 (2), NULL);
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant);
  g_assert_cmpint (notified, ==, 2);
  g_variant_unref (variant);

  variant = g_variant_ref_sink (g_variant_new_int32 (3));
  adw_toast_set_action_target (toast, "i", 3);
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant);
  g_assert_cmpint (notified, ==, 3);
  g_variant_unref (variant);

  g_assert_finalize_object (toast);
}

static void
test_adw_toast_detailed_action_name (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  GVariant *variant = g_variant_ref_sink (g_variant_new_int32 (2));

  g_assert_nonnull (toast);

  g_assert_null (adw_toast_get_action_name (toast));
  g_assert_null (adw_toast_get_action_target_value (toast));

  adw_toast_set_detailed_action_name (toast, "win.something");
  g_assert_cmpstr (adw_toast_get_action_name (toast), ==, "win.something");
  g_assert_null (adw_toast_get_action_target_value (toast));

  adw_toast_set_detailed_action_name (toast, "win.something(2)");
  g_assert_cmpstr (adw_toast_get_action_name (toast), ==, "win.something");
  g_assert_cmpvariant (adw_toast_get_action_target_value (toast), variant);

  g_variant_unref (variant);
  g_assert_finalize_object (toast);
}

static void
test_adw_toast_priority (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  AdwToastPriority priority;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::priority", G_CALLBACK (increment), &notified);

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
test_adw_toast_timeout (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  guint timeout;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::timeout", G_CALLBACK (increment), &notified);

  g_object_get (toast, "timeout", &timeout, NULL);
  g_assert_cmpint (timeout, ==, 5);

  adw_toast_set_timeout (toast, 10);
  g_assert_cmpint (adw_toast_get_timeout (toast), ==, 10);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (toast, "timeout", 5, NULL);
  g_assert_cmpint (adw_toast_get_timeout (toast), ==, 5);
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

  /* Repeat dismiss() calls should no-op */
  adw_toast_overlay_add_toast (overlay, g_object_ref (toast));
  adw_toast_dismiss (toast);
  adw_toast_dismiss (toast);
  adw_toast_dismiss (toast);

  g_assert_finalize_object (overlay);
  g_assert_finalize_object (toast);
}

static void
test_adw_toast_custom_title (void)
{
  AdwToast *toast = adw_toast_new ("Title");
  GtkWidget *widget = NULL;
  char *title;
  int notified = 0;

  g_assert_nonnull (toast);

  g_signal_connect_swapped (toast, "notify::custom-title", G_CALLBACK (increment), &notified);

  g_object_get (toast, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "Title");
  g_object_get (toast, "custom-title", &widget, NULL);
  g_assert_null (widget);

  adw_toast_set_title (toast, "Another title");
  g_assert_cmpint (notified, ==, 0);

  widget = g_object_ref_sink (gtk_label_new ("Custom title"));
  adw_toast_set_custom_title (toast, widget);
  g_assert_true (adw_toast_get_custom_title (toast) == widget);
  g_assert_null (adw_toast_get_title (toast));
  g_assert_cmpint (notified, ==, 1);

  adw_toast_set_title (toast, "Final title");
  g_assert_null (adw_toast_get_custom_title (toast));
  g_assert_cmpstr (adw_toast_get_title (toast), ==, "Final title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (toast);
  g_assert_finalize_object (widget);
}

static void
test_adw_toast_custom_title_overlay (void)
{
  AdwToastOverlay *first_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));
  AdwToastOverlay *second_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new ()));
  AdwToast *toast = adw_toast_new ("");
  GtkWidget *widget = gtk_label_new ("Custom title");

  g_assert_nonnull (first_overlay);
  g_assert_nonnull (second_overlay);
  g_assert_nonnull (toast);

  adw_toast_set_custom_title (toast, g_object_ref (widget));

  adw_toast_overlay_add_toast (first_overlay, g_object_ref (toast));
  adw_toast_dismiss (toast);
  adw_toast_overlay_add_toast (second_overlay, g_object_ref (toast));

  g_assert_finalize_object (first_overlay);
  g_assert_finalize_object (second_overlay);
  g_assert_finalize_object (toast);
  g_assert_finalize_object (widget);
}

static void
test_adw_toast_use_markup (void)
{
  AdwToastOverlay *toast_overlay = g_object_ref_sink (ADW_TOAST_OVERLAY (adw_toast_overlay_new()));
  AdwToast *toast = adw_toast_new ("");

  g_assert_nonnull (toast_overlay);
  g_assert_nonnull (toast);

  adw_toast_overlay_add_toast (toast_overlay, g_object_ref (toast));
  adw_toast_set_use_markup (toast, FALSE);
  adw_toast_set_title (toast, "<span false>bad markup</sp>");

  g_assert_cmpstr (adw_toast_get_title (toast), ==, "<span false>bad markup</sp>");

  g_assert_finalize_object (toast_overlay);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Toast/title", test_adw_toast_title);
  g_test_add_func ("/Adwaita/Toast/title_format", test_adw_toast_title_format);
  g_test_add_func ("/Adwaita/Toast/button_label", test_adw_toast_button_label);
  g_test_add_func ("/Adwaita/Toast/action_name", test_adw_toast_action_name);
  g_test_add_func ("/Adwaita/Toast/action_target", test_adw_toast_action_target);
  g_test_add_func ("/Adwaita/Toast/detailed_action_name", test_adw_toast_detailed_action_name);
  g_test_add_func ("/Adwaita/Toast/priority", test_adw_toast_priority);
  g_test_add_func ("/Adwaita/Toast/timeout", test_adw_toast_timeout);
  g_test_add_func ("/Adwaita/Toast/dismiss", test_adw_toast_dismiss);
  g_test_add_func ("/Adwaita/Toast/custom_title", test_adw_toast_custom_title);
  g_test_add_func ("/Adwaita/Toast/custom_title_overlay", test_adw_toast_custom_title_overlay);
  g_test_add_func ("/Adwaita/Toast/use_markup", test_adw_toast_use_markup);

  return g_test_run ();
}
