/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static void
test_adw_about_window_create (void)
{
  AdwAboutWindow *window = ADW_ABOUT_WINDOW (adw_about_window_new ());

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

  g_assert_nonnull (window);

  g_object_set (G_OBJECT (window),
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

  g_assert_cmpstr (adw_about_window_get_application_name (window), ==, "Example");
  g_assert_cmpstr (adw_about_window_get_application_icon (window), ==, "org.gnome.Example");
  g_assert_cmpstr (adw_about_window_get_developer_name (window), ==, "Angela Avery");
  g_assert_cmpstr (adw_about_window_get_version (window), ==, "1.2.3");
  g_assert_cmpstr (adw_about_window_get_release_notes_version (window), ==, "1.2.0");
  g_assert_cmpstr (adw_about_window_get_release_notes (window), ==, "<p>Example</p>");
  g_assert_cmpstr (adw_about_window_get_comments (window), ==, "Comments");
  g_assert_cmpstr (adw_about_window_get_website (window), ==, "https://example.org");
  g_assert_cmpstr (adw_about_window_get_issue_url (window), ==, "https://example.org");
  g_assert_cmpstr (adw_about_window_get_support_url (window), ==, "https://example.org");
  g_assert_cmpstr (adw_about_window_get_debug_info (window), ==, "Debug");
  g_assert_cmpstr (adw_about_window_get_debug_info_filename (window), ==, "debug.txt");
  g_assert_cmpstrv (adw_about_window_get_developers (window), developers);
  g_assert_cmpstrv (adw_about_window_get_designers (window), designers);
  g_assert_cmpstrv (adw_about_window_get_artists (window), artists);
  g_assert_cmpstrv (adw_about_window_get_documenters (window), documenters);
  g_assert_cmpstr (adw_about_window_get_translator_credits (window), ==, "translator-credits");
  g_assert_cmpstr (adw_about_window_get_copyright (window), ==, "© 2022 Angela Avery");
  g_assert_cmpuint (adw_about_window_get_license_type (window), ==, GTK_LICENSE_GPL_3_0);

  adw_about_window_add_link (window, "Example", "https://example.org");
  adw_about_window_add_credit_section (window, "Example", credits);
  adw_about_window_add_acknowledgement_section (window, "Example", acknowledgements);
  adw_about_window_add_legal_section (window, "Example", "© 2022 Example", GTK_LICENSE_GPL_3_0, NULL);
  adw_about_window_add_legal_section (window, "Example", "© 2022 Example", GTK_LICENSE_CUSTOM, "License");

  g_assert_finalize_object (window);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/AboutWindow/create", test_adw_about_window_create);

  return g_test_run ();
}
