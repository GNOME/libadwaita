/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-view-switcher-sidebar.h"

#include "adw-bin.h"
#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"

/**
 * AdwViewSwitcherSidebar:
 *
 * An adaptive sidebar that controls an [class@ViewStack].
 *
 * <picture>
 *   <source srcset="view-switcher-sidebar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="view-switcher-sidebar.png" alt="view-switcher-sidebar">
 * </picture>
 *
 * `AdwViewSwitcherSidebar` is a view switcher implemented using a
 * [class@Sidebar], in a similar fashion to [class@Gtk.StackSidebar].
 *
 * `AdwViewSwitcherSidebar` items have an icon, a label, as well as an unread
 * dot or a badge.
 *
 * Unlike other switchers, `AdwViewSwitcherSidebar` supports grouping pages into
 * sections, using the [property@ViewStackPage:starts-section] and
 * [property@ViewStackPage:section-title] properties.
 *
 * Like [class@Sidebar], `AdwViewSwitcherSidebar` is adaptive and can behave as
 * a sidebar or a page, via the [property@ViewSwitcherSidebar:mode] property.
 *
 * <picture>
 *   <source srcset="view-switcher-sidebar-modes-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="view-switcher-sidebar-modes.png" alt="view-switcher-sidebar-modes">
 * </picture>
 *
 * Connect to the [signal@ViewSwitcherSidebar::activated] signal to run code when
 * an item has been activated. This can be used to toggle the visible pane when
 * used in a split view.
 *
 * Like `AdwSidebar`, `AdwViewSwitcherSidebar` supports filtering items via the
 * [property@ViewSwitcherSidebar:filter] property.
 *
 * Use [property@ViewSwitcherSidebar:placeholder] to provide an empty state
 * widget. It will be shown when all items have been filtered out, or the
 * sidebar has no items otherwise.
 *
 * ## CSS nodes
 *
 * `AdwViewSwitcherSidebar` has a single CSS node with name
 * `view-switcher-sidebar`.
 *
 * See also: [class@ViewSwitcher], [class@ViewSwitcherBar],
 * [class@InlineViewSwitcher].
 *
 * Since: 1.9
 */

struct _AdwViewSwitcherSidebar
{
  GtkWidget parent_instance;

  GtkWidget *sidebar;

  AdwViewStack *stack;
  GListModel *pages;
  GHashTable *items;
};

G_DEFINE_FINAL_TYPE (AdwViewSwitcherSidebar, adw_view_switcher_sidebar, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_STACK,
  PROP_MODE,
  PROP_FILTER,
  PROP_PLACEHOLDER,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
activated_cb (AdwViewSwitcherSidebar *self,
              guint                   index)
{
  gtk_selection_model_select_item (GTK_SELECTION_MODEL (self->pages), index, TRUE);

  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0);
}

static void
set_badge_text (GtkLabel *label,
                guint     badge_number)
{
  char *text;

  if (badge_number > 999) {
    gtk_label_set_label (label, "999+");
    return;
  }

  text = g_strdup_printf ("%u", badge_number);

  gtk_label_set_label (label, text);

  g_free (text);
}

static void
update_badge (AdwViewStackPage *page,
              GParamSpec       *pspec,
              AdwSidebarItem   *item)
{
  GtkWidget *bin = adw_sidebar_item_get_suffix (item);
  GtkWidget *label = NULL;
  gboolean needs_attention = adw_view_stack_page_get_needs_attention (page);
  guint badge_number = adw_view_stack_page_get_badge_number (page);

  if (bin)
    label = gtk_widget_get_first_child (bin);

  if (!needs_attention && badge_number == 0) {
    /* We don't need any indicators here */
    adw_sidebar_item_set_suffix (item, NULL);
    return;
  }

  if (bin && !label && needs_attention && badge_number == 0) {
    /* Nothing to do here, it's already a dot */
    return;
  }

  if (!bin) {
    bin = adw_bin_new ();

    gtk_widget_set_valign (bin, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class (bin, "indicator");
    gtk_widget_add_css_class (bin, "dot");

    adw_sidebar_item_set_suffix (item, bin);
  }

  if (badge_number > 0) {
    if (!label) {
      label = gtk_label_new (NULL);

      gtk_widget_add_css_class (label, "numeric");

      adw_bin_set_child (ADW_BIN (bin), label);
    }

    set_badge_text (GTK_LABEL (label), badge_number);

    gtk_widget_remove_css_class (bin, "dot");
  } else if (label) {
    adw_bin_set_child (ADW_BIN (bin), NULL);
    label = NULL;

    gtk_widget_add_css_class (bin, "dot");
  }

  if (needs_attention)
    gtk_widget_add_css_class (bin, "needs-attention");
  else
    gtk_widget_remove_css_class (bin, "needs-attention");
}

static void
add_item (AdwViewSwitcherSidebar *self,
          AdwSidebarSection      *section,
          AdwViewStackPage       *page,
          guint                   index)
{
  AdwSidebarItem *item = adw_sidebar_item_new ("");

  g_hash_table_insert (self->items, page, g_object_ref (item));

  g_object_bind_property (page, "title", item, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "icon-name", item, "icon-name", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "use-underline", item, "use-underline", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "visible", item, "visible", G_BINDING_SYNC_CREATE);

  g_signal_connect_object (page, "notify::needs-attention", G_CALLBACK (update_badge), item, 0);
  g_signal_connect_object (page, "notify::badge-number", G_CALLBACK (update_badge), item, 0);

  update_badge (page, NULL, item);

  adw_sidebar_section_append (section, item);
}

static void
populate_sidebar (AdwViewSwitcherSidebar *self)
{
  guint i, n = g_list_model_get_n_items (self->pages);
  GtkWidget *visible_child;
  AdwViewStackPage *page;
  guint index = GTK_INVALID_LIST_POSITION;
  AdwSidebarSection *section = NULL;

  for (i = 0; i < n; i++) {
    page = g_list_model_get_item (self->pages, i);

    if (!section || adw_view_stack_page_get_starts_section (page)) {
      if (section)
        adw_sidebar_append (ADW_SIDEBAR (self->sidebar), section);

      section = adw_sidebar_section_new ();
      g_object_bind_property (page, "section-title", section, "title", G_BINDING_SYNC_CREATE);
    }

    add_item (self, section, page, i);
  }

  if (section)
    adw_sidebar_append (ADW_SIDEBAR (self->sidebar), section);

  visible_child = adw_view_stack_get_visible_child (self->stack);
  if (visible_child) {
    AdwSidebarItem *item;

    page = adw_view_stack_get_page (self->stack, visible_child);
    item = g_hash_table_lookup (self->items, page);
    index = adw_sidebar_item_get_index (item);
  }

  adw_sidebar_set_selected (ADW_SIDEBAR (self->sidebar), index);
}

static void
repopulate_sidebar (AdwViewSwitcherSidebar *self)
{
  adw_sidebar_remove_all (ADW_SIDEBAR (self->sidebar));
  populate_sidebar (self);
}

static void
selection_changed_cb (AdwViewSwitcherSidebar *self,
                      guint                   position,
                      guint                   n_items,
                      GtkSelectionModel      *model)
{
  GtkWidget *visible_child = adw_view_stack_get_visible_child (self->stack);
  guint index = GTK_INVALID_LIST_POSITION;

  if (visible_child) {
    AdwViewStackPage *page = adw_view_stack_get_page (self->stack, visible_child);
    AdwSidebarItem *item = g_hash_table_lookup (self->items, page);
    index = adw_sidebar_item_get_index (item);
  }

  adw_sidebar_set_selected (ADW_SIDEBAR (self->sidebar), index);
}

static void
set_stack (AdwViewSwitcherSidebar *self,
           AdwViewStack           *stack)
{
  guint i, n;

  if (!stack)
    return;

  self->stack = g_object_ref (stack);
  self->pages = g_object_ref (G_LIST_MODEL (adw_view_stack_get_pages (stack)));

  populate_sidebar (self);

  n = g_list_model_get_n_items (self->pages);

  for (i = 0; i < n; i++) {
    AdwViewStackPage *page = g_list_model_get_item (self->pages, i);

    g_signal_connect_swapped (page, "notify::visible", G_CALLBACK (repopulate_sidebar), self);

    g_object_unref (page);
  }

  g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (repopulate_sidebar), self);
  g_signal_connect_swapped (self->pages, "sections-changed", G_CALLBACK (repopulate_sidebar), self);
  g_signal_connect_swapped (self->pages, "selection-changed", G_CALLBACK (selection_changed_cb), self);
}

static void
unset_stack (AdwViewSwitcherSidebar *self)
{
  guint i, n;

  if (!self->stack)
    return;

  adw_sidebar_remove_all (ADW_SIDEBAR (self->sidebar));

  n = g_list_model_get_n_items (self->pages);

  for (i = 0; i < n; i++) {
    AdwViewStackPage *page = g_list_model_get_item (self->pages, i);

    g_signal_handlers_disconnect_by_func (page, repopulate_sidebar, self);

    g_object_unref (page);
  }

  g_signal_handlers_disconnect_by_func (self->pages, repopulate_sidebar, self);
  g_signal_handlers_disconnect_by_func (self->pages, selection_changed_cb, self);
  g_clear_object (&self->pages);
  g_clear_object (&self->stack);
}

static void
adw_view_switcher_sidebar_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  AdwViewSwitcherSidebar *self = ADW_VIEW_SWITCHER_SIDEBAR (object);

  switch (prop_id) {
  case PROP_STACK:
    g_value_set_object (value, adw_view_switcher_sidebar_get_stack (self));
    break;
  case PROP_MODE:
    g_value_set_enum (value, adw_view_switcher_sidebar_get_mode (self));
    break;
  case PROP_FILTER:
    g_value_set_object (value, adw_view_switcher_sidebar_get_filter (self));
    break;
  case PROP_PLACEHOLDER:
    g_value_set_object (value, adw_view_switcher_sidebar_get_placeholder (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_view_switcher_sidebar_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  AdwViewSwitcherSidebar *self = ADW_VIEW_SWITCHER_SIDEBAR (object);

  switch (prop_id) {
  case PROP_STACK:
    adw_view_switcher_sidebar_set_stack (self, g_value_get_object (value));
    break;
  case PROP_MODE:
    adw_view_switcher_sidebar_set_mode (self, g_value_get_enum (value));
    break;
  case PROP_FILTER:
    adw_view_switcher_sidebar_set_filter (self, g_value_get_object (value));
    break;
  case PROP_PLACEHOLDER:
    adw_view_switcher_sidebar_set_placeholder (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_view_switcher_sidebar_dispose (GObject *object)
{
  AdwViewSwitcherSidebar *self = ADW_VIEW_SWITCHER_SIDEBAR (object);

  unset_stack (self);

  g_clear_pointer (&self->items, g_hash_table_unref);
  g_clear_pointer (&self->sidebar, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_view_switcher_sidebar_parent_class)->dispose (object);
}

static void
adw_view_switcher_sidebar_class_init (AdwViewSwitcherSidebarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_view_switcher_sidebar_get_property;
  object_class->set_property = adw_view_switcher_sidebar_set_property;
  object_class->dispose = adw_view_switcher_sidebar_dispose;

  widget_class->compute_expand = adw_widget_compute_expand;
  widget_class->focus = adw_widget_focus_child;

  /**
   * AdwViewSwitcherSidebar:stack:
   *
   * The stack the sidebar controls.
   *
   * Since: 1.9
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack", NULL, NULL,
                         ADW_TYPE_VIEW_STACK,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewSwitcherSidebar:mode:
   *
   * Determines the sidebar's look and behavior.
   *
   * If set to [enum@Adw.SidebarMode.SIDEBAR], behaves like a sidebar: with a
   * sidebar style and a persistent selection.
   *
   * If set to [enum@Adw.SidebarMode.PAGE], behaves like a page of boxed lists.
   *
   * The page mode is intended to be used with [class@NavigationSplitView] when
   * collapsed, as the sidebar pane becomes a page there.
   *
   * When used with [class@OverlaySplitView], the sidebar should stay in sidebar
   * mode, as the sidebar pane is still a sidebar when collapsed.
   *
   * See [property@Sidebar:mode].
   *
   * Since: 1.9
   */
  props[PROP_MODE] =
    g_param_spec_enum ("mode", NULL, NULL,
                       ADW_TYPE_SIDEBAR_MODE,
                       ADW_SIDEBAR_MODE_SIDEBAR,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewSwitcherSidebar:filter:
   *
   * The item filter.
   *
   * Can be used to implement search within the sidebar.
   *
   * Use [property@ViewSwitcherSidebar:placeholder] to provide an empty state.
   *
   * See [property@Sidebar:filter].
   *
   * Since: 1.9
   */
  props[PROP_FILTER] =
    g_param_spec_object ("filter", NULL, NULL,
                         GTK_TYPE_FILTER,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewSwitcherSidebar:placeholder:
   *
   * The placeholder widget.
   *
   * This widget will be shown if the sidebar has no items, or all of its items
   * have been filtered out by [property@ViewSwitcherSidebar:filter].
   *
   * See [property@Sidebar:placeholder].
   *
   * Since: 1.9
   */
  props[PROP_PLACEHOLDER] =
    g_param_spec_object ("placeholder", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwViewSwitcherSidebar::activated:
   *
   * Emitted when an item has been activated.
   *
   * Since: 1.9
   */
  signals[SIGNAL_ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "view-switcher-sidebar");
}

static void
adw_view_switcher_sidebar_init (AdwViewSwitcherSidebar *self)
{
  self->sidebar = adw_sidebar_new ();

  gtk_widget_set_parent (self->sidebar, GTK_WIDGET (self));

  g_signal_connect_swapped (self->sidebar, "activated",
                            G_CALLBACK (activated_cb), self);

  self->items = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, NULL);
}

/**
 * adw_view_switcher_sidebar_new:
 *
 * Creates a new `AdwViewSwitcherSidebar`.
 *
 * Returns: the newly created `AdwViewSwitcherSidebar`
 *
 * Since: 1.9
 */
GtkWidget *
adw_view_switcher_sidebar_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_SIDEBAR, NULL);
}

/**
 * adw_view_switcher_sidebar_get_stack:
 * @self: a view switcher sidebar
 *
 * Gets the stack @self controls.
 *
 * Returns: (nullable) (transfer none): The stack of @self
 *
 * Since: 1.9
 */
AdwViewStack *
adw_view_switcher_sidebar_get_stack (AdwViewSwitcherSidebar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self), NULL);

  return self->stack;
}

/**
 * adw_view_switcher_sidebar_set_stack:
 * @self: a view switcher sidebar
 * @stack: (nullable): a stack
 *
 * Sets the stack to control.
 *
 * Since: 1.9
 */
void
adw_view_switcher_sidebar_set_stack (AdwViewSwitcherSidebar *self,
                                     AdwViewStack           *stack)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self));
  g_return_if_fail (stack == NULL || ADW_IS_VIEW_STACK (stack));

  if (self->stack == stack)
    return;

  unset_stack (self);
  set_stack (self, stack);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * adw_view_switcher_sidebar_get_mode:
 * @self: a view switcher sidebar
 *
 * Gets @self's look and behavior.
 *
 * See [method@Sidebar.get_mode].
 *
 * Returns: the current mode
 *
 * Since: 1.9
 */
AdwSidebarMode
adw_view_switcher_sidebar_get_mode (AdwViewSwitcherSidebar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self), ADW_SIDEBAR_MODE_SIDEBAR);

  return adw_sidebar_get_mode (ADW_SIDEBAR (self->sidebar));
}

/**
 * adw_view_switcher_sidebar_set_mode:
 * @self: a view switcher sidebar
 * @mode: the new mode
 *
 * Sets @self's look and behavior.
 *
 * If set to [enum@Adw.SidebarMode.SIDEBAR], behaves like a sidebar: with a
 * sidebar style and a persistent selection.
 *
 * If set to [enum@Adw.SidebarMode.PAGE], behaves like a page of boxed lists.
 * In this mode, the selection is invisible and only tracked to determine the
 * initially selected item once switched back to sidebar mode.
 *
 * The page mode is intended to be used with [class@NavigationSplitView] when
 * collapsed, as the sidebar pane becomes a page there.
 *
 * When used with [class@OverlaySplitView], the sidebar should stay in sidebar
 * mode, as the sidebar pane is still a sidebar when collapsed.
 *
 * See [method@Sidebar.set_mode].
 *
 * Since: 1.9
 */
void
adw_view_switcher_sidebar_set_mode (AdwViewSwitcherSidebar *self,
                                    AdwSidebarMode          mode)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self));
  g_return_if_fail (mode >= ADW_SIDEBAR_MODE_SIDEBAR);
  g_return_if_fail (mode <= ADW_SIDEBAR_MODE_PAGE);

  if (mode == adw_view_switcher_sidebar_get_mode (self))
    return;

  adw_sidebar_set_mode (ADW_SIDEBAR (self->sidebar), mode);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODE]);
}

/**
 * adw_view_switcher_sidebar_get_filter:
 * @self: a view switcher sidebar
 *
 * Gets the item filter for @self.
 *
 * Returns: (transfer none) (nullable): the item filter
 *
 * Since: 1.9
 */
GtkFilter *
adw_view_switcher_sidebar_get_filter (AdwViewSwitcherSidebar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self), NULL);

  return adw_sidebar_get_filter (ADW_SIDEBAR (self->sidebar));
}

/**
 * adw_view_switcher_sidebar_set_filter:
 * @self: a view switcher sidebar
 * @filter: (transfer none) (nullable): the item filter
 *
 * Sets the item filter for @self.
 *
 * Can be used to implement search within the sidebar.
 *
 * Use [property@ViewSwitcherSidebar:placeholder] to provide an empty state.
 *
 * See [method@Sidebar.set_filter].
 *
 * Since: 1.9
 */
void
adw_view_switcher_sidebar_set_filter (AdwViewSwitcherSidebar *self,
                                      GtkFilter              *filter)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self));
  g_return_if_fail (filter == NULL || GTK_IS_FILTER (filter));

  if (filter == adw_view_switcher_sidebar_get_filter (self))
    return;

  adw_sidebar_set_filter (ADW_SIDEBAR (self->sidebar), filter);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FILTER]);
}

/**
 * adw_view_switcher_sidebar_get_placeholder:
 * @self: a view switcher sidebar
 *
 * Gets the placeholder widget for @self.
 *
 * Returns: (transfer none) (nullable): the placeholder widget
 *
 * Since: 1.9
 */
GtkWidget *
adw_view_switcher_sidebar_get_placeholder (AdwViewSwitcherSidebar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self), NULL);

  return adw_sidebar_get_placeholder (ADW_SIDEBAR (self->sidebar));
}

/**
 * adw_view_switcher_sidebar_set_placeholder:
 * @self: a view switcher sidebar
 * @placeholder: (transfer none) (nullable): the placeholder widget
 *
 * Sets the placeholder widget for @self.
 *
 * This widget will be shown if @self has no items, or all of its items have
 * been filtered out by [property@ViewSwitcherSidebar:filter].
 *
 * See [method@Sidebar.set_placeholder].
 *
 * Since: 1.9
 */
void
adw_view_switcher_sidebar_set_placeholder (AdwViewSwitcherSidebar *self,
                                           GtkWidget              *placeholder)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_SIDEBAR (self));
  g_return_if_fail (placeholder == NULL || GTK_IS_WIDGET (placeholder));

  if (placeholder == adw_view_switcher_sidebar_get_placeholder (self))
    return;

  adw_sidebar_set_placeholder (ADW_SIDEBAR (self->sidebar), placeholder);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PLACEHOLDER]);
}
