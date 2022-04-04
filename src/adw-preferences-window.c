/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-preferences-window.h"

#include "adw-animation-util.h"
#include "adw-action-row.h"
#include "adw-leaflet.h"
#include "adw-macros-private.h"
#include "adw-preferences-group-private.h"
#include "adw-preferences-page-private.h"
#include "adw-toast-overlay.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-bar.h"
#include "adw-view-switcher-title.h"

/**
 * AdwPreferencesWindow:
 *
 * A window to present an application's preferences.
 *
 * <picture>
 *   <source srcset="preferences-window-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="preferences-window.png" alt="preferences-window">
 * </picture>
 *
 * The `AdwPreferencesWindow` widget presents an application's preferences
 * gathered into pages and groups. The preferences are searchable by the user.
 *
 * ## CSS nodes
 *
 * `AdwPreferencesWindow` has a main CSS node with the name `window` and the
 * style class `.preferences`.
 *
 * Since: 1.0
 */

typedef struct
{
  AdwToastOverlay *toast_overlay;
  AdwLeaflet *subpages_leaflet;
  GtkWidget *preferences;
  GtkStack *content_stack;
  AdwViewStack *pages_stack;
  GtkToggleButton *search_button;
  GtkSearchEntry *search_entry;
  GtkListBox *search_results;
  GtkStack *search_stack;
  GtkStack *title_stack;
  AdwViewSwitcherBar *view_switcher_bar;
  AdwViewSwitcherTitle *view_switcher_title;

  gboolean search_enabled;
  gboolean can_navigate_back;

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
  PROP_VISIBLE_PAGE,
  PROP_VISIBLE_PAGE_NAME,
  PROP_SEARCH_ENABLED,
  PROP_CAN_NAVIGATE_BACK,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

/* Copied and modified from gtklabel.c, separate_uline_pattern() */
static char *
strip_mnemonic (const char *src)
{
  char *new_str = g_new (char, strlen (src) + 1);
  char *dest = new_str;
  gboolean underscore = FALSE;

  while (*src) {
    gunichar c;
    const char *next_src;

    c = g_utf8_get_char (src);
    if (c == (gunichar) -1) {
      g_warning ("Invalid input string");

      g_free (new_str);

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

  return new_str;
}

static gboolean
filter_search_results (AdwPreferencesRow    *row,
                       AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);
  char *terms, *title;
  gboolean result = FALSE;

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

  // TODO: Looks like a bug, that markup is not removed here?

  if (!!strstr (title, terms)) {
    result = TRUE;
  } else if (ADW_IS_ACTION_ROW (row)) {
    char *subtitle = g_utf8_casefold (adw_action_row_get_subtitle (ADW_ACTION_ROW (row)), -1);

    if (!!strstr (subtitle, terms))
      result = TRUE;

    g_free (subtitle);
  }

  g_free (title);
  g_free (terms);

  return result;
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
    AdwViewStackPage *page = adw_view_stack_get_page (priv->pages_stack, child);

    if (adw_view_stack_page_get_visible (page))
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
  char *page_title = NULL;

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
      g_clear_pointer (&page_title, g_free);
  }

  if (group_title) {
    gchar *result;

    if (get_n_pages (self) > 1)
      result = g_strdup_printf ("%s → %s", page_title ? page_title : _("Untitled page"), group_title);
    else
      result = g_strdup (group_title);

    g_free (page_title);

    return result;
  }

  return page_title;
}

static GtkWidget *
new_search_row_for_preference (AdwPreferencesRow    *row,
                               AdwPreferencesWindow *self)
{
  AdwActionRow *widget;
  GtkWidget *page;
  char *subtitle;

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

  g_clear_pointer (&subtitle, g_free);

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

  adw_view_stack_set_visible_child (priv->pages_stack, GTK_WIDGET (page));
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
notify_visible_page_cb (AdwPreferencesWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE]);
}

static void
notify_visible_page_name_cb (AdwPreferencesWindow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_PAGE_NAME]);
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
  case PROP_VISIBLE_PAGE:
    g_value_set_object (value, adw_preferences_window_get_visible_page (self));
    break;
  case PROP_VISIBLE_PAGE_NAME:
    g_value_set_string (value, adw_preferences_window_get_visible_page_name (self));
    break;
  case PROP_SEARCH_ENABLED:
    g_value_set_boolean (value, adw_preferences_window_get_search_enabled (self));
    break;
  case PROP_CAN_NAVIGATE_BACK:
    g_value_set_boolean (value, adw_preferences_window_get_can_navigate_back (self));
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
  case PROP_VISIBLE_PAGE:
    adw_preferences_window_set_visible_page (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE_PAGE_NAME:
    adw_preferences_window_set_visible_page_name (self, g_value_get_string (value));
    break;
  case PROP_SEARCH_ENABLED:
    adw_preferences_window_set_search_enabled (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_NAVIGATE_BACK:
    adw_preferences_window_set_can_navigate_back (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_window_dispose (GObject *object)
{
  AdwPreferencesWindow *self = ADW_PREFERENCES_WINDOW (object);
  AdwPreferencesWindowPrivate *priv = adw_preferences_window_get_instance_private (self);

  g_clear_object (&priv->filter_model);

  G_OBJECT_CLASS (adw_preferences_window_parent_class)->dispose (object);
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
    if (!adw_preferences_window_get_can_navigate_back (self))
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
  object_class->dispose = adw_preferences_window_dispose;

  /**
   * AdwViewStack:visible-page: (attributes org.gtk.Property.get=adw_preferences_window_get_visible_page org.gtk.Property.set=adw_preferences_window_set_visible_page)
   *
   * The currently visible page.
   *
   * Since: 1.0
   */
  props[PROP_VISIBLE_PAGE] =
      g_param_spec_object ("visible-page",
                           "Visible page",
                           "The currently visible page",
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewStack:visible-page-name: (attributes org.gtk.Property.get=adw_preferences_window_get_visible_page_name org.gtk.Property.set=adw_preferences_window_set_visible_page_name)
   *
   * The name of the currently visible page.
   *
   * See [property@ViewStack:visible-child].
   *
   * Since: 1.0
   */
  props[PROP_VISIBLE_PAGE_NAME] =
      g_param_spec_string ("visible-page-name",
                           "Name of visible page",
                           "The name of the currently visible page",
                           NULL,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwPreferencesWindow:search-enabled: (attributes org.gtk.Property.get=adw_preferences_window_get_search_enabled org.gtk.Property.set=adw_preferences_window_set_search_enabled)
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
   * AdwPreferencesWindow:can-navigate-back: (attributes org.gtk.Property.get=adw_preferences_window_get_can_navigate_back org.gtk.Property.set=adw_preferences_window_set_can_navigate_back)
   *
   * Whether gestures and shortcuts for closing subpages are enabled.
   *
   * The supported gestures are:
   *
   * - One-finger swipe on touchscreens
   * - Horizontal scrolling on touchpads (usually two-finger swipe)
   * - Back mouse button
   *
   * The keyboard back key is also supported, as well as the
   * <kbd>Alt</kbd>+<kbd>←</kbd> shortcut.
   *
   * For right-to-left locales, gestures and shortcuts are reversed.
   *
   * Since: 1.0
   */
  props[PROP_CAN_NAVIGATE_BACK] =
      g_param_spec_boolean ("can-navigate-back",
                            "Can navigate back",
                            "Whether gestures and shortcuts for closing subpages are enabled",
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_f, GDK_CONTROL_MASK, search_open_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Escape, 0, close_cb, NULL);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-preferences-window.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesWindow, toast_overlay);
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
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_page_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_page_name_cb);
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
  GtkWidget *child = adw_view_stack_page_get_child (ADW_VIEW_STACK_PAGE (page));

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
  expr = gtk_property_expression_new (GTK_TYPE_STACK_PAGE, NULL, "visible");

  model = G_LIST_MODEL (adw_view_stack_get_pages (priv->pages_stack));
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
  else if (ADW_IS_TOAST (child))
    adw_preferences_window_add_toast (self, g_object_ref (ADW_TOAST (child)));
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
 * Creates a new `AdwPreferencesWindow`.
 *
 * Returns: the newly created `AdwPreferencesWindow`
 *
 * Since: 1.0
 */
GtkWidget *
adw_preferences_window_new (void)
{
  return g_object_new (ADW_TYPE_PREFERENCES_WINDOW, NULL);
}

/**
 * adw_preferences_window_get_search_enabled: (attributes org.gtk.Method.get_property=search-enabled)
 * @self: a preferences window
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
 * adw_preferences_window_set_search_enabled: (attributes org.gtk.Method.set_property=search-enabled)
 * @self: a preferences window
 * @search_enabled: whether search is enabled
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
 * adw_preferences_window_set_can_navigate_back: (attributes org.gtk.Method.set_property=can-navigate-back)
 * @self: a preferences window
 * @can_navigate_back: the new value
 *
 * Sets whether gestures and shortcuts for closing subpages are enabled.
 *
 * Since: 1.0
 */
void
adw_preferences_window_set_can_navigate_back (AdwPreferencesWindow *self,
                                              gboolean              can_navigate_back)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));

  priv = adw_preferences_window_get_instance_private (self);

  can_navigate_back = !!can_navigate_back;

  if (priv->can_navigate_back == can_navigate_back)
    return;

  priv->can_navigate_back = can_navigate_back;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_NAVIGATE_BACK]);
}

/**
 * adw_preferences_window_get_can_navigate_back: (attributes org.gtk.Method.get_property=can-navigate-back)
 * @self: a preferences window
 *
 * Gets whether gestures and shortcuts for closing subpages are enabled.
 *
 * Returns: whether gestures and shortcuts are enabled.
 *
 * Since: 1.0
 */
gboolean
adw_preferences_window_get_can_navigate_back (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_WINDOW (self), FALSE);

  priv = adw_preferences_window_get_instance_private (self);

  return priv->can_navigate_back;
}

/**
 * adw_preferences_window_present_subpage:
 * @self: a preferences window
 * @subpage: the subpage
 *
 * Sets @subpage as the window's subpage and opens it.
 *
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
 * @self: a preferences window
 *
 * Closes the current subpage.
 *
 * If there is no presented subpage, this does nothing.
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

/**
 * adw_preferences_window_add:
 * @self: a preferences window
 * @page: the page to add
 *
 * Adds a preferences page to @self.
 *
 * Since: 1.0
 */
void
adw_preferences_window_add (AdwPreferencesWindow *self,
                            AdwPreferencesPage   *page)
{
  AdwPreferencesWindowPrivate *priv;
  AdwViewStackPage *stack_page;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (ADW_IS_PREFERENCES_PAGE (page));

  priv = adw_preferences_window_get_instance_private (self);

  stack_page = adw_view_stack_add_named (priv->pages_stack, GTK_WIDGET (page), adw_preferences_page_get_name (page));

  g_object_bind_property (page, "icon-name", stack_page, "icon-name", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "title", stack_page, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "use-underline", stack_page, "use-underline", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "name", stack_page, "name", G_BINDING_SYNC_CREATE);
}

/**
 * adw_preferences_window_remove:
 * @self: a preferences window
 * @page: the page to remove
 *
 * Removes a page from @self.
 *
 * Since: 1.0
 */
void
adw_preferences_window_remove (AdwPreferencesWindow *self,
                               AdwPreferencesPage   *page)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (ADW_IS_PREFERENCES_PAGE (page));

  priv = adw_preferences_window_get_instance_private (self);

  if (gtk_widget_get_parent (GTK_WIDGET (page)) == GTK_WIDGET (priv->pages_stack))
    adw_view_stack_remove (priv->pages_stack, GTK_WIDGET (page));
  else
    ADW_CRITICAL_CANNOT_REMOVE_CHILD (self, page);
}

/**
 * adw_preferences_window_get_visible_page:
 * @self: a preferences window
 *
 * Gets the currently visible page of @self.
 *
 * Returns: (transfer none) (nullable): the visible page
 *
 * Since: 1.0
 */
AdwPreferencesPage *
adw_preferences_window_get_visible_page (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_WINDOW (self), NULL);

  priv = adw_preferences_window_get_instance_private (self);

  return ADW_PREFERENCES_PAGE (adw_view_stack_get_visible_child (priv->pages_stack));
}

/**
 * adw_preferences_window_set_visible_page:
 * @self: a preferences window
 * @page: a page of @self
 *
 * Makes @page the visible page of @self.
 *
 * Since: 1.0
 */
void
adw_preferences_window_set_visible_page (AdwPreferencesWindow *self,
                                         AdwPreferencesPage   *page)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));
  g_return_if_fail (ADW_IS_PREFERENCES_PAGE (page));

  priv = adw_preferences_window_get_instance_private (self);

  adw_view_stack_set_visible_child (priv->pages_stack, GTK_WIDGET (page));
}

/**
 * adw_preferences_window_get_visible_page_name:
 * @self: a preferences window
 *
 * Gets the name of currently visible page of @self.
 *
 * Returns: (transfer none) (nullable): the name of the visible page
 *
 * Since: 1.0
 */
const char *
adw_preferences_window_get_visible_page_name (AdwPreferencesWindow *self)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_WINDOW (self), NULL);

  priv = adw_preferences_window_get_instance_private (self);

  return adw_view_stack_get_visible_child_name (priv->pages_stack);
}

/**
 * adw_preferences_window_set_visible_page_name:
 * @self: a preferences window
 * @name: the name of the page to make visible
 *
 * Makes the page with the given name visible.
 *
 * Since: 1.0
 */
void
adw_preferences_window_set_visible_page_name (AdwPreferencesWindow *self,
                                              const char           *name)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));

  priv = adw_preferences_window_get_instance_private (self);

  adw_view_stack_set_visible_child_name (priv->pages_stack, name);
}

/**
 * adw_preferences_window_add_toast:
 * @self: a preferences window
 * @toast: (transfer full): a toast
 *
 * Displays @toast.
 *
 * See [method@ToastOverlay.add_toast].
 *
 * Since: 1.0
 */
void
adw_preferences_window_add_toast (AdwPreferencesWindow *self,
                                  AdwToast             *toast)
{
  AdwPreferencesWindowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_WINDOW (self));

  priv = adw_preferences_window_get_instance_private (self);

  adw_toast_overlay_add_toast (priv->toast_overlay, toast);
}
