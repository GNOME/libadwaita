/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-expander-row.h"

#include <glib/gi18n-lib.h>
#include "hdy-action-row.h"

/**
 * SECTION:hdy-expander-row
 * @short_description: A #GtkListBox row used to reveal widgets.
 * @Title: HdyExpanderRow
 *
 * The #HdyExpanderRow allows the user to reveal or hide widgets below it. It
 * also allows the user to enable the expansion of the row, allowing to disable
 * all that the row contains.
 *
 * It also supports adding a child as an action widget by specifying “action” as
 * the “type” attribute of a &lt;child&gt; element. It also supports setting a
 * child as a prefix widget by specifying “prefix” as the “type” attribute of a
 * &lt;child&gt; element.
 *
 * # CSS nodes
 *
 * #HdyExpanderRow has a main CSS node with name row, and the .expander style
 * class. It has the .empty style class when it contains no children.
 *
 * It contains the subnodes row.header for its main embedded row, list.nested
 * for the list it can expand, and image.expander-row-arrow for its arrow.
 *
 * When expanded, #HdyExpanderRow will add the
 * .checked-expander-row-previous-sibling style class to its previous sibling,
 * and remove it when retracted.
 *
 * Since: 0.0.6
 */

typedef struct
{
  GtkBox *box;
  GtkBox *actions;
  GtkBox *prefixes;
  GtkListBox *list;
  HdyActionRow *action_row;
  GtkSwitch *enable_switch;
  GtkImage *image;

  gboolean expanded;
  gboolean enable_expansion;
  gboolean show_enable_switch;
} HdyExpanderRowPrivate;

static void hdy_expander_row_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyExpanderRow, hdy_expander_row, HDY_TYPE_PREFERENCES_ROW,
                         G_ADD_PRIVATE (HdyExpanderRow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_expander_row_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_SUBTITLE,
  PROP_USE_UNDERLINE,
  PROP_ICON_NAME,
  PROP_EXPANDED,
  PROP_ENABLE_EXPANSION,
  PROP_SHOW_ENABLE_SWITCH,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
update_arrow (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv = hdy_expander_row_get_instance_private (self);
  GtkWidget *previous_sibling = gtk_widget_get_prev_sibling (GTK_WIDGET (self));

  if (priv->expanded)
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED, FALSE);
  else
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_CHECKED);

  if (previous_sibling) {
    if (priv->expanded)
      gtk_widget_add_css_class (previous_sibling, "checked-expander-row-previous-sibling");
    else
      gtk_widget_remove_css_class (previous_sibling, "checked-expander-row-previous-sibling");
  }
}

static void
hdy_expander_row_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  HdyExpanderRow *self = HDY_EXPANDER_ROW (object);

  switch (prop_id) {
  case PROP_SUBTITLE:
    g_value_set_string (value, hdy_expander_row_get_subtitle (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, hdy_expander_row_get_use_underline (self));
    break;
  case PROP_ICON_NAME:
    g_value_set_string (value, hdy_expander_row_get_icon_name (self));
    break;
  case PROP_EXPANDED:
    g_value_set_boolean (value, hdy_expander_row_get_expanded (self));
    break;
  case PROP_ENABLE_EXPANSION:
    g_value_set_boolean (value, hdy_expander_row_get_enable_expansion (self));
    break;
  case PROP_SHOW_ENABLE_SWITCH:
    g_value_set_boolean (value, hdy_expander_row_get_show_enable_switch (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_expander_row_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HdyExpanderRow *self = HDY_EXPANDER_ROW (object);

  switch (prop_id) {
  case PROP_SUBTITLE:
    hdy_expander_row_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_USE_UNDERLINE:
    hdy_expander_row_set_use_underline (self, g_value_get_boolean (value));
    break;
  case PROP_ICON_NAME:
    hdy_expander_row_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_EXPANDED:
    hdy_expander_row_set_expanded (self, g_value_get_boolean (value));
    break;
  case PROP_ENABLE_EXPANSION:
    hdy_expander_row_set_enable_expansion (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_ENABLE_SWITCH:
    hdy_expander_row_set_show_enable_switch (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_expander_row_dispose (GObject *object)
{
  HdyExpanderRow *self = HDY_EXPANDER_ROW (object);
  HdyExpanderRowPrivate *priv = hdy_expander_row_get_instance_private (self);

  g_clear_pointer ((GtkWidget **) &priv->box, gtk_widget_unparent);

  G_OBJECT_CLASS (hdy_expander_row_parent_class)->dispose (object);
}

static void
activate_cb (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv = hdy_expander_row_get_instance_private (self);

  hdy_expander_row_set_expanded (self, !priv->expanded);
}

static void
hdy_expander_row_class_init (HdyExpanderRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_expander_row_get_property;
  object_class->set_property = hdy_expander_row_set_property;
  object_class->dispose = hdy_expander_row_dispose;

  /**
   * HdyExpanderRow:subtitle:
   *
   * The subtitle for this row.
   *
   * Since: 1.0
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         _("Subtitle"),
                         _("The subtitle for this row"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyExpanderRow:use-underline:
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
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyExpanderRow:icon-name:
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
   * HdyExpanderRow:expanded:
   *
   * %TRUE if the row is expanded.
   */
  props[PROP_EXPANDED] =
    g_param_spec_boolean ("expanded",
                          _("Expanded"),
                          _("Whether the row is expanded"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyExpanderRow:enable-expansion:
   *
   * %TRUE if the expansion is enabled.
   */
  props[PROP_ENABLE_EXPANSION] =
    g_param_spec_boolean ("enable-expansion",
                          _("Enable expansion"),
                          _("Whether the expansion is enabled"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyExpanderRow:show-enable-switch:
   *
   * %TRUE if the switch enabling the expansion is visible.
   */
  props[PROP_SHOW_ENABLE_SWITCH] =
    g_param_spec_boolean ("show-enable-switch",
                          _("Show enable switch"),
                          _("Whether the switch enabling the expansion is visible"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-expander-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyExpanderRow, action_row);
  gtk_widget_class_bind_template_child_private (widget_class, HdyExpanderRow, box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyExpanderRow, actions);
  gtk_widget_class_bind_template_child_private (widget_class, HdyExpanderRow, list);
  gtk_widget_class_bind_template_child_private (widget_class, HdyExpanderRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, HdyExpanderRow, enable_switch);
  gtk_widget_class_bind_template_callback (widget_class, activate_cb);
}

#define NOTIFY(func, prop) \
static void \
func (gpointer this) { \
  g_object_notify_by_pspec (G_OBJECT (this), props[prop]); \
} \

NOTIFY (notify_subtitle_cb, PROP_SUBTITLE);
NOTIFY (notify_use_underline_cb, PROP_USE_UNDERLINE);
NOTIFY (notify_icon_name_cb, PROP_ICON_NAME);

static void
hdy_expander_row_init (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv = hdy_expander_row_get_instance_private (self);

  priv->prefixes = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  hdy_expander_row_set_enable_expansion (self, TRUE);
  hdy_expander_row_set_expanded (self, FALSE);

  g_signal_connect_object (priv->action_row, "notify::subtitle", G_CALLBACK (notify_subtitle_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->action_row, "notify::use-underline", G_CALLBACK (notify_use_underline_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->action_row, "notify::icon-name", G_CALLBACK (notify_icon_name_cb), self, G_CONNECT_SWAPPED);
}

static void
hdy_expander_row_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const gchar  *type)
{
  HdyExpanderRow *self = HDY_EXPANDER_ROW (buildable);
  HdyExpanderRowPrivate *priv = hdy_expander_row_get_instance_private (self);

  if (!priv->box)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (type && strcmp (type, "action") == 0)
    hdy_expander_row_add_action (self, GTK_WIDGET (child));
  else if (type && strcmp (type, "prefix") == 0)
    hdy_expander_row_add_prefix (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    hdy_expander_row_add (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_expander_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = hdy_expander_row_buildable_add_child;
}

/**
 * hdy_expander_row_new:
 *
 * Creates a new #HdyExpanderRow.
 *
 * Returns: a new #HdyExpanderRow
 *
 * Since: 0.0.6
 */
GtkWidget *
hdy_expander_row_new (void)
{
  return g_object_new (HDY_TYPE_EXPANDER_ROW, NULL);
}

/**
 * hdy_expander_row_get_subtitle:
 * @self: a #HdyExpanderRow
 *
 * Gets the subtitle for @self.
 *
 * Returns: (transfer none) (nullable): the subtitle for @self, or %NULL.
 *
 * Since: 1.0
 */
const gchar *
hdy_expander_row_get_subtitle (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_EXPANDER_ROW (self), NULL);

  priv = hdy_expander_row_get_instance_private (self);

  return hdy_action_row_get_subtitle (priv->action_row);
}

/**
 * hdy_expander_row_set_subtitle:
 * @self: a #HdyExpanderRow
 * @subtitle: (nullable): the subtitle
 *
 * Sets the subtitle for @self.
 *
 * Since: 1.0
 */
void
hdy_expander_row_set_subtitle (HdyExpanderRow *self,
                               const gchar    *subtitle)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));

  priv = hdy_expander_row_get_instance_private (self);

  hdy_action_row_set_subtitle (priv->action_row, subtitle);
}

/**
 * hdy_expander_row_get_use_underline:
 * @self: a #HdyExpanderRow
 *
 * Gets whether an embedded underline in the text of the title and subtitle
 * labels indicates a mnemonic. See hdy_expander_row_set_use_underline().
 *
 * Returns: %TRUE if an embedded underline in the title and subtitle labels
 *          indicates the mnemonic accelerator keys.
 *
 * Since: 1.0
 */
gboolean
hdy_expander_row_get_use_underline (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_EXPANDER_ROW (self), FALSE);

  priv = hdy_expander_row_get_instance_private (self);

  return hdy_action_row_get_use_underline (priv->action_row);
}

/**
 * hdy_expander_row_set_use_underline:
 * @self: a #HdyExpanderRow
 * @use_underline: %TRUE if underlines in the text indicate mnemonics
 *
 * If true, an underline in the text of the title and subtitle labels indicates
 * the next character should be used for the mnemonic accelerator key.
 *
 * Since: 1.0
 */
void
hdy_expander_row_set_use_underline (HdyExpanderRow *self,
                                    gboolean        use_underline)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));

  priv = hdy_expander_row_get_instance_private (self);

  hdy_action_row_set_use_underline (priv->action_row, use_underline);
}

/**
 * hdy_expander_row_get_icon_name:
 * @self: a #HdyExpanderRow
 *
 * Gets the icon name for @self.
 *
 * Returns: the icon name for @self.
 *
 * Since: 1.0
 */
const gchar *
hdy_expander_row_get_icon_name (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_EXPANDER_ROW (self), NULL);

  priv = hdy_expander_row_get_instance_private (self);

  return hdy_action_row_get_icon_name (priv->action_row);
}

/**
 * hdy_expander_row_set_icon_name:
 * @self: a #HdyExpanderRow
 * @icon_name: the icon name
 *
 * Sets the icon name for @self.
 *
 * Since: 1.0
 */
void
hdy_expander_row_set_icon_name (HdyExpanderRow *self,
                                const gchar    *icon_name)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));

  priv = hdy_expander_row_get_instance_private (self);

  hdy_action_row_set_icon_name (priv->action_row, icon_name);
}

gboolean
hdy_expander_row_get_expanded (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_EXPANDER_ROW (self), FALSE);

  priv = hdy_expander_row_get_instance_private (self);

  return priv->expanded;
}

void
hdy_expander_row_set_expanded (HdyExpanderRow *self,
                               gboolean        expanded)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));

  priv = hdy_expander_row_get_instance_private (self);

  expanded = !!expanded && priv->enable_expansion;

  if (priv->expanded == expanded)
    return;

  priv->expanded = expanded;

  update_arrow (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXPANDED]);
}

/**
 * hdy_expander_row_get_enable_expansion:
 * @self: a #HdyExpanderRow
 *
 * Gets whether the expansion of @self is enabled.
 *
 * Returns: whether the expansion of @self is enabled.
 *
 * Since: 0.0.6
 */
gboolean
hdy_expander_row_get_enable_expansion (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_EXPANDER_ROW (self), FALSE);

  priv = hdy_expander_row_get_instance_private (self);

  return priv->enable_expansion;
}

/**
 * hdy_expander_row_set_enable_expansion:
 * @self: a #HdyExpanderRow
 * @enable_expansion: %TRUE to enable the expansion
 *
 * Sets whether the expansion of @self is enabled.
 *
 * Since: 0.0.6
 */
void
hdy_expander_row_set_enable_expansion (HdyExpanderRow *self,
                                       gboolean        enable_expansion)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));

  priv = hdy_expander_row_get_instance_private (self);

  enable_expansion = !!enable_expansion;

  if (priv->enable_expansion == enable_expansion)
    return;

  priv->enable_expansion = enable_expansion;

  hdy_expander_row_set_expanded (self, priv->enable_expansion);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLE_EXPANSION]);
}

/**
 * hdy_expander_row_get_show_enable_switch:
 * @self: a #HdyExpanderRow
 *
 * Gets whether the switch enabling the expansion of @self is visible.
 *
 * Returns: whether the switch enabling the expansion of @self is visible.
 *
 * Since: 0.0.6
 */
gboolean
hdy_expander_row_get_show_enable_switch (HdyExpanderRow *self)
{
  HdyExpanderRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_EXPANDER_ROW (self), FALSE);

  priv = hdy_expander_row_get_instance_private (self);

  return priv->show_enable_switch;
}

/**
 * hdy_expander_row_set_show_enable_switch:
 * @self: a #HdyExpanderRow
 * @show_enable_switch: %TRUE to show the switch enabling the expansion
 *
 * Sets whether the switch enabling the expansion of @self is visible.
 *
 * Since: 0.0.6
 */
void
hdy_expander_row_set_show_enable_switch (HdyExpanderRow *self,
                                         gboolean        show_enable_switch)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));

  priv = hdy_expander_row_get_instance_private (self);

  show_enable_switch = !!show_enable_switch;

  if (priv->show_enable_switch == show_enable_switch)
    return;

  priv->show_enable_switch = show_enable_switch;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_ENABLE_SWITCH]);
}

/**
 * hdy_expander_row_add_action:
 * @self: a #HdyExpanderRow
 * @widget: the action widget
 *
 * Adds an action widget to @self.
 *
 * Since: 1.0
 */
void
hdy_expander_row_add_action (HdyExpanderRow *self,
                             GtkWidget      *widget)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (self));

  priv = hdy_expander_row_get_instance_private (self);

  gtk_box_prepend (priv->actions, widget);
  gtk_widget_show (GTK_WIDGET (priv->actions));
}

/**
 * hdy_expander_row_add_prefix:
 * @self: a #HdyExpanderRow
 * @widget: the prefix widget
 *
 * Adds a prefix widget to @self.
 *
 * Since: 1.0
 */
void
hdy_expander_row_add_prefix (HdyExpanderRow *self,
                             GtkWidget      *widget)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  priv = hdy_expander_row_get_instance_private (self);

  if (priv->prefixes == NULL) {
    priv->prefixes = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12));
    hdy_action_row_add_prefix (HDY_ACTION_ROW (priv->action_row), GTK_WIDGET (priv->prefixes));
  }
  gtk_box_append (priv->prefixes, widget);
}

void
hdy_expander_row_add (HdyExpanderRow *self,
                      GtkWidget      *child)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = hdy_expander_row_get_instance_private (self);

  /* When constructing the widget, we want the box to be added as the child of
   * the GtkListBoxRow, as an implementation detail.
   */
  gtk_list_box_append (priv->list, child);

  gtk_widget_remove_css_class (GTK_WIDGET (self), "empty");
}

void
hdy_expander_row_remove (HdyExpanderRow *self,
                         GtkWidget      *child)
{
  HdyExpanderRowPrivate *priv;

  g_return_if_fail (HDY_IS_EXPANDER_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = hdy_expander_row_get_instance_private (self);

  if (gtk_widget_get_parent (child) == GTK_WIDGET (priv->actions))
    gtk_box_remove (priv->actions, child);
  else if (gtk_widget_get_parent (child) == GTK_WIDGET (priv->prefixes))
    gtk_box_remove (priv->prefixes, child);
  else {
    gtk_list_box_remove (priv->list, child);

    if (!gtk_widget_get_first_child (GTK_WIDGET (priv->list)))
      gtk_widget_add_css_class (GTK_WIDGET (self), "empty");
  }
}
