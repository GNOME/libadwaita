/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "adw-action-row.h"

#include <glib/gi18n-lib.h>

/**
 * SECTION:adw-action-row
 * @short_description: A #GtkListBox row used to present actions.
 * @Title: AdwActionRow
 *
 * The #AdwActionRow widget can have a title, a subtitle and an icon. The row
 * can receive additional widgets at its end, or prefix widgets at its start.
 *
 * It is convenient to present a preference and its related actions.
 *
 * #AdwActionRow is unactivatable by default, giving it an activatable widget
 * will automatically make it activatable, but unsetting it won't change the
 * row's activatability.
 *
 * # AdwActionRow as GtkBuildable
 *
 * The GtkWindow implementation of the GtkBuildable interface supports setting a
 * child at its end by omitting the “type” attribute of a &lt;child&gt; element.
 *
 * It also supports setting a child as a prefix widget by specifying “prefix” as
 * the “type” attribute of a &lt;child&gt; element.
 *
 * # CSS nodes
 *
 * #AdwActionRow has a main CSS node with name row.
 *
 * It contains the subnode box.header for its main horizontal box, and box.title
 * for the vertical box containing the title and subtitle labels.
 *
 * It contains subnodes label.title and label.subtitle representing respectively
 * the title label and subtitle label.
 *
 * Since: 1.0
 */

typedef struct
{
  GtkWidget *header;
  GtkImage *image;
  GtkBox *prefixes;
  GtkLabel *subtitle;
  GtkBox *suffixes;
  GtkLabel *title;
  GtkBox *title_box;

  GtkWidget *previous_parent;

  gboolean use_underline;
  gint title_lines;
  gint subtitle_lines;
  GtkWidget *activatable_widget;
} AdwActionRowPrivate;

static void adw_action_row_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwActionRow, adw_action_row, ADW_TYPE_PREFERENCES_ROW,
                         G_ADD_PRIVATE (AdwActionRow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adw_action_row_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_ACTIVATABLE_WIDGET,
  PROP_SUBTITLE,
  PROP_USE_UNDERLINE,
  PROP_TITLE_LINES,
  PROP_SUBTITLE_LINES,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ACTIVATED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
row_activated_cb (AdwActionRow  *self,
                  GtkListBoxRow *row)
{
  /* No need to use GTK_LIST_BOX_ROW() for a pointer comparison. */
  if ((GtkListBoxRow *) self == row)
    adw_action_row_activate (self);
}

static void
parent_cb (AdwActionRow *self)
{
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);
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
update_subtitle_visibility (AdwActionRow *self)
{
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->subtitle),
                          gtk_label_get_text (priv->subtitle) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->subtitle), "") != 0);
}

static void
adw_action_row_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwActionRow *self = ADW_ACTION_ROW (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_action_row_get_icon_name (self));
    break;
  case PROP_ACTIVATABLE_WIDGET:
    g_value_set_object (value, (GObject *) adw_action_row_get_activatable_widget (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, adw_action_row_get_subtitle (self));
    break;
  case PROP_SUBTITLE_LINES:
    g_value_set_int (value, adw_action_row_get_subtitle_lines (self));
    break;
  case PROP_TITLE_LINES:
    g_value_set_int (value, adw_action_row_get_title_lines (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_action_row_get_use_underline (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_action_row_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwActionRow *self = ADW_ACTION_ROW (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adw_action_row_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_ACTIVATABLE_WIDGET:
    adw_action_row_set_activatable_widget (self, (GtkWidget*) g_value_get_object (value));
    break;
  case PROP_SUBTITLE:
    adw_action_row_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE_LINES:
    adw_action_row_set_subtitle_lines (self, g_value_get_int (value));
    break;
  case PROP_TITLE_LINES:
    adw_action_row_set_title_lines (self, g_value_get_int (value));
    break;
  case PROP_USE_UNDERLINE:
    adw_action_row_set_use_underline (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_action_row_dispose (GObject *object)
{
  AdwActionRow *self = ADW_ACTION_ROW (object);
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);

  if (priv->previous_parent != NULL) {
    g_signal_handlers_disconnect_by_func (priv->previous_parent, G_CALLBACK (row_activated_cb), self);
    priv->previous_parent = NULL;
  }

  adw_action_row_set_activatable_widget (self, NULL);

  gtk_widget_unparent (priv->header);

  G_OBJECT_CLASS (adw_action_row_parent_class)->dispose (object);
}

static void
adw_action_row_activate_real (AdwActionRow *self)
{
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);

  if (priv->activatable_widget)
    gtk_widget_mnemonic_activate (priv->activatable_widget, FALSE);

  g_signal_emit (self, signals[SIGNAL_ACTIVATED], 0);
}

static void
adw_action_row_class_init (AdwActionRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_action_row_get_property;
  object_class->set_property = adw_action_row_set_property;
  object_class->dispose = adw_action_row_dispose;

  klass->activate = adw_action_row_activate_real;

  /**
   * AdwActionRow:icon-name:
   *
   * The icon name for this row.
   *
   * Since: 1.0
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         _("Icon name"),
                         _("Icon name"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwActionRow:activatable-widget:
   *
   * The activatable widget for this row.
   *
   * Since: 1.0
   */
  props[PROP_ACTIVATABLE_WIDGET] =
      g_param_spec_object ("activatable-widget",
                           _("Activatable widget"),
                           _("The widget to be activated when the row is activated"),
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE);

  /**
   * AdwActionRow:subtitle:
   *
   * The subtitle for this row.
   *
   * Since: 1.0
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         _("Subtitle"),
                         _("Subtitle"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwActionRow:use-underline:
   *
   * Whether an embedded underline in the text of the title and subtitle labels
   * indicates a mnemonic.
   *
   * Since: 1.0
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline",
                          _("Use underline"),
                          _("If set, an underline in the text indicates the next character should be used for the mnemonic accelerator key"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwActionRow:title-lines:
   *
   * The number of lines at the end of which the title label will be ellipsized.
   * Set this property to 0 if you don't want to limit the number of lines.
   *
   * Since: 1.0
   */
  props[PROP_TITLE_LINES] =
    g_param_spec_int ("title-lines",
                      _("Number of title lines"),
                      _("The desired number of title lines"),
                      0, G_MAXINT,
                      1,
                      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwActionRow:subtitle-lines:
   *
   * The number of lines at the end of which the subtitle label will be
   * ellipsized.
   * Set this property to 0 if you don't want to limit the number of lines.
   *
   * Since: 1.0
   */
  props[PROP_SUBTITLE_LINES] =
    g_param_spec_int ("subtitle-lines",
                      _("Number of subtitle lines"),
                      _("The desired number of subtitle lines"),
                      0, G_MAXINT,
                      1,
                      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwActionRow::activated:
   * @self: The #AdwActionRow instance
   *
   * This signal is emitted after the row has been activated.
   *
   * Since: 1.0
   */
  signals[SIGNAL_ACTIVATED] =
    g_signal_new ("activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-action-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, header);
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, prefixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, subtitle);
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, suffixes);
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, title);
  gtk_widget_class_bind_template_child_private (widget_class, AdwActionRow, title_box);
}

static gboolean
string_is_not_empty (GBinding     *binding,
                     const GValue *from_value,
                     GValue       *to_value,
                     gpointer      user_data)
{
  const gchar *string = g_value_get_string (from_value);

  g_value_set_boolean (to_value, string != NULL && g_strcmp0 (string, "") != 0);

  return TRUE;
}

static void
adw_action_row_init (AdwActionRow *self)
{
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);

  priv->title_lines = 1;
  priv->subtitle_lines = 1;

  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_bind_property_full (self, "title", priv->title, "visible", G_BINDING_SYNC_CREATE,
                               string_is_not_empty, NULL, NULL, NULL);

  update_subtitle_visibility (self);

  g_signal_connect (self, "notify::parent", G_CALLBACK (parent_cb), NULL);

}

static void
adw_action_row_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const gchar  *type)
{
  AdwActionRow *self = ADW_ACTION_ROW (buildable);
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);

  if (!priv->header)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (g_strcmp0 (type, "prefix") == 0)
    adw_action_row_add_prefix (self, GTK_WIDGET (child));
  else if (g_strcmp0 (type, "suffix") == 0)
    adw_action_row_add_suffix (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_action_row_add_suffix (self, GTK_WIDGET (child));
  else
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
}

static void
adw_action_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adw_action_row_buildable_add_child;
}

/**
 * adw_action_row_new:
 *
 * Creates a new #AdwActionRow.
 *
 * Returns: a new #AdwActionRow
 *
 * Since: 1.0
 */
GtkWidget *
adw_action_row_new (void)
{
  return g_object_new (ADW_TYPE_ACTION_ROW, NULL);
}

/**
 * adw_action_row_get_subtitle:
 * @self: a #AdwActionRow
 *
 * Gets the subtitle for @self.
 *
 * Returns: (transfer none) (nullable): the subtitle for @self, or %NULL.
 *
 * Since: 1.0
 */
const gchar *
adw_action_row_get_subtitle (AdwActionRow *self)
{
  AdwActionRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ACTION_ROW (self), NULL);

  priv = adw_action_row_get_instance_private (self);

  return gtk_label_get_text (priv->subtitle);
}

/**
 * adw_action_row_set_subtitle:
 * @self: a #AdwActionRow
 * @subtitle: (nullable): the subtitle
 *
 * Sets the subtitle for @self.
 *
 * Since: 1.0
 */
void
adw_action_row_set_subtitle (AdwActionRow *self,
                             const gchar  *subtitle)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));

  priv = adw_action_row_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_text (priv->subtitle), subtitle) == 0)
    return;

  gtk_label_set_text (priv->subtitle, subtitle);
  gtk_widget_set_visible (GTK_WIDGET (priv->subtitle),
                          subtitle != NULL && g_strcmp0 (subtitle, "") != 0);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * adw_action_row_get_icon_name:
 * @self: a #AdwActionRow
 *
 * Gets the icon name for @self.
 *
 * Returns: (transfer none): the icon name for @self.
 * The returned string is owned by the #AdwActionRow and should not be freed.
 *
 * Since: 1.0
 */
const gchar *
adw_action_row_get_icon_name (AdwActionRow *self)
{
  AdwActionRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ACTION_ROW (self), NULL);

  priv = adw_action_row_get_instance_private (self);

  return gtk_image_get_icon_name (priv->image);
}

/**
 * adw_action_row_set_icon_name:
 * @self: a #AdwActionRow
 * @icon_name: the icon name
 *
 * Sets the icon name for @self.
 *
 * Since: 1.0
 */
void
adw_action_row_set_icon_name (AdwActionRow *self,
                              const gchar  *icon_name)
{
  AdwActionRowPrivate *priv;
  const gchar *old_icon_name;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));

  priv = adw_action_row_get_instance_private (self);

  old_icon_name = gtk_image_get_icon_name (priv->image);
  if (g_strcmp0 (old_icon_name, icon_name) == 0)
    return;

  gtk_image_set_from_icon_name (priv->image, icon_name);
  gtk_widget_set_visible (GTK_WIDGET (priv->image),
                          icon_name != NULL && g_strcmp0 (icon_name, "") != 0);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adw_action_row_get_activatable_widget:
 * @self: a #AdwActionRow
 *
 * Gets the widget activated when @self is activated.
 *
 * Returns: (nullable) (transfer none): the widget activated when @self is
 *          activated, or %NULL if none has been set.
 *
 * Since: 1.0
 */
GtkWidget *
adw_action_row_get_activatable_widget (AdwActionRow *self)
{
  AdwActionRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ACTION_ROW (self), NULL);

  priv = adw_action_row_get_instance_private (self);

  return priv->activatable_widget;
}

static void
activatable_widget_weak_notify (gpointer  data,
                                GObject  *where_the_object_was)
{
  AdwActionRow *self = ADW_ACTION_ROW (data);
  AdwActionRowPrivate *priv = adw_action_row_get_instance_private (self);

  priv->activatable_widget = NULL;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATABLE_WIDGET]);
}

/**
 * adw_action_row_set_activatable_widget:
 * @self: a #AdwActionRow
 * @widget: (nullable): the target #GtkWidget, or %NULL to unset
 *
 * Sets the widget to activate when @self is activated, either by clicking
 * on it, by calling adw_action_row_activate(), or via mnemonics in the title or
 * the subtitle. See the “use_underline” property to enable mnemonics.
 *
 * The target widget will be activated by emitting the
 * GtkWidget::mnemonic-activate signal on it.
 *
 * Since: 1.0
 */
void
adw_action_row_set_activatable_widget (AdwActionRow *self,
                                       GtkWidget    *widget)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));
  g_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  priv = adw_action_row_get_instance_private (self);

  if (priv->activatable_widget == widget)
    return;

  if (priv->activatable_widget)
    g_object_weak_unref (G_OBJECT (priv->activatable_widget),
                         activatable_widget_weak_notify,
                         self);

  priv->activatable_widget = widget;

  if (priv->activatable_widget != NULL) {
    g_object_weak_ref (G_OBJECT (priv->activatable_widget),
                       activatable_widget_weak_notify,
                       self);
    gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (self), TRUE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATABLE_WIDGET]);
}

/**
 * adw_action_row_get_use_underline:
 * @self: a #AdwActionRow
 *
 * Gets whether an embedded underline in the text of the title and subtitle
 * labels indicates a mnemonic. See adw_action_row_set_use_underline().
 *
 * Returns: %TRUE if an embedded underline in the title and subtitle labels
 *          indicates the mnemonic accelerator keys.
 *
 * Since: 1.0
 */
gboolean
adw_action_row_get_use_underline (AdwActionRow *self)
{
  AdwActionRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ACTION_ROW (self), FALSE);

  priv = adw_action_row_get_instance_private (self);

  return priv->use_underline;
}

/**
 * adw_action_row_set_use_underline:
 * @self: a #AdwActionRow
 * @use_underline: %TRUE if underlines in the text indicate mnemonics
 *
 * If true, an underline in the text of the title and subtitle labels indicates
 * the next character should be used for the mnemonic accelerator key.
 *
 * Since: 1.0
 */
void
adw_action_row_set_use_underline (AdwActionRow *self,
                                  gboolean      use_underline)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));

  priv = adw_action_row_get_instance_private (self);

  use_underline = !!use_underline;

  if (priv->use_underline == use_underline)
    return;

  priv->use_underline = use_underline;
  adw_preferences_row_set_use_underline (ADW_PREFERENCES_ROW (self), priv->use_underline);
  gtk_label_set_use_underline (priv->title, priv->use_underline);
  gtk_label_set_use_underline (priv->subtitle, priv->use_underline);
  gtk_label_set_mnemonic_widget (priv->title, GTK_WIDGET (self));
  gtk_label_set_mnemonic_widget (priv->subtitle, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_UNDERLINE]);
}

/**
 * adw_action_row_get_title_lines:
 * @self: a #AdwActionRow
 *
 * Gets the number of lines at the end of which the title label will be
 * ellipsized.
 * If the value is 0, the number of lines won't be limited.
 *
 * Returns: the number of lines at the end of which the title label will be
 *          ellipsized.
 *
 * Since: 1.0
 */
gint
adw_action_row_get_title_lines (AdwActionRow *self)
{
  AdwActionRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ACTION_ROW (self), 0);

  priv = adw_action_row_get_instance_private (self);

  return priv->title_lines;
}

/**
 * adw_action_row_set_title_lines:
 * @self: a #AdwActionRow
 * @title_lines: the number of lines at the end of which the title label will be ellipsized
 *
 * Sets the number of lines at the end of which the title label will be
 * ellipsized.
 * If the value is 0, the number of lines won't be limited.
 *
 * Since: 1.0
 */
void
adw_action_row_set_title_lines (AdwActionRow *self,
                                gint          title_lines)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));
  g_return_if_fail (title_lines >= 0);

  priv = adw_action_row_get_instance_private (self);

  if (priv->title_lines == title_lines)
    return;

  priv->title_lines = title_lines;

  gtk_label_set_lines (priv->title, title_lines);
  gtk_label_set_ellipsize (priv->title, title_lines == 0 ? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE_LINES]);
}

/**
 * adw_action_row_get_subtitle_lines:
 * @self: a #AdwActionRow
 *
 * Gets the number of lines at the end of which the subtitle label will be
 * ellipsized.
 * If the value is 0, the number of lines won't be limited.
 *
 * Returns: the number of lines at the end of which the subtitle label will be
 *          ellipsized.
 *
 * Since: 1.0
 */
gint
adw_action_row_get_subtitle_lines (AdwActionRow *self)
{
  AdwActionRowPrivate *priv;

  g_return_val_if_fail (ADW_IS_ACTION_ROW (self), 0);

  priv = adw_action_row_get_instance_private (self);

  return priv->subtitle_lines;
}

/**
 * adw_action_row_set_subtitle_lines:
 * @self: a #AdwActionRow
 * @subtitle_lines: the number of lines at the end of which the subtitle label will be ellipsized
 *
 * Sets the number of lines at the end of which the subtitle label will be
 * ellipsized.
 * If the value is 0, the number of lines won't be limited.
 *
 * Since: 1.0
 */
void
adw_action_row_set_subtitle_lines (AdwActionRow *self,
                                   gint          subtitle_lines)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));
  g_return_if_fail (subtitle_lines >= 0);

  priv = adw_action_row_get_instance_private (self);

  if (priv->subtitle_lines == subtitle_lines)
    return;

  priv->subtitle_lines = subtitle_lines;

  gtk_label_set_lines (priv->subtitle, subtitle_lines);
  gtk_label_set_ellipsize (priv->subtitle, subtitle_lines == 0 ? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE_LINES]);
}

/**
 * adw_action_row_add_prefix:
 * @self: a #AdwActionRow
 * @widget: the prefix widget
 *
 * Adds a prefix widget to @self.
 *
 * Since: 1.0
 */
void
adw_action_row_add_prefix (AdwActionRow *self,
                           GtkWidget    *widget)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  priv = adw_action_row_get_instance_private (self);

  gtk_box_prepend (priv->prefixes, widget);
  gtk_widget_show (GTK_WIDGET (priv->prefixes));
}

/**
 * adw_action_row_add_suffix:
 * @self: a #AdwActionRow
 * @widget: the suffix widget
 *
 * Adds a suffix widget to @self.
 *
 * Since: 1.0
 */
void
adw_action_row_add_suffix (AdwActionRow *self,
                           GtkWidget    *widget)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  priv = adw_action_row_get_instance_private (self);

  gtk_box_append (priv->suffixes, widget);
  gtk_widget_show (GTK_WIDGET (priv->suffixes));
}

/**
 * adw_action_row_remove
 * @self: a #AdwActionRow
 * @widget: the #GtkWidget to be removed
 *
 * Removes a child from @self.
 *
 * Since: 1.0
 */
void
adw_action_row_remove (AdwActionRow *self,
                       GtkWidget    *child)
{
  AdwActionRowPrivate *priv;

  g_return_if_fail (ADW_IS_ACTION_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adw_action_row_get_instance_private (self);

  if (gtk_widget_get_parent (child) == GTK_WIDGET (priv->prefixes))
    gtk_box_remove (priv->prefixes, child);
  else
    gtk_box_remove (priv->suffixes, child);
}

void
adw_action_row_activate (AdwActionRow *self)
{
  g_return_if_fail (ADW_IS_ACTION_ROW (self));

  ADW_ACTION_ROW_GET_CLASS (self)->activate (self);
}
