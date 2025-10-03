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

static AdwSidebarSection *
create_section (const char *title,
                guint       n_items,
                ...)
{
  AdwSidebarSection *section = adw_sidebar_section_new ();
  va_list args;
  guint i;

  adw_sidebar_section_set_title (section, title);

  va_start (args, n_items);

  for (i = 0; i < n_items; i++) {
    const char *item_title = va_arg (args, const char *);
    AdwSidebarItem *item = adw_sidebar_item_new (item_title);

    adw_sidebar_section_append (section, item);
  }

  return section;
}

static void
check_items (AdwSidebar *sidebar,
             guint       selected,
             guint       n_items,
             ...)
{
  GListModel *items = G_LIST_MODEL (adw_sidebar_get_items (sidebar));
  va_list args;
  guint i;

  va_start (args, n_items);

  g_assert_cmpuint (adw_sidebar_get_selected (sidebar), ==, selected);
  g_assert_cmpuint (g_list_model_get_n_items (items), ==, n_items);

  for (i = 0; i < n_items; i++) {
    AdwSidebarItem *item = g_list_model_get_item (items, i);
    const char *title = va_arg (args, const char *);

    g_assert_true (item == adw_sidebar_get_item (sidebar, i));

    g_assert_cmpstr (adw_sidebar_item_get_title (item), ==, title);
    g_assert_cmpuint (adw_sidebar_item_get_index (item), ==, i);

    if (i == selected) {
      g_assert_true (adw_sidebar_get_selected_item (sidebar) == item);
      g_assert_true (gtk_selection_model_is_selected (GTK_SELECTION_MODEL (items), i));
    } else {
      g_assert_false (gtk_selection_model_is_selected (GTK_SELECTION_MODEL (items), i));
    }

    g_object_unref (item);
  }

  va_end (args);

  g_object_unref (items);
}

static void
check_sections (AdwSidebar *sidebar,
                guint       n_sections,
                ...)
{
  GListModel *sections = adw_sidebar_get_sections (sidebar);
  GListModel *items = G_LIST_MODEL (adw_sidebar_get_items (sidebar));
  va_list args;
  guint i;
  guint section_start = 0;

  va_start (args, n_sections);

  g_assert_cmpuint (g_list_model_get_n_items (sections), ==, n_sections);

  for (i = 0; i < n_sections; i++) {
    AdwSidebarSection *section = g_list_model_get_item (sections, i);
    GListModel *section_items = adw_sidebar_section_get_items (section);
    const char *title = va_arg (args, const char *);
    guint j, n_items;

    g_assert_true (section == adw_sidebar_get_section (sidebar, i));

    g_assert_cmpstr (adw_sidebar_section_get_title (section), ==, title);
    g_assert_true (adw_sidebar_section_get_sidebar (section) == sidebar);

    n_items = g_list_model_get_n_items (section_items);

    for (j = 0; j < n_items; j++) {
      AdwSidebarItem *item1 = g_list_model_get_item (section_items, j);
      AdwSidebarItem *item2 = g_list_model_get_item (items, section_start + j);
      guint start, end;

      g_assert_true (item1 == item2);
      g_assert_true (adw_sidebar_item_get_section (item2) == section);
      g_assert_cmpuint (adw_sidebar_item_get_section_index (item2), ==, j);

      gtk_section_model_get_section (GTK_SECTION_MODEL (items),
                                     section_start + j, &start, &end);

      g_assert_cmpuint (start, ==, section_start);
      g_assert_cmpuint (end, ==, section_start + n_items);

      g_object_unref (item1);
      g_object_unref (item2);
    }

    section_start += n_items;

    g_object_unref (section_items);
    g_object_unref (section);
  }

  va_end (args);

  g_object_unref (sections);
  g_object_unref (items);
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

static void
test_adw_sidebar_filter (void)
{
  AdwSidebar *sidebar = g_object_ref_sink (ADW_SIDEBAR (adw_sidebar_new ()));
  GtkFilter *filter = NULL;
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::filter", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "filter", &filter, NULL);
  g_assert_null (filter);

  adw_sidebar_set_filter (sidebar, NULL);
  g_assert_cmpint (notified, ==, 0);

  filter = GTK_FILTER (gtk_bool_filter_new (NULL));
  adw_sidebar_set_filter (sidebar, filter);
  g_assert_true (adw_sidebar_get_filter (sidebar) == filter);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "filter", NULL, NULL);
  g_assert_null (adw_sidebar_get_filter (sidebar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (filter);
}

static void
test_adw_sidebar_placeholder (void)
{
  AdwSidebar *sidebar = g_object_ref_sink (ADW_SIDEBAR (adw_sidebar_new ()));
  GtkWidget *placeholder = NULL;
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::placeholder", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "placeholder", &placeholder, NULL);
  g_assert_null (placeholder);

  adw_sidebar_set_placeholder (sidebar, NULL);
  g_assert_cmpint (notified, ==, 0);

  placeholder = gtk_button_new ();
  adw_sidebar_set_placeholder (sidebar, placeholder);
  g_assert_true (adw_sidebar_get_placeholder (sidebar) == placeholder);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "placeholder", NULL, NULL);
  g_assert_null (adw_sidebar_get_placeholder (sidebar));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
}

static void
test_adw_sidebar_add_remove (void)
{
  AdwSidebar *sidebar = g_object_ref_sink (ADW_SIDEBAR (adw_sidebar_new ()));
  AdwSidebarSection *section1 = create_section ("Section 1", 1, "1");
  AdwSidebarSection *section2 = create_section ("Section 2", 2, "2a", "2b");
  AdwSidebarSection *section3 = create_section ("Section 3", 3, "3a", "3b", "3c");

  g_assert_nonnull (sidebar);
  g_assert_nonnull (section1);
  g_assert_nonnull (section2);
  g_assert_nonnull (section3);

  check_items (sidebar, GTK_INVALID_LIST_POSITION, 0);
  check_sections (sidebar, 0);


  adw_sidebar_append (sidebar, g_object_ref (section1));
  check_items (sidebar, 0, 1, "1");
  check_sections (sidebar, 1, "Section 1");

  adw_sidebar_append (sidebar, g_object_ref (section2));
  check_items (sidebar, 0, 3, "1", "2a", "2b");
  check_sections (sidebar, 2, "Section 1", "Section 2");

  adw_sidebar_append (sidebar, g_object_ref (section3));
  check_items (sidebar, 0, 6, "1", "2a", "2b", "3a", "3b", "3c");
  check_sections (sidebar, 3, "Section 1", "Section 2", "Section 3");


  adw_sidebar_remove_all (sidebar);
  check_items (sidebar, GTK_INVALID_LIST_POSITION, 0);
  check_sections (sidebar, 0);


  adw_sidebar_prepend (sidebar, g_object_ref (section1));
  check_items (sidebar, 0, 1, "1");
  check_sections (sidebar, 1, "Section 1");

  adw_sidebar_prepend (sidebar, g_object_ref (section2));
  check_items (sidebar, 2, 3, "2a", "2b", "1");
  check_sections (sidebar, 2, "Section 2", "Section 1");

  adw_sidebar_prepend (sidebar, g_object_ref (section3));
  check_items (sidebar, 5, 6, "3a", "3b", "3c", "2a", "2b", "1");
  check_sections (sidebar, 3, "Section 3", "Section 2", "Section 1");

  adw_sidebar_remove (sidebar, section2);
  check_items (sidebar, 3, 4, "3a", "3b", "3c", "1");
  check_sections (sidebar, 2, "Section 3", "Section 1");

  adw_sidebar_remove (sidebar, section1);
  check_items (sidebar, GTK_INVALID_LIST_POSITION, 3, "3a", "3b", "3c");
  check_sections (sidebar, 1, "Section 3");

  adw_sidebar_remove_all (sidebar);
  check_items (sidebar, GTK_INVALID_LIST_POSITION, 0);
  check_sections (sidebar, 0);


  adw_sidebar_insert (sidebar, g_object_ref (section1), 1);
  check_items (sidebar, 0, 1, "1");
  check_sections (sidebar, 1, "Section 1");

  adw_sidebar_insert (sidebar, g_object_ref (section2), 1);
  check_items (sidebar, 0, 3, "1", "2a", "2b");
  check_sections (sidebar, 2, "Section 1", "Section 2");

  adw_sidebar_insert (sidebar, g_object_ref (section3), 1);
  check_items (sidebar, 0, 6, "1", "3a", "3b", "3c", "2a", "2b");
  check_sections (sidebar, 3, "Section 1", "Section 3", "Section 2");

  adw_sidebar_remove_all (sidebar);
  check_items (sidebar, GTK_INVALID_LIST_POSITION, 0);
  check_sections (sidebar, 0);


  adw_sidebar_insert (sidebar, g_object_ref (section1), -1);
  check_items (sidebar, 0, 1, "1");
  check_sections (sidebar, 1, "Section 1");

  adw_sidebar_insert (sidebar, g_object_ref (section2), -1);
  check_items (sidebar, 0, 3, "1", "2a", "2b");
  check_sections (sidebar, 2, "Section 1", "Section 2");

  adw_sidebar_insert (sidebar, g_object_ref (section3), -1);
  check_items (sidebar, 0, 6, "1", "2a", "2b", "3a", "3b", "3c");
  check_sections (sidebar, 3, "Section 1", "Section 2", "Section 3");


  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (section1);
  g_assert_finalize_object (section2);
  g_assert_finalize_object (section3);
}

static void
test_adw_sidebar_menu_model (void)
{
  AdwSidebar *sidebar = g_object_ref_sink (ADW_SIDEBAR (adw_sidebar_new ()));
  GMenuModel *model;
  GMenuModel *model1 = G_MENU_MODEL (g_menu_new ());
  GMenuModel *model2 = G_MENU_MODEL (g_menu_new ());
  int notified = 0;

  g_assert_nonnull (sidebar);

  g_signal_connect_swapped (sidebar, "notify::menu-model", G_CALLBACK (increment), &notified);

  g_object_get (sidebar, "menu-model", &model, NULL);
  g_assert_null (model);
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_set_menu_model (sidebar, model1);
  g_assert_true (adw_sidebar_get_menu_model (sidebar) == model1);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (sidebar, "menu-model", model2, NULL);
  g_assert_true (adw_sidebar_get_menu_model (sidebar) == model2);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (sidebar);
  g_assert_finalize_object (model1);
  g_assert_finalize_object (model2);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/Sidebar/mode", test_adw_sidebar_mode);
  g_test_add_func("/Adwaita/Sidebar/filter", test_adw_sidebar_filter);
  g_test_add_func("/Adwaita/Sidebar/placeholder", test_adw_sidebar_placeholder);
  g_test_add_func("/Adwaita/Sidebar/add_remove", test_adw_sidebar_add_remove);
  g_test_add_func("/Adwaita/Sidebar/menu_model", test_adw_sidebar_menu_model);

  return g_test_run();
}
