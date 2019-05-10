/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-preferences-window.h"

#include "hdy-action-row.h"
#include "hdy-list-box.h"
#include "hdy-preferences-group-private.h"
#include "hdy-preferences-page-private.h"
#include "hdy-squeezer.h"
#include "hdy-view-switcher.h"
#include "hdy-view-switcher-bar.h"

/**
 * SECTION:hdy-preferences-window
 * @short_description: A window to present an application's preferences.
 * @Title: HdyPreferencesWindow
 *
 * The #HdyPreferencesWindow widget presents an application's preferences
 * gathered into pages and groups. The preferences are searchable by the user.
 *
 * Since: 0.0.10
 */

typedef struct
{
  GtkStack *content_stack;
  GtkStack *pages_stack;
  GtkToggleButton *search_button;
  GtkSearchEntry *search_entry;
  GtkListBox *search_results;
  GtkStack *search_stack;
  HdySqueezer *squeezer;
  GtkLabel *title_label;
  GtkStack *title_stack;
  HdyViewSwitcherBar *view_switcher_bar;
  HdyViewSwitcher *view_switcher_narrow;
  HdyViewSwitcher *view_switcher_wide;

  gint n_last_search_results;
} HdyPreferencesWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyPreferencesWindow, hdy_preferences_window, GTK_TYPE_WINDOW)

static gboolean
is_title_label_visible (GBinding     *binding,
                        const GValue *from_value,
                        GValue       *to_value,
                        gpointer      user_data)
{
  g_value_set_boolean (to_value, g_value_get_object (from_value) == user_data);

  return TRUE;
}

static gboolean
filter_search_results (HdyActionRow         *row,
                       HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  g_autofree gchar *text = g_utf8_casefold (gtk_entry_get_text (GTK_ENTRY (priv->search_entry)), -1);
  g_autofree gchar *title = g_utf8_casefold (hdy_action_row_get_title (row), -1);
  g_autofree gchar *subtitle = NULL;

  if (strstr (title, text)) {
    priv->n_last_search_results++;

    return TRUE;
  }

  subtitle = g_utf8_casefold (hdy_action_row_get_subtitle (row), -1);

  if (!!strstr (subtitle, text)) {
    priv->n_last_search_results++;

    return TRUE;
  }

  return FALSE;
}

static GtkWidget *
new_search_row_for_preference (HdyPreferencesRow    *row,
                               HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  HdyActionRow *widget;
  HdyPreferencesGroup *group;
  HdyPreferencesPage *page;
  const gchar *group_title, *page_title;
  GtkWidget *parent;

  g_assert (HDY_IS_PREFERENCES_ROW (row));

  widget = hdy_action_row_new ();
  g_object_bind_property (row, "title", widget, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (row, "use-underline", widget, "use-underline", G_BINDING_SYNC_CREATE);

  for (parent = gtk_widget_get_parent (GTK_WIDGET (row));
       parent != NULL && !HDY_IS_PREFERENCES_GROUP (parent);
       parent = gtk_widget_get_parent (parent));
  group = parent != NULL ? HDY_PREFERENCES_GROUP (parent) : NULL;
  group_title = group != NULL ? hdy_preferences_group_get_title (group) : NULL;
  if (g_strcmp0 (group_title, "") == 0)
    group_title = NULL;

  for (parent = gtk_widget_get_parent (GTK_WIDGET (group));
       parent != NULL && !HDY_IS_PREFERENCES_PAGE (parent);
       parent = gtk_widget_get_parent (parent));
  page = parent != NULL ? HDY_PREFERENCES_PAGE (parent) : NULL;
  page_title = page != NULL ? hdy_preferences_page_get_title (page) : NULL;
  if (g_strcmp0 (page_title, "") == 0)
    page_title = NULL;

  if (group_title && !gtk_widget_get_visible (GTK_WIDGET (priv->view_switcher_wide)))
    hdy_action_row_set_subtitle (widget, group_title);
  if (group_title) {
    g_autofree gchar *subtitle = g_strdup_printf ("%s → %s", page_title != NULL ? page_title : _("Untitled page"), group_title);
    hdy_action_row_set_subtitle (widget, subtitle);
  } else if (page_title)
    hdy_action_row_set_subtitle (widget, page_title);

  gtk_widget_show (GTK_WIDGET (widget));

  g_object_set_data (G_OBJECT (widget), "page", page);
  g_object_set_data (G_OBJECT (widget), "row", row);

  return GTK_WIDGET (widget);
}

static void
update_search_results (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  g_autoptr (GListStore) model;

  model = g_list_store_new (HDY_TYPE_PREFERENCES_ROW);
  gtk_container_foreach (GTK_CONTAINER (priv->pages_stack), (GtkCallback) hdy_preferences_page_add_preferences_to_model, model);
  gtk_container_foreach (GTK_CONTAINER (priv->search_results), (GtkCallback) gtk_widget_destroy, NULL);
  for (guint i = 0; i < g_list_model_get_n_items (G_LIST_MODEL (model)); i++)
    gtk_container_add (GTK_CONTAINER (priv->search_results),
                       new_search_row_for_preference ((HdyPreferencesRow *) g_list_model_get_item (G_LIST_MODEL (model), i), self));
}

static void
search_result_activated (HdyPreferencesWindow *self,
                         HdyActionRow         *widget)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  HdyPreferencesPage *page;
  HdyPreferencesRow *row;
  GtkAdjustment *adjustment;
  GtkAllocation allocation;
  gint y = 0;

  gtk_toggle_button_set_active (priv->search_button, FALSE);
  page = HDY_PREFERENCES_PAGE (g_object_get_data (G_OBJECT (widget), "page"));
  row = HDY_PREFERENCES_ROW (g_object_get_data (G_OBJECT (widget), "row"));

  g_assert (page != NULL);
  g_assert (row != NULL);

  adjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (page));

  g_assert (adjustment != NULL);

  gtk_stack_set_visible_child (priv->pages_stack, GTK_WIDGET (page));
  gtk_widget_set_can_focus (GTK_WIDGET (row), TRUE);
  gtk_widget_grab_focus (GTK_WIDGET (row));

  if (!gtk_widget_translate_coordinates (GTK_WIDGET (row), GTK_WIDGET (page), 0, 0, NULL, &y))
    return;

  gtk_container_set_focus_child (GTK_CONTAINER (page), GTK_WIDGET (row));
  y += gtk_adjustment_get_value (adjustment);
  gtk_widget_get_allocation (GTK_WIDGET (row), &allocation);
  gtk_adjustment_clamp_page (adjustment, y, y + allocation.height);
}

static gboolean
key_pressed (GtkWidget            *sender,
             GdkEventKey          *event,
             HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  GdkModifierType default_modifiers = gtk_accelerator_get_default_mod_mask ();
  guint keyval;
  GdkModifierType state;
  gunichar c;

  gdk_event_get_keyval ((GdkEvent *) event, &keyval);
  gdk_event_get_state ((GdkEvent *) event, &state);

  if ((keyval == GDK_KEY_f || keyval == GDK_KEY_F) &&
      (state & default_modifiers) == GDK_CONTROL_MASK) {
    gtk_toggle_button_set_active (priv->search_button, TRUE);

    return TRUE;
  }

  if (keyval == GDK_KEY_Escape &&
      gtk_toggle_button_get_active (priv->search_button)) {
    gtk_toggle_button_set_active (priv->search_button, FALSE);

    return TRUE;
  }

  c = gdk_keyval_to_unicode (keyval);
  if (g_unichar_isgraph (c)) {
    gchar text[6] = { 0 };
    g_unichar_to_utf8 (c, text);
    gtk_entry_set_text (GTK_ENTRY (priv->search_entry), text);
    gtk_toggle_button_set_active (priv->search_button, TRUE);

    return TRUE;
  }

  return FALSE;
}

static void
header_bar_size_allocated (HdyPreferencesWindow *self,
                           GdkRectangle         *allocation)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  hdy_squeezer_set_child_enabled (priv->squeezer, GTK_WIDGET (priv->view_switcher_wide), allocation->width > 540);
  hdy_squeezer_set_child_enabled (priv->squeezer, GTK_WIDGET (priv->view_switcher_narrow), allocation->width > 360);
}

static void
search_button_activated (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (gtk_toggle_button_get_active (priv->search_button)) {
    update_search_results (self);
    gtk_stack_set_visible_child_name (priv->title_stack, "search");
    gtk_stack_set_visible_child_name (priv->content_stack, "search");
    gtk_entry_grab_focus_without_selecting (GTK_ENTRY (priv->search_entry));
    /* Grabbing without selecting puts the cursor at the start of the buffer, so
     * for "type to search" to work we must move the cursor at the end. We can't
     * use GTK_MOVEMENT_BUFFER_ENDS because it causes a sound to be played.
     */
    g_signal_emit_by_name (priv->search_entry, "move-cursor",
                           GTK_MOVEMENT_LOGICAL_POSITIONS, G_MAXINT, FALSE, NULL);
  } else {
    gtk_stack_set_visible_child_name (priv->title_stack, "pages");
    gtk_stack_set_visible_child_name (priv->content_stack, "pages");
  }
}

static void
search_changed (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  priv->n_last_search_results = 0;
  gtk_list_box_invalidate_filter (priv->search_results);
  gtk_stack_set_visible_child_name (priv->search_stack,
                                    priv->n_last_search_results > 0 ? "results" : "no-results");
}

static void
count_children_cb (GtkWidget *widget,
                   gint      *count)
{
  (*count)++;
}

static void
update_pages_switcher_visibility (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  gint count = 0;

  gtk_container_foreach (GTK_CONTAINER (priv->pages_stack), (GtkCallback) count_children_cb, &count);

  gtk_widget_set_visible (GTK_WIDGET (priv->view_switcher_wide), count > 1);
  gtk_widget_set_visible (GTK_WIDGET (priv->view_switcher_narrow), count > 1);
  gtk_widget_set_visible (GTK_WIDGET (priv->view_switcher_bar), count > 1);
}

static void
on_page_icon_name_changed (HdyPreferencesPage   *page,
                           GParamSpec           *pspec,
                           HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  gtk_container_child_set (GTK_CONTAINER (priv->pages_stack), GTK_WIDGET (page),
                           "icon-name", hdy_preferences_page_get_icon_name (page),
                           NULL);
}

static void
on_page_title_changed (HdyPreferencesPage   *page,
                       GParamSpec           *pspec,
                       HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  gtk_container_child_set (GTK_CONTAINER (priv->pages_stack), GTK_WIDGET (page),
                           "title", hdy_preferences_page_get_title (page),
                           NULL);
}

static void
hdy_preferences_window_add (GtkContainer *container,
                            GtkWidget    *child)
{
  HdyPreferencesWindow *self = HDY_PREFERENCES_WINDOW (container);
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (priv->content_stack == NULL)
    GTK_CONTAINER_CLASS (hdy_preferences_window_parent_class)->add (container, child);
  else if (HDY_IS_PREFERENCES_PAGE (child)) {
    gtk_container_add (GTK_CONTAINER (priv->pages_stack), child);
    on_page_icon_name_changed (HDY_PREFERENCES_PAGE (child), NULL, self);
    on_page_title_changed (HDY_PREFERENCES_PAGE (child), NULL, self);
    g_signal_connect (child, "notify::icon-name",
                      G_CALLBACK (on_page_icon_name_changed), self);
    g_signal_connect (child, "notify::title",
                      G_CALLBACK (on_page_title_changed), self);

    update_pages_switcher_visibility (self);
  } else
    g_warning ("Can't add children of type %s to %s",
               G_OBJECT_TYPE_NAME (child),
               G_OBJECT_TYPE_NAME (container));
}

static void
hdy_preferences_window_class_init (HdyPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  container_class->add = hdy_preferences_window_add;

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-preferences-window.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, content_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, pages_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_button);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_entry);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_results);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, squeezer);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, title_label);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, title_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, view_switcher_bar);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, view_switcher_narrow);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, view_switcher_wide);
  gtk_widget_class_bind_template_callback (widget_class, header_bar_size_allocated);
  gtk_widget_class_bind_template_callback (widget_class, key_pressed);
  gtk_widget_class_bind_template_callback (widget_class, search_button_activated);
  gtk_widget_class_bind_template_callback (widget_class, search_changed);
  gtk_widget_class_bind_template_callback (widget_class, search_result_activated);
}

static void
hdy_preferences_window_init (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_bind_property_full (priv->squeezer,
                               "visible-child",
                               priv->view_switcher_bar,
                               "reveal",
                               G_BINDING_SYNC_CREATE,
                               is_title_label_visible,
                               NULL,
                               priv->title_label,
                               NULL);

  gtk_list_box_set_header_func (priv->search_results, hdy_list_box_separator_header, NULL, NULL);
  gtk_list_box_set_filter_func (priv->search_results, (GtkListBoxFilterFunc) filter_search_results, self, NULL);

  update_pages_switcher_visibility (self);
}

/**
 * hdy_preferences_window_new:
 *
 * Creates a new #HdyPreferencesWindow.
 *
 * Returns: a new #HdyPreferencesWindow
 *
 * Since: 0.0.10
 */
HdyPreferencesWindow *
hdy_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_PREFERENCES_WINDOW, NULL);
}
