/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"
#include "adw-button-row.h"

#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"

/**
 * AdwButtonRow:
 *
 * A [class@Gtk.ListBoxRow] that looks like a button.
 *
 * <picture>
 *   <source srcset="button-rows-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="button-rows.png" alt="button-rows">
 * </picture>
 *
 * The `AdwButtonRow` widget has a title and two icons: before and after the
 * title.
 *
 * It is convenient to present actions like "Delete" at the end of a boxed list.
 *
 * `AdwButtonRow` is always activatable.
 *
 * ## CSS nodes
 *
 * `AdwButtonRow` has a main CSS node with name `row` and the style class
 * `.button`.
 *
 * It contains the subnode `box` for its main horizontal box, which contains the
 * nodes: `image.icon.start` for the start icon, `label.title` for the title,
 * and `image.icon.end` for the end icon.
 *
 * `AdwButtonRow` can have the
 * [`.suggested-action`](style-classes.html#suggested-action) or
 * [`.destructive-action`](style-classes.html#destructive-action) style classes.
 *
 * Since: 1.6
 */

typedef struct
{
  char *start_icon_name;
  char *end_icon_name;

  GtkWidget *previous_parent;
} AdwButtonRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwButtonRow, adw_button_row, ADW_TYPE_PREFERENCES_ROW)

enum {
  PROP_0,
  PROP_START_ICON_NAME,
  PROP_END_ICON_NAME,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static gboolean
string_is_not_empty (AdwButtonRow *self,
                     const char   *string)
{
  return string && string[0];
}

static void
row_activated_cb (AdwButtonRow  *self,
                  GtkListBoxRow *row)
{
  /* No need to use GTK_LIST_BOX_ROW() for a pointer comparison. */
  if ((GtkListBoxRow *) self == row)
    adw_button_row_activate (self);
}

static void
parent_cb (AdwButtonRow *self)
{
  AdwButtonRowPrivate *priv = adw_button_row_get_instance_private (self);
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (priv->previous_parent != NULL) {
    g_signal_handlers_disconnect_by_func (priv->previous_parent, G_CALLBACK (row_activated_cb), self);
    priv->previous_parent = NULL;
  }

  if (parent == NULL || !GTK_IS_LIST_BOX (parent))
    return;

  priv->previous_parent = parent;
  g_signal_connect_swapped (parent, "row-activated", G_CALLBACK (row_activated_cb), self);
}

static void
adw_button_row_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwButtonRow *self = ADW_BUTTON_ROW (object);

  switch (prop_id) {
  case PROP_START_ICON_NAME:
    g_value_set_string (value, adw_button_row_get_start_icon_name (self));
    break;
  case PROP_END_ICON_NAME:
    g_value_set_string (value, adw_button_row_get_end_icon_name (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_button_row_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwButtonRow *self = ADW_BUTTON_ROW (object);

  switch (prop_id) {
  case PROP_START_ICON_NAME:
    adw_button_row_set_start_icon_name (self, g_value_get_string (value));
    break;
  case PROP_END_ICON_NAME:
    adw_button_row_set_end_icon_name (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_button_row_dispose (GObject *object)
{
  AdwButtonRow *self = ADW_BUTTON_ROW (object);
  AdwButtonRowPrivate *priv = adw_button_row_get_instance_private (self);

  if (priv->previous_parent != NULL) {
    g_signal_handlers_disconnect_by_func (priv->previous_parent, G_CALLBACK (row_activated_cb), self);
    priv->previous_parent = NULL;
  }

  G_OBJECT_CLASS (adw_button_row_parent_class)->dispose (object);
}

static void
adw_button_row_finalize (GObject *object)
{
  AdwButtonRow *self = ADW_BUTTON_ROW (object);
  AdwButtonRowPrivate *priv = adw_button_row_get_instance_private (self);

  g_free (priv->start_icon_name);
  g_free (priv->end_icon_name);

  G_OBJECT_CLASS (adw_button_row_parent_class)->finalize (object);
}

static void
adw_button_row_activate_real (AdwButtonRow *self)
{
  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0);
}

static void
adw_button_row_class_init (AdwButtonRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_button_row_get_property;
  object_class->set_property = adw_button_row_set_property;
  object_class->dispose = adw_button_row_dispose;
  object_class->finalize = adw_button_row_finalize;

  klass->activate = adw_button_row_activate_real;

  /**
   * AdwButtonRow:start-icon-name: (attributes org.gtk.Property.get=adw_button_row_get_start_icon_name org.gtk.Property.set=adw_button_row_set_start_icon_name)
   *
   * The icon name to show before the title.
   *
   * Since: 1.6
   */
  props[PROP_START_ICON_NAME] =
    g_param_spec_string ("start-icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwButtonRow:end-icon-name: (attributes org.gtk.Property.get=adw_button_row_get_end_icon_name org.gtk.Property.set=adw_button_row_set_end_icon_name)
   *
   * The icon name to show after the title.
   *
   * Since: 1.6
   */
  props[PROP_END_ICON_NAME] =
    g_param_spec_string ("end-icon-name", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwButtonRow::activated:
   *
   * This signal is emitted after the row has been activated.
   *
   * Since: 1.6
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

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-button-row.ui");
  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);
}

static void
adw_button_row_init (AdwButtonRow *self)
{
  AdwButtonRowPrivate *priv = adw_button_row_get_instance_private (self);

  priv->start_icon_name = g_strdup ("");
  priv->end_icon_name = g_strdup ("");

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self, "notify::parent", G_CALLBACK (parent_cb), NULL);
}

/**
 * adw_button_row_new:
 *
 * Creates a new `AdwButtonRow`.
 *
 * Returns: the newly created `AdwButtonRow`
 *
 * Since: 1.6
 */
GtkWidget *
adw_button_row_new (void)
{
  return g_object_new (ADW_TYPE_BUTTON_ROW, NULL);
}

/**
 * adw_button_row_get_start_icon_name: (attributes org.gtk.Method.get_property=start-icon-name)
 * @self: a button row
 *
 * Gets the start icon name for @self.
 *
 * Returns: (nullable): the start icon name for @self
 *
 * Since: 1.6
 */
const char *
adw_button_row_get_start_icon_name (AdwButtonRow *self)
{
  AdwButtonRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_BUTTON_ROW (self), NULL);

  priv = adw_button_row_get_instance_private (self);

  return priv->start_icon_name;
}

/**
 * adw_button_row_set_start_icon_name: (attributes org.gtk.Method.set_property=start-icon-name)
 * @self: a button row
 * @icon_name: (nullable): the start icon name
 *
 * Sets the start icon name for @self.
 *
 * Since: 1.6
 */
void
adw_button_row_set_start_icon_name (AdwButtonRow *self,
                                    const char   *icon_name)
{
  AdwButtonRowPrivate *priv;

  g_return_if_fail (ADW_IS_BUTTON_ROW (self));

  priv = adw_button_row_get_instance_private (self);

  if (!g_set_str (&priv->start_icon_name, icon_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_START_ICON_NAME]);
}

/**
 * adw_button_row_get_end_icon_name: (attributes org.gtk.Method.get_property=end-icon-name)
 * @self: a button row
 *
 * Gets the end icon name for @self.
 *
 * Returns: (nullable): the end icon name for @self
 *
 * Since: 1.6
 */
const char *
adw_button_row_get_end_icon_name (AdwButtonRow *self)
{
  AdwButtonRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_BUTTON_ROW (self), NULL);

  priv = adw_button_row_get_instance_private (self);

  return priv->end_icon_name;
}

/**
 * adw_button_row_set_end_icon_name: (attributes org.gtk.Method.set_property=end-icon-name)
 * @self: a button row
 * @icon_name: (nullable): the end icon name
 *
 * Sets the end icon name for @self.
 *
 * Since: 1.6
 */
void
adw_button_row_set_end_icon_name (AdwButtonRow *self,
                                  const char   *icon_name)
{
  AdwButtonRowPrivate *priv;

  g_return_if_fail (ADW_IS_BUTTON_ROW (self));

  priv = adw_button_row_get_instance_private (self);

  if (!g_set_str (&priv->end_icon_name, icon_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_END_ICON_NAME]);
}

/**
 * adw_button_row_activate:
 * @self: a button row
 *
 * Activates @self.
 *
 * Since: 1.6
 */
void
adw_button_row_activate (AdwButtonRow *self)
{
  g_return_if_fail (ADW_IS_BUTTON_ROW (self));

  ADW_BUTTON_ROW_GET_CLASS (self)->activate (self);
}
