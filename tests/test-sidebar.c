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
check_items (AdwSidebarSection *section,
             int                n_items,
             ...)
{
  GListModel *items = adw_sidebar_section_get_items (section);
  va_list args;
  guint i;

  va_start (args, n_items);

  g_assert_cmpint (g_list_model_get_n_items (items), ==, n_items);

  for (i = 0; i < n_items; i++) {
    AdwSidebarItem *item = g_list_model_get_item (items, i);
    const char *title = va_arg (args, const char *);

    g_assert_true (item == adw_sidebar_section_get_item (section, i));

    g_assert_cmpstr (adw_sidebar_item_get_title (item), ==, title);
    g_assert_cmpint (adw_sidebar_item_get_index (item), ==, i);
    g_assert_true (adw_sidebar_item_get_section (item) == section);

    g_object_unref (item);
  }

  va_end (args);

  g_assert_finalize_object (items);
}

static void
test_adw_sidebar_mode (void)
{
  AdwSidebar *sidebar = g_object_ref_sink (ADW_SIDEBAR (adw_sidebar_new ()));
  AdwSidebarMode mode;
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::mode", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "mode", &mode, NULL);
  g_assert_cmpint (mode, ==, ADW_SIDEBAR_MODE_SIDEBAR);

  adw_sidebar_set_mode (sidebar, ADW_SIDEBAR_MODE_PAGE);
  g_assert_cmpint (adw_sidebar_get_mode (sidebar), ==, ADW_SIDEBAR_MODE_PAGE);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "mode", ADW_SIDEBAR_MODE_SIDEBAR, NULL);
  g_assert_cmpint (adw_sidebar_get_mode (sidebar), ==, ADW_SIDEBAR_MODE_SIDEBAR);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/Sidebar/mode", test_adw_sidebar_mode);

  return g_test_run();
}
