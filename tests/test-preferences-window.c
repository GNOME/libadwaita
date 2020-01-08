/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_preferences_window_add (void)
{
  g_autoptr (HdyPreferencesWindow) window = NULL;
  HdyPreferencesPage *page;
  GtkWidget *widget;

  window = g_object_ref_sink (HDY_PREFERENCES_WINDOW (hdy_preferences_window_new ()));
  g_assert_nonnull (window);

  page = HDY_PREFERENCES_PAGE (hdy_preferences_page_new ());
  g_assert_nonnull (page);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (page));

  widget = gtk_switch_new ();
  g_assert_nonnull (widget);
  g_test_expect_message (HDY_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "Can't add children of type GtkSwitch to HdyPreferencesWindow");
  gtk_container_add (GTK_CONTAINER (window), widget);
  g_test_assert_expected_messages ();
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/PreferencesWindow/add", test_hdy_preferences_window_add);

  return g_test_run();
}
