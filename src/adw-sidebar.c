/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-sidebar.h"

#include "adw-action-row.h"
#include "adw-marshalers.h"
#include "adw-preferences-group.h"
#include "adw-preferences-page-private.h"
#include "adw-preferences-row.h"
#include "adw-sidebar-item.h"
#include "adw-sidebar-section-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwSidebarMode:
 * @ADW_SIDEBAR_MODE_SIDEBAR: The sidebar should be displayed as a sidebar
 * @ADW_SIDEBAR_MODE_PAGE: The sidebar should be displayed as boxed lists page
 *
 * Determines how an [class@Sidebar] should look and behave.
 *
 * See [property@Sidebar:mode] and [property@ViewSwitcherSidebar:mode].
 *
 * Since: 1.9
 */

/**
 * AdwSidebar:
 *
 * Adaptive sidebar widget.
 *
 * <picture>
 *   <source srcset="sidebar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="sidebar.png" alt="sidebar">
 * </picture>
 *
 * `AdwSidebar` contains [class@SidebarSection] objects, which in turn contain
 * [class@SidebarItem] objects.
 *
 * To add sections, use [method@Sidebar.append], [method@Sidebar.prepend] or
 * [method@Sidebar.insert].
 *
 * To remove sections, use [method@Sidebar.remove] or
 * [method@Sidebar.remove_all].
 *
 * To inspect the items, use [method@Sidebar.get_item] or
 * [property@Sidebar:items].
 *
 * To inspect sections themselves, use [method@Sidebar.get_section] or
 * [property@Sidebar:sections].
 *
 * ## Selection and activation
 *
 * `AdwSidebar` has zero or one selected items. The index of the item can be
 * accessed and changed via [property@Sidebar:selected]. Set it to
 * [const@Gtk.INVALID_LIST_POSITION] to remove selection.
 *
 * Selection cannot be permanently disabled.
 *
 * [property@Sidebar:selected-item] can be used to access the selected item.
 *
 * Connect to the [signal@Sidebar::activated] signal to run code when an item
 * has been activated. This can be used to toggle the visible pane when used in
 * a split view.
 *
 * See also: [class@ViewSwitcherSidebar].
 *
 * ## Modes
 *
 * <picture>
 *   <source srcset="sidebar-modes-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="sidebar-modes.png" alt="sidebar-modes">
 * </picture>
 *
 * `AdwSidebar` is adaptive and can act as either a regular sidebar, or a page
 * of boxed lists.
 *
 * Use the [property@Sidebar:mode] to determine its look and behavior.
 *
 * A typical use case involves using `AdwSidebar` inside the sidebar pane of a
 * [class@NavigationSplitView], and switching mode to page whenever it's
 * collapsed, as follows:
 *
 * ```xml
 * <object class="AdwWindow">
 *   <property name="default-width">800</property>
 *   <property name="default-height">600</property>
 *   <child>
 *     <object class="AdwBreakpoint">
 *       <condition>max-width: 400sp</condition>
 *       <setter object="split_view" property="collapsed">True</setter>
 *       <setter object="sidebar" property="mode">page</setter>
 *     </object>
 *   </child>
 *   <property name="content">
 *     <object class="AdwNavigationSplitView" id="split_view">
 *       <property name="sidebar">
 *         <object class="AdwNavigationPage">
 *           <property name="title" translatable="yes">Sidebar</property>
 *           <property name="child">
 *             <object class="AdwToolbarView">
 *               <child type="top">
 *                 <object class="AdwHeaderBar"/>
 *               </child>
 *               <property name="content">
 *                 <object class="AdwSidebar" id="sidebar">
 *                   <!-- Calls adw_navigation_split_view_set_show_content (split_view, TRUE); -->
 *                   <signal name="activated" handler="sidebar_activated_cb"/>
 *                   <!-- ... -->
 *                 </object>
 *               </property>
 *             </object>
 *           </property>
 *         </object>
 *       </property>
 *       <property name="content">
 *         <object class="AdwNavigationPage">
 *           <property name="title" translatable="yes">Content</property>
 *           <property name="child">
 *             <!-- ... -->
 *           </property>
 *         </object>
 *       </property>
 *     </object>
 *   </property>
 * </object>
 * ```
 *
 * When used with [class@OverlaySplitView], the sidebar should stay in sidebar
 * mode, as the sidebar pane is still a sidebar when collapsed.
 *
 * ## Search
 *
 * `AdwSidebar` supports filtering items via the [property@Sidebar:filter]
 * property.
 *
 * Use [property@Sidebar:placeholder] to provide an empty state widget. It will
 * be shown when all items have been filtered out, or the sidebar has no items
 * otherwise.
 *
 * ## `AdwSidebar` as `GtkBuildable`
 *
 * `AdwSidebar` allows adding sections as children.
 *
 * Example of an `AdwSidebar` UI definition:
 *
 * ```xml
 * <object class="AdwSidebar">
 *   <child>
 *     <object class="AdwSidebarSection">
 *       <child>
 *         <object class="AdwSidebarItem">
 *           <property name="title" translatable="yes">Recent</property>
 *           <property name="icon-name">document-open-recent-symbolic</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwSidebarItem">
 *           <property name="title" translatable="yes">Starred</property>
 *           <property name="icon-name">starred-symbolic</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwSidebarSection">
 *       <property name="title" translatable="yes">Places</property>
 *       <child>
 *         <object class="AdwSidebarItem">
 *           <property name="title" translatable="yes">Music</property>
 *           <property name="icon-name">folder-music-symbolic</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwSidebarItem">
 *           <property name="title" translatable="yes">Pictures</property>
 *           <property name="icon-name">folder-pictures-symbolic</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="AdwSidebarItem">
 *           <property name="title" translatable="yes">Videos</property>
 *           <property name="icon-name">folder-videos-symbolic</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwSidebarSection">
 *       <child>
 *         <object class="AdwSidebarItem">
 *           <property name="title" translatable="yes">Trash</property>
 *           <property name="icon-name">user-trash-symbolic</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwSidebar` has a main CSS node with the name `sidebar`.
 *
 * Internally, it's using a [class@Gtk.ListBox] with the
 * [`.navigation-sidebar`](style-classes.html#sidebars) style class in sidebar
 * mode, or an [class@PreferencesPage] in page mode.
 *
 * ## Accessibility
 *
 * `AdwSidebar` uses the [enum@Gtk.AccessibleRole.generic] role.
 *
 * Since: 1.9
 */

struct _AdwSidebar
{
  GtkWidget parent_instance;

  AdwSidebarMode mode;

  GPtrArray *sections;
  GListModel *sections_model;

  GListModel *items_model;
  GtkFilterListModel *filtered_items;

  GtkWidget *swindow;
  GtkWidget *listbox;
  GtkWidget *page;
  GtkWidget *placeholder;

  guint selected;
  guint n_items;

  gboolean in_dispose;

  int block_row_selected;

  guint restore_scroll_idle_id;
};

static void adw_sidebar_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSidebar, adw_sidebar, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_sidebar_buildable_init))

enum {
  PROP_0,
  PROP_MODE,
  PROP_SELECTED,
  PROP_SELECTED_ITEM,
  PROP_ITEMS,
  PROP_SECTIONS,
  PROP_FILTER,
  PROP_PLACEHOLDER,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

#define ADW_TYPE_SIDEBAR_SECTIONS (adw_sidebar_sections_get_type ())

G_DECLARE_FINAL_TYPE (AdwSidebarSections, adw_sidebar_sections, ADW, SIDEBAR_SECTIONS, GObject)

struct _AdwSidebarSections
{
  GObject parent_instance;

  AdwSidebar *sidebar;
};

static void adw_sidebar_sections_list_model_init (GListModelInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSidebarSections, adw_sidebar_sections, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_sidebar_sections_list_model_init))

static void
adw_sidebar_sections_dispose (GObject *object)
{
  AdwSidebarSections *self = ADW_SIDEBAR_SECTIONS (object);

  g_clear_weak_pointer (&self->sidebar);

  G_OBJECT_CLASS (adw_sidebar_sections_parent_class)->dispose (object);
}

static void
adw_sidebar_sections_class_init (AdwSidebarSectionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_sidebar_sections_dispose;
}

static void
adw_sidebar_sections_init (AdwSidebarSections *self)
{
}

static GType
adw_sidebar_sections_get_item_type (GListModel *model)
{
  return ADW_TYPE_SIDEBAR_SECTION;
}

static guint
adw_sidebar_sections_get_n_items (GListModel *model)
{
  AdwSidebarSections *self = ADW_SIDEBAR_SECTIONS (model);

  return self->sidebar->sections->len;
}

static gpointer
adw_sidebar_sections_get_item (GListModel *model,
                               guint       position)
{
  AdwSidebarSections *self = ADW_SIDEBAR_SECTIONS (model);
  AdwSidebarSection *section;

  if (position >= g_list_model_get_n_items (model))
    return NULL;

  section = g_ptr_array_index (self->sidebar->sections, position);

  return g_object_ref (section);
}

static void
adw_sidebar_sections_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_sidebar_sections_get_item_type;
  iface->get_n_items = adw_sidebar_sections_get_n_items;
  iface->get_item = adw_sidebar_sections_get_item;
}

static AdwSidebarSections *
adw_sidebar_sections_new (AdwSidebar *sidebar)
{
  AdwSidebarSections *sections;

  sections = g_object_new (ADW_TYPE_SIDEBAR_SECTIONS, NULL);
  g_set_weak_pointer (&sections->sidebar, sidebar);

  return sections;
}

#define ADW_TYPE_SIDEBAR_ITEMS (adw_sidebar_items_get_type ())

G_DECLARE_FINAL_TYPE (AdwSidebarItems, adw_sidebar_items, ADW, SIDEBAR_ITEMS, GObject)

struct _AdwSidebarItems
{
  GObject parent_instance;

  AdwSidebar *sidebar;
  GtkMapListModel *map_model;
  GtkFlattenListModel *flatten_model;
};

static void adw_sidebar_items_list_model_init (GListModelInterface *iface);
static void adw_sidebar_items_section_model_init (GtkSectionModelInterface *iface);
static void adw_sidebar_items_selection_model_init (GtkSelectionModelInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSidebarItems, adw_sidebar_items, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_sidebar_items_list_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SECTION_MODEL, adw_sidebar_items_section_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, adw_sidebar_items_selection_model_init))

static void
adw_sidebar_items_dispose (GObject *object)
{
  AdwSidebarItems *self = ADW_SIDEBAR_ITEMS (object);

  g_clear_weak_pointer (&self->sidebar);
  g_clear_object (&self->flatten_model);

  G_OBJECT_CLASS (adw_sidebar_items_parent_class)->dispose (object);
}

static void
adw_sidebar_items_class_init (AdwSidebarItemsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_sidebar_items_dispose;
}

static void
adw_sidebar_items_init (AdwSidebarItems *self)
{
}

static GType
adw_sidebar_items_get_item_type (GListModel *model)
{
  return ADW_TYPE_SIDEBAR_ITEM;
}

static guint
adw_sidebar_items_get_n_items (GListModel *model)
{
  AdwSidebarItems *self = ADW_SIDEBAR_ITEMS (model);

  return g_list_model_get_n_items (G_LIST_MODEL (self->flatten_model));
}

static gpointer
adw_sidebar_items_get_item (GListModel *model,
                            guint       position)
{
  AdwSidebarItems *self = ADW_SIDEBAR_ITEMS (model);

  return g_list_model_get_item (G_LIST_MODEL (self->flatten_model), position);
}

static void
adw_sidebar_items_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_sidebar_items_get_item_type;
  iface->get_n_items = adw_sidebar_items_get_n_items;
  iface->get_item = adw_sidebar_items_get_item;
}

static void
adw_sidebar_items_get_section (GtkSectionModel *model,
                               guint            position,
                               guint           *out_start,
                               guint           *out_end)
{
  AdwSidebarItems *self = ADW_SIDEBAR_ITEMS (model);

  gtk_section_model_get_section (GTK_SECTION_MODEL (self->flatten_model),
                                 position, out_start, out_end);
}

static void
adw_sidebar_items_section_model_init (GtkSectionModelInterface *iface)
{
  iface->get_section = adw_sidebar_items_get_section;
}

static gboolean
adw_sidebar_items_is_selected (GtkSelectionModel *model,
                               guint              position)
{
  AdwSidebarItems *self = ADW_SIDEBAR_ITEMS (model);

  return position == self->sidebar->selected;
}

static gboolean
adw_sidebar_items_select_item (GtkSelectionModel *model,
                               guint              position,
                               gboolean           exclusive)
{
  AdwSidebarItems *self = ADW_SIDEBAR_ITEMS (model);

  adw_sidebar_set_selected (self->sidebar, position);

  return TRUE;
}

static void
adw_sidebar_items_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = adw_sidebar_items_is_selected;
  iface->select_item = adw_sidebar_items_select_item;
}

static GObject *
get_section_items (AdwSidebarSection *section,
                   AdwSidebar        *sidebar)
{
  GListModel *items = adw_sidebar_section_get_items (section);

  g_object_unref (section);

  return G_OBJECT (items);
}

static AdwSidebarItems *
adw_sidebar_items_new (AdwSidebar *sidebar)
{
  AdwSidebarItems *items;
  GListModel *sections;

  items = g_object_new (ADW_TYPE_SIDEBAR_ITEMS, NULL);
  g_set_weak_pointer (&items->sidebar, sidebar);

  sections = adw_sidebar_get_sections (sidebar);

  items->map_model =
    gtk_map_list_model_new (sections,
                            (GtkMapListModelMapFunc) get_section_items,
                            sidebar, NULL);

  items->flatten_model = gtk_flatten_list_model_new (G_LIST_MODEL (items->map_model));

  g_signal_connect_swapped (items->flatten_model, "items-changed",
                            G_CALLBACK (g_list_model_items_changed), items);

  return items;
}

static GtkWidget *
find_page_row (AdwSidebar     *self,
               AdwSidebarItem *item)
{
  AdwSidebarSection *section = adw_sidebar_item_get_section (item);
  AdwPreferencesGroup *group = NULL;
  GtkFilter *filter = adw_sidebar_get_filter (self);
  GtkFilterMatch strictness = GTK_FILTER_MATCH_ALL;
  GtkWidget *row;
  guint i = 0;

  while (TRUE) {
    AdwSidebarSection *s;

    group = adw_preferences_page_get_group (ADW_PREFERENCES_PAGE (self->page), i++);
    if (!group)
      break;

    s = g_object_get_data (G_OBJECT (group), "-adw-sidebar-section");

    if (s == section)
      break;
  };

  if (!group)
    return NULL;

  if (filter)
    strictness = gtk_filter_get_strictness (filter);

  switch (strictness) {
  case GTK_FILTER_MATCH_SOME:
    /* Continue and just do a linear search */
    break;
  case GTK_FILTER_MATCH_NONE:
    return NULL;
    break;
  case GTK_FILTER_MATCH_ALL:
    guint index = adw_sidebar_item_get_section_index (item);

    return adw_preferences_group_get_row (group, index);
  default:
    g_assert_not_reached ();
  }

  i = 0;

  while (TRUE) {
    row = adw_preferences_group_get_row (group, i++);
    AdwSidebarItem *item2;

    if (!row)
      break;

    item2 = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item");

    if (item2 == item)
      break;
  }

  return row;
}

static GtkWidget *
find_list_row (AdwSidebar     *self,
               AdwSidebarItem *item)
{
  GtkFilter *filter = adw_sidebar_get_filter (self);
  GtkFilterMatch strictness = GTK_FILTER_MATCH_ALL;
  guint i = 0;

  if (filter)
    strictness = gtk_filter_get_strictness (filter);

  switch (strictness) {
  case GTK_FILTER_MATCH_SOME:
    /* Continue and just do a linear search */
    break;
  case GTK_FILTER_MATCH_NONE:
    return NULL;
    break;
  case GTK_FILTER_MATCH_ALL:
    guint index = adw_sidebar_item_get_index (item);
    GtkListBoxRow *row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->listbox), index);

    return GTK_WIDGET (row);
  default:
    g_assert_not_reached ();
  }

  while (TRUE) {
    GtkListBoxRow *row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->listbox), i++);
    AdwSidebarItem *item2;

    if (!row)
      break;

    item2 = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item");

    if (item2 == item)
      return GTK_WIDGET (row);
  }

  return NULL;
}

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

static gboolean
string_is_not_empty (GBinding     *binding,
                     const GValue *from_value,
                     GValue       *to_value,
                     gpointer      user_data)
{
  const char *str = g_value_get_string (from_value);

  g_value_set_boolean (to_value, str && *str);

  return TRUE;
}

static void
notify_suffix_cb (AdwSidebarItem *item,
                  GParamSpec     *pspec,
                  GtkListBoxRow  *row)
{
  GtkWidget *old_suffix = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item-suffix");
  GtkBox *box = g_object_get_data (G_OBJECT (row), "-adw-sidebar-box");
  GtkWidget *suffix;

  if (old_suffix)
    gtk_box_remove (box, old_suffix);

  suffix = adw_sidebar_item_get_suffix (item);

  if (suffix)
    gtk_box_append (box, suffix);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-item-suffix", suffix);
}

static GtkWidget *
create_row (AdwSidebarItem *item)
{
  GtkWidget *row, *box, *icon, *title_box, *title, *subtitle;

  row = gtk_list_box_row_new ();

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-item", item);

  g_object_bind_property (item, "visible", row, "visible", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "enabled", row, "sensitive", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "tooltip", row, "tooltip-markup", G_BINDING_SYNC_CREATE);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), box);

  icon = g_object_new (GTK_TYPE_IMAGE,
                       "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                       NULL);
  gtk_widget_add_css_class (icon, "icon");
  g_signal_connect_object (item, "notify::icon-name",
                           G_CALLBACK (notify_icon_cb), icon, 0);
  g_signal_connect_object (item, "notify::icon-paintable",
                           G_CALLBACK (notify_icon_cb), icon, 0);
  notify_icon_cb (item, NULL, icon);
  gtk_box_append (GTK_BOX (box), icon);

  title_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_hexpand (title_box, TRUE);
  gtk_widget_set_valign (title_box, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), title_box);

  title = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (title), PANGO_ELLIPSIZE_END);
  gtk_label_set_xalign (GTK_LABEL (title), 0.0);
  gtk_widget_add_css_class (title, "title");
  g_object_bind_property (item, "title", title, "label", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "use-underline", title, "use-underline", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (item, "title", title, "visible", G_BINDING_SYNC_CREATE,
                               string_is_not_empty, NULL, NULL, NULL);
  gtk_box_append (GTK_BOX (title_box), title);

  subtitle = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (subtitle), PANGO_ELLIPSIZE_END);
  gtk_label_set_xalign (GTK_LABEL (subtitle), 0.0);
  gtk_widget_add_css_class (subtitle, "subtitle");
  g_object_bind_property (item, "subtitle", subtitle, "label", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (item, "subtitle", subtitle, "visible", G_BINDING_SYNC_CREATE,
                               string_is_not_empty, NULL, NULL, NULL);
  gtk_box_append (GTK_BOX (title_box), subtitle);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-box", box);

  g_signal_connect_object (item, "notify::suffix",
                           G_CALLBACK (notify_suffix_cb), row, 0);
  notify_suffix_cb (item, NULL, GTK_LIST_BOX_ROW (row));

  return row;
}

static gboolean
get_header_stack_page (GBinding     *binding,
                       const GValue *from_value,
                       GValue       *to_value,
                       gpointer      user_data)
{
  const char *str = g_value_get_string (from_value);

  if (str && *str)
    g_value_set_string (to_value, "title");
  else
    g_value_set_string (to_value, "separator");

  return TRUE;
}

static GtkWidget *
create_header (AdwSidebarSection *section,
               gboolean           first_section)
{
  GtkWidget *stack, *title, *separator;

  stack = gtk_stack_new ();
  gtk_widget_add_css_class (stack, "header");
  gtk_stack_set_hhomogeneous (GTK_STACK (stack), FALSE);
  gtk_stack_set_vhomogeneous (GTK_STACK (stack), FALSE);

  title = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (title), PANGO_ELLIPSIZE_END);
  gtk_label_set_xalign (GTK_LABEL (title), 0.0);
  gtk_widget_add_css_class (title, "heading");
  gtk_stack_add_named (GTK_STACK (stack), title, "title");

  separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_stack_add_named (GTK_STACK (stack), separator, "separator");

  g_object_bind_property (section, "title", title, "label", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (section, "title", stack, "visible-child-name",
                               G_BINDING_SYNC_CREATE,
                               get_header_stack_page, NULL, NULL, NULL);

  if (first_section) {
    g_object_bind_property_full (section, "title", stack, "visible",
                                 G_BINDING_SYNC_CREATE,
                                 string_is_not_empty, NULL, NULL, NULL);

    gtk_widget_add_css_class (stack, "first");
  }

  return stack;
}

static void
set_header_cb (GtkListBoxRow *row,
               GtkListBoxRow *before,
               AdwSidebar    *self)
{
  AdwSidebarSection *section, *prev_section;
  AdwSidebarItem *item;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));
  section = adw_sidebar_item_get_section (item);

  g_assert (section != NULL);

  if (before) {
    AdwSidebarItem *prev_item =
      ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (before), "-adw-sidebar-item"));

    prev_section = adw_sidebar_item_get_section (prev_item);
  } else {
    prev_section = NULL;
  }

  if (prev_section == section) {
    gtk_list_box_row_set_header (row, NULL);
    return;
  }

  gtk_list_box_row_set_header (row, create_header (section, !before));
}

static void
update_list_selection (AdwSidebar *self)
{
  GtkWidget *row = NULL;

  if (!self->listbox)
    return;

  if (self->selected != GTK_INVALID_LIST_POSITION) {
    AdwSidebarItem *selected_item = adw_sidebar_get_selected_item (self);
    row = find_list_row (self, selected_item);
  }

  if (row)
    gtk_list_box_select_row (GTK_LIST_BOX (self->listbox), GTK_LIST_BOX_ROW (row));
  else
    gtk_list_box_unselect_all (GTK_LIST_BOX (self->listbox));
}

static void
row_selected_cb (AdwSidebar    *self,
                 GtkListBoxRow *row)
{
  AdwSidebarItem *item;
  guint index;

  if (!row || self->block_row_selected)
    return;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));
  index = adw_sidebar_item_get_index (item);

  adw_sidebar_set_selected (self, index);
}

static void
row_activated_cb (AdwSidebar    *self,
                  GtkListBoxRow *row)
{
  AdwSidebarItem *item;
  guint index;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));
  index = adw_sidebar_item_get_index (item);

  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0, index);
}

static void
boxed_notify_suffix_cb (AdwSidebarItem *item,
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

static void
boxed_row_activated_cb (AdwSidebar   *self,
                        AdwActionRow *row)
{
  AdwSidebarItem *item;
  guint index;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));
  index = adw_sidebar_item_get_index (item);

  adw_sidebar_set_selected (self, index);
  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0, index);
}

static GtkWidget *
create_boxed_row (AdwSidebarItem *item,
                  AdwSidebar     *self)
{
  GtkWidget *row, *icon, *arrow;

  row = adw_action_row_new ();

  adw_preferences_row_set_use_markup (ADW_PREFERENCES_ROW (row), FALSE);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-item", item);

  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (row), TRUE);

  g_object_bind_property (item, "visible", row, "visible", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "enabled", row, "sensitive", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "title", row, "title", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "subtitle", row, "subtitle", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "use-underline", row, "use-underline", G_BINDING_SYNC_CREATE);
  g_object_bind_property (item, "tooltip", row, "tooltip-markup", G_BINDING_SYNC_CREATE);

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
                           G_CALLBACK (boxed_notify_suffix_cb), row, 0);
  boxed_notify_suffix_cb (item, NULL, ADW_ACTION_ROW (row));

  arrow = g_object_new (GTK_TYPE_IMAGE,
                        "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                        "icon-name", "go-next-symbolic",
                        NULL);
  gtk_widget_add_css_class (arrow, "arrow");
  adw_action_row_add_suffix (ADW_ACTION_ROW (row), arrow);

  g_object_set_data (G_OBJECT (row), "-adw-sidebar-arrow", arrow);

  g_signal_connect_swapped (row, "activated", G_CALLBACK (boxed_row_activated_cb), self);

  return row;
}

static gboolean
escape_markup (GBinding     *binding,
               const GValue *from_value,
               GValue       *to_value,
               gpointer      user_data)
{
  const char *str = g_value_get_string (from_value);

  if (str)
    g_value_take_string (to_value, g_markup_escape_text (str, -1));
  else
    g_value_take_string (to_value, NULL);

  return TRUE;
}

static gboolean
has_items (GBinding     *binding,
           const GValue *from_value,
           GValue       *to_value,
           gpointer      user_data)
{
  uint n_items = g_value_get_uint (from_value);

  g_value_set_boolean (to_value, n_items > 0);

  return TRUE;
}

static GtkWidget *
create_section (AdwSidebar        *self,
                AdwSidebarSection *section)
{
  GtkWidget *group = adw_preferences_group_new ();
  GListModel *items = adw_sidebar_section_get_items (section);
  GtkFilterListModel *filtered = gtk_filter_list_model_new (items, NULL);

  g_object_bind_property (self, "filter", filtered, "filter", G_BINDING_SYNC_CREATE);
  g_object_bind_property_full (section, "title", group, "title", G_BINDING_SYNC_CREATE,
                               escape_markup, NULL, NULL, NULL);
  g_object_bind_property_full (filtered, "n-items", group, "visible", G_BINDING_SYNC_CREATE,
                               has_items, NULL, NULL, NULL);

  g_object_set_data (G_OBJECT (group), "-adw-sidebar-section", section);

  adw_preferences_group_bind_model (ADW_PREFERENCES_GROUP (group),
                                    G_LIST_MODEL (filtered),
                                    (GtkListBoxCreateWidgetFunc) create_boxed_row,
                                    self, NULL);

  g_object_unref (filtered);

  return group;
}

static void
sections_changed_cb (AdwPreferencesPage *page,
                     guint               index,
                     guint               removed,
                     guint               added,
                     GListModel         *sections)
{
  AdwSidebar *self = g_object_get_data (G_OBJECT (page), "-adw-sidebar");
  guint i;

  for (i = 0; i < removed; i++) {
    AdwPreferencesGroup *group = adw_preferences_page_get_group (page, index);

    adw_preferences_page_remove (page, group);
  }

  for (i = 0; i < added; i++) {
    AdwSidebarSection *section = g_list_model_get_item (sections, index + i);
    GtkWidget *group = create_section (self, section);

    adw_preferences_page_insert (page,
                                 ADW_PREFERENCES_GROUP (group),
                                 index + added);

    g_object_unref (section);
  }
}

static void
update_placeholder (AdwSidebar *self)
{
  guint n_items = g_list_model_get_n_items (G_LIST_MODEL (self->filtered_items));

  if (self->page)
    gtk_widget_set_child_visible (self->page, n_items > 0 || self->placeholder == NULL);

  if (self->swindow)
    gtk_widget_set_child_visible (self->swindow, n_items > 0 || self->placeholder == NULL);

  if (self->placeholder)
    gtk_widget_set_child_visible (self->placeholder, n_items == 0);

  if (n_items == 0)
    gtk_widget_add_css_class (GTK_WIDGET (self), "empty");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "empty");
}

static gboolean
scroll_to (GtkWidget *viewport,
           GtkWidget *row)
{
  graphene_rect_t bounds;
  GtkAdjustment *vadj;
  double row_center, page_size;

  if (!gtk_widget_compute_bounds (row, viewport, &bounds))
    return FALSE;

  row_center = bounds.origin.y + bounds.size.height / 2.0;

  vadj = gtk_scrollable_get_vadjustment (GTK_SCROLLABLE (viewport));

  page_size = gtk_adjustment_get_page_size (vadj);
  if (page_size < 0.5)
    return FALSE;

  gtk_adjustment_set_value (vadj, row_center - page_size / 2.0);

  return TRUE;
}

static gboolean
list_mapped_idle_cb (AdwSidebar *self)
{
  AdwSidebarItem *selected;
  GtkWidget *row, *viewport;

  g_assert (self->swindow);
  g_assert (self->listbox);

  selected = adw_sidebar_get_selected_item (self);
  if (!selected) {
    self->restore_scroll_idle_id = 0;
    return G_SOURCE_REMOVE;
  }

  row = find_list_row (self, selected);
  if (!row) {
    self->restore_scroll_idle_id = 0;
    return G_SOURCE_REMOVE;
  }

  viewport = gtk_scrolled_window_get_child (GTK_SCROLLED_WINDOW (self->swindow));

  if (scroll_to (viewport, GTK_WIDGET (row))) {
    self->restore_scroll_idle_id = 0;
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
list_mapped_cb (AdwSidebar *self)
{
  g_assert (self->swindow);

  g_signal_handlers_disconnect_by_func (self->listbox, list_mapped_cb, self);

  g_clear_handle_id (&self->restore_scroll_idle_id, g_source_remove);

  if (list_mapped_idle_cb (self) == G_SOURCE_REMOVE)
    return;

  /* Sometimes we're already mapped, but not able to scroll yet, so try until it succeeds */
  self->restore_scroll_idle_id = g_idle_add (G_SOURCE_FUNC (list_mapped_idle_cb), self);
}

static gboolean
page_mapped_idle_cb (AdwSidebar *self)
{
  AdwSidebarItem *selected;
  GtkWidget *row, *viewport;

  g_assert (self->page);

  selected = adw_sidebar_get_selected_item (self);
  if (!selected) {
    self->restore_scroll_idle_id = 0;
    return G_SOURCE_REMOVE;
  }

  row = find_page_row (self, selected);
  if (!row) {
    self->restore_scroll_idle_id = 0;
    return G_SOURCE_REMOVE;
  }

  viewport = adw_preferences_page_get_viewport (ADW_PREFERENCES_PAGE (self->page));

  if (scroll_to (viewport, GTK_WIDGET (row))) {
    self->restore_scroll_idle_id = 0;
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
page_mapped_cb (AdwSidebar *self)
{
  g_assert (self->page);

  g_signal_handlers_disconnect_by_func (self->page, page_mapped_cb, self);

  g_clear_handle_id (&self->restore_scroll_idle_id, g_source_remove);

  if (page_mapped_idle_cb (self) == G_SOURCE_REMOVE)
    return;

  /* Sometimes we're already mapped, but not able to scroll yet, so try until it succeeds */
  self->restore_scroll_idle_id = g_idle_add (G_SOURCE_FUNC (page_mapped_idle_cb), self);
}

static void
recreate_ui (AdwSidebar *self)
{
  g_clear_handle_id (&self->restore_scroll_idle_id, g_source_remove);

  if (self->page) {
    AdwPreferencesGroup *group;
    guint index = 0;

    while ((group = adw_preferences_page_get_group (ADW_PREFERENCES_PAGE (self->page), index++)) != NULL) {
      GtkWidget *row;
      guint row_index = 0;

      while ((row = adw_preferences_group_get_row (group, row_index++)) != NULL) {
        GtkWidget *suffix = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item-suffix");

        if (suffix) {
          adw_action_row_remove (ADW_ACTION_ROW (row), suffix);
          g_object_set_data (G_OBJECT (row), "-adw-sidebar-item-suffix", NULL);
        }
      }
    }

    g_clear_pointer (&self->page, gtk_widget_unparent);
  }

  if (self->swindow) {
    GtkListBoxRow *row;
    int index = 0;

    while ((row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->listbox), index++)) != NULL) {
      GtkWidget *suffix = g_object_get_data (G_OBJECT (row), "-adw-sidebar-item-suffix");

      if (suffix) {
        GtkBox *box = g_object_get_data (G_OBJECT (row), "-adw-sidebar-box");

        gtk_box_remove (box, suffix);
        g_object_set_data (G_OBJECT (row), "-adw-sidebar-item-suffix", NULL);
      }
    }

    g_clear_pointer (&self->swindow, gtk_widget_unparent);
    self->listbox = NULL;
  }

  if (self->mode == ADW_SIDEBAR_MODE_SIDEBAR) {
    self->swindow = gtk_scrolled_window_new ();
    gtk_scrolled_window_set_propagate_natural_height (GTK_SCROLLED_WINDOW (self->swindow), TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self->swindow),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    self->listbox = gtk_list_box_new ();
    gtk_widget_add_css_class (self->listbox, "navigation-sidebar");
    gtk_list_box_set_selection_mode (GTK_LIST_BOX (self->listbox), GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (self->swindow), self->listbox);

    gtk_list_box_set_header_func (GTK_LIST_BOX (self->listbox),
                                  (GtkListBoxUpdateHeaderFunc) set_header_cb,
                                  self, NULL);
    gtk_list_box_bind_model (GTK_LIST_BOX (self->listbox),
                             G_LIST_MODEL (self->filtered_items),
                             (GtkListBoxCreateWidgetFunc) create_row,
                             NULL, NULL);

    g_signal_connect_object (self->filtered_items, "sections-changed",
                             G_CALLBACK (gtk_list_box_invalidate_headers),
                             self->listbox, G_CONNECT_SWAPPED);

    update_list_selection (self);

    g_signal_connect_swapped (self->listbox, "row-selected",
                              G_CALLBACK (row_selected_cb), self);
    g_signal_connect_swapped (self->listbox, "row-activated",
                              G_CALLBACK (row_activated_cb), self);

    gtk_widget_set_parent (self->swindow, GTK_WIDGET (self));

    g_signal_connect_swapped (self->listbox, "map",
                              G_CALLBACK (list_mapped_cb), self);
  } else {
    self->page = adw_preferences_page_new ();

    g_object_set_data (G_OBJECT (self->page), "-adw-sidebar", self);

    sections_changed_cb (ADW_PREFERENCES_PAGE (self->page),
                         0, 0, g_list_model_get_n_items (self->sections_model),
                         self->sections_model);

    g_signal_connect_object (self->sections_model, "items-changed",
                             G_CALLBACK (sections_changed_cb), self->page,
                             G_CONNECT_SWAPPED);

    gtk_widget_set_parent (self->page, GTK_WIDGET (self));

    g_signal_connect_swapped (self->page, "map",
                              G_CALLBACK (page_mapped_cb), self);
  }

  update_placeholder (self);
}

static void
items_changed_cb (AdwSidebar *self,
                  guint       index,
                  guint       removed,
                  guint       added)
{
  guint selected = self->selected;
  guint old_n_items = self->n_items;
  guint i, current = 0;

  self->n_items = g_list_model_get_n_items (self->items_model);

  if (self->in_dispose)
    return;

  /* Update first index for each section */
  for (i = 0; i < self->sections->len; i++) {
    AdwSidebarSection *section = g_ptr_array_index (self->sections, i);

    adw_sidebar_section_set_first_index (section, current);

    current += adw_sidebar_section_get_n_items (section);
  }

  /* Select the first item when adding them */
  if (old_n_items == 0 && self->n_items > 0) {
    adw_sidebar_set_selected (self, 0);
    return;
  }

  if (index <= selected && index + removed > selected) {
    adw_sidebar_set_selected (self, GTK_INVALID_LIST_POSITION);
    return;
  }

  if (index <= selected) {
    adw_sidebar_set_selected (self, selected + added - removed);
    return;
  }
}

static void
adw_sidebar_dispose (GObject *object)
{
  AdwSidebar *self = ADW_SIDEBAR (object);

  self->in_dispose = TRUE;

  g_clear_handle_id (&self->restore_scroll_idle_id, g_source_remove);

  g_clear_pointer (&self->swindow, gtk_widget_unparent);
  g_clear_pointer (&self->page, gtk_widget_unparent);
  g_clear_pointer (&self->placeholder, gtk_widget_unparent);

  if (self->sections_model) {
    guint n = g_list_model_get_n_items (self->sections_model);

    g_list_model_items_changed (G_LIST_MODEL (self->sections_model), 0, n, 0);
  }

  if (self->items_model) {
    guint n = g_list_model_get_n_items (self->items_model);

    g_list_model_items_changed (G_LIST_MODEL (self->items_model), 0, n, 0);
  }

  g_clear_pointer (&self->sections, g_ptr_array_unref);
  g_clear_object (&self->sections_model);
  g_clear_object (&self->filtered_items);

  self->items_model = NULL;
  self->listbox = NULL;

  G_OBJECT_CLASS (adw_sidebar_parent_class)->dispose (object);
}

static void
adw_sidebar_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  AdwSidebar *self = ADW_SIDEBAR (object);

  switch (prop_id) {
  case PROP_MODE:
    g_value_set_enum (value, adw_sidebar_get_mode (self));
    break;
  case PROP_SELECTED:
    g_value_set_uint (value, adw_sidebar_get_selected (self));
    break;
  case PROP_SELECTED_ITEM:
    g_value_set_object (value, adw_sidebar_get_selected_item (self));
    break;
  case PROP_FILTER:
    g_value_set_object (value, adw_sidebar_get_filter (self));
    break;
  case PROP_PLACEHOLDER:
    g_value_set_object (value, adw_sidebar_get_placeholder (self));
    break;
  case PROP_ITEMS:
    g_value_take_object (value, adw_sidebar_get_items (self));
    break;
  case PROP_SECTIONS:
    g_value_take_object (value, adw_sidebar_get_sections (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sidebar_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AdwSidebar *self = ADW_SIDEBAR (object);

  switch (prop_id) {
  case PROP_MODE:
    adw_sidebar_set_mode (self, g_value_get_enum (value));
    break;
  case PROP_SELECTED:
    adw_sidebar_set_selected (self, g_value_get_uint (value));
    break;
  case PROP_FILTER:
    adw_sidebar_set_filter (self, g_value_get_object (value));
    break;
  case PROP_PLACEHOLDER:
    adw_sidebar_set_placeholder (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sidebar_class_init (AdwSidebarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_sidebar_dispose;
  object_class->get_property = adw_sidebar_get_property;
  object_class->set_property = adw_sidebar_set_property;

  widget_class->compute_expand = adw_widget_compute_expand;
  widget_class->focus = adw_widget_focus_child;

  /**
   * AdwSidebar:mode:
   *
   * Determines the sidebar's look and behavior.
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
   * Since: 1.9
   */
  props[PROP_MODE] =
    g_param_spec_enum ("mode", NULL, NULL,
                       ADW_TYPE_SIDEBAR_MODE,
                       ADW_SIDEBAR_MODE_SIDEBAR,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebar:selected:
   *
   * The index of the currently selected item.
   *
   * If set to [const@Gtk.INVALID_LIST_POSITION], no item is selected.
   *
   * If [property@Sidebar:mode] is set to [enum@Adw.SidebarMode.PAGE], the
   * selection is invisible, but still tracked, indicating which item will be
   * selected once the mode is changed to [enum@Adw.SidebarMode.SIDEBAR].
   *
   * See also: [property@Sidebar:selected-item].
   *
   * Since: 1.9
   */
  props[PROP_SELECTED] =
    g_param_spec_uint ("selected", NULL, NULL,
                       0, G_MAXUINT, GTK_INVALID_LIST_POSITION,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebar:selected-item:
   *
   * The currently selected item.
   *
   * This is a convenience property, equivalent to calling
   * [method@Sidebar.get_item] with [property@Sidebar:selected] provided as the
   * index.
   *
   * To change selection, use [property@Sidebar:selected].
   *
   * Since: 1.9
   */
  props[PROP_SELECTED_ITEM] =
    g_param_spec_object ("selected-item", NULL, NULL,
                         ADW_TYPE_SIDEBAR_ITEM,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwSidebar:filter:
   *
   * The item filter.
   *
   * Can be used to implement search within the sidebar.
   *
   * Use [property@Sidebar:placeholder] to provide an empty state.
   *
   * Since: 1.9
   */
  props[PROP_FILTER] =
    g_param_spec_object ("filter", NULL, NULL,
                         GTK_TYPE_FILTER,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebar:placeholder:
   *
   * The placeholder widget.
   *
   * This widget will be shown if the sidebar has no items, or all of its items
   * have been filtered out by [property@Sidebar:filter].
   *
   * Since: 1.9
   */
  props[PROP_PLACEHOLDER] =
    g_param_spec_object ("placeholder", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebar:items:
   *
   * A list model with the sidebar's items.
   *
   * This can be used to keep an up-to-date view.
   *
   * The model implements [iface@Gtk.SectionModel] and creates sections
   * corresponding to the sidebar's sections.
   *
   * The model also implements [iface@Gtk.SelectionModel] and can be used to
   * track and change the selection.
   *
   * To only track sections, use [property@Sidebar:sections] instead.
   *
   * Since: 1.9
   */
  props[PROP_ITEMS] =
    g_param_spec_object ("items", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwSidebar:sections:
   *
   * A list model with the sidebar's sections.
   *
   * This can be used to keep an up-to-date view.
   *
   * To track items, use [property@Sidebar:items] instead.
   *
   * Since: 1.9
   */
  props[PROP_SECTIONS] =
    g_param_spec_object ("sections", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwSidebar::activated:
   * @self: a sidebar
   * @index: the item index
   *
   * Emitted when an item at @index has been activated.
   *
   * Since: 1.9
   */
  signals[SIGNAL_ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__UINT,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_UINT);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__UINTv);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "sidebar");
}

static void
adw_sidebar_init (AdwSidebar *self)
{
  self->mode = ADW_SIDEBAR_MODE_SIDEBAR;
  self->sections = g_ptr_array_new_with_free_func (g_object_unref);
  self->selected = GTK_INVALID_LIST_POSITION;
  self->sections_model = G_LIST_MODEL (adw_sidebar_sections_new (self));
  self->items_model = G_LIST_MODEL (adw_sidebar_items_new (self));

  self->filtered_items = gtk_filter_list_model_new (self->items_model, NULL);

  g_signal_connect_swapped (self->items_model, "items-changed",
                            G_CALLBACK (items_changed_cb), self);

  g_signal_connect_swapped (self->filtered_items, "items-changed",
                            G_CALLBACK (update_placeholder), self);

  recreate_ui (self);
}

static void
adw_sidebar_add_child (GtkBuildable *buildable,
                       GtkBuilder   *builder,
                       GObject      *child,
                       const char   *type)
{
  if (ADW_IS_SIDEBAR_SECTION (child)) {
    adw_sidebar_append (ADW_SIDEBAR (buildable),
                        ADW_SIDEBAR_SECTION (g_object_ref (child)));
  } else {
    g_warning ("Cannot add an object of type %s to AdwSidebar",
               g_type_name (G_OBJECT_TYPE (child)));
  }
}

static void
adw_sidebar_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adw_sidebar_add_child;
}

/**
 * adw_sidebar_new:
 *
 * Creates a new `AdwSidebar`.
 *
 * Returns: the newly created `AdwSidebar`
 *
 * Since: 1.9
 */
GtkWidget *
adw_sidebar_new (void)
{
  return g_object_new (ADW_TYPE_SIDEBAR, NULL);
}

/**
 * adw_sidebar_get_mode:
 * @self: a sidebar
 *
 * Gets @self's look and behavior.
 *
 * Returns: the current mode
 *
 * Since: 1.9
 */
AdwSidebarMode
adw_sidebar_get_mode (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), ADW_SIDEBAR_MODE_SIDEBAR);

  return self->mode;
}

/**
 * adw_sidebar_set_mode:
 * @self: a sidebar
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
 * Since: 1.9
 */
void
adw_sidebar_set_mode (AdwSidebar     *self,
                      AdwSidebarMode  mode)
{
  g_return_if_fail (ADW_IS_SIDEBAR (self));
  g_return_if_fail (mode >= ADW_SIDEBAR_MODE_SIDEBAR);
  g_return_if_fail (mode <= ADW_SIDEBAR_MODE_PAGE);

  if (mode == self->mode)
    return;

  self->mode = mode;

  recreate_ui (self);

  if (mode == ADW_SIDEBAR_MODE_PAGE)
    gtk_widget_add_css_class (GTK_WIDGET (self), "page");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "page");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODE]);
}

/**
 * adw_sidebar_get_selected:
 * @self: a sidebar
 *
 * Gets the index of the currently selected item.
 *
 * See also: [method@Sidebar.get_selected_item].
 *
 * Returns: index of the currently selected item
 *
 * Since: 1.9
 */
guint
adw_sidebar_get_selected (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), 0);

  return self->selected;
}

/**
 * adw_sidebar_set_selected:
 * @self: a sidebar
 * @selected: index of the newly selected item
 *
 * Selects the item at @selected.
 *
 * If set to [const@Gtk.INVALID_LIST_POSITION], no item is selected.
 *
 * If [property@Sidebar:mode] is set to [enum@Adw.SidebarMode.PAGE], the
 * selection is invisible, but still tracked, indicating which item will be
 * selected once the mode is changed to [enum@Adw.SidebarMode.SIDEBAR].
 *
 * See also: [property@Sidebar:selected-item].
 *
 * Since: 1.9
 */
void
adw_sidebar_set_selected (AdwSidebar *self,
                          guint       selected)
{
  guint old_selected;

  g_return_if_fail (ADW_IS_SIDEBAR (self));

  if (selected >= self->n_items)
    selected = GTK_INVALID_LIST_POSITION;

  if (selected == self->selected)
    return;

  old_selected = self->selected;

  self->selected = selected;

  self->block_row_selected++;
  update_list_selection (self);
  self->block_row_selected--;

  if (self->items_model) {
    if (old_selected == GTK_INVALID_LIST_POSITION) {
      gtk_selection_model_selection_changed (GTK_SELECTION_MODEL (self->items_model),
                                             selected, 1);
    } else if (selected == GTK_INVALID_LIST_POSITION) {
      if (old_selected < g_list_model_get_n_items (self->items_model)) {
        gtk_selection_model_selection_changed (GTK_SELECTION_MODEL (self->items_model),
                                               old_selected, 1);
      }
    } else {
      guint last_item = g_list_model_get_n_items (self->items_model) - 1;
      guint start = MIN (old_selected, selected);
      guint end = MAX (old_selected, selected);

      start = MIN (start, last_item);
      end = MIN (end, last_item);

      gtk_selection_model_selection_changed (GTK_SELECTION_MODEL (self->items_model),
                                             start, end - start + 1);

      /* FIXME: listview crashes if we bind get_items() to it, select the last
       * item and then add an item somewhere above. Not sure if it's a bug here
       * or in listview */
    }
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_ITEM]);
}

/**
 * adw_sidebar_get_selected_item:
 * @self: a sidebar
 *
 * Gets the currently selected item.
 *
 * This is a convenience method, equivalent to calling [method@Sidebar.get_item]
 * with [property@Sidebar:selected] provided as the index.
 *
 * To change selection, use [method@Sidebar.set_selected].
 *
 * Returns: (transfer none) (nullable): the selected item
 *
 * Since: 1.9
 */
AdwSidebarItem *
adw_sidebar_get_selected_item (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  return adw_sidebar_get_item (self, self->selected);
}

/**
 * adw_sidebar_get_filter:
 * @self: a sidebar
 *
 * Gets the item filter for @self.
 *
 * Returns: (transfer none) (nullable): the item filter
 *
 * Since: 1.9
 */
GtkFilter *
adw_sidebar_get_filter (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  return gtk_filter_list_model_get_filter (self->filtered_items);
}

/**
 * adw_sidebar_set_filter:
 * @self: a sidebar
 * @filter: (transfer none) (nullable): the item filter
 *
 * Sets the item filter for @self.
 *
 * Can be used to implement search within the sidebar.
 *
 * Use [property@Sidebar:placeholder] to provide an empty state.
 *
 * Since: 1.9
 */
void
adw_sidebar_set_filter (AdwSidebar *self,
                        GtkFilter  *filter)
{
  g_return_if_fail (ADW_IS_SIDEBAR (self));
  g_return_if_fail (filter == NULL || GTK_IS_FILTER (filter));

  if (filter == adw_sidebar_get_filter (self))
    return;

  gtk_filter_list_model_set_filter (self->filtered_items, filter);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FILTER]);
}

/**
 * adw_sidebar_get_placeholder:
 * @self: a sidebar
 *
 * Gets the placeholder widget for @self.
 *
 * Returns: (transfer none) (nullable): the placeholder widget
 *
 * Since: 1.9
 */
GtkWidget *
adw_sidebar_get_placeholder (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  return self->placeholder;
}

/**
 * adw_sidebar_set_placeholder:
 * @self: a sidebar
 * @placeholder: (transfer none) (nullable): the placeholder widget
 *
 * Sets the placeholder widget for @self.
 *
 * This widget will be shown if @self has no items, or all of its items have
 * been filtered out by [property@Sidebar:filter].
 *
 * Since: 1.9
 */
void
adw_sidebar_set_placeholder (AdwSidebar *self,
                             GtkWidget  *placeholder)
{
  g_return_if_fail (ADW_IS_SIDEBAR (self));
  g_return_if_fail (placeholder == NULL || GTK_IS_WIDGET (placeholder));

  if (placeholder == self->placeholder)
    return;

  if (self->placeholder) {
    gtk_widget_set_child_visible (self->placeholder, TRUE);
    gtk_widget_unparent (self->placeholder);
  }

  self->placeholder = placeholder;

  if (self->placeholder)
    gtk_widget_set_parent (self->placeholder, GTK_WIDGET (self));

  update_placeholder (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PLACEHOLDER]);
}

/**
 * adw_sidebar_get_items:
 * @self: a sidebar
 *
 * Gets a list model with @self's items.
 *
 * This can be used to keep an up-to-date view.
 *
 * The model implements [iface@Gtk.SectionModel] and creates sections
 * corresponding to the sidebar's sections.
 *
 * The model also implements [iface@Gtk.SelectionModel] and can be used to
 * track and change the selection.
 *
 * To only track sections, use [property@Sidebar:sections] instead.
 *
 * Returns: (transfer full): a model containing the items
 *
 * Since: 1.9
 */
GtkSelectionModel *
adw_sidebar_get_items (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  return GTK_SELECTION_MODEL (g_object_ref (self->items_model));
}

/**
 * adw_sidebar_get_sections:
 * @self: a sidebar
 *
 * Gets a list model with @self's sections.
 *
 * This can be used to keep an up-to-date view.
 *
 * To track items, use [property@Sidebar:items] instead.
 *
 * Returns: (transfer full): a model containing the sections
 *
 * Since: 1.9
 */
GListModel *
adw_sidebar_get_sections (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  return g_object_ref (self->sections_model);
}

/**
 * adw_sidebar_get_item:
 * @self: a sidebar
 * @index: index of the item
 *
 * Gets the item at @index within @self.
 *
 * The index starts from 0 at the top of the sidebar, and is same as the one
 * returned by [method@SidebarItem.get_index].
 *
 * Can return `NULL` if @index is larger or equal to the number of items.
 *
 * Returns: (transfer none) (nullable): the item at @index
 *
 * Since: 1.9
 */
AdwSidebarItem *
adw_sidebar_get_item (AdwSidebar *self,
                      guint       index)
{
  AdwSidebarItem *ret;

  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  ret = g_list_model_get_item (self->items_model, index);

  if (ret)
    g_object_unref (ret);

  return ret;
}

/**
 * adw_sidebar_get_section:
 * @self: a sidebar
 * @index: index of the section
 *
 * Gets the section at @index within @self.
 *
 * Can return `NULL` if @index is larger or equal to the number of sections.
 *
 * Returns: (transfer none) (nullable): the section at @index
 *
 * Since: 1.9
 */
AdwSidebarSection *
adw_sidebar_get_section (AdwSidebar *self,
                         guint       index)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  if (index >= self->sections->len)
    return NULL;

  return g_ptr_array_index (self->sections, index);
}

/**
 * adw_sidebar_append:
 * @self: a sidebar
 * @section: (transfer full): a section to append
 *
 * Appends @section to @self.
 *
 * Since: 1.9
 */
void
adw_sidebar_append (AdwSidebar        *self,
                    AdwSidebarSection *section)
{
  adw_sidebar_insert (self, section, -1);
}

/**
 * adw_sidebar_prepend:
 * @self: a sidebar
 * @section: (transfer full): a section to prepend
 *
 * Prepends @section to @self.
 *
 * Since: 1.9
 */
void
adw_sidebar_prepend (AdwSidebar        *self,
                     AdwSidebarSection *section)
{
  adw_sidebar_insert (self, section, 0);
}

/**
 * adw_sidebar_insert:
 * @self: a sidebar
 * @section: (transfer full): a section to insert
 * @position: position to insert @section at
 *
 * Inserts @section at @position to @self.
 *
 * If @position is -1, or larger than the total number of sections in @self,
 * the section will be appended to the end.
 *
 * Since: 1.9
 */
void
adw_sidebar_insert (AdwSidebar        *self,
                    AdwSidebarSection *section,
                    int                position)
{
  g_return_if_fail (ADW_IS_SIDEBAR (self));
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (section));

  adw_sidebar_section_set_sidebar (section, self);

  if (position < 0 || position >= self->sections->len) {
    g_ptr_array_add (self->sections, section);
    g_list_model_items_changed (self->sections_model, self->sections->len - 1, 0, 1);
  } else {
    g_ptr_array_insert (self->sections, position, section);
    g_list_model_items_changed (self->sections_model, position, 0, 1);
  }
}

/**
 * adw_sidebar_remove:
 * @self: a sidebar
 * @section: a section to remove
 *
 * Removes @section from @self.
 *
 * Since: 1.9
 */
void
adw_sidebar_remove (AdwSidebar        *self,
                    AdwSidebarSection *section)
{
  guint index;

  g_return_if_fail (ADW_IS_SIDEBAR (self));
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (section));

  if (!g_ptr_array_find (self->sections, section, &index)) {
    g_critical ("Attempting to remove %s %p, but it's not in %s %p",
                G_OBJECT_TYPE_NAME (section), section,
                G_OBJECT_TYPE_NAME (self), self);

    return;
  }

  g_object_ref (section);

  g_ptr_array_remove_index (self->sections, index);

  g_list_model_items_changed (self->sections_model, index, 1, 0);

  adw_sidebar_section_set_sidebar (section, NULL);
  adw_sidebar_section_set_first_index (section, 0);

  g_object_unref (section);
}

/**
 * adw_sidebar_remove_all:
 * @self: a sidebar
 *
 * Removes all sections from @self.
 *
 * Since: 1.9
 */
void
adw_sidebar_remove_all (AdwSidebar *self)
{
  GPtrArray *old_sections;
  guint i, len;

  g_return_if_fail (ADW_IS_SIDEBAR (self));

  len = self->sections->len;

  old_sections = self->sections;
  self->sections = g_ptr_array_new_with_free_func (g_object_unref);

  g_list_model_items_changed (self->sections_model, 0, len, 0);

  for (i = 0; i < len; i++) {
    AdwSidebarSection *section = g_ptr_array_index (old_sections, i);

    adw_sidebar_section_set_sidebar (section, NULL);
    adw_sidebar_section_set_first_index (section, 0);
  }

  g_ptr_array_unref (old_sections);
}
