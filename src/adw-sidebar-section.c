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
 * TODO
 *
 * Since: 1.8
 */

struct _AdwSidebarSection
{
  GObject parent_instance;

  char *title;

  GPtrArray *items;
  GListModel *items_model;

  guint first_index;
};

static void adw_sidebar_section_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSidebarSection, adw_sidebar_section, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_sidebar_section_buildable_init))

enum {
  PROP_0,
  PROP_TITLE,
  PROP_ITEMS,
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

  return self->section->items->len;
}

static gpointer
adw_sidebar_section_items_get_item (GListModel *model,
                                    guint       position)
{
  AdwSidebarSectionItems *self = ADW_SIDEBAR_SECTION_ITEMS (model);
  AdwSidebarItem *item;

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
adw_sidebar_section_dispose (GObject *object)
{
  AdwSidebarSection *self = ADW_SIDEBAR_SECTION (object);

  if (self->items_model) {
    g_list_model_items_changed (G_LIST_MODEL (self->items_model),
                                0, self->items->len, 0);
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
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarSection:items:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_ITEMS] =
    g_param_spec_object ("items", NULL, NULL,
                         G_TYPE_LIST_MODEL,
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
 * Since: 1.8
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
 * TODO
 *
 * Returns: (nullable): TODO
 *
 * Since: 1.8
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
 * @title: TODO
 *
 * TODO
 *
 * Since: 1.8
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
 * TODO
 *
 * Returns: (transfer full): TODO
 *
 * Since: 1.8
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
 * @position: TODO
 *
 * TODO
 *
 * Returns: (transfer none) (nullable): TODO
 *
 * Since: 1.8
 */
AdwSidebarItem *
adw_sidebar_section_get_item (AdwSidebarSection *self,
                              guint              position)
{
  g_return_val_if_fail (ADW_IS_SIDEBAR_SECTION (self), NULL);

  if (position >= self->items->len)
    return NULL;

  return g_ptr_array_index (self->items, position);
}

/**
 * adw_sidebar_section_append:
 * @self: a sidebar section
 * @item: (transfer full): an item to append
 *
 * TODO
 *
 * Since: 1.8
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
 * TODO
 *
 * Since: 1.8
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
 * Since: 1.8
 */
void
adw_sidebar_section_insert (AdwSidebarSection *self,
                            AdwSidebarItem    *item,
                            int                position)
{
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (item));

  if (position < 0 || position >= self->items->len) {
    g_ptr_array_add (self->items, item);
    g_list_model_items_changed (self->items_model, self->items->len - 1, 0, 1);
  } else {
    g_ptr_array_insert (self->items, position, item);
    g_list_model_items_changed (self->items_model, position, 0, 1);
  }
}

/**
 * adw_sidebar_section_remove:
 * @self: a sidebar section
 * @item: an item to remove
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_section_remove (AdwSidebarSection *self,
                            AdwSidebarItem    *item)
{
  guint index, i;

  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));
  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (item));

  index = adw_sidebar_item_get_index (item) - self->first_index;

  g_ptr_array_remove_index (self->items, index);

  /* Update index on the subsequent items since we removed one */
  for (i = index; i < self->items->len; i++) {
    AdwSidebarItem *item2 = g_ptr_array_index (self->items, i);

    adw_sidebar_item_set_index (item2, i);
  }

  if (self->items_model)
    g_list_model_items_changed (self->items_model, index, 1, 0);
}

/**
 * adw_sidebar_section_remove_all:
 * @self: a sidebar section
 *
 * Removes all items from @self.
 *
 * Since: 1.8
 */
void
adw_sidebar_section_remove_all (AdwSidebarSection *self)
{
  guint len;

  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (self));

  len = self->items->len;

  g_ptr_array_remove_range (self->items, 0, len);
  g_list_model_items_changed (self->items_model, 0, len, 0);
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
