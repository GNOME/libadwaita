/*
 * Copyright (C) 2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

#include "adwaita-test-resources.h"

static void
test_adw_about_dialog_from_appdata (void)
{
  AdwAboutDialog *dialog = g_object_ref_sink (ADW_ABOUT_DIALOG (adw_about_dialog_new_from_appdata ("/org/gnome/Adwaita1/Test/org.gnome.Adwaita1.Test.metainfo.xml", "1.0")));

  g_assert_nonnull (dialog);

  g_assert_cmpstr (adw_about_dialog_get_release_notes (dialog), ==, "<p>Testing Build</p>\n");
  g_assert_cmpstr (adw_about_dialog_get_release_notes_version (dialog), ==, "1.0");
  g_assert_cmpstr (adw_about_dialog_get_version (dialog), ==, "1.0");
  g_assert_cmpstr (adw_about_dialog_get_application_icon (dialog), ==, "org.gnome.Adwaita1.Test");
  g_assert_cmpstr (adw_about_dialog_get_application_name (dialog), ==, "Adwaita Test");
  g_assert_cmpstr (adw_about_dialog_get_developer_name (dialog), ==, "The GNOME Project");
  g_assert_cmpstr (adw_about_dialog_get_issue_url (dialog), ==, "https://gitlab.gnome.org/GNOME/libadwaita/issues");
  g_assert_cmpstr (adw_about_dialog_get_support_url (dialog), ==, "http://www.gnome.org/friends/");
  g_assert_cmpstr (adw_about_dialog_get_website (dialog), ==, "https://gitlab.gnome.org/GNOME/libadwaita");
  g_assert_cmpuint (adw_about_dialog_get_license_type (dialog), ==, GTK_LICENSE_LGPL_2_1);

  g_assert_finalize_object (dialog);

  dialog = g_object_ref_sink (ADW_ABOUT_DIALOG (adw_about_dialog_new_from_appdata ("/org/gnome/Adwaita1/Test/org.gnome.Adwaita1.Test.metainfo.xml", "0.1")));

  g_assert_nonnull (dialog);

  g_assert_cmpstr (adw_about_dialog_get_release_notes (dialog), ==, "<p>Testing Build Older</p>\n");
  g_assert_cmpstr (adw_about_dialog_get_release_notes_version (dialog), ==, "0.1");
  g_assert_cmpstr (adw_about_dialog_get_version (dialog), ==, "1.0");

  g_assert_finalize_object (dialog);

  dialog = g_object_ref_sink (ADW_ABOUT_DIALOG (adw_about_dialog_new_from_appdata ("/org/gnome/Adwaita1/Test/org.gnome.Adwaita1.Test.metainfo.xml", NULL)));

  g_assert_nonnull (dialog);

  g_assert_cmpstr (adw_about_dialog_get_release_notes (dialog), ==, "");
  g_assert_cmpstr (adw_about_dialog_get_release_notes_version (dialog), ==, "");
  g_assert_cmpstr (adw_about_dialog_get_version (dialog), ==, "1.0");

  g_assert_finalize_object (dialog);
}

static void
test_adw_about_dialog_create (void)
{
  AdwAboutDialog *dialog = g_object_ref_sink (ADW_ABOUT_DIALOG (adw_about_dialog_new ()));

  const char *developers[] = {
    "Angela Avery",
    NULL
  };

  const char *designers[] = {
    "GNOME Design Team",
    NULL
  };

  const char *artists[] = {
    "GNOME Design Team",
    NULL
  };

  const char *documenters[] = {
    "Angela Avery",
    NULL
  };

  const char *credits[] = {
    "Angela Avery",
    NULL
  };

  const char *acknowledgements[] = {
    "Angela Avery",
    NULL
  };

  g_assert_nonnull (dialog);

  g_object_set (G_OBJECT (dialog),
                "application-name", "Example",
                "application-icon", "org.gnome.Example",
                "developer-name", "Angela Avery",
                "version", "1.2.3",
                "release-notes-version", "1.2.0",
                "release-notes", "<p>Example</p>",
                "comments", "Comments",
                "website", "https://example.org",
                "issue-url", "https://example.org",
                "support-url", "https://example.org",
                "debug-info", "Debug",
                "debug-info-filename", "debug.txt",
                "developers", developers,
                "designers", designers,
                "artists", artists,
                "documenters", documenters,
                "translator-credits", "translator-credits",
                "copyright", "© 2022 Angela Avery",
                "license-type", GTK_LICENSE_GPL_3_0,
                NULL);

  g_assert_cmpstr (adw_about_dialog_get_application_name (dialog), ==, "Example");
  g_assert_cmpstr (adw_about_dialog_get_application_icon (dialog), ==, "org.gnome.Example");
  g_assert_cmpstr (adw_about_dialog_get_developer_name (dialog), ==, "Angela Avery");
  g_assert_cmpstr (adw_about_dialog_get_version (dialog), ==, "1.2.3");
  g_assert_cmpstr (adw_about_dialog_get_release_notes_version (dialog), ==, "1.2.0");
  g_assert_cmpstr (adw_about_dialog_get_release_notes (dialog), ==, "<p>Example</p>");
  g_assert_cmpstr (adw_about_dialog_get_comments (dialog), ==, "Comments");
  g_assert_cmpstr (adw_about_dialog_get_website (dialog), ==, "https://example.org");
  g_assert_cmpstr (adw_about_dialog_get_issue_url (dialog), ==, "https://example.org");
  g_assert_cmpstr (adw_about_dialog_get_support_url (dialog), ==, "https://example.org");
  g_assert_cmpstr (adw_about_dialog_get_debug_info (dialog), ==, "Debug");
  g_assert_cmpstr (adw_about_dialog_get_debug_info_filename (dialog), ==, "debug.txt");
  g_assert_cmpstrv (adw_about_dialog_get_developers (dialog), developers);
  g_assert_cmpstrv (adw_about_dialog_get_designers (dialog), designers);
  g_assert_cmpstrv (adw_about_dialog_get_artists (dialog), artists);
  g_assert_cmpstrv (adw_about_dialog_get_documenters (dialog), documenters);
  g_assert_cmpstr (adw_about_dialog_get_translator_credits (dialog), ==, "translator-credits");
  g_assert_cmpstr (adw_about_dialog_get_copyright (dialog), ==, "© 2022 Angela Avery");
  g_assert_cmpuint (adw_about_dialog_get_license_type (dialog), ==, GTK_LICENSE_GPL_3_0);

  adw_about_dialog_add_link (dialog, "Example", "https://example.org");
  adw_about_dialog_add_credit_section (dialog, "Example", credits);
  adw_about_dialog_add_acknowledgement_section (dialog, "Example", acknowledgements);
  adw_about_dialog_add_legal_section (dialog, "Example", "© 2022 Example", GTK_LICENSE_GPL_3_0, NULL);
  adw_about_dialog_add_legal_section (dialog, "Example", "© 2022 Example", GTK_LICENSE_CUSTOM, "License");
  adw_about_dialog_add_other_app (dialog, "org.example.App", "Example App", "Example summary");

  g_assert_finalize_object (dialog);
}

int
main (int   argc,
      char *argv[])
{
  GResource *test_resources;

  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  test_resources = test_get_resource ();
  g_resources_register (test_resources);

  g_test_add_func ("/Adwaita/AboutDialog/create", test_adw_about_dialog_create);
  g_test_add_func ("/Adwaita/AboutDialog/from_appdata", test_adw_about_dialog_from_appdata);

  return g_test_run ();
}
