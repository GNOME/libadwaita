/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-sidebar-section-private.h"

#include "adw-sidebar-item-private.h"

/**
 * AdwSidebarSection:
 *
 * A section within [class@Sidebar].
 *
 * `AdwSidebarSection` contains [class@SidebarItem] objects.
 *
 * Section can optionally have a title, set with the
 * [property@SidebarSection:title] property. If a title is not set, the section
 * will have a separator in front of it, or just spacing in the
 * [enum@Adw.SidebarMode.PAGE] mode.
 *
 * To add items, use [method@SidebarSection.append],
 * [method@SidebarSection.prepend] or [method@SidebarSection.insert].
 *
 * To remove items, use [method@SidebarSection.remove] or
 * [method@SidebarSection.remove_all].
 *
 * To inspect the items, use [method@SidebarSection.get_item] or
 * [property@SidebarSection:items].
 *
 * To get the sidebar the section is in, use[property@SidebarSection:sidebar].
 *
 * ## Binding models
 *
 * `AdwSidebarSection` can show items from a provided [iface@Gio.ListModel],
 * using [method@SidebarSection.bind_model]. It works the same way as
 * [method@Gtk.ListBox.bind_model], except the provided function creates an
 * [class@SidebarItem] rather than a [class@Gtk.ListBoxRow].
 *
 * While a model is bound, adding or removing items manually is not allowed.
 * Inspecting them is still allowed, but discouraged.
 *
 * ## `AdwSidebarSection` as `GtkBuildable`
 *
 * `AdwSidebarSection` allows adding items as children.
 *
 * Example of an `AdwSidebarSection` UI definition:
 *
 * ```xml
 * <object class="AdwSidebarSection">
 *   <property name="title" translatable="yes">Places</property>
 *   <child>
 *     <object class="AdwSidebarItem">
 *       <property name="title" translatable="yes">Music</property>
 *       <property name="icon-name">folder-music-symbolic</property>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwSidebarItem">
 *       <property name="title" translatable="yes">Pictures</property>
 *       <property name="icon-name">folder-pictures-symbolic</property>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="AdwSidebarItem">
 *       <property name="title" translatable="yes">Videos</property>
 *       <property name="icon-name">folder-videos-symbolic</property>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * Result:
 *
 * <picture>
 *   <source srcset="sidebar-section-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="sidebar-section.png" alt="sidebar-section">
 * </picture>
 *
 * Since: 1.9
 */

struct _AdwSidebarSection
{
  GObject parent_instance;

  char *title;

  GPtrArray *items;
  GListModel *items_model;

  guint first_index;

  GListModel *bound_model;
  AdwSidebarSectionCreateItemFunc create_item_func;
  gpointer create_item_func_data;
  GDestroyNotify create_item_func_data_destroy;

  AdwSidebar *sidebar;
};

static void adw_sidebar_section_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSidebarSection, adw_sidebar_section, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_sidebar_section_buildable_init))

enum {
  PROP_0,
  PROP_TITLE,
  PROP_ITEMS,
  PROP_SIDEBAR,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

#define ADW_TYPE_SIDEBAR_SECTION_ITEMS (adw_sidebar_section_items_get_type ())

G_DECLARE_FINAL_TYPE (AdwSidebarSectionItems, adw_sidebar_section_items, ADW, SIDEBAR_SECTION_ITEMS, GObject)

struct _AdwSidebarSectionItems
{
  GObject parent_instance;

  AdwSidebarSection *section;
};

static void adw_sidebar_section_items_list_model_init (GListModelInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSidebarSectionItems, adw_sidebar_section_items, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_sidebar_section_items_list_model_init))

static void
adw_sidebar_section_items_dispose (GObject *object)
{
  AdwSidebarSectionItems *self = ADW_SIDEBAR_SECTION_ITEMS (object);

  g_clear_weak_pointer (&self->section);

  G_OBJECT_CLASS (adw_sidebar_section_items_parent_class)->dispose (object);
}

static void
adw_sidebar_section_items_class_init (AdwSidebarSectionItemsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_sidebar_section_items_dispose;
}

static void
adw_sidebar_section_items_init (AdwSidebarSectionItems *self)
{
}

static GType
adw_sidebar_section_items_get_item_type (GListModel *model)
{
  return ADW_TYPE_SIDEBAR_ITEM;
}

static guint
adw_sidebar_section_items_get_n_items (GListModel *model)
{
  AdwSidebarSectionItems *self = ADW_SIDEBAR_SECTION_ITEMS (model);

  if (!self->section)
    return 0;

  return self->section->items->len;
}

static gpointer
adw_sidebar_section_items_get_item (GListModel *model,
                                    guint       position)
{
  AdwSidebarSectionItems *self = ADW_SIDEBAR_SECTION_ITEMS (model);
  AdwSidebarItem *item;

  if (!self->section)
    return NULL;

  if (position >= g_list_model_get_n_items (model))
    return NULL;

  item = g_ptr_array_index (self->section->items, position);

  return g_object_ref (item);
}

static void
adw_sidebar_section_items_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_sidebar_section_items_get_item_type;
  iface->get_n_items = adw_sidebar_section_items_get_n_items;
  iface->get_item = adw_sidebar_section_items_get_item;
}

static AdwSidebarSectionItems *
adw_sidebar_section_items_new (AdwSidebarSection *section)
{
  AdwSidebarSectionItems *items;

  items = g_object_new (ADW_TYPE_SIDEBAR_SECTION_ITEMS, NULL);
  g_set_weak_pointer (&items->section, section);

  return items;
}

static void
sidebar_weak_notify_cb (AdwSidebarSection *self,
                        GObject           *object)
{
  self->sidebar = NULL;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR]);
}

static void
bound_model_changed_cb (AdwSidebarSection *self,
                        guint              position,
                        guint              removed,
                        guint              added)
{
  guint i;

  for (i = position; i < position + removed; i++) {
    AdwSidebarItem *item = g_ptr_array_index (self->items, i);

    adw_sidebar_item_set_section (item, NULL);
    adw_sidebar_item_set_index (item, 0);
  }

  g_ptr_array_remove_range (self->items, position, removed);

  for (i = 0; i < added; i++) {
    GObject *model_item = g_list_model_get_item (self->bound_model, position + i);
    AdwSidebarItem *item = self->create_item_func (model_item, self->create_item_func_data);

    g_ptr_array_insert (self->items, position + i, item);

    adw_sidebar_item_set_section (item, self);

    g_object_unref (model_item);
  }

  /* Update index on the subsequent items */
  for (i = position; i < self->items->len; i++) {
    AdwSidebarItem *item = g_ptr_array_index (self->items, i);

    adw_sidebar_item_set_index (item, i);
  }

  if (self->items_model)
    g_list_model_items_changed (self->items_model, position, removed, added);
}

static void
adw_sidebar_section_dispose (GObject *object)
{
  AdwSidebarSection *self = ADW_SIDEBAR_SECTION (object);

  if (self->sidebar)
    g_object_weak_unref (G_OBJECT (self->sidebar), (GWeakNotify) sidebar_weak_notify_cb, self);

  self->sidebar = NULL;

  if (self->items_model) {
    g_list_model_items_changed (G_LIST_MODEL (self->items_model),
                                0, self->items->len, 0);
  }

  if (self->bound_model) {
    if (self->create_item_func_data_destroy)
      self->create_item_func_data_destroy (self->create_item_func_data);

    g_signal_handlers_disconnect_by_func (self->bound_model, bound_model_changed_cb, self);
    g_clear_object (&self->bound_model);
  }

  g_clear_pointer (&self->items, g_ptr_array_unref);

  G_OBJECT_CLASS (adw_sidebar_section_parent_class)->dispose (object);
}

static void
adw_sidebar_section_finalize (GObject *object)
{
  AdwSidebarSection *self = ADW_SIDEBAR_SECTION (object);

  g_free (self->title);
  g_clear_weak_pointer (&self->items_model);

  G_OBJECT_CLASS (adw_sidebar_section_parent_class)->finalize (object);
}

static void
adw_sidebar_section_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwSidebarSection *self = ADW_SIDEBAR_SECTION (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_sidebar_section_get_title (self));
    break;
  case PROP_ITEMS:
    g_value_set_object (value, adw_sidebar_section_get_items (self));
    break;
  case PROP_SIDEBAR:
    g_value_set_object (value, adw_sidebar_section_get_sidebar (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sidebar_section_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwSidebarSection *self = ADW_SIDEBAR_SECTION (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_sidebar_section_set_title (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sidebar_section_class_init (AdwSidebarSectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_sidebar_section_dispose;
  object_class->finalize = adw_sidebar_section_finalize;
  object_class->get_property = adw_sidebar_section_get_property;
  object_class->set_property = adw_sidebar_section_set_property;

  /**
   * AdwSidebarSection:title:
   *
   * Title of the section.
   *
   * If set, it will be displayed instead of the separator before the section.
   *
   * Since: 1.9
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarSection:items:
   *
   * A list model with the section's items.
   *
   * This can be used to keep an up-to-date view.
   *
   * Since: 1.9
   */
  props[PROP_ITEMS] =
    g_param_spec_object ("items", NULL, NULL,
                         G_TYPE_LIST_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwSidebarSection:sidebar:
   *
   * The sidebar the section is in.
   *
   * Since: 1.9
   */
  props[PROP_SIDEBAR] =
    g_param_spec_object ("sidebar", NULL, NULL,
                         ADW_TYPE_SIDEBAR,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_sidebar_section_init (AdwSidebarSection *self)
{
  self->title = g_strdup ("");
  self->items = g_ptr_array_new_with_free_func (g_object_unref);
}

static void
adw_sidebar_section_add_child (GtkBuildable *buildable,
                               GtkBuilder   *builder,
                               GObject      *child,
                               const char   *type)
{
  if (ADW_IS_SIDEBAR_ITEM (child)) {
    adw_sidebar_section_append (ADW_SIDEBAR_SECTION (buildable),
                                ADW_SIDEBAR_ITEM (g_object_ref (child)));
  } else {
    g_warning ("Cannot add an object of type %s to AdwSidebarSection",
               g_type_name (G_OBJECT_TYPE (child)));
  }
}

static void
adw_sidebar_section_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adw_sidebar_section_add_child;
}

/**
 * adw_sidebar_section_new:
 *
 * Creates a new `AdwSidebarSection`.
 *
 * Returns: the newly created `AdwSidebarSection`
 *
 * Since: 1.9
 */
AdwSidebarSection *
adw_sidebar_section_new (void)
{
  return g_object_new (ADW_TYPE_SIDEBAR_SECTION, NULL);
}

/**
 * adw_sidebar_section_get_title:
 * @self: a sidebar section
 *
 * Gets the title of @self.
 *
 * Returns: (nullable): the title
 *
 * Since: 1.9
 */
const char *
adw_sidebar_section_get_title (AdwSidebarSection *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), NULL);

  return self->title;
}

/**
 * adw_sidebar_section_set_title:
 * @self: a sidebar section
 * @title: (nullable): the title
 *
 * Sets the title of @self.
 *
 * If set, it will be displayed instead of the separator before the section.
 *
 * Since: 1.9
 */
void
adw_sidebar_section_set_title (AdwSidebarSection *self,
                               const char        *title)
{
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));

  if (!g_set_str (&self->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_sidebar_section_get_items:
 * @self: a sidebar section
 *
 * Gets a list model with @self's items.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a model containing the items
 *
 * Since: 1.9
 */
GListModel *
adw_sidebar_section_get_items (AdwSidebarSection *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), NULL);

  if (self->items_model)
    return g_object_ref (self->items_model);

  g_set_weak_pointer (&self->items_model,
                      G_LIST_MODEL (adw_sidebar_section_items_new (self)));

  return self->items_model;
}

/**
 * adw_sidebar_section_get_item:
 * @self: a sidebar section
 * @index: index of the item
 *
 * Gets the item at @index within @self.
 *
 * The index starts from 0 at the top of the section, and is same as the one
 * returned by [method@SidebarItem.get_section_index].
 *
 * Can return `NULL` if @index is larger or equal to the number of items.
 *
 * Returns: (transfer none) (nullable): the item at @index
 *
 * Since: 1.9
 */
AdwSidebarItem *
adw_sidebar_section_get_item (AdwSidebarSection *self,
                              guint              index)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), NULL);

  if (index >= self->items->len)
    return NULL;

  return g_ptr_array_index (self->items, index);
}

/**
 * adw_sidebar_section_append:
 * @self: a sidebar section
 * @item: (transfer full): an item to append
 *
 * Appends @item to @self.
 *
 * Cannot be used while a model is bound via [method@SidebarSection.bind_model].
 *
 * Since: 1.9
 */
void
adw_sidebar_section_append (AdwSidebarSection *self,
                            AdwSidebarItem    *item)
{
  adw_sidebar_section_insert (self, item, -1);
}

/**
 * adw_sidebar_section_prepend:
 * @self: a sidebar section
 * @item: (transfer full): an item to prepend
 *
 * Prepends @item to @self.
 *
 * Cannot be used while a model is bound via [method@SidebarSection.bind_model].
 *
 * Since: 1.9
 */
void
adw_sidebar_section_prepend (AdwSidebarSection *self,
                             AdwSidebarItem    *item)
{
  adw_sidebar_section_insert (self, item, 0);
}

/**
 * adw_sidebar_section_insert:
 * @self: a sidebar section
 * @item: (transfer full): an item to insert
 * @position: position to insert @item at
 *
 * Inserts @item at @position to @self.
 *
 * If @position is -1, or larger than the total number of items in @self,
 * the item will be appended to the end.
 *
 * Cannot be used while a model is bound via [method@SidebarSection.bind_model].
 *
 * Since: 1.9
 */
void
adw_sidebar_section_insert (AdwSidebarSection *self,
                            AdwSidebarItem    *item,
                            int                position)
{
  guint i;

  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (item));
  g_return_if_fail (adw_sidebar_item_get_section (item) == NULL);
  g_return_if_fail (self->bound_model == NULL);

  if (position < 0 || position >= self->items->len) {
    g_ptr_array_add (self->items, item);

    adw_sidebar_item_set_section (item, self);
    adw_sidebar_item_set_index (item, self->items->len - 1);

    if (self->items_model)
      g_list_model_items_changed (self->items_model, self->items->len - 1, 0, 1);

    return;
  }

  adw_sidebar_item_set_section (item, self);
  g_ptr_array_insert (self->items, position, item);

  /* Update index on the subsequent items since we removed one */
  for (i = position; i < self->items->len; i++) {
    AdwSidebarItem *item2 = g_ptr_array_index (self->items, i);

    adw_sidebar_item_set_index (item2, i);
  }

  if (self->items_model)
    g_list_model_items_changed (self->items_model, position, 0, 1);
}

/**
 * adw_sidebar_section_remove:
 * @self: a sidebar section
 * @item: an item to remove
 *
 * Removes @item from @self.
 *
 * Cannot be used while a model is bound via [method@SidebarSection.bind_model].
 *
 * Since: 1.9
 */
void
adw_sidebar_section_remove (AdwSidebarSection *self,
                            AdwSidebarItem    *item)
{
  guint index, i;

  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (item));
  g_return_if_fail (adw_sidebar_item_get_section (item) == self);
  g_return_if_fail (self->bound_model == NULL);

  index = adw_sidebar_item_get_index (item) - self->first_index;

  g_object_ref (item);

  g_ptr_array_remove_index (self->items, index);

  /* Update index on the subsequent items since we removed one */
  for (i = index; i < self->items->len; i++) {
    AdwSidebarItem *item2 = g_ptr_array_index (self->items, i);

    adw_sidebar_item_set_index (item2, i);
  }

  if (self->items_model)
    g_list_model_items_changed (self->items_model, index, 1, 0);

  adw_sidebar_item_set_section (item, NULL);
  adw_sidebar_item_set_index (item, 0);
  g_object_unref (item);
}

/**
 * adw_sidebar_section_remove_all:
 * @self: a sidebar section
 *
 * Removes all items from @self.
 *
 * Cannot be used while a model is bound via [method@SidebarSection.bind_model].
 *
 * Since: 1.9
 */
void
adw_sidebar_section_remove_all (AdwSidebarSection *self)
{
  guint i, len;
  GPtrArray *old_items;

  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (self->bound_model == NULL);

  len = self->items->len;

  old_items = self->items;
  self->items = g_ptr_array_new_with_free_func (g_object_unref);

  if (self->items_model)
    g_list_model_items_changed (self->items_model, 0, len, 0);

  for (i = 0; i < len; i++) {
    AdwSidebarItem *item = g_ptr_array_index (old_items, i);

    adw_sidebar_item_set_section (item, NULL);
    adw_sidebar_item_set_index (item, 0);
  }

  g_ptr_array_unref (old_items);
}

/**
 * adw_sidebar_section_bind_model:
 * @self: a sidebar section
 * @model: (nullable): the model to be bound
 * @create_item_func: (nullable) (scope notified) (closure user_data) (destroy user_data_free_func):
 *     a function that creates [class@SidebarItem] for model items, or `NULL` in
 *     case @model is also `NULL`
 * @user_data: user data passed to @create_widget_func
 * @user_data_free_func: function for freeing @user_data
 *
 * Binds @model to @self.
 *
 * If @self was already bound to a model, that previous binding is
 * destroyed.
 *
 * The contents of @self are cleared and then filled with items that
 * represent items from @model. @self is updated whenever @model changes.
 *
 * If @model is `NULL`, @self is left empty.
 *
 * Calling [method@SidebarSection.prepend], [method@SidebarSection.insert],
 * [method@SidebarSection.append], [method@SidebarSection.remove] or
 * [method@SidebarSection.remove_all] while a model is bound is not allowed.
 *
 * Accessing items and modifying them is allowed, but the changes will be erased
 * whenever that part of the model changes, so it's not recommended.
 *
 * Since: 1.9
 */
void
adw_sidebar_section_bind_model (AdwSidebarSection               *self,
                                GListModel                      *model,
                                AdwSidebarSectionCreateItemFunc  create_item_func,
                                gpointer                         user_data,
                                GDestroyNotify                   user_data_free_func)
{
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (model == NULL || G_IS_LIST_MODEL (model));
  g_return_if_fail (model == NULL || create_item_func != NULL);

  if (self->bound_model) {
    if (self->create_item_func_data_destroy)
      self->create_item_func_data_destroy (self->create_item_func_data);

    g_signal_handlers_disconnect_by_func (self->bound_model, bound_model_changed_cb, self);
    g_clear_object (&self->bound_model);
  }

  adw_sidebar_section_remove_all (self);

  if (!model)
    return;

  self->bound_model = g_object_ref (model);
  self->create_item_func = create_item_func;
  self->create_item_func_data = user_data;
  self->create_item_func_data_destroy = user_data_free_func;

  g_signal_connect_swapped (model, "items-changed", G_CALLBACK (bound_model_changed_cb), self);
  bound_model_changed_cb (self, 0, 0, g_list_model_get_n_items (model));
}

/**
 * adw_sidebar_section_get_sidebar:
 * @self: a sidebar section
 *
 * Gets the sidebar @self is in.
 *
 * Returns: (transfer none) (nullable): the sidebar of @self
 *
 * Since: 1.9
 */
AdwSidebar *
adw_sidebar_section_get_sidebar (AdwSidebarSection *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), NULL);

  return self->sidebar;
}

guint
adw_sidebar_section_get_n_items (AdwSidebarSection *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), 0);

  return self->items->len;
}

guint
adw_sidebar_section_get_first_index (AdwSidebarSection *self)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), 0);

  return self->first_index;
}

void
adw_sidebar_section_set_first_index (AdwSidebarSection *self,
                                     guint              index)
{
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));

  if (self->first_index == index)
    return;

  self->first_index = index;
}

void
adw_sidebar_section_set_sidebar (AdwSidebarSection *self,
                                 AdwSidebar        *sidebar)
{
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (sidebar == NULL || ADW_IS_SIDEBAR (sidebar));

  if (self->sidebar) {
    g_object_weak_unref (G_OBJECT (self->sidebar),
                         (GWeakNotify) sidebar_weak_notify_cb, self);
  }

  self->sidebar = sidebar;

  if (self->sidebar) {
    g_object_weak_ref (G_OBJECT (self->sidebar),
                       (GWeakNotify) sidebar_weak_notify_cb, self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SIDEBAR]);
}
