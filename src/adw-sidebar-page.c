/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-sidebar-page-private.h"

#include "adw-action-row.h"
#include "adw-marshalers.h"
#include "adw-preferences-group.h"
#include "adw-preferences-page.h"
#include "adw-preferences-row.h"
#include "adw-sidebar-item.h"

struct _AdwSidebarPage
{
  AdwPreferencesPage parent_instance;

  GListModel *model;

  GListStore *groups;
};

G_DEFINE_FINAL_TYPE (AdwSidebarPage, adw_sidebar_page, ADW_TYPE_PREFERENCES_PAGE)

enum {
  SIGNAL_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
notify_icon_cb (AdwSidebarItem *item,
                GParamSpec     *pspec,
                GtkWidget      *image)
{
  const char *icon_name = adw_sidebar_item_get_icon_name (item);
  GdkPaintable *paintable = adw_sidebar_item_get_icon_paintable (item);

  if (paintable)
    gtk_image_set_from_paintable (GTK_IMAGE (image), paintable);
  else
    gtk_image_set_from_icon_name (GTK_IMAGE (image), icon_name);

  gtk_widget_set_visible (image, paintable || (icon_name && *icon_name));
}

static void
row_activated_cb (AdwSidebarPage *self,
                  AdwActionRow   *row)
{
  AdwSidebarItem *item;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));

  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0, item);
}

static void
notify_suffix_cb (AdwSidebarItem *item,
                  GParamSpec     *pspec,
                  AdwActionRow   *row)
{
  GtkWidget *old_suffix = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item-suffix");
  GtkWidget *arrow = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item-arrow");
  GtkWidget *suffix;

  if (old_suffix)
    adw_action_row_remove (row, old_suffix);

  suffix = adw_sidebar_item_get_suffix (item);

  if (suffix)
    adw_action_row_add_suffix (row, suffix);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-item-suffix", suffix);

  /* FIXME: Action row has no way to prepend a suffix
   * so we have to re-insert the arrow */
  if (arrow) {
    g_object_ref (arrow);
    adw_action_row_remove (row, arrow);
    adw_action_row_add_suffix (row, arrow);
    g_object_unref (arrow);
  }
}

static GtkWidget *
create_row (AdwSidebarItem *item,
            AdwSidebarPage *self)
{
  GtkWidget *row, *icon, *arrow;

  row = adw_action_row_new ();

  adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-item", item);

  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), TRUE);

  g_object_bind_property (item, "enabled", row, "sensitive", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "title", row, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "subtitle", row, "subtitle", G_BINDING_SYNC_CREATE);

  icon = g_object_new (GTK_TYPE_IMAGE,
                       "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                       NULL);
  gtk_widget_add_css_class (icon, "icon");
  g_signal_connect_object (item, "notify::icon-name",
                           G_CALLBACK (notify_icon_cb), icon, 0);
  g_signal_connect_object (item, "notify::icon-paintable",
                           G_CALLBACK (notify_icon_cb), icon, 0);
  notify_icon_cb (item, NULL, icon);
  adw_action_row_add_prefix (ADW_ACTION_ROW (row), icon);

  g_signal_connect_object (item, "notify::suffix",
                           G_CALLBACK (notify_suffix_cb), row, 0);
  notify_suffix_cb (item, NULL, ADW_ACTION_ROW (row));

  arrow = g_object_new (GTK_TYPE_IMAGE,
                        "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                        "icon-name", "go-next-symbolic",
                        NULL);
  gtk_widget_add_css_class (arrow, "arrow");
  adw_action_row_add_suffix (ADW_ACTION_ROW (row), arrow);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-arrow", arrow);

  g_signal_connect_swapped (row, "activated", G_CALLBACK (row_activated_cb), self);

  return row;
}

static gboolean
escape_markup (GBinding     *binding,
               const GValue *from_value,
               GValue       *to_value,
               gpointer      user_data)
{
  const char *str = g_value_get_string (from_value);

  g_value_take_string (to_value, g_markup_escape_text (str, -1));

  return TRUE;
}

static GtkWidget *
create_section (AdwSidebarPage    *self,
                AdwSidebarSection *section)
{
  GtkWidget *group = adw_preferences_group_new ();
  GtkWidget *list;
  GListModel *items = adw_sidebar_section_get_items (section);

  g_object_bind_property_full (section, "title", group, "title", G_BINDING_SYNC_CREATE,
                               escape_markup, NULL, NULL, NULL);

  list = gtk_list_box_new ();
  gtk_widget_add_css_class (list, "boxed-list");
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (list), GTK_SELECTION_NONE);
  gtk_list_box_bind_model (GTK_LIST_BOX (list), items,
                           (GtkListBoxCreateWidgetFunc) create_row,
                           self, NULL);

  adw_preferences_group_add (ADW_PREFERENCES_GROUP (group), list);

  g_object_set_data (G_OBJECT (group), "-adw-sidebar-section-list", list);

  g_object_unref (items);

  return group;
}

static void
sections_changed_cb (AdwSidebarPage *self,
                     guint           index,
                     guint           removed,
                     guint           added,
                     GListModel     *sections)
{
  guint i, n;

  if (!self->model)
    return;

  n = g_list_model_get_n_items (G_LIST_MODEL (self->groups));

  /* Preferences page cannot insert items in the middle so we rebuild everything
   * afterwards too */
  if (added > 0 && index + removed + 1 < n) {
    uint remaining = n - index;
    added += remaining - removed;
    removed = remaining;
  }

  if (removed > 0) {
    for (i = 0; i < removed; i++) {
      AdwPreferencesGroup *group = g_list_model_get_item (G_LIST_MODEL (self->groups), index + i);
      adw_preferences_page_remove (ADW_PREFERENCES_PAGE (self), group);
      g_object_unref (group);
    }

    g_list_store_splice (self->groups, index, removed, NULL, 0);
  }

  for (i = 0; i < added; i++) {
    AdwSidebarSection *section = g_list_model_get_item (self->model, index + i);
    GtkWidget *group = create_section (self, section);

    adw_preferences_page_add (ADW_PREFERENCES_PAGE (self), ADW_PREFERENCES_GROUP (group));

    g_list_store_append (self->groups, group);
    g_object_unref (section);
  }
}

static void
adw_sidebar_page_dispose (GObject *object)
{
  AdwSidebarPage *self = ADW_SIDEBAR_PAGE (object);

  g_clear_object (&self->model);
  g_clear_object (&self->groups);

  G_OBJECT_CLASS (adw_sidebar_page_parent_class)->dispose (object);
}

static void
adw_sidebar_page_class_init (AdwSidebarPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_sidebar_page_dispose;

  signals[SIGNAL_ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  ADW_TYPE_SIDEBAR_ITEM);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__OBJECTv);
}

static void
adw_sidebar_page_init (AdwSidebarPage *self)
{
  self->groups = g_list_store_new (ADW_TYPE_PREFERENCES_GROUP);
}

GtkWidget *
adw_sidebar_page_new (AdwSidebar *sidebar)
{
  AdwSidebarPage *page = g_object_new (ADW_TYPE_SIDEBAR_PAGE, NULL);

  page->model = adw_sidebar_get_sections (sidebar);

  sections_changed_cb (page,
                       0, 0, g_list_model_get_n_items (page->model),
                       page->model);

  g_signal_connect_object (page->model, "items-changed",
                           G_CALLBACK (sections_changed_cb), page,
                           G_CONNECT_SWAPPED);

  return GTK_WIDGET (page);
}

void
adw_sidebar_page_unparent_children (AdwSidebarPage *self)
{
  guint i, n = g_list_model_get_n_items (G_LIST_MODEL (self->groups));

  for (i = 0; i < n; i++) {
    AdwPreferencesGroup *group;
    GtkListBox *list;
    GtkListBoxRow *row;
    int index = 0;

    group = g_list_model_get_item (G_LIST_MODEL (self->groups), i);
    list = g_object_get_data (G_OBJECT (group), "-adw-sidebar-section-list");

    while ((row = gtk_list_box_get_row_at_index (list, index++)) != NULL) {
      GtkWidget *suffix = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item-suffix");

      if (suffix) {
        adw_action_row_remove (ADW_ACTION_ROW (row), suffix);
        g_object_set_data (G_OBJECT (row), "-adw-sidebar-item-suffix", NULL);
      }
    }

    g_object_unref (group);
  }
}
