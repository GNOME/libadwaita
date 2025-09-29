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
 * An item within [class@SidebarSection].
 *
 * Sidebar items must have a title, set via [property@SidebarItem:title].
 *
 * Sidebar items should, but are not required to, have an icon. Icons can be set
 * from an icon name, via [property@SidebarItem:icon-name], or a
 * [iface@Gdk.Paintable], via [property@SidebarItem:icon-paintable].
 *
 * Items can also have subtitles, set with the [property@SidebarItem:subtitle]
 * property. Subtitles should be used sparingly.
 *
 * To add a tooltip, use [property@SidebarItem:tooltip]. Tooltips always use
 * Pango markup.
 *
 * Items can have an arbitrary suffix widget, set with the
 * [property@SidebarItem:suffix] properties. It will be displayed at the end of
 * its row, or before the arrow in the [enum@Adw.SidebarMode.PAGE] mode.
 *
 * To hide or disable the item, use the [property@SidebarItem:visible] and
 * [property@SidebarItem:enabled] properties respectively.
 *
 * To access the items's section, use [property@SidebarItem:section].
 *
 * It's also possible to access the index of the item in both the section and
 * the sidebar, using [method@SidebarItem.get_section_index] and
 * [method@SidebarItem.get_index] respectively.
 *
 * Dragging content over sidebar items activates them by default. To disable
 * this behavior, set [property@SidebarItem:drag-motion-activate] to `FALSE`.
 *
 * `AdwSidebarItem` is derivable, and applications that need to associate each
 * page with data can store it in the items themselves  this way.
 *
 * Since: 1.9
 */

typedef struct
{
  GObject parent_instance;

  char *title;
  char *subtitle;
  gboolean use_underline;
  char *icon_name;
  GdkPaintable *icon_paintable;
  char *tooltip;
  GtkWidget *suffix;
  gboolean visible;
  gboolean enabled;
  gboolean drag_motion_activate;

  AdwSidebarSection *section;
  guint local_index;
} AdwSidebarItemPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwSidebarItem, adw_sidebar_item, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_USE_UNDERLINE,
  PROP_ICON_NAME,
  PROP_ICON_PAINTABLE,
  PROP_TOOLTIP,
  PROP_SUFFIX,
  PROP_VISIBLE,
  PROP_ENABLED,
  PROP_DRAG_MOTION_ACTIVATE,
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

  g_clear_object (&priv->icon_paintable);
  g_clear_object (&priv->suffix);

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
  g_free (priv->tooltip);

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
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_sidebar_item_get_use_underline (self));
    break;
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_sidebar_item_get_icon_name (self));
    break;
  case PROP_ICON_PAINTABLE:
    g_value_set_object (value, adw_sidebar_item_get_icon_paintable (self));
    break;
  case PROP_TOOLTIP:
    g_value_set_string (value, adw_sidebar_item_get_tooltip (self));
    break;
  case PROP_SUFFIX:
    g_value_set_object (value, adw_sidebar_item_get_suffix (self));
    break;
  case PROP_VISIBLE:
    g_value_set_boolean (value, adw_sidebar_item_get_visible (self));
    break;
  case PROP_ENABLED:
    g_value_set_boolean (value, adw_sidebar_item_get_enabled (self));
    break;
  case PROP_DRAG_MOTION_ACTIVATE:
    g_value_set_boolean (value, adw_sidebar_item_get_drag_motion_activate (self));
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
  case PROP_USE_UNDERLINE:
    adw_sidebar_item_set_use_underline (self, g_value_get_boolean (value));
    break;
  case PROP_ICON_NAME:
    adw_sidebar_item_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_ICON_PAINTABLE:
    adw_sidebar_item_set_icon_paintable (self, g_value_get_object (value));
    break;
  case PROP_TOOLTIP:
    adw_sidebar_item_set_tooltip (self, g_value_get_string (value));
    break;
  case PROP_SUFFIX:
    adw_sidebar_item_set_suffix (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE:
    adw_sidebar_item_set_visible (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLED:
    adw_sidebar_item_set_enabled (self, g_value_get_boolean (value));
    break;
  case PROP_DRAG_MOTION_ACTIVATE:
    adw_sidebar_item_set_drag_motion_activate (self, g_value_get_boolean (value));
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
   * Title of the item.
   *
   * Since: 1.9
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:subtitle:
   *
   * Subtitle of the item.
   *
   * Since: 1.9
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:use-underline:
   *
   * Whether an underline in the title indicates a mnemonic.
   *
   * The mnemonic can be used to activate the item.
   *
   * Since: 1.9
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:icon-name:
   *
   * The icon name for this item.
   *
   * Mutually exclusive with [property@SidebarItem:icon-paintable].
   *
   * Since: 1.9
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:icon-paintable:
   *
   * The paintable to use as the icon for this item.
   *
   * Mutually exclusive with [property@SidebarItem:icon-name].
   *
   * Since: 1.9
   */
  props[PROP_ICON_PAINTABLE] =
    g_param_spec_object ("icon-paintable", NULL, NULL,
                         GDK_TYPE_PAINTABLE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:tooltip:
   *
   * The tooltip of the item.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * Since: 1.9
   */
  props[PROP_TOOLTIP] =
    g_param_spec_string ("tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:suffix:
   *
   * The suffix widget for this item.
   *
   * Suffix will be shown at the end of the item's row, or before the arrow in
   * the [enum@Adw.SidebarMode.PAGE] mode.
   *
   * Since: 1.9
   */
  props[PROP_SUFFIX] =
    g_param_spec_object ("suffix", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:visible:
   *
   * Whether the item is visible.
   *
   * Since: 1.9
   */
  props[PROP_VISIBLE] =
    g_param_spec_boolean ("visible", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:enabled:
   *
   * Whether the item is enabled.
   *
   * See [property@Gtk.Widget:sensitive].
   *
   * Since: 1.9
   */
  props[PROP_ENABLED] =
    g_param_spec_boolean ("enabled", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:drag-motion-activate:
   *
   * Whether to activate the item on pointer motion during Drag-and-Drop.
   *
   * This is needed to be able to drag content into the page the item
   * represents, when the sidebar is used as a page switcher. However, it may be
   * unwanted when dropping content onto the item itself, so it can be disabled.
   *
   * Since: 1.9
   */
  props[PROP_DRAG_MOTION_ACTIVATE] =
    g_param_spec_boolean ("drag-motion-activate", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSidebarItem:section:
   *
   * The section the item is in.
   *
   * Since: 1.9
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
  priv->tooltip = g_strdup ("");
  priv->visible = TRUE;
  priv->enabled = TRUE;
  priv->drag_motion_activate = TRUE;
}

/**
 * adw_sidebar_item_new:
 * @title: the item title
 *
 * Creates a new `AdwSidebarItem` with @title as its title.
 *
 * Returns: the newly created `AdwSidebarItem`
 *
 * Since: 1.9
 */
AdwSidebarItem *
adw_sidebar_item_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADW_TYPE_SIDEBAR_ITEM, "title", title, NULL);
}

/**
 * adw_sidebar_item_get_title:
 * @self: a sidebar item
 *
 * Gets the title of @self.
 *
 * Returns: (nullable): the title
 *
 * Since: 1.9
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
 * @title: (nullable): the title
 *
 * Sets the title of @self.
 *
 * Since: 1.9
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
 * Gets the subtitle of @self.
 *
 * Returns: (nullable): the subtitle
 *
 * Since: 1.9
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
 * @subtitle: (nullable): the subtitle
 *
 * Sets the subtitle of @self.
 *
 * Since: 1.9
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
 * adw_sidebar_item_get_use_underline:
 * @self: a sidebar item
 *
 * Gets whether an underline in the title indicates a mnemonic.
 *
 * Returns: whether an underline in the text indicates a mnemonic
 *
 * Since: 1.9
 */
gboolean
adw_sidebar_item_get_use_underline (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), FALSE);

  return priv->use_underline;
}

/**
 * adw_sidebar_item_set_use_underline:
 * @self: a sidebar item
 * @use_underline: whether an underline in the text indicates a mnemonic
 *
 * Sets whether an underline in the title indicates a mnemonic.
 *
 * The mnemonic can be used to activate the item.
 *
 * Since: 1.9
 */
void
adw_sidebar_item_set_use_underline (AdwSidebarItem *self,
                                    gboolean        use_underline)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  use_underline = !!use_underline;

  if (use_underline == priv->use_underline)
    return;

  priv->use_underline = use_underline;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_UNDERLINE]);
}

/**
 * adw_sidebar_item_get_icon_name:
 * @self: a sidebar item
 *
 * Gets the icon name for @item.
 *
 * Returns: (nullable): the icon name
 *
 * Since: 1.9
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
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @item.
 *
 * Mutually exclusive with [property@SidebarItem:icon-paintable].
 *
 * Since: 1.9
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
 * Gets the paintable used as the icon for @item.
 *
 * Returns: (nullable) (transfer none): the icon paintable
 *
 * Since: 1.9
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
 * @paintable: (nullable): the icon paintable
 *
 * Sets the paintable to use as the icon for @item.
 *
 * Mutually exclusive with [property@SidebarItem:icon-name].
 *
 * Since: 1.9
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
 * adw_sidebar_item_get_tooltip:
 * @self: a sidebar item
 *
 * Gets the tooltip of @self.
 *
 * Returns: (nullable): the tooltip
 *
 * Since: 1.9
 */
const char *
adw_sidebar_item_get_tooltip (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), NULL);

  return priv->tooltip;
}

/**
 * adw_sidebar_item_set_tooltip:
 * @self: a sidebar item
 * @tooltip: (nullable): the tooltip
 *
 * Sets the tooltip of @self.
 *
 * The tooltip can be marked up with the Pango text markup language.
 *
 * Since: 1.9
 */
void
adw_sidebar_item_set_tooltip (AdwSidebarItem *self,
                              const char     *tooltip)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  if (!g_set_str (&priv->tooltip, tooltip))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TOOLTIP]);
}

/**
 * adw_sidebar_item_get_suffix:
 * @self: a sidebar item
 *
 * Gets the suffix widget for @self.
 *
 * Returns: (nullable) (transfer none): the suffix widget
 *
 * Since: 1.9
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
 * @suffix: (nullable): the suffix widget
 *
 * Sets the suffix widget for @self.
 *
 * Suffix will be shown at the end of the item's row, or before the arrow in
 * the [enum@Adw.SidebarMode.PAGE] mode.
 *
 * Since: 1.9
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
 * adw_sidebar_item_get_visible:
 * @self: a sidebar item
 *
 * Gets whether @self is visible.
 *
 * Returns: whether the item is visible
 *
 * Since: 1.9
 */
gboolean
adw_sidebar_item_get_visible (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), FALSE);

  return priv->visible;
}

/**
 * adw_sidebar_item_set_visible:
 * @self: a sidebar item
 * @visible: whether the item is visible
 *
 * Sets whether @self is visible.
 *
 * Since: 1.9
 */
void
adw_sidebar_item_set_visible (AdwSidebarItem *self,
                              gboolean        visible)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  visible = !!visible;

  if (visible == priv->visible)
    return;

  priv->visible = visible;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE]);
}

/**
 * adw_sidebar_item_get_enabled:
 * @self: a sidebar item
 *
 * Gets whether @self is enabled.
 *
 * Returns: whether the item is enabled
 *
 * Since: 1.9
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
 * @enabled: whether to enable the item
 *
 * Sets whether @self is enabled.
 *
 * See [property@Gtk.Widget:sensitive].
 *
 * Since: 1.9
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
 * adw_sidebar_item_get_drag_motion_activate:
 * @self: a sidebar item
 *
 * Gets whether @self will be activated on pointer motion during Drag-and-Drop.
 *
 * Returns: whether to enable the item on drag motion
 *
 * Since: 1.9
 */
gboolean
adw_sidebar_item_get_drag_motion_activate (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), FALSE);

  return priv->drag_motion_activate;
}

/**
 * adw_sidebar_item_set_drag_motion_activate:
 * @self: a sidebar item
 * @drag_motion_activate: whether to enable the item on drag motion
 *
 * Sets whether to activate @self on pointer motion during Drag-and-Drop.
 *
 * This is needed to be able to drag content into the page the item
 * represents, when the sidebar is used as a page switcher. However, it may be
 * unwanted when dropping content onto the item itself, so it can be disabled.
 *
 * Since: 1.9
 */
void
adw_sidebar_item_set_drag_motion_activate (AdwSidebarItem *self,
                                           gboolean        drag_motion_activate)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));

  drag_motion_activate = !!drag_motion_activate;

  if (drag_motion_activate == priv->drag_motion_activate)
    return;

  priv->drag_motion_activate = drag_motion_activate;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DRAG_MOTION_ACTIVATE]);
}

/**
 * adw_sidebar_item_get_section:
 * @self: a sidebar item
 *
 * Gets the section @self is in.
 *
 * Returns: (transfer none) (nullable): the section of @self
 *
 * Since: 1.9
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
 * Gets index of @self within its [class@Sidebar].
 *
 * If @self is within a section, but that section is not in a sidebar, index
 * will be within the section only.
 *
 * If @self is not within a section, the index will be 0.
 *
 * The item can later be retrieved by passing this index into
 * [method@Sidebar.get_item].
 *
 * Returns: the index of @self
 *
 * Since: 1.9
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

/**
 * adw_sidebar_item_get_section_index:
 * @self: a sidebar item
 *
 * Gets index of @self within its [class@SidebarSection].
 *
 * If @self is not within a section, the index will be 0.
 *
 * The item can later be retrieved by passing this index into
 * [method@SidebarSection.get_item].
 *
 * Returns: the index of @self
 *
 * Since: 1.9
 */
guint
adw_sidebar_item_get_section_index (AdwSidebarItem *self)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_SIDEBAR_ITEM (self), 0);

  if (!priv->section)
    return 0;

  return priv->local_index;
}

void
adw_sidebar_item_set_section (AdwSidebarItem    *self,
                              AdwSidebarSection *section)
{
  AdwSidebarItemPrivate *priv = adw_sidebar_item_get_instance_private (self);

  g_return_if_fail (ADW_IS_SIDEBAR_ITEM (self));
  g_return_if_fail (section == NULL || ADW_IS_SIDEBAR_SECTION (section));

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
