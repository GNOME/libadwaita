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
             guint              n_items,
             ...)
{
  GListModel *items = adw_sidebar_section_get_items (section);
  va_list args;
  guint i;

  va_start (args, n_items);

  g_assert_cmpuint (g_list_model_get_n_items (items), ==, n_items);

  for (i = 0; i < n_items; i++) {
    AdwSidebarItem *item = g_list_model_get_item (items, i);
    const char *title = va_arg (args, const char *);

    g_assert_true (item == adw_sidebar_section_get_item (section, i));

    g_assert_cmpstr (adw_sidebar_item_get_title (item), ==, title);
    g_assert_cmpuint (adw_sidebar_item_get_section_index (item), ==, i);
    g_assert_true (adw_sidebar_item_get_section (item) == section);

    g_object_unref (item);
  }

  va_end (args);

  g_assert_finalize_object (items);
}

static void
test_adw_sidebar_section_title (void)
{
  AdwSidebarSection *section = adw_sidebar_section_new ();
  char *title;
  int notified = 0;

  g_assert_nonnull (section);

  g_signal_connect_swapped (section, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (section, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_section_set_title (section, "Some title");
  g_assert_cmpstr (adw_sidebar_section_get_title (section), ==, "Some title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (section, "title", "Some other title", NULL);
  g_assert_cmpstr (adw_sidebar_section_get_title (section), ==, "Some other title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (section);
}

static void
test_adw_sidebar_section_add_remove (void)
{
  AdwSidebarSection *section = adw_sidebar_section_new ();
  AdwSidebarItem *item1 = adw_sidebar_item_new ("Item 1");
  AdwSidebarItem *item2 = adw_sidebar_item_new ("Item 2");
  AdwSidebarItem *item3 = adw_sidebar_item_new ("Item 3");

  g_assert_nonnull (section);
  g_assert_nonnull (item1);
  g_assert_nonnull (item2);
  g_assert_nonnull (item3);

  check_items (section, 0);

  adw_sidebar_section_append (section, g_object_ref (item1));
  check_items (section, 1, "Item 1");
  adw_sidebar_section_append (section, g_object_ref (item2));
  check_items (section, 2, "Item 1", "Item 2");
  adw_sidebar_section_append (section, g_object_ref (item3));
  check_items (section, 3, "Item 1", "Item 2", "Item 3");

  adw_sidebar_section_remove_all (section);
  check_items (section, 0);

  adw_sidebar_section_prepend (section, g_object_ref (item1));
  check_items (section, 1, "Item 1");
  adw_sidebar_section_prepend (section, g_object_ref (item2));
  check_items (section, 2, "Item 2", "Item 1");
  adw_sidebar_section_prepend (section, g_object_ref (item3));
  check_items (section, 3, "Item 3", "Item 2", "Item 1");

  adw_sidebar_section_remove (section, item2);
  check_items (section, 2, "Item 3", "Item 1");

  adw_sidebar_section_remove_all (section);
  check_items (section, 0);

  adw_sidebar_section_insert (section, g_object_ref (item1), 1);
  check_items (section, 1, "Item 1");
  adw_sidebar_section_insert (section, g_object_ref (item2), 1);
  check_items (section, 2, "Item 1", "Item 2");
  adw_sidebar_section_insert (section, g_object_ref (item3), 1);
  check_items (section, 3, "Item 1", "Item 3", "Item 2");

  adw_sidebar_section_remove_all (section);
  check_items (section, 0);

  adw_sidebar_section_insert (section, g_object_ref (item1), -1);
  check_items (section, 1, "Item 1");
  adw_sidebar_section_insert (section, g_object_ref (item2), -1);
  check_items (section, 2, "Item 1", "Item 2");
  adw_sidebar_section_insert (section, g_object_ref (item3), -1);
  check_items (section, 3, "Item 1", "Item 2", "Item 3");

  g_assert_finalize_object (section);
  g_assert_finalize_object (item1);
  g_assert_finalize_object (item2);
  g_assert_finalize_object (item3);
}

static AdwSidebarItem *
create_item (gpointer item,
             gpointer user_data)
{
  const char *title = gtk_string_object_get_string (GTK_STRING_OBJECT (item));

  return adw_sidebar_item_new (title);
}

static void
test_adw_sidebar_section_bind_model (void)
{
  AdwSidebarSection *section = adw_sidebar_section_new ();
  GtkStringList *list = gtk_string_list_new (NULL);

  g_assert_nonnull (section);
  g_assert_nonnull (list);

  gtk_string_list_append (list, "Item 1");
  gtk_string_list_append (list, "Item 2");
  gtk_string_list_append (list, "Item 3");

  adw_sidebar_section_append (section, adw_sidebar_item_new ("Item"));
  check_items (section, 1, "Item");

  adw_sidebar_section_bind_model (section, G_LIST_MODEL (list),
                                  create_item, NULL, NULL);
  check_items (section, 3, "Item 1", "Item 2", "Item 3");

  gtk_string_list_append (list, "Item 4");
  check_items (section, 4, "Item 1", "Item 2", "Item 3", "Item 4");

  gtk_string_list_remove (list, 2);
  check_items (section, 3, "Item 1", "Item 2", "Item 4");

  adw_sidebar_section_bind_model (section, NULL, NULL, NULL, NULL);
  check_items (section, 0);

  adw_sidebar_section_append (section, adw_sidebar_item_new ("Item"));
  check_items (section, 1, "Item");

  g_assert_finalize_object (section);
  g_assert_finalize_object (list);
}

static void
test_adw_sidebar_section_get_sidebar (void)
{
  AdwSidebarSection *section = adw_sidebar_section_new ();
  AdwSidebar *sidebar = g_object_ref_sink (ADW_SIDEBAR (adw_sidebar_new ()));
  int notified = 0;

  g_assert_nonnull (section);
  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (section, "notify::sidebar", G_CALLBACK (increment), &notified);

  g_assert_null (adw_sidebar_section_get_sidebar (section));
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_append (sidebar, g_object_ref (section));
  g_assert_true (adw_sidebar_section_get_sidebar (section) == sidebar);
  g_assert_cmpint (notified, ==, 1);

  adw_sidebar_remove (sidebar, section);
  g_assert_null (adw_sidebar_section_get_sidebar (section));
  g_assert_cmpint (notified, ==, 2);

  adw_sidebar_append (sidebar, g_object_ref (section));
  g_assert_true (adw_sidebar_section_get_sidebar (section) == sidebar);
  g_assert_cmpint (notified, ==, 3);

  adw_sidebar_remove_all (sidebar);
  g_assert_null (adw_sidebar_section_get_sidebar (section));
  g_assert_cmpint (notified, ==, 4);

  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (section);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/SidebarSection/title", test_adw_sidebar_section_title);
  g_test_add_func("/Adwaita/SidebarSection/add_remove", test_adw_sidebar_section_add_remove);
  g_test_add_func("/Adwaita/SidebarSection/bind_model", test_adw_sidebar_section_bind_model);
  g_test_add_func("/Adwaita/SidebarSection/get_sidebar", test_adw_sidebar_section_get_sidebar);

  return g_test_run();
}
