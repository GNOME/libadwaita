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
test_adw_sidebar_item_title (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("");
  char *title;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::title", G_CALLBACK (increment), &notified);

  g_object_get (item, "title", &title, NULL);
  g_assert_cmpstr (title, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_item_set_title (item, "Some title");
  g_assert_cmpstr (adw_sidebar_item_get_title (item), ==, "Some title");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "title", "Some other title", NULL);
  g_assert_cmpstr (adw_sidebar_item_get_title (item), ==, "Some other title");
  g_assert_cmpint (notified, ==, 2);

  g_free (title);
  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_subtitle (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  char *subtitle;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::subtitle", G_CALLBACK (increment), &notified);

  g_object_get (item, "subtitle", &subtitle, NULL);
  g_assert_cmpstr (subtitle, ==, "");
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_item_set_subtitle (item, "Some subtitle");
  g_assert_cmpstr (adw_sidebar_item_get_subtitle (item), ==, "Some subtitle");
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "subtitle", "Some other subtitle", NULL);
  g_assert_cmpstr (adw_sidebar_item_get_subtitle (item), ==, "Some other subtitle");
  g_assert_cmpint (notified, ==, 2);

  g_free (subtitle);
  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_use_underline (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  gboolean use_underline;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::use-underline", G_CALLBACK (increment), &notified);

  g_object_get (item, "use-underline", &use_underline, NULL);
  g_assert_false (use_underline);
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_item_set_use_underline (item, TRUE);
  g_assert_true (adw_sidebar_item_get_use_underline (item));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "use-underline", FALSE, NULL);
  g_assert_false (adw_sidebar_item_get_use_underline (item));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_icon (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  char *icon_name;
  GdkPaintable *icon_paintable;
  int notified_name = 0;
  int notified_paintable = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::icon-name", G_CALLBACK (increment), &notified_name);
  g_signal_connect_swapped (item, "notify::icon-paintable", G_CALLBACK (increment), &notified_paintable);

  g_object_get (item,
                "icon-name", &icon_name,
                "icon-paintable", &icon_paintable,
                NULL);

  g_assert_null (icon_name);
  g_assert_null (icon_paintable);

  g_assert_cmpint (notified_name, ==, 0);
  g_assert_cmpint (notified_paintable, ==, 0);

  adw_sidebar_item_set_icon_name (item, "something-symbolic");
  g_assert_cmpstr (adw_sidebar_item_get_icon_name (item), ==, "something-symbolic");
  g_assert_null (adw_sidebar_item_get_icon_paintable (item));
  g_assert_cmpint (notified_name, ==, 1);
  g_assert_cmpint (notified_paintable, ==, 0);

  g_object_set (item, "icon-name", "something-else-symbolic", NULL);
  g_assert_cmpstr (adw_sidebar_item_get_icon_name (item), ==, "something-else-symbolic");
  g_assert_null (adw_sidebar_item_get_icon_paintable (item));
  g_assert_cmpint (notified_name, ==, 2);
  g_assert_cmpint (notified_paintable, ==, 0);

  icon_paintable = GDK_PAINTABLE (adw_spinner_paintable_new (NULL));
  adw_sidebar_item_set_icon_paintable (item, icon_paintable);
  g_assert_null (adw_sidebar_item_get_icon_name (item));
  g_assert_true (adw_sidebar_item_get_icon_paintable (item) == icon_paintable);
  g_assert_cmpint (notified_name, ==, 3);
  g_assert_cmpint (notified_paintable, ==, 1);

  adw_sidebar_item_set_icon_paintable (item, NULL);
  g_assert_null (adw_sidebar_item_get_icon_name (item));
  g_assert_null (adw_sidebar_item_get_icon_paintable (item));
  g_assert_cmpint (notified_name, ==, 3);
  g_assert_cmpint (notified_paintable, ==, 2);

  g_clear_object (&icon_paintable);
  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_suffix (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  GtkWidget *suffix;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::suffix", G_CALLBACK (increment), &notified);

  g_object_get (item, "suffix", &suffix, NULL);
  g_assert_null (suffix);
  g_assert_cmpint (notified, ==, 0);

  suffix = g_object_ref_sink (gtk_switch_new ());
  adw_sidebar_item_set_suffix (item, suffix);
  g_assert_true (adw_sidebar_item_get_suffix (item) == suffix);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "suffix", NULL, NULL);
  g_assert_null (adw_sidebar_item_get_suffix (item));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (suffix);
  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_visible (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  gboolean visible;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::visible", G_CALLBACK (increment), &notified);

  g_object_get (item, "visible", &visible, NULL);
  g_assert_true (visible);
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_item_set_visible (item, FALSE);
  g_assert_false (adw_sidebar_item_get_visible (item));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "visible", TRUE, NULL);
  g_assert_true (adw_sidebar_item_get_visible (item));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_enabled (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  gboolean enabled;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::enabled", G_CALLBACK (increment), &notified);

  g_object_get (item, "enabled", &enabled, NULL);
  g_assert_true (enabled);
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_item_set_enabled (item, FALSE);
  g_assert_false (adw_sidebar_item_get_enabled (item));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "enabled", TRUE, NULL);
  g_assert_true (adw_sidebar_item_get_enabled (item));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_drag_motion_activate (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  gboolean drag_motion_activate;
  int notified = 0;

  g_assert_nonnull (item);

  g_signal_connect_swapped (item, "notify::drag-motion-activate", G_CALLBACK (increment), &notified);

  g_object_get (item, "drag-motion-activate", &drag_motion_activate, NULL);
  g_assert_true (drag_motion_activate);
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_item_set_drag_motion_activate (item, FALSE);
  g_assert_false (adw_sidebar_item_get_drag_motion_activate (item));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (item, "drag-motion-activate", TRUE, NULL);
  g_assert_true (adw_sidebar_item_get_drag_motion_activate (item));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_get_section (void)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("Item");
  AdwSidebarSection *section = adw_sidebar_section_new ();
  int notified = 0;

  g_assert_nonnull (item);
  g_assert_nonnull (section);

  g_signal_connect_swapped (item, "notify::section", G_CALLBACK (increment), &notified);

  g_assert_null (adw_sidebar_item_get_section (item));
  g_assert_cmpint (notified, ==, 0);

  adw_sidebar_section_append (section, g_object_ref (item));
  g_assert_true (adw_sidebar_item_get_section (item) == section);
  g_assert_cmpint (notified, ==, 1);

  adw_sidebar_section_remove (section, item);
  g_assert_null (adw_sidebar_item_get_section (item));
  g_assert_cmpint (notified, ==, 2);

  adw_sidebar_section_append (section, g_object_ref (item));
  g_assert_true (adw_sidebar_item_get_section (item) == section);
  g_assert_cmpint (notified, ==, 3);

  adw_sidebar_section_remove_all (section);
  g_assert_null (adw_sidebar_item_get_section (item));
  g_assert_cmpint (notified, ==, 4);

  g_assert_finalize_object (section);
  g_assert_finalize_object (item);
}

static void
test_adw_sidebar_item_get_section_index (void)
{
  AdwSidebarSection *section = adw_sidebar_section_new ();
  AdwSidebarItem *item1 = adw_sidebar_item_new ("Item 1");
  AdwSidebarItem *item2 = adw_sidebar_item_new ("Item 2");
  AdwSidebarItem *item3 = adw_sidebar_item_new ("Item 3");

  g_assert_nonnull (section);
  g_assert_nonnull (item1);
  g_assert_nonnull (item2);
  g_assert_nonnull (item3);

  g_assert_cmpuint (adw_sidebar_item_get_section_index (item1), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item2), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item3), ==, 0);

  adw_sidebar_section_prepend (section, g_object_ref (item1));
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item1), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item2), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item3), ==, 0);

  adw_sidebar_section_prepend (section, g_object_ref (item2));
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item1), ==, 1);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item2), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item3), ==, 0);

  adw_sidebar_section_append (section, g_object_ref (item3));
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item1), ==, 1);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item2), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item3), ==, 2);

  adw_sidebar_section_remove_all (section);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item1), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item2), ==, 0);
  g_assert_cmpuint (adw_sidebar_item_get_section_index (item3), ==, 0);

  g_assert_finalize_object (section);
  g_assert_finalize_object (item1);
  g_assert_finalize_object (item2);
  g_assert_finalize_object (item3);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/SidebarItem/title", test_adw_sidebar_item_title);
  g_test_add_func("/Adwaita/SidebarItem/subtitle", test_adw_sidebar_item_subtitle);
  g_test_add_func("/Adwaita/SidebarItem/use_underline", test_adw_sidebar_item_use_underline);
  g_test_add_func("/Adwaita/SidebarItem/icon", test_adw_sidebar_item_icon);
  g_test_add_func("/Adwaita/SidebarItem/suffix", test_adw_sidebar_item_suffix);
  g_test_add_func("/Adwaita/SidebarItem/visible", test_adw_sidebar_item_visible);
  g_test_add_func("/Adwaita/SidebarItem/enabled", test_adw_sidebar_item_enabled);
  g_test_add_func("/Adwaita/SidebarItem/drag_motion_activate", test_adw_sidebar_item_drag_motion_activate);
  g_test_add_func("/Adwaita/SidebarItem/get_section", test_adw_sidebar_item_get_section);
  g_test_add_func("/Adwaita/SidebarItem/get_section_index", test_adw_sidebar_item_get_section_index);

  return g_test_run();
}
