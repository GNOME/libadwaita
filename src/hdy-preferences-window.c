/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-preferences-window.h"

#include "hdy-animation.h"
#include "hdy-action-row.h"
#include "hdy-deck.h"
#include "hdy-preferences-group-private.h"
#include "hdy-preferences-page-private.h"
#include "hdy-view-switcher.h"
#include "hdy-view-switcher-bar.h"
#include "hdy-view-switcher-title.h"

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
  HdyDeck *subpages_deck;
  GtkWidget *preferences;
  GtkStack *content_stack;
  GtkStack *pages_stack;
  GtkToggleButton *search_button;
  GtkSearchEntry *search_entry;
  GtkListBox *search_results;
  GtkStack *search_stack;
  GtkStack *title_stack;
  HdyViewSwitcherBar *view_switcher_bar;
  HdyViewSwitcherTitle *view_switcher_title;

  gboolean search_enabled;
  gboolean can_swipe_back;
  gint n_last_search_results;
  GtkWidget *subpage;
} HdyPreferencesWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyPreferencesWindow, hdy_preferences_window, HDY_TYPE_WINDOW)

enum {
  PROP_0,
  PROP_SEARCH_ENABLED,
  PROP_CAN_SWIPE_BACK,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

/* Copied and modified from gtklabel.c, separate_uline_pattern() */
static gchar *
strip_mnemonic (const gchar *src)
{
  g_autofree gchar *new_str = g_new (gchar, strlen (src) + 1);
  gchar *dest = new_str;
  gboolean underscore = FALSE;

  while (*src) {
    gunichar c;
    const gchar *next_src;

    c = g_utf8_get_char (src);
    if (c == (gunichar) -1) {
      g_warning ("Invalid input string");

      return NULL;
    }

    next_src = g_utf8_next_char (src);

    if (underscore) {
      while (src < next_src)
        *dest++ = *src++;

      underscore = FALSE;
    } else {
      if (c == '_'){
        underscore = TRUE;
        src = next_src;
      } else {
        while (src < next_src)
          *dest++ = *src++;
      }
    }
  }

  *dest = 0;

  return g_steal_pointer (&new_str);
}

static gboolean
filter_search_results (HdyActionRow         *row,
                       HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  g_autofree gchar *text = g_utf8_casefold (gtk_entry_get_text (GTK_ENTRY (priv->search_entry)), -1);
  g_autofree gchar *title = g_utf8_casefold (hdy_preferences_row_get_title (HDY_PREFERENCES_ROW (row)), -1);
  g_autofree gchar *subtitle = NULL;

  /* The CSS engine works in such a way that invisible children are treated as
   * visible widgets, which breaks the expectations of the .preferences  style
   * class when filtering a row, leading to straight corners when the first row
   * or last row are filtered out.
   *
   * This works around it by explicitly toggling the row's visibility, while
   * keeping GtkListBox's filtering logic.
   *
   * See https://gitlab.gnome.org/GNOME/libhandy/-/merge_requests/424
   */

  if (hdy_preferences_row_get_use_underline (HDY_PREFERENCES_ROW (row))) {
    gchar *stripped_title = strip_mnemonic (title);

    if (stripped_title) {
      g_free (title);
      title = stripped_title;
    }
  }

  if (!!strstr (title, text)) {
    priv->n_last_search_results++;
    gtk_widget_show (GTK_WIDGET (row));

    return TRUE;
  }

  subtitle = g_utf8_casefold (hdy_action_row_get_subtitle (row), -1);

  if (!!strstr (subtitle, text)) {
    priv->n_last_search_results++;
    gtk_widget_show (GTK_WIDGET (row));

    return TRUE;
  }

  gtk_widget_hide (GTK_WIDGET (row));

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

  widget = HDY_ACTION_ROW (hdy_action_row_new ());
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (widget), TRUE);
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

  if (group_title && !hdy_view_switcher_title_get_title_visible (priv->view_switcher_title))
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
search_result_activated_cb (HdyPreferencesWindow *self,
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

  adjustment = hdy_preferences_page_get_vadjustment (page);

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
key_press_event_cb (GtkWidget            *sender,
                    GdkEvent             *event,
                    HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);
  GdkModifierType default_modifiers = gtk_accelerator_get_default_mod_mask ();
  guint keyval;
  GdkModifierType state;
  GdkKeymap *keymap;
  GdkEventKey *key_event = (GdkEventKey *) event;

  gdk_event_get_state (event, &state);

  keymap = gdk_keymap_get_for_display (gtk_widget_get_display (sender));

  gdk_keymap_translate_keyboard_state (keymap,
                                       key_event->hardware_keycode,
                                       state,
                                       key_event->group,
                                       &keyval, NULL, NULL, NULL);

  if (priv->subpage) {
    if (keyval == GDK_KEY_Escape &&
        hdy_preferences_window_get_can_swipe_back (self)) {
      hdy_preferences_window_close_subpage (self);

      return GDK_EVENT_STOP;
    }

    return GDK_EVENT_PROPAGATE;
  }

  if (priv->search_enabled &&
      (keyval == GDK_KEY_f || keyval == GDK_KEY_F) &&
      (state & default_modifiers) == GDK_CONTROL_MASK) {
    gtk_toggle_button_set_active (priv->search_button, TRUE);

    return GDK_EVENT_STOP;
  }

  if (priv->search_enabled &&
      gtk_search_entry_handle_event (priv->search_entry, event)) {
    gtk_toggle_button_set_active (priv->search_button, TRUE);

    return GDK_EVENT_STOP;
  }

  if (keyval == GDK_KEY_Escape) {
    gtk_window_close (GTK_WINDOW (self));

    return GDK_EVENT_STOP;
  }

  return GDK_EVENT_PROPAGATE;
}

static void
try_remove_subpages (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (hdy_deck_get_transition_running (priv->subpages_deck))
    return;

  if (hdy_deck_get_visible_child (priv->subpages_deck) == priv->preferences)
    priv->subpage = NULL;

  for (GList *child = gtk_container_get_children (GTK_CONTAINER (priv->subpages_deck));
       child;
       child = child->next)
    if (child->data != priv->preferences && child->data != priv->subpage)
      gtk_container_remove (GTK_CONTAINER (priv->subpages_deck), child->data);
}

static void
subpages_deck_transition_running_cb (HdyPreferencesWindow *self)
{
  try_remove_subpages (self);
}

static void
subpages_deck_visible_child_cb (HdyPreferencesWindow *self)
{
  try_remove_subpages (self);
}

static void
header_bar_size_allocate_cb (HdyPreferencesWindow *self,
                             GdkRectangle         *allocation)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  hdy_view_switcher_title_set_view_switcher_enabled (priv->view_switcher_title, allocation->width > 360);
}

static void
title_stack_notify_transition_running_cb (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (gtk_stack_get_transition_running (priv->title_stack) ||
      gtk_stack_get_visible_child (priv->title_stack) != GTK_WIDGET (priv->view_switcher_title))
    return;

  gtk_entry_set_text (GTK_ENTRY (priv->search_entry), "");
}

static void
title_stack_notify_visible_child_cb (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (hdy_get_enable_animations (GTK_WIDGET (priv->title_stack)) ||
      gtk_stack_get_visible_child (priv->title_stack) != GTK_WIDGET (priv->view_switcher_title))
    return;

  gtk_entry_set_text (GTK_ENTRY (priv->search_entry), "");
}


static void
search_button_notify_active_cb (HdyPreferencesWindow *self)
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
search_changed_cb (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  priv->n_last_search_results = 0;
  gtk_list_box_invalidate_filter (priv->search_results);
  gtk_stack_set_visible_child_name (priv->search_stack,
                                    priv->n_last_search_results > 0 ? "results" : "no-results");
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
stop_search_cb (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  gtk_toggle_button_set_active (priv->search_button, FALSE);
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
hdy_preferences_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  HdyPreferencesWindow *self = HDY_PREFERENCES_WINDOW (object);

  switch (prop_id) {
  case PROP_SEARCH_ENABLED:
    g_value_set_boolean (value, hdy_preferences_window_get_search_enabled (self));
    break;
  case PROP_CAN_SWIPE_BACK:
    g_value_set_boolean (value, hdy_preferences_window_get_can_swipe_back (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_preferences_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  HdyPreferencesWindow *self = HDY_PREFERENCES_WINDOW (object);

  switch (prop_id) {
  case PROP_SEARCH_ENABLED:
    hdy_preferences_window_set_search_enabled (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_BACK:
    hdy_preferences_window_set_can_swipe_back (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
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
  } else
    g_warning ("Can't add children of type %s to %s",
               G_OBJECT_TYPE_NAME (child),
               G_OBJECT_TYPE_NAME (container));
}

static void
hdy_preferences_window_remove (GtkContainer *container,
                               GtkWidget    *child)
{
  HdyPreferencesWindow *self = HDY_PREFERENCES_WINDOW (container);
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (child == GTK_WIDGET (priv->content_stack))
    GTK_CONTAINER_CLASS (hdy_preferences_window_parent_class)->remove (container, child);
  else
    gtk_container_remove (GTK_CONTAINER (priv->pages_stack), child);
}

static void
hdy_preferences_window_forall (GtkContainer *container,
                               gboolean      include_internals,
                               GtkCallback   callback,
                               gpointer      callback_data)
{
  HdyPreferencesWindow *self = HDY_PREFERENCES_WINDOW (container);
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  if (include_internals)
    GTK_CONTAINER_CLASS (hdy_preferences_window_parent_class)->forall (container,
                                                                       include_internals,
                                                                       callback,
                                                                       callback_data);
  else if (priv->pages_stack)
    gtk_container_foreach (GTK_CONTAINER (priv->pages_stack), callback, callback_data);
}

static void
hdy_preferences_window_class_init (HdyPreferencesWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_preferences_window_get_property;
  object_class->set_property = hdy_preferences_window_set_property;

  container_class->add = hdy_preferences_window_add;
  container_class->remove = hdy_preferences_window_remove;
  container_class->forall = hdy_preferences_window_forall;

  /**
   * HdyPreferencesWindow:search-enabled:
   *
   * Whether search is enabled.
   *
   * Since: 1.0
   */
  props[PROP_SEARCH_ENABLED] =
    g_param_spec_boolean ("search-enabled",
                          _("Search enabled"),
                          _("Whether search is enabled"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyPreferencesWindow:can-swipe-back:
   *
   * Whether or not the window allows closing the subpage via a swipe gesture.
   *
   * Since: 1.0
   */
  props[PROP_CAN_SWIPE_BACK] =
      g_param_spec_boolean ("can-swipe-back",
                            _("Can swipe back"),
                            _("Whether or not swipe gesture can be used to switch from a subpage to the preferences"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-preferences-window.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, subpages_deck);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, preferences);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, content_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, pages_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_button);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_entry);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_results);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, search_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, title_stack);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, view_switcher_bar);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesWindow, view_switcher_title);
  gtk_widget_class_bind_template_callback (widget_class, subpages_deck_transition_running_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpages_deck_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, header_bar_size_allocate_cb);
  gtk_widget_class_bind_template_callback (widget_class, title_stack_notify_transition_running_cb);
  gtk_widget_class_bind_template_callback (widget_class, title_stack_notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, key_press_event_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_button_notify_active_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_result_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, stop_search_cb);
}

static void
hdy_preferences_window_init (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv = hdy_preferences_window_get_instance_private (self);

  priv->search_enabled = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_list_box_set_filter_func (priv->search_results, (GtkListBoxFilterFunc) filter_search_results, self, NULL);
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
GtkWidget *
hdy_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_PREFERENCES_WINDOW, NULL);
}

/**
 * hdy_preferences_window_get_search_enabled:
 * @self: a #HdyPreferencesWindow
 *
 * Gets whether search is enabled for @self.
 *
 * Returns: whether search is enabled for @self.
 *
 * Since: 1.0
 */
gboolean
hdy_preferences_window_get_search_enabled (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv;

  g_return_val_if_fail (HDY_IS_PREFERENCES_WINDOW (self), FALSE);

  priv = hdy_preferences_window_get_instance_private (self);

  return priv->search_enabled;
}

/**
 * hdy_preferences_window_set_search_enabled:
 * @self: a #HdyPreferencesWindow
 * @search_enabled: %TRUE to enable search, %FALSE to disable it
 *
 * Sets whether search is enabled for @self.
 *
 * Since: 1.0
 */
void
hdy_preferences_window_set_search_enabled (HdyPreferencesWindow *self,
                                           gboolean              search_enabled)
{
  HdyPreferencesWindowPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_WINDOW (self));

  priv = hdy_preferences_window_get_instance_private (self);

  search_enabled = !!search_enabled;

  if (priv->search_enabled == search_enabled)
    return;

  priv->search_enabled = search_enabled;
  gtk_widget_set_visible (GTK_WIDGET (priv->search_button), search_enabled);
  if (!search_enabled)
    gtk_toggle_button_set_active (priv->search_button, FALSE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SEARCH_ENABLED]);
}

/**
 * hdy_preferences_window_set_can_swipe_back:
 * @self: a #HdyPreferencesWindow
 * @can_swipe_back: the new value
 *
 * Sets whether or not @self allows switching from a subpage to the preferences
 * via a swipe gesture.
 *
 * Since: 1.0
 */
void
hdy_preferences_window_set_can_swipe_back (HdyPreferencesWindow *self,
                                           gboolean              can_swipe_back)
{
  HdyPreferencesWindowPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_WINDOW (self));

  priv = hdy_preferences_window_get_instance_private (self);

  can_swipe_back = !!can_swipe_back;

  if (priv->can_swipe_back == can_swipe_back)
    return;

  priv->can_swipe_back = can_swipe_back;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SWIPE_BACK]);
}

/**
 * hdy_preferences_window_get_can_swipe_back
 * @self: a #HdyPreferencesWindow
 *
 * Returns whether or not @self allows switching from a subpage to the
 * preferences via a swipe gesture.
 *
 * Returns: %TRUE if back swipe is enabled.
 *
 * Since: 1.0
 */
gboolean
hdy_preferences_window_get_can_swipe_back (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv;

  g_return_val_if_fail (HDY_IS_PREFERENCES_WINDOW (self), FALSE);

  priv = hdy_preferences_window_get_instance_private (self);

  return priv->can_swipe_back;
}

/**
 * hdy_preferences_window_present_subpage:
 * @self: a #HdyPreferencesWindow
 * @subpage: the subpage
 *
 * Sets @subpage as the window's subpage and present it.
 * The transition can be cancelled by the user, in which case visible child will
 * change back to the previously visible child.
 *
 * Since: 1.0
 */
void
hdy_preferences_window_present_subpage (HdyPreferencesWindow *self,
                                        GtkWidget            *subpage)
{
  HdyPreferencesWindowPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (subpage));

  priv = hdy_preferences_window_get_instance_private (self);

  if (priv->subpage == subpage)
    return;

  priv->subpage = subpage;

  /* The check below avoids a warning when re-entering a subpage during the
   * transition between the that subpage to the preferences.
   */
  if (gtk_widget_get_parent (subpage) != GTK_WIDGET (priv->subpages_deck))
    gtk_container_add (GTK_CONTAINER (priv->subpages_deck), subpage);

  hdy_deck_set_visible_child (priv->subpages_deck, subpage);
}

/**
 * hdy_preferences_window_close_subpage:
 * @self: a #HdyPreferencesWindow
 *
 * Closes the current subpage to return back to the preferences, if there is no
 * presented subpage, this does nothing.
 *
 * Since: 1.0
 */
void
hdy_preferences_window_close_subpage (HdyPreferencesWindow *self)
{
  HdyPreferencesWindowPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_WINDOW (self));

  priv = hdy_preferences_window_get_instance_private (self);

  if (priv->subpage == NULL)
    return;

  hdy_deck_set_visible_child (priv->subpages_deck, priv->preferences);
}
