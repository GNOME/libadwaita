/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-shortcuts-section.h"

#include <glib/gi18n.h>

/**
 * AdwShortcutsSection:
 *
 * An object representing a section in [class@ShortcutsDialog].
 *
 * It contains [class@ShortcutsItem] objects, use [method@ShortcutsSection.add] to
 * add them.
 *
 * `AdwShortcutsSection` implements the [iface@Gio.ListModel] interface and
 * allows to access the added shortcut items through it.
 *
 * ## `AdwShortcutsSection` as `GtkBuildable`
 *
 * `AdwShortcutsSection` allows adding `AdwShortcutsItem` objects as children.
 *
 * Since: 1.8
 */

struct _AdwShortcutsSection
{
  GObject parent_instance;

  char *title;
  GPtrArray *items;
};

static void adw_shortcuts_section_buildable_init (GtkBuildableIface *iface);
static void adw_shortcuts_section_list_model_init (GListModelInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwShortcutsSection, adw_shortcuts_section, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_shortcuts_section_buildable_init)
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_shortcuts_section_list_model_init))

enum {
  PROP_0,
  PROP_TITLE,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
adw_shortcuts_section_dispose (GObject *object)
{
  AdwShortcutsSection *self = ADW_SHORTCUTS_SECTION (object);

  g_clear_pointer (&self->items, g_ptr_array_unref);
  g_free (self->title);

  G_OBJECT_CLASS (adw_shortcuts_section_parent_class)->dispose (object);
}

static void
adw_shortcuts_section_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwShortcutsSection *self = ADW_SHORTCUTS_SECTION (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_shortcuts_section_get_title (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcuts_section_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwShortcutsSection *self = ADW_SHORTCUTS_SECTION (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_shortcuts_section_set_title (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcuts_section_class_init (AdwShortcutsSectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_shortcuts_section_dispose;
  object_class->get_property = adw_shortcuts_section_get_property;
  object_class->set_property = adw_shortcuts_section_set_property;

  /**
   * AdwShortcutsSection:title:
   *
   * The title of the section, can be `NULL`.
   *
   * Since: 1.8
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_shortcuts_section_init (AdwShortcutsSection *self)
{
  self->items = g_ptr_array_new_full (0, g_object_unref);
}

static void
adw_shortcuts_section_add_child (GtkBuildable *buildable,
                                 GtkBuilder   *builder,
                                 GObject      *child,
                                 const char   *type)
{
  AdwShortcutsSection *self = ADW_SHORTCUTS_SECTION (buildable);

  if (ADW_IS_SHORTCUTS_ITEM (child)) {
    adw_shortcuts_section_add (self, g_object_ref (ADW_SHORTCUTS_ITEM (child)));
  } else {
    g_warning ("Cannot add an object of type %s to AdwShortcutsSection",
               g_type_name (G_OBJECT_TYPE (child)));
  }
}

static void
adw_shortcuts_section_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = adw_shortcuts_section_add_child;
}

static GType
adw_shortcuts_section_get_item_type (GListModel *model)
{
  return ADW_TYPE_SHORTCUTS_ITEM;
}

static guint
adw_shortcuts_section_get_n_items (GListModel *model)
{
  AdwShortcutsSection *self = ADW_SHORTCUTS_SECTION (model);

  return self->items->len;
}

static gpointer
adw_shortcuts_section_get_item (GListModel *model,
                                guint       position)
{
  AdwShortcutsSection *self = ADW_SHORTCUTS_SECTION (model);

  if (position >= self->items->len)
    return NULL;

  return g_object_ref (g_ptr_array_index (self->items, position));
}

static void
adw_shortcuts_section_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_shortcuts_section_get_item_type;
  iface->get_n_items = adw_shortcuts_section_get_n_items;
  iface->get_item = adw_shortcuts_section_get_item;
}

/**
 * adw_shortcuts_section_new:
 * @title: (nullable): the section title
 *
 * Creates a new `AdwShortcutsSection` with @title as its title if provided.
 *
 * Returns: the newly created `AdwShortcutsSection`
 *
 * Since: 1.8
 */
AdwShortcutsSection *
adw_shortcuts_section_new (const char *title)
{
  return g_object_new (ADW_TYPE_SHORTCUTS_SECTION,
                       "title", title,
                       NULL);
}

/**
 * adw_shortcuts_section_get_title:
 * @self: a shortcuts section
 *
 * Gets the title of @self.
 *
 * Returns: (nullable): the title
 *
 * Since: 1.8
 */
const char *
adw_shortcuts_section_get_title (AdwShortcutsSection *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_SECTION (self), NULL);

  return self->title;
}

/**
 * adw_shortcuts_section_set_title:
 * @self: a shortcuts section
 * @title: (nullable): the title to use
 *
 * Sets the title of @self.
 *
 * Since: 1.8
 */
void
adw_shortcuts_section_set_title (AdwShortcutsSection *self,
                                 const char          *title)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_SECTION (self));

  if (!g_set_str (&self->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_shortcuts_section_add:
 * @self: a shortcuts section
 * @item: (transfer full): the item to add
 *
 * Adds @item to @self.
 *
 * Since: 1.8
 */
void
adw_shortcuts_section_add (AdwShortcutsSection *self,
                           AdwShortcutsItem    *item)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_SECTION (self));
  g_return_if_fail (ADW_IS_SHORTCUTS_ITEM (item));

  g_ptr_array_add (self->items, item);

  g_list_model_items_changed (G_LIST_MODEL (self), self->items->len - 1, 0, 1);
}
