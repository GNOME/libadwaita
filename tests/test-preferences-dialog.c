/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_preferences_dialog_add_remove (void)
{
  AdwPreferencesDialog *dialog = g_object_ref_sink (ADW_PREFERENCES_DIALOG (adw_preferences_dialog_new ()));
  AdwPreferencesPage *page;

  g_assert_nonnull (dialog);

  page = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
  g_assert_nonnull (page);
  adw_preferences_dialog_add (dialog, page);

  adw_preferences_dialog_remove (dialog, page);

  g_assert_finalize_object (dialog);
}

static void
test_adw_preferences_dialog_add_toast (void)
{
  AdwPreferencesDialog *dialog = g_object_ref_sink (ADW_PREFERENCES_DIALOG (adw_preferences_dialog_new ()));
  AdwToast *toast = adw_toast_new ("Test Notification");

  g_assert_nonnull (dialog);
  g_assert_nonnull (toast);

  adw_preferences_dialog_add_toast (dialog, g_object_ref (toast));

  g_assert_finalize_object (dialog);
  g_assert_finalize_object (toast);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/PreferencesDialog/add_remove", test_adw_preferences_dialog_add_remove);
  g_test_add_func("/Adwaita/PreferencesDialog/add_toast", test_adw_preferences_dialog_add_toast);

  return g_test_run();
}
