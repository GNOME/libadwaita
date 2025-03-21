/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-sidebar.h"

#include "adw-marshalers.h"
#include "adw-sidebar-item.h"
#include "adw-sidebar-page-private.h"
#include "adw-sidebar-section-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwSidebarMode:
 * @ADW_SIDEBAR_MODE_SIDEBAR: TODO
 * @ADW_SIDEBAR_MODE_PAGE: TODO
 *
 * TODO
 */

/**
 * AdwSidebar:
 *
 * TODO
 *
 * Since: 1.8
 */

struct _AdwSidebar
{
  GtkWidget parent_instance;

  AdwSidebarMode mode;

  GPtrArray *sections;
  GListModel *sections_model;

  GListModel *items_model;

  GtkWidget *swindow;
  GtkWidget *listbox;
  GtkWidget *page;

  guint selected;
  guint n_items;
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

  g_object_bind_property (item, "enabled", row, "sensitive", G_BINDING_SYNC_CREATE);

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
  }

  return stack;
}

static void
set_header_cb (GtkListBoxRow *row,
               GtkListBoxRow *before,
               AdwSidebar    *self)
{
  guint index, section_start;
  AdwSidebarItem *item;
  AdwSidebarSection *section;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));
  index = adw_sidebar_item_get_index (item);

  section = adw_sidebar_item_get_section (item);
  section_start = adw_sidebar_section_get_first_index (section);

  if (section_start != index) {
    gtk_list_box_row_set_header (row, NULL);
    return;
  }

  gtk_list_box_row_set_header (row, create_header (section, section_start == 0));
}

static void
update_list_selection (AdwSidebar *self)
{
  GtkListBoxRow *row = NULL;

  if (!self->listbox)
    return;

  if (self->selected != GTK_INVALID_LIST_POSITION)
    row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->listbox), self->selected);

  if (row)
    gtk_list_box_select_row (GTK_LIST_BOX (self->listbox), row);
  else
    gtk_list_box_unselect_all (GTK_LIST_BOX (self->listbox));
}

static void
row_selected_cb (AdwSidebar    *self,
                 GtkListBoxRow *row)
{
  AdwSidebarItem *item;
  guint index;

  if (!row)
    return;

  item = ADW_SIDEBAR_ITEM (g_object_get_data (G_OBJECT (row), "-adw-sidebar-item"));
  index = adw_sidebar_item_get_index (item);

  adw_sidebar_set_selected (self, index);
}

static void
row_activated_cb (AdwSidebar    *self,
                  GtkListBoxRow *row)
{
  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0);
}

static void
page_activated_cb (AdwSidebar     *self,
                   AdwSidebarItem *item)
{
  guint index = adw_sidebar_item_get_index (item);

  adw_sidebar_set_selected (self, index);

  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0);
}

static void
recreate_ui (AdwSidebar *self)
{
  if (self->page) {
    adw_sidebar_page_unparent_children (ADW_SIDEBAR_PAGE (self->page));
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
    gtk_list_box_bind_model (GTK_LIST_BOX (self->listbox), self->items_model,
                             (GtkListBoxCreateWidgetFunc) create_row,
                             NULL, NULL);

    g_signal_connect_object (self->items_model, "sections-changed",
                             G_CALLBACK (gtk_list_box_invalidate_headers),
                             self->listbox, G_CONNECT_SWAPPED);

    update_list_selection (self);

    g_signal_connect_swapped (self->listbox, "row-selected",
                             G_CALLBACK (row_selected_cb), self);
    g_signal_connect_swapped (self->listbox, "row-activated",
                             G_CALLBACK (row_activated_cb), self);

    gtk_widget_set_parent (self->swindow, GTK_WIDGET (self));
  } else {
    self->page = adw_sidebar_page_new (self);

    g_signal_connect_swapped (self->page, "activated",
                              G_CALLBACK (page_activated_cb), self);

    gtk_widget_set_parent (self->page, GTK_WIDGET (self));
  }
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

  /* Update first index for each section */
  for (i = 0; i < self->sections->len; i++) {
    AdwSidebarSection *section = g_ptr_array_index (self->sections, i);
    guint section_start, section_end;

    gtk_section_model_get_section (GTK_SECTION_MODEL (self->items_model),
                                   current, &section_start, &section_end);

    adw_sidebar_section_set_first_index (section, section_start);

    current = section_end;
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

  if (index < selected) {
    adw_sidebar_set_selected (self, selected + added - removed);
    return;
  }
}

static void
adw_sidebar_dispose (GObject *object)
{
  AdwSidebar *self = ADW_SIDEBAR (object);

  if (self->sections_model) {
    guint n = g_list_model_get_n_items (self->sections_model);

    g_list_model_items_changed (G_LIST_MODEL (self->sections_model), 0, n, 0);
  }

  if (self->items_model) {
    guint n = g_list_model_get_n_items (self->items_model);

    g_list_model_items_changed (G_LIST_MODEL (self->items_model), 0, n, 0);
  }

  g_clear_pointer (&self->sections, g_ptr_array_unref);
  g_clear_pointer (&self->swindow, gtk_widget_unparent);
  g_clear_pointer (&self->page, gtk_widget_unparent);
  g_clear_object (&self->items_model);

  self->listbox = NULL;

  G_OBJECT_CLASS (adw_sidebar_parent_class)->dispose (object);
}

static void
adw_sidebar_finalize (GObject *object)
{
  AdwSidebar *self = ADW_SIDEBAR (object);

  g_clear_weak_pointer (&self->sections_model);

  G_OBJECT_CLASS (adw_sidebar_parent_class)->finalize (object);
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
  case PROP_ITEMS:
    g_value_set_object (value, adw_sidebar_get_items (self));
    break;
  case PROP_SECTIONS:
    g_value_set_object (value, adw_sidebar_get_sections (self));
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
  object_class->finalize = adw_sidebar_finalize;
  object_class->get_property = adw_sidebar_get_property;
  object_class->set_property = adw_sidebar_set_property;

  /**
   * AdwSidebar:mode:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_MODE] =
    g_param_spec_enum ("mode", NULL, NULL,
                       ADW_TYPE_SIDEBAR_MODE,
                       ADW_SIDEBAR_MODE_SIDEBAR,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebar:selected:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SELECTED] =
    g_param_spec_uint ("selected", NULL, NULL,
                       0, G_MAXUINT, GTK_INVALID_LIST_POSITION,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebar:selected-item:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SELECTED_ITEM] =
    g_param_spec_object ("selected-item", NULL, NULL,
                         ADW_TYPE_SIDEBAR_ITEM,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwSidebar:items:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_ITEMS] =
    g_param_spec_object ("items", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwSidebar:sections:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SECTIONS] =
    g_param_spec_object ("sections", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwSidebar::activated:
   *
   * TODO
   *
   * Since: 1.8
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
  gtk_widget_class_set_css_name (widget_class, "sidebar");
}

static void
adw_sidebar_init (AdwSidebar *self)
{
  self->mode = ADW_SIDEBAR_MODE_SIDEBAR;
  self->sections = g_ptr_array_new_with_free_func (g_object_unref);
  self->selected = GTK_INVALID_LIST_POSITION;
  self->items_model = G_LIST_MODEL (adw_sidebar_items_new (self));

  g_signal_connect_swapped (self->items_model, "items-changed",
                            G_CALLBACK (items_changed_cb), self);

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
 * Since: 1.8
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
 * TODO
 *
 * Returns: TODO
 *
 * Since: 1.8
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
 * @mode: TODO
 *
 * TODO
 *
 * Since: 1.8
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

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODE]);
}

/**
 * adw_sidebar_get_selected:
 * @self: a sidebar
 *
 * TODO
 *
 * Returns: TODO
 *
 * Since: 1.8
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
 * @selected: TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_set_selected (AdwSidebar *self,
                          guint       selected)
{
  g_return_if_fail (ADW_IS_SIDEBAR (self));

  if (selected >= self->n_items)
    selected = GTK_INVALID_LIST_POSITION;

  if (selected == self->selected)
    return;

  self->selected = selected;

  update_list_selection (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SELECTED_ITEM]);
}

/**
 * adw_sidebar_get_selected_item:
 * @self: a sidebar
 *
 * TODO
 *
 * Returns: (transfer none) (nullable): TODO
 *
 * Since: 1.8
 */
AdwSidebarItem *
adw_sidebar_get_selected_item (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  return adw_sidebar_get_item (self, self->selected);
}

/**
 * adw_sidebar_get_items:
 * @self: a sidebar
 *
 * TODO
 *
 * Returns: (transfer full): TODO
 *
 * Since: 1.8
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
 * TODO
 *
 * Returns: (transfer full): TODO
 *
 * Since: 1.8
 */
GListModel *
adw_sidebar_get_sections (AdwSidebar *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR (self), NULL);

  if (self->sections_model)
    return g_object_ref (self->sections_model);

  g_set_weak_pointer (&self->sections_model,
                      G_LIST_MODEL (adw_sidebar_sections_new (self)));

  return self->sections_model;
}

/**
 * adw_sidebar_get_item:
 * @self: a sidebar
 *
 * TODO
 *
 * Returns: (transfer none) (nullable): TODO
 *
 * Since: 1.8
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
 *
 * TODO
 *
 * Returns: (transfer none) (nullable): TODO
 *
 * Since: 1.8
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
 * @section: (transfer full): an section to append
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_append (AdwSidebar        *self,
                    AdwSidebarSection *section)
{
  g_return_if_fail (ADW_IS_SIDEBAR (self));
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (section));

  g_ptr_array_add (self->sections, section);

  if (self->sections_model)
    g_list_model_items_changed (self->sections_model, self->sections->len - 1, 0, 1);
}
