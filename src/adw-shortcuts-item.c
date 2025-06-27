/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-shortcuts-item.h"

/**
 * AdwShortcutsItem:
 *
 * An object representing an individual shortcut in [class@ShortcutsSection].
 *
 * A shortcut has a title, an optional subtitle, and an accelerator.
 *
 * Accelerator must be specified in the format [class@ShortcutLabel] accepts.
 *
 * Alternatively, the [property@ShortcutsItem:action-name] property can be used
 * to automatically get accelerator associated with the specified action, as set
 * via [method@Gtk.Application.set_accels_for_action].
 *
 * If both are specified, the accelerator will be used if the action couldn't
 * be found or doesn't have an accelerator associated for it.
 *
 * If [property@ShortcutsItem:direction] is set, the shortcut will only be
 * displayed for the specified text direction. This allows to display different
 * shortcuts for different text directions.
 *
 * Since: 1.8
 */

struct _AdwShortcutsItem
{
  GObject parent_instance;

  char *title;
  char *subtitle;
  char *accelerator;
  char *action_name;
  GtkTextDirection direction;
};

G_DEFINE_FINAL_TYPE (AdwShortcutsItem, adw_shortcuts_item, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_ACCELERATOR,
  PROP_ACTION_NAME,
  PROP_DIRECTION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
adw_shortcuts_item_finalize (GObject *object)
{
  AdwShortcutsItem *self = ADW_SHORTCUTS_ITEM (object);

  g_free (self->title);
  g_free (self->subtitle);
  g_free (self->accelerator);
  g_free (self->action_name);

  G_OBJECT_CLASS (adw_shortcuts_item_parent_class)->finalize (object);
}

static void
adw_shortcuts_item_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  AdwShortcutsItem *self = ADW_SHORTCUTS_ITEM (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_shortcuts_item_get_title (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, adw_shortcuts_item_get_subtitle (self));
    break;
  case PROP_ACCELERATOR:
    g_value_set_string (value, adw_shortcuts_item_get_accelerator (self));
    break;
  case PROP_ACTION_NAME:
    g_value_set_string (value, adw_shortcuts_item_get_action_name (self));
    break;
  case PROP_DIRECTION:
    g_value_set_enum (value, adw_shortcuts_item_get_direction (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcuts_item_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  AdwShortcutsItem *self = ADW_SHORTCUTS_ITEM (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_shortcuts_item_set_title (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE:
    adw_shortcuts_item_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_ACCELERATOR:
    adw_shortcuts_item_set_accelerator (self, g_value_get_string (value));
    break;
  case PROP_ACTION_NAME:
    adw_shortcuts_item_set_action_name (self, g_value_get_string (value));
    break;
  case PROP_DIRECTION:
    adw_shortcuts_item_set_direction (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_shortcuts_item_class_init (AdwShortcutsItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = adw_shortcuts_item_finalize;
  object_class->get_property = adw_shortcuts_item_get_property;
  object_class->set_property = adw_shortcuts_item_set_property;

  /**
   * AdwShortcutsItem:title:
   *
   * The title of the shortcut.
   *
   * Since: 1.8
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwShortcutsItem:subtitle:
   *
   * The subtitle of the shortcut.
   *
   * Since: 1.8
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwShortcutsItem:accelerator:
   *
   * The shortcut accelerator.
   *
   * Accelerator must be in the format [class@ShortcutLabel] accepts.
   *
   * Since: 1.8
   */
  props[PROP_ACCELERATOR] =
    g_param_spec_string ("accelerator", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwShortcutsItem:action-name:
   *
   * Fully qualified action name to get the accelerator from.
   *
   * Since: 1.8
   */
  props[PROP_ACTION_NAME] =
    g_param_spec_string ("action-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwShortcutsItem:direction:
   *
   * The shortcut direction.
   *
   * If set to [enum@Gtk.TextDirection.LTR] or [enum@Gtk.TextDirection.RTL], the
   * shortcut will only be displayed for this direction.
   *
   * Since: 1.8
   */
  props[PROP_DIRECTION] =
    g_param_spec_enum ("direction", NULL, NULL,
                       GTK_TYPE_TEXT_DIRECTION,
                       GTK_TEXT_DIR_NONE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_shortcuts_item_init (AdwShortcutsItem *self)
{
  self->title = g_strdup ("");
  self->subtitle = g_strdup ("");
  self->accelerator = g_strdup ("");
  self->action_name = g_strdup ("");
  self->direction = GTK_TEXT_DIR_NONE;
}

/**
 * adw_shortcuts_item_new:
 * @title: the shortcut title
 * @accelerator: the shortcut accelerator
 *
 * Creates a new `AdwShortcutsItem` with @title and @accelerator.
 *
 * Returns: the newly created `AdwShortcutsItem`
 *
 * Since: 1.8
 */
AdwShortcutsItem *
adw_shortcuts_item_new (const char *title,
                        const char *accelerator)
{
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (accelerator != NULL, NULL);

  return g_object_new (ADW_TYPE_SHORTCUTS_ITEM,
                       "title", title,
                       "accelerator", accelerator,
                       NULL);
}

/**
 * adw_shortcuts_item_new_from_action:
 * @title: the shortcut title
 * @action_name: the shortcut action name
 *
 * Creates a new `AdwShortcutsItem` with @title and @action_name.
 *
 * Returns: the newly created `AdwShortcutsItem`
 *
 * Since: 1.8
 */
AdwShortcutsItem *
adw_shortcuts_item_new_from_action (const char *title,
                                    const char *action_name)
{
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (action_name != NULL, NULL);

  return g_object_new (ADW_TYPE_SHORTCUTS_ITEM,
                       "title", title,
                       "action-name", action_name,
                       NULL);
}

/**
 * adw_shortcuts_item_get_title:
 * @self: a shortcuts item
 *
 * Gets the title of @self.
 *
 * Returns: the title
 *
 * Since: 1.8
 */
const char *
adw_shortcuts_item_get_title (AdwShortcutsItem *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_ITEM (self), NULL);

  return self->title;
}

/**
 * adw_shortcuts_item_set_title:
 * @self: a shortcuts item
 * @title: the title to use
 *
 * Sets the title of @self.
 *
 * Since: 1.8
 */
void
adw_shortcuts_item_set_title (AdwShortcutsItem *self,
                              const char       *title)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_ITEM (self));
  g_return_if_fail (title != NULL);

  if (!g_set_str (&self->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_shortcuts_item_get_subtitle:
 * @self: a shortcuts item
 *
 * Gets the subtitle of @self.
 *
 * Returns: the subtitle
 *
 * Since: 1.8
 */
const char *
adw_shortcuts_item_get_subtitle (AdwShortcutsItem *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_ITEM (self), NULL);

  return self->subtitle;
}

/**
 * adw_shortcuts_item_set_subtitle:
 * @self: a shortcuts item
 * @subtitle: the subtitle to use
 *
 * Sets the subtitle of @self.
 *
 * Since: 1.8
 */
void
adw_shortcuts_item_set_subtitle (AdwShortcutsItem *self,
                                 const char       *subtitle)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_ITEM (self));
  g_return_if_fail (subtitle != NULL);

  if (!g_set_str (&self->subtitle, subtitle))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * adw_shortcuts_item_get_accelerator:
 * @self: a shortcuts item
 *
 * Gets the accelerator of @self.
 *
 * Returns: the accelerator
 *
 * Since: 1.8
 */
const char *
adw_shortcuts_item_get_accelerator (AdwShortcutsItem *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_ITEM (self), NULL);

  return self->accelerator;
}

/**
 * adw_shortcuts_item_set_accelerator:
 * @self: a shortcuts item
 * @accelerator: the accelerator to use
 *
 * Sets the accelerator of @self.
 *
 * @accelerator must be in the format [class@ShortcutLabel] accepts.
 *
 * Since: 1.8
 */
void
adw_shortcuts_item_set_accelerator (AdwShortcutsItem *self,
                                    const char       *accelerator)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_ITEM (self));
  g_return_if_fail (accelerator != NULL);

  if (!g_set_str (&self->accelerator, accelerator))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCELERATOR]);
}

/**
 * adw_shortcuts_item_get_action_name:
 * @self: a shortcuts item
 *
 * Gets the action name to get the accelerator from.
 *
 * Returns: the action name
 *
 * Since: 1.8
 */
const char *
adw_shortcuts_item_get_action_name (AdwShortcutsItem *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_ITEM (self), NULL);

  return self->action_name;
}

/**
 * adw_shortcuts_item_set_action_name:
 * @self: a shortcuts item
 * @action_name: the action name to use
 *
 * Sets the action name to get the accelerator from.
 *
 * Since: 1.8
 */
void
adw_shortcuts_item_set_action_name (AdwShortcutsItem *self,
                                    const char       *action_name)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_ITEM (self));
  g_return_if_fail (action_name != NULL);

  if (!g_set_str (&self->action_name, action_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTION_NAME]);
}

/**
 * adw_shortcuts_item_get_direction:
 * @self: a shortcuts item
 *
 * Gets the direction of @self.
 *
 * Returns: the shortcut direction
 *
 * Since: 1.8
 */
GtkTextDirection
adw_shortcuts_item_get_direction (AdwShortcutsItem *self)
{
  g_return_val_if_fail (ADW_IS_SHORTCUTS_ITEM (self), GTK_TEXT_DIR_NONE);

  return self->direction;
}

/**
 * adw_shortcuts_item_set_direction:
 * @self: a shortcuts item
 * @direction: the shortcut direction
 *
 * Sets the direction of @self.
 *
 * If set to [enum@Gtk.TextDirection.LTR] or [enum@Gtk.TextDirection.RTL], the
 * shortcut will only be displayed for this direction.
 *
 * Since: 1.8
 */
void
adw_shortcuts_item_set_direction (AdwShortcutsItem *self,
                                  GtkTextDirection  direction)
{
  g_return_if_fail (ADW_IS_SHORTCUTS_ITEM (self));
  g_return_if_fail (direction >= GTK_TEXT_DIR_NONE);
  g_return_if_fail (direction <= GTK_TEXT_DIR_RTL);

  if (direction == self->direction)
    return;

  self->direction = direction;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DIRECTION]);
}
