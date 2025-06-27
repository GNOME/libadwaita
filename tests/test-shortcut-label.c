/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_shortcut_label_accelerator (void)
{
  AdwShortcutLabel *label = g_object_ref_sink (ADW_SHORTCUT_LABEL (adw_shortcut_label_new ("<Control>C")));
  char *accel;
  int notified = 0;

  g_assert_nonnull (label);

  g_signal_connect_swapped (label, "notify::accelerator", G_CALLBACK (increment), &notified);

  g_object_get (label, "accelerator", &accel, NULL);
  g_assert_cmpstr (accel, ==, "<Control>C");
  g_assert_cmpint (notified, ==, 0);

  adw_shortcut_label_set_accelerator (label, "<Control>X");
  g_assert_cmpstr (adw_shortcut_label_get_accelerator (label), ==, "<Control>X");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (label, "accelerator", "<Control>C", NULL);
  g_assert_cmpstr (adw_shortcut_label_get_accelerator (label), ==, "<Control>C");
  g_assert_cmpint (notified, ==, 2);

  g_free (accel);
  g_assert_finalize_object (label);
}

static void
test_adw_shortcut_label_disabled_text (void)
{
  AdwShortcutLabel *label = g_object_ref_sink (ADW_SHORTCUT_LABEL (adw_shortcut_label_new ("<Control>C")));
  char *accel;
  int notified = 0;

  g_assert_nonnull (label);

  g_signal_connect_swapped (label, "notify::disabled-text", G_CALLBACK (increment), &notified);

  g_object_get (label, "disabled-text", &accel, NULL);
  g_assert_cmpstr (accel, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adw_shortcut_label_set_disabled_text (label, "No Shortcut");
  g_assert_cmpstr (adw_shortcut_label_get_disabled_text (label), ==, "No Shortcut");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (label, "disabled-text", "Disabled", NULL);
  g_assert_cmpstr (adw_shortcut_label_get_disabled_text (label), ==, "Disabled");
  g_assert_cmpint (notified, ==, 2);

  g_free (accel);
  g_assert_finalize_object (label);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ShortcutLabel/accelerator", test_adw_shortcut_label_accelerator);
  g_test_add_func("/Adwaita/ShortcutLabel/disabled_text", test_adw_shortcut_label_disabled_text);

  return g_test_run();
}
