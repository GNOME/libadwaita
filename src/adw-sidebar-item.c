/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-sidebar-item-private.h"

#include "adw-sidebar-section-private.h"

/**
 * AdwSidebarItem:
 *
 * TODO
 *
 * Since: 1.8
 */

typedef struct
{
  GObject parent_instance;

  char *title;
  char *subtitle;
  char *icon_name;
  GdkPaintable *icon_paintable;
  GtkWidget *suffix;
  gboolean enabled;

  AdwSidebarSection *section;
  guint local_index;
} AdwSidebarItemPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwSidebarItem, adw_sidebar_item, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_ICON_NAME,
  PROP_ICON_PAINTABLE,
  PROP_SUFFIX,
  PROP_ENABLED,
  PROP_SECTION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
section_weak_notify_cb (AdwSidebarItem *self,
                        GObject        *object)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  priv->section = NULL;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SECTION]);
}

static void
adw_sidebar_item_dispose (GObject *object)
{
  AdwSidebarItem *self = ADW_SIDEBAR_ITEM (object);
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  if (priv->section)
    g_object_weak_unref (G_OBJECT (priv->section), (GWeakNotify) section_weak_notify_cb, self);

  priv->section = NULL;

  G_OBJECT_CLASS (adw_sidebar_item_parent_class)->dispose (object);
}

static void
adw_sidebar_item_finalize (GObject *object)
{
  AdwSidebarItem *self = ADW_SIDEBAR_ITEM (object);
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_free (priv->title);
  g_free (priv->subtitle);
  g_free (priv->icon_name);

  G_OBJECT_CLASS (adw_sidebar_item_parent_class)->finalize (object);
}

static void
adw_sidebar_item_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwSidebarItem *self = ADW_SIDEBAR_ITEM (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_sidebar_item_get_title (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, adw_sidebar_item_get_subtitle (self));
    break;
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_sidebar_item_get_icon_name (self));
    break;
  case PROP_ICON_PAINTABLE:
    g_value_set_object (value, adw_sidebar_item_get_icon_paintable (self));
    break;
  case PROP_SUFFIX:
    g_value_set_object (value, adw_sidebar_item_get_suffix (self));
    break;
  case PROP_ENABLED:
    g_value_set_boolean (value, adw_sidebar_item_get_enabled (self));
    break;
  case PROP_SECTION:
    g_value_set_object (value, adw_sidebar_item_get_section (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sidebar_item_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwSidebarItem *self = ADW_SIDEBAR_ITEM (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_sidebar_item_set_title (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE:
    adw_sidebar_item_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_ICON_NAME:
    adw_sidebar_item_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_ICON_PAINTABLE:
    adw_sidebar_item_set_icon_paintable (self, g_value_get_object (value));
    break;
  case PROP_SUFFIX:
    adw_sidebar_item_set_suffix (self, g_value_get_object (value));
    break;
  case PROP_ENABLED:
    adw_sidebar_item_set_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_sidebar_item_class_init (AdwSidebarItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_sidebar_item_dispose;
  object_class->finalize = adw_sidebar_item_finalize;
  object_class->get_property = adw_sidebar_item_get_property;
  object_class->set_property = adw_sidebar_item_set_property;

  /**
   * AdwSidebarItem:title:
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
   * AdwSidebarItem:subtitle:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:icon-name:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:icon-paintable:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_ICON_PAINTABLE] =
    g_param_spec_object ("icon-paintable", NULL, NULL,
                         GDK_TYPE_PAINTABLE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:suffix:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SUFFIX] =
    g_param_spec_object ("suffix", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:enabled:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_ENABLED] =
    g_param_spec_boolean ("enabled", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:section:
   *
   * TODO
   *
   * Since: 1.8
   */
  props[PROP_SECTION] =
    g_param_spec_object ("section", NULL, NULL,
                         ADW_TYPE_SIDEBAR_SECTION,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_sidebar_item_init (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  priv->title = g_strdup ("");
  priv->subtitle = g_strdup ("");
  priv->enabled = TRUE;
}

/**
 * adw_sidebar_item_new:
 *
 * Creates a new `AdwSidebarItem`.
 *
 * Returns: the newly created `AdwSidebarItem`
 *
 * Since: 1.8
 */
AdwSidebarItem *
adw_sidebar_item_new (void)
{
  return g_object_new (ADW_TYPE_SIDEBAR_ITEM, NULL);
}

/**
 * adw_sidebar_item_get_title:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: (nullable): TODO
 *
 * Since: 1.8
 */
const char *
adw_sidebar_item_get_title (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->title;
}

/**
 * adw_sidebar_item_set_title:
 * @self: a sidebar item
 * @title: TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_item_set_title (AdwSidebarItem *self,
                            const char     *title)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  if (!g_set_str (&priv->title, title))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_sidebar_item_get_subtitle:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: (nullable): TODO
 *
 * Since: 1.8
 */
const char *
adw_sidebar_item_get_subtitle (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->subtitle;
}

/**
 * adw_sidebar_item_set_subtitle:
 * @self: a sidebar item
 * @subtitle: TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_item_set_subtitle (AdwSidebarItem *self,
                               const char     *subtitle)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  if (!g_set_str (&priv->subtitle, subtitle))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * adw_sidebar_item_get_icon_name:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: (nullable): TODO
 *
 * Since: 1.8
 */
const char *
adw_sidebar_item_get_icon_name (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->icon_name;
}

/**
 * adw_sidebar_item_set_icon_name:
 * @self: a sidebar item
 * @icon_name: (nullable): TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_item_set_icon_name (AdwSidebarItem *self,
                                const char     *icon_name)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  if (!g_strcmp0 (priv->icon_name, icon_name))
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (priv->icon_paintable) {
    g_clear_object (&priv->icon_paintable);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_PAINTABLE]);
  }

  g_set_str (&priv->icon_name, icon_name);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_sidebar_item_get_icon_paintable:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: (nullable) (transfer none): TODO
 *
 * Since: 1.8
 */
GdkPaintable *
adw_sidebar_item_get_icon_paintable (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->icon_paintable;
}

/**
 * adw_sidebar_item_set_icon_paintable:
 * @self: a sidebar item
 * @paintable: (nullable): TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_item_set_icon_paintable (AdwSidebarItem *self,
                                     GdkPaintable   *paintable)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));
  g_return_if_fail (paintable == NULL || GDK_IS_PAINTABLE (paintable));

  if (priv->icon_paintable == paintable)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (priv->icon_name) {
    g_clear_pointer (&priv->icon_name, g_free);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
  }

  g_set_object (&priv->icon_paintable, paintable);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_PAINTABLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_sidebar_item_get_suffix:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: (nullable) (transfer none): TODO
 *
 * Since: 1.8
 */
GtkWidget *
adw_sidebar_item_get_suffix (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->suffix;
}

/**
 * adw_sidebar_item_set_suffix:
 * @self: a sidebar item
 * @suffix: (nullable): TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_item_set_suffix (AdwSidebarItem *self,
                             GtkWidget      *suffix)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));
  g_return_if_fail (suffix == NULL || GTK_IS_WIDGET (suffix));

  if (suffix == priv->suffix)
    return;

  if (priv->suffix)
    g_object_unref (priv->suffix);

  priv->suffix = suffix;

  if (priv->suffix)
    g_object_ref_sink (priv->suffix);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUFFIX]);
}

/**
 * adw_sidebar_item_get_enabled:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: TODO
 *
 * Since: 1.8
 */
gboolean
adw_sidebar_item_get_enabled (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), FALSE);

  return priv->enabled;
}

/**
 * adw_sidebar_item_set_enabled:
 * @self: a sidebar item
 * @enabled: TODO
 *
 * TODO
 *
 * Since: 1.8
 */
void
adw_sidebar_item_set_enabled (AdwSidebarItem *self,
                              gboolean        enabled)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  enabled = !!enabled;

  if (enabled == priv->enabled)
    return;

  priv->enabled = enabled;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLED]);
}

/**
 * adw_sidebar_item_get_section:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: (transfer none) (nullable): TODO
 *
 * Since: 1.8
 */
AdwSidebarSection *
adw_sidebar_item_get_section (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->section;
}

/**
 * adw_sidebar_item_get_index:
 * @self: a sidebar item
 *
 * TODO
 *
 * Returns: TODO
 *
 * Since: 1.8
 */
guint
adw_sidebar_item_get_index (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), 0);

  if (!priv->section)
    return 0;

  return adw_sidebar_section_get_first_index (priv->section) + priv->local_index;
}

void
adw_sidebar_item_set_section (AdwSidebarItem    *self,
                              AdwSidebarSection *section)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));
  g_return_if_fail (ADW_IS_SIDEBAR_SECTION (section));

  if (priv->section) {
    g_object_weak_unref (G_OBJECT (priv->section),
                         (GWeakNotify) section_weak_notify_cb, self);
  }

  priv->section = section;

  if (priv->section) {
    g_object_weak_ref (G_OBJECT (priv->section),
                       (GWeakNotify) section_weak_notify_cb, self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SECTION]);
}

void
adw_sidebar_item_set_index (AdwSidebarItem *self,
                            guint           index)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  priv->local_index = index;
}
