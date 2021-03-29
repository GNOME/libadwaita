/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-preferences-window.h"

#include "adw-animation.h"
#include "adw-action-row.h"
#include "adw-leaflet.h"
#include "adw-preferences-group-private.h"
#include "adw-preferences-page-private.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-bar.h"
#include "adw-view-switcher-title.h"

/**
 * SECTION:adw-preferences-window
 * @short_description: A window to present an application's preferences.
 * @Title: AdwPreferencesWindow
 *
 * The #AdwPreferencesWindow widget presents an application's preferences
 * gathered into pages and groups. The preferences are searchable by the user.
 *
 * Since: 1.0
 */

typedef struct
{
  AdwLeaflet *subpages_leaflet;
  GtkWidget *preferences;
  GtkStack *content_stack;
  GtkStack *pages_stack;
  GtkToggleButton *search_button;
  GtkSearchEntry *search_entry;
  GtkListBox *search_results;
  GtkStack *search_stack;
  GtkStack *title_stack;
  AdwViewSwitcherBar *view_switcher_bar;
  AdwViewSwitcherTitle *view_switcher_title;

  gboolean search_enabled;
  gboolean can_swipe_back;

  GtkFilter *filter;
  GtkFilterListModel *filter_model;

  GtkWidget *subpage;
} AdwPreferencesWindowPrivate;

static void adw_preferences_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwPreferencesWindow, adw_preferences_window, ADW_TYPE_WINDOW,
                         G_ADD_PRIVATE (AdwPreferencesWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adw_preferences_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_SEARCH_ENABLED,
  PROP_CAN_SWIPE_BACK,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

/* Copied and modified from gtklabel.c, separate_uline_pattern() */
static char *
strip_mnemonic (const char *src)
{
  g_autofree char *new_str = g_new (char, strlen (src) + 1);
  char *dest = new_str;
  gboolean underscore = FALSE;

  while (*src) {
    gunichar c;
    const char *next_src;

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
filter_search_results (AdwPreferencesRow    *row,
                       AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  g_autofree char *terms = NULL;
  g_autofree char *title = NULL;

  g_assert (ADW_IS_PREFERENCES_ROW (row));

  terms = g_utf8_casefold (gtk_editable_get_text (GTK_EDITABLE (priv->search_entry)), -1);
  title = g_utf8_casefold (adw_preferences_row_get_title (row), -1);

  if (adw_preferences_row_get_use_underline (ADW_PREFERENCES_ROW (row))) {
    char *stripped_title = strip_mnemonic (title);

    if (stripped_title) {
      g_free (title);
      title = stripped_title;
    }
  }

  if (!!strstr (title, terms))
    return TRUE;

  if (ADW_IS_ACTION_ROW (row)) {
    g_autofree char *subtitle = g_utf8_casefold (adw_action_row_get_subtitle (ADW_ACTION_ROW (row)), -1);

    if (!!strstr (subtitle, terms))
      return TRUE;
  }

  return FALSE;
}

static int
get_n_pages (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  int count = 0;
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (GTK_WIDGET (priv->pages_stack));
       child;
       child = gtk_widget_get_next_sibling (child)) {
    GtkStackPage *page = gtk_stack_get_page (priv->pages_stack, child);

    if (gtk_stack_page_get_visible (page))
      count++;
  }

  return count;
}

static gchar *
create_search_row_subtitle (AdwPreferencesWindow *self,
                            GtkWidget            *row)
{
  GtkWidget *group, *page;
  const char *group_title = NULL;
  g_autofree char *page_title = NULL;

  group = gtk_widget_get_ancestor (row, ADW_TYPE_PREFERENCES_GROUP);

  if (group) {
    group_title = adw_preferences_group_get_title (ADW_PREFERENCES_GROUP (group));

    if (g_strcmp0 (group_title, "") == 0)
      group_title = NULL;
  }

  page = gtk_widget_get_ancestor (group, ADW_TYPE_PREFERENCES_PAGE);

  if (page) {
    const char *title = adw_preferences_page_get_title (ADW_PREFERENCES_PAGE (page));

    if (adw_preferences_page_get_use_underline (ADW_PREFERENCES_PAGE (page)))
      page_title = strip_mnemonic (title);
    else
      page_title = g_strdup (title);

    if (g_strcmp0 (page_title, "") == 0)
      page_title = NULL;
  }

  if (group_title) {
    if (get_n_pages (self) > 1)
      return g_strdup_printf ("%s → %s", page_title ? page_title : _("Untitled page"), group_title);

    return g_strdup (group_title);
  }

  if (page_title)
    return g_steal_pointer (&page_title);

  return NULL;
}

static GtkWidget *
new_search_row_for_preference (AdwPreferencesRow    *row,
                               AdwPreferencesWindow *self)
{
  AdwActionRow *widget;
  GtkWidget *page;
  g_autofree char *subtitle = NULL;

  g_assert (ADW_IS_PREFERENCES_ROW (row));

  subtitle = create_search_row_subtitle (self, GTK_WIDGET (row));
  page = gtk_widget_get_ancestor (GTK_WIDGET (row), ADW_TYPE_PREFERENCES_PAGE);

  widget = ADW_ACTION_ROW (adw_action_row_new ());
  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (widget), TRUE);
  g_object_bind_property (row, "title", widget, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (row, "use-underline", widget, "use-underline", G_BINDING_SYNC_CREATE);
  adw_action_row_set_subtitle (widget, subtitle);

  g_object_set_data (G_OBJECT (widget), "page", page);
  g_object_set_data (G_OBJECT (widget), "row", row);

  return GTK_WIDGET (widget);
}

static void
search_result_activated_cb (AdwPreferencesWindow *self,
                            AdwActionRow         *widget)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  AdwPreferencesPage *page;
  AdwPreferencesRow *row;

  gtk_toggle_button_set_active (priv->search_button, FALSE);
  page = ADW_PREFERENCES_PAGE (g_object_get_data (G_OBJECT (widget), "page"));
  row = ADW_PREFERENCES_ROW (g_object_get_data (G_OBJECT (widget), "row"));

  g_assert (page != NULL);
  g_assert (row != NULL);

  gtk_stack_set_visible_child (priv->pages_stack, GTK_WIDGET (page));
  gtk_widget_set_can_focus (GTK_WIDGET (row), TRUE);
  gtk_widget_grab_focus (GTK_WIDGET (row));
  gtk_window_set_focus_visible (GTK_WINDOW (self), TRUE);
}

static void
search_results_map (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  gtk_list_box_bind_model (priv->search_results,
                           G_LIST_MODEL (priv->filter_model),
                           (GtkListBoxCreateWidgetFunc) new_search_row_for_preference,
                           self,
                           NULL);
}

static void
search_results_unmap (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  gtk_list_box_bind_model (priv->search_results, NULL, NULL, NULL, NULL);
}

static void
try_remove_subpages (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  GtkWidget *child;

  if (adw_leaflet_get_child_transition_running (priv->subpages_leaflet))
    return;

  if (adw_leaflet_get_visible_child (priv->subpages_leaflet) == priv->preferences)
    priv->subpage = NULL;

  child = gtk_widget_get_first_child (GTK_WIDGET (priv->subpages_leaflet));
  while (child) {
    GtkWidget *page = child;

    child = gtk_widget_get_next_sibling (child);

    if (page != priv->preferences && page != priv->subpage)
      adw_leaflet_remove (priv->subpages_leaflet, page);
  }
}

static void
subpages_leaflet_child_transition_running_cb (AdwPreferencesWindow *self)
{
  try_remove_subpages (self);
}

static void
subpages_leaflet_visible_child_cb (AdwPreferencesWindow *self)
{
  try_remove_subpages (self);
}

static void
title_stack_notify_transition_running_cb (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  if (gtk_stack_get_transition_running (priv->title_stack) ||
      gtk_stack_get_visible_child (priv->title_stack) != GTK_WIDGET (priv->view_switcher_title))
    return;

  gtk_editable_set_text (GTK_EDITABLE (priv->search_entry), "");
}

static void
title_stack_notify_visible_child_cb (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  if (adw_get_enable_animations (GTK_WIDGET (priv->title_stack)) ||
      gtk_stack_get_visible_child (priv->title_stack) != GTK_WIDGET (priv->view_switcher_title))
    return;

  gtk_editable_set_text (GTK_EDITABLE (priv->search_entry), "");
}


static void
search_button_notify_active_cb (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  if (gtk_toggle_button_get_active (priv->search_button)) {
    gtk_stack_set_visible_child_name (priv->title_stack, "search");
    gtk_stack_set_visible_child_name (priv->content_stack, "search");
    gtk_widget_grab_focus (GTK_WIDGET (priv->search_entry));
    /* Grabbing without selecting puts the cursor at the start of the buffer, so
     * for "type to search" to work we must move the cursor at the end.
     */
    gtk_editable_set_position (GTK_EDITABLE (priv->search_entry), -1);
  } else {
    gtk_stack_set_visible_child_name (priv->title_stack, "pages");
    gtk_stack_set_visible_child_name (priv->content_stack, "pages");
  }
}

static void
search_started_cb (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  gtk_toggle_button_set_active (priv->search_button, TRUE);
}

static void
search_changed_cb (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  guint n;

  gtk_filter_changed (priv->filter, GTK_FILTER_CHANGE_DIFFERENT);

  n = g_list_model_get_n_items (G_LIST_MODEL (priv->filter_model));

  gtk_stack_set_visible_child_name (priv->search_stack, n > 0 ? "results" : "no-results");
}

static void
stop_search_cb (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  gtk_toggle_button_set_active (priv->search_button, FALSE);
}

static void
adw_preferences_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdwPreferencesWindow *self = ADW_PREFERENCES_WINDOW (object);

  switch (prop_id) {
  case PROP_SEARCH_ENABLED:
    g_value_set_boolean (value, adw_preferences_window_get_search_enabled (self));
    break;
  case PROP_CAN_SWIPE_BACK:
    g_value_set_boolean (value, adw_preferences_window_get_can_swipe_back (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdwPreferencesWindow *self = ADW_PREFERENCES_WINDOW (object);

  switch (prop_id) {
  case PROP_SEARCH_ENABLED:
    adw_preferences_window_set_search_enabled (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_BACK:
    adw_preferences_window_set_can_swipe_back (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static gboolean
search_open_cb (GtkWidget *widget,
                GVariant  *args,
                gpointer   user_data)
{
  AdwPreferencesWindow *self = ADW_PREFERENCES_WINDOW (widget);
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  if (!priv->search_enabled || gtk_toggle_button_get_active (priv->search_button))
    return GDK_EVENT_PROPAGATE;

  gtk_toggle_button_set_active (priv->search_button, TRUE);

  return GDK_EVENT_STOP;
}

static gboolean
close_cb (GtkWidget *widget,
          GVariant  *args,
          gpointer   user_data)
{
  AdwPreferencesWindow *self = ADW_PREFERENCES_WINDOW (widget);
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  if (priv->subpage) {
    if (!adw_preferences_window_get_can_swipe_back (self))
      return GDK_EVENT_PROPAGATE;

    adw_preferences_window_close_subpage (self);

    return GDK_EVENT_STOP;
  }

  gtk_window_close (GTK_WINDOW (self));

  return GDK_EVENT_STOP;
}

static void
adw_preferences_window_class_init (AdwPreferencesWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_preferences_window_get_property;
  object_class->set_property = adw_preferences_window_set_property;

  /**
   * AdwPreferencesWindow:search-enabled:
   *
   * Whether search is enabled.
   *
   * Since: 1.0
   */
  props[PROP_SEARCH_ENABLED] =
    g_param_spec_boolean ("search-enabled",
                          "Search enabled",
                          "Whether search is enabled",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwPreferencesWindow:can-swipe-back:
   *
   * Whether or not the window allows closing the subpage via a swipe gesture.
   *
   * Since: 1.0
   */
  props[PROP_CAN_SWIPE_BACK] =
      g_param_spec_boolean ("can-swipe-back",
                            "Can swipe back",
                            "Whether or not swipe gesture can be used to switch from a subpage to the preferences",
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_f, GDK_CONTROL_MASK, search_open_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Escape, 0, close_cb, NULL);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-preferences-window.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, subpages_leaflet);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, preferences);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, content_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, pages_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, search_button);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, search_entry);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, search_results);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, search_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, title_stack);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, view_switcher_bar);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, view_switcher_title);
  gtk_widget_class_bind_template_callback (widget_class, subpages_leaflet_child_transition_running_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpages_leaflet_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, title_stack_notify_transition_running_cb);
  gtk_widget_class_bind_template_callback (widget_class, title_stack_notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_button_notify_active_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_started_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_result_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, search_results_map);
  gtk_widget_class_bind_template_callback (widget_class, search_results_unmap);
  gtk_widget_class_bind_template_callback (widget_class, stop_search_cb);
}

static gpointer
preferences_page_to_rows (gpointer page,
                          gpointer user_data)
{
  GtkWidget *child = gtk_stack_page_get_child (GTK_STACK_PAGE (page));

  return adw_preferences_page_get_rows (ADW_PREFERENCES_PAGE (child));
}

static void
adw_preferences_window_init (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  GListModel *model;
  GtkExpression *expr;

  priv->search_enabled = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->filter = GTK_FILTER (gtk_custom_filter_new ((GtkCustomFilterFunc) filter_search_results, self, NULL));
  expr = gtk_property_expression_new (GTK_TYPE_WIDGET, NULL, "visible");

  model = G_LIST_MODEL (gtk_stack_get_pages (priv->pages_stack));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (gtk_bool_filter_new (expr))));
  model = G_LIST_MODEL (gtk_map_list_model_new (model, preferences_page_to_rows, NULL, NULL));
  model = G_LIST_MODEL (gtk_flatten_list_model_new (model));
  priv->filter_model = gtk_filter_list_model_new (model, priv->filter);

  gtk_search_entry_set_key_capture_widget (priv->search_entry, GTK_WIDGET (self));
}

static void
adw_preferences_window_buildable_add_child (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const char   *type)
{
  AdwPreferencesWindow *self = ADW_PREFERENCES_WINDOW (buildable);
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  if (priv->content_stack && ADW_IS_PREFERENCES_PAGE (child))
    adw_preferences_window_add (self, ADW_PREFERENCES_PAGE (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_preferences_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adw_preferences_window_buildable_add_child;
}

/**
 * adw_preferences_window_new:
 *
 * Creates a new #AdwPreferencesWindow.
 *
 * Returns: a new #AdwPreferencesWindow
 *
 * Since: 1.0
 */
GtkWidget *
adw_preferences_window_new (void)
{
  return g_object_new (ADW_TYPE_PREFERENCES_WINDOW, NULL);
}

/**
 * adw_preferences_window_get_search_enabled:
 * @self: a #AdwPreferencesWindow
 *
 * Gets whether search is enabled for @self.
 *
 * Returns: whether search is enabled for @self.
 *
 * Since: 1.0
 */
gboolean
adw_preferences_window_get_search_enabled (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_WINDOW (self), FALSE);

  priv = adw_preferences_window_get_instance_private (self);

  return priv->search_enabled;
}

/**
 * adw_preferences_window_set_search_enabled:
 * @self: a #AdwPreferencesWindow
 * @search_enabled: %TRUE to enable search, %FALSE to disable it
 *
 * Sets whether search is enabled for @self.
 *
 * Since: 1.0
 */
void
adw_preferences_window_set_search_enabled (AdwPreferencesWindow *self,
                                           gboolean              search_enabled)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));

  priv = adw_preferences_window_get_instance_private (self);

  search_enabled = !!search_enabled;

  if (priv->search_enabled == search_enabled)
    return;

  priv->search_enabled = search_enabled;
  gtk_widget_set_visible (GTK_WIDGET (priv->search_button), search_enabled);
  if (search_enabled) {
    gtk_search_entry_set_key_capture_widget (priv->search_entry, GTK_WIDGET (self));
  } else {
    gtk_toggle_button_set_active (priv->search_button, FALSE);
    gtk_search_entry_set_key_capture_widget (priv->search_entry, NULL);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SEARCH_ENABLED]);
}

/**
 * adw_preferences_window_set_can_swipe_back:
 * @self: a #AdwPreferencesWindow
 * @can_swipe_back: the new value
 *
 * Sets whether or not @self allows switching from a subpage to the preferences
 * via a swipe gesture.
 *
 * Since: 1.0
 */
void
adw_preferences_window_set_can_swipe_back (AdwPreferencesWindow *self,
                                           gboolean              can_swipe_back)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));

  priv = adw_preferences_window_get_instance_private (self);

  can_swipe_back = !!can_swipe_back;

  if (priv->can_swipe_back == can_swipe_back)
    return;

  priv->can_swipe_back = can_swipe_back;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SWIPE_BACK]);
}

/**
 * adw_preferences_window_get_can_swipe_back
 * @self: a #AdwPreferencesWindow
 *
 * Returns whether or not @self allows switching from a subpage to the
 * preferences via a swipe gesture.
 *
 * Returns: %TRUE if back swipe is enabled.
 *
 * Since: 1.0
 */
gboolean
adw_preferences_window_get_can_swipe_back (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_WINDOW (self), FALSE);

  priv = adw_preferences_window_get_instance_private (self);

  return priv->can_swipe_back;
}

/**
 * adw_preferences_window_present_subpage:
 * @self: a #AdwPreferencesWindow
 * @subpage: the subpage
 *
 * Sets @subpage as the window's subpage and present it.
 * The transition can be cancelled by the user, in which case visible child will
 * change back to the previously visible child.
 *
 * Since: 1.0
 */
void
adw_preferences_window_present_subpage (AdwPreferencesWindow *self,
                                        GtkWidget            *subpage)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (subpage));

  priv = adw_preferences_window_get_instance_private (self);

  if (priv->subpage == subpage)
    return;

  priv->subpage = subpage;

  /* The check below avoids a warning when re-entering a subpage during the
   * transition between the that subpage to the preferences.
   */
  if (gtk_widget_get_parent (subpage) != GTK_WIDGET (priv->subpages_leaflet))
    adw_leaflet_append (priv->subpages_leaflet, subpage);

  adw_leaflet_set_visible_child (priv->subpages_leaflet, subpage);
}

/**
 * adw_preferences_window_close_subpage:
 * @self: a #AdwPreferencesWindow
 *
 * Closes the current subpage to return back to the preferences, if there is no
 * presented subpage, this does nothing.
 *
 * Since: 1.0
 */
void
adw_preferences_window_close_subpage (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));

  priv = adw_preferences_window_get_instance_private (self);

  if (priv->subpage == NULL)
    return;

  adw_leaflet_set_visible_child (priv->subpages_leaflet, priv->preferences);
}

void
adw_preferences_window_add (AdwPreferencesWindow *self,
                            AdwPreferencesPage   *page)
{
  AdwPreferencesWindowPrivate *priv;
  GtkStackPage *stack_page;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (ADW_IS_PREFERENCES_PAGE (page));

  priv = adw_preferences_window_get_instance_private (self);

  stack_page = gtk_stack_add_child (priv->pages_stack, GTK_WIDGET (page));

  g_object_bind_property (page, "icon-name", stack_page, "icon-name", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "title", stack_page, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "use-underline", stack_page, "use-underline", G_BINDING_SYNC_CREATE);
}

void
adw_preferences_window_remove (AdwPreferencesWindow *self,
                               AdwPreferencesPage   *page)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (ADW_IS_PREFERENCES_PAGE (page));

  priv = adw_preferences_window_get_instance_private (self);

  gtk_stack_remove (priv->pages_stack, GTK_WIDGET (page));
}
