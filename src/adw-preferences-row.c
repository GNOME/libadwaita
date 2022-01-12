/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-preferences-row.h"

/**
 * AdwPreferencesRow:
 *
 * A [class@Gtk.ListBoxRow] used to present preferences.
 *
 * The `AdwPreferencesRow` widget has a title that [class@PreferencesWindow]
 * will use to let the user look for a preference. It doesn't present the title
 * in any way and lets you present the preference as you please.
 *
 * [class@ActionRow] and its derivatives are convenient to use as preference
 * rows as they take care of presenting the preference's title while letting you
 * compose the inputs of the preference around it.
 *
 * Since: 1.0
 */

typedef struct
{
  char *title;

  gboolean use_underline;
  gboolean title_selectable;
} AdwPreferencesRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwPreferencesRow, adw_preferences_row, GTK_TYPE_LIST_BOX_ROW)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_USE_UNDERLINE,
  PROP_TITLE_SELECTABLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
adw_preferences_row_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  AdwPreferencesRow *self = ADW_PREFERENCES_ROW (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_preferences_row_get_title (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_preferences_row_get_use_underline (self));
    break;
  case PROP_TITLE_SELECTABLE:
    g_value_set_boolean (value, adw_preferences_row_get_title_selectable (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_row_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  AdwPreferencesRow *self = ADW_PREFERENCES_ROW (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_preferences_row_set_title (self, g_value_get_string (value));
    break;
  case PROP_USE_UNDERLINE:
    adw_preferences_row_set_use_underline (self, g_value_get_boolean (value));
    break;
  case PROP_TITLE_SELECTABLE:
    adw_preferences_row_set_title_selectable (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_row_finalize (GObject *object)
{
  AdwPreferencesRow *self = ADW_PREFERENCES_ROW (object);
  AdwPreferencesRowPrivate *priv = adw_preferences_row_get_instance_private (self);

  g_free (priv->title);

  G_OBJECT_CLASS (adw_preferences_row_parent_class)->finalize (object);
}

static void
adw_preferences_row_class_init (AdwPreferencesRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_preferences_row_get_property;
  object_class->set_property = adw_preferences_row_set_property;
  object_class->finalize = adw_preferences_row_finalize;

  /**
   * AdwPreferencesRow:title: (attributes org.gtk.Property.get=adw_preferences_row_get_title org.gtk.Property.set=adw_preferences_row_set_title)
   *
   * The title of the preference represented by this row.
   *
   * Since: 1.0
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "The title of the preference represented by this row",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwPreferencesRow:use-underline: (attributes org.gtk.Property.get=adw_preferences_row_get_use_underline org.gtk.Property.set=adw_preferences_row_set_use_underline)
   *
   * Whether an embedded underline in the title indicates a mnemonic.
   *
   * Since: 1.0
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline",
                          "Use underline",
                          "Whether an embedded underline in the title indicates a mnemonic",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwPreferencesRow:title-selectable: (attributes org.gtk.Property.get=adw_preferences_row_get_title_selectable org.gtk.Property.set=adw_preferences_row_set_title_selectable)
   *
   * Whether the user can copy the title from the label.
   *
   * See also [property@Gtk.Label:selectable].
   *
   * Since: 1.0
   */
  props[PROP_TITLE_SELECTABLE] =
    g_param_spec_boolean ("title-selectable",
                          "Title selectable",
                          "Whether the title should be selectable (i.e. the user can copy it)",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_preferences_row_init (AdwPreferencesRow *self)
{
    AdwPreferencesRowPrivate *priv = adw_preferences_row_get_instance_private (self);
    priv->title = g_strdup ("");
}

/**
 * adw_preferences_row_new:
 *
 * Creates a new `AdwPreferencesRow`.
 *
 * Returns: the newly created `AdwPreferencesRow`
 *
 * Since: 1.0
 */
GtkWidget *
adw_preferences_row_new (void)
{
  return g_object_new (ADW_TYPE_PREFERENCES_ROW, NULL);
}

/**
 * adw_preferences_row_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a preferences row
 *
 * Gets the title of the preference represented by @self.
 *
 * Returns: the title
 *
 * Since: 1.0
 */
const char *
adw_preferences_row_get_title (AdwPreferencesRow *self)
{
  AdwPreferencesRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_ROW (self), NULL);

  priv = adw_preferences_row_get_instance_private (self);

  return priv->title;
}

/**
 * adw_preferences_row_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a preferences row
 * @title: the title
 *
 * Sets the title of the preference represented by @self.
 *
 * Since: 1.0
 */
void
adw_preferences_row_set_title (AdwPreferencesRow *self,
                               const char        *title)
{
  AdwPreferencesRowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_ROW (self));

  priv = adw_preferences_row_get_instance_private (self);

  if (g_strcmp0 (priv->title, title) == 0)
    return;

  g_free (priv->title);
  priv->title = g_strdup (title ? title : "");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_preferences_row_get_use_underline: (attributes org.gtk.Method.get_property=use-underline)
 * @self: a preferences row
 *
 * Gets whether an embedded underline in the title indicates a mnemonic.
 *
 * Returns: whether an embedded underline in the title indicates a mnemonic
 *
 * Since: 1.0
 */
gboolean
adw_preferences_row_get_use_underline (AdwPreferencesRow *self)
{
  AdwPreferencesRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_ROW (self), FALSE);

  priv = adw_preferences_row_get_instance_private (self);

  return priv->use_underline;
}

/**
 * adw_preferences_row_set_use_underline: (attributes org.gtk.Method.set_property=use-underline)
 * @self: a preferences row
 * @use_underline: `TRUE` if underlines in the text indicate mnemonics
 *
 * Sets whether an embedded underline in the title indicates a mnemonic.
 *
 * Since: 1.0
 */
void
adw_preferences_row_set_use_underline (AdwPreferencesRow *self,
                                       gboolean           use_underline)
{
  AdwPreferencesRowPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_ROW (self));

  priv = adw_preferences_row_get_instance_private (self);

  use_underline = !!use_underline;

  if (priv->use_underline == use_underline)
    return;

  priv->use_underline = use_underline;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_UNDERLINE]);
}

/**
 * adw_preferences_row_get_title_selectable: (attributes org.gtk.Method.get_property=title-selectable)
 * @self: a `AdwPreferencesRow`
 *
 * Gets whether the user can copy the title from the label
 *
 * Returns: whether the user can copy the title from the label
 *
 * Since: 1.0
 */
gboolean
adw_preferences_row_get_title_selectable (AdwPreferencesRow *self)
{
  AdwPreferencesRowPrivate *priv = adw_preferences_row_get_instance_private (self);

  g_return_val_if_fail (ADW_IS_PREFERENCES_ROW (self), FALSE);

  return priv->title_selectable;
}

/**
 * adw_preferences_row_set_title_selectable: (attributes org.gtk.Method.set_property=title-selectable)
 * @self: a `AdwPreferencesRow`
 * @title_selectable: `TRUE` if the user can copy the title from the label
 *
 * Sets whether the user can copy the title from the label
 *
 * Since: 1.0
 */
void
adw_preferences_row_set_title_selectable (AdwPreferencesRow *self,
                                          gboolean           title_selectable)
{
  AdwPreferencesRowPrivate *priv = adw_preferences_row_get_instance_private (self);

  g_return_if_fail (ADW_IS_PREFERENCES_ROW (self));

  title_selectable = !!title_selectable;

  if (priv->title_selectable == title_selectable)
    return;

  priv->title_selectable = title_selectable;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE_SELECTABLE]);
}
