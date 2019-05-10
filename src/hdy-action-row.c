/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-action-row.h"

#include <glib/gi18n-lib.h>

/**
 * SECTION:hdy-action-row
 * @short_description: A #GtkListBox row used to present actions
 * @Title: HdyActionRow
 *
 * The #HdyActionRow widget can have a title, a subtitle and an icon. The row
 * can receive action widgets at its end, prefix widgets at its start or widgets
 * below it.
 *
 * Note that action widgets are packed starting from the end.
 *
 * It is convenient to present a list of preferences and their related actions.
 *
 * # HdyActionRow as GtkBuildable
 *
 * The GtkWindow implementation of the GtkBuildable interface supports setting a
 * child as an action widget by specifying “action” as the “type” attribute of a
 * &lt;child&gt; element.
 *
 * It also supports setting a child as a prefix widget by specifying “prefix” as
 * the “type” attribute of a &lt;child&gt; element.
 *
 * Since: 0.0.6
 */

typedef struct
{
  GtkBox *box;
  GtkBox *header;
  GtkImage *image;
  GtkBox *prefixes;
  GtkLabel *subtitle;
  GtkLabel *title;
  GtkBox *title_box;

  GtkWidget *previous_parent;

  gboolean use_underline;
  GtkWidget *activatable_widget;
} HdyActionRowPrivate;

static void hdy_action_row_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyActionRow, hdy_action_row, HDY_TYPE_PREFERENCES_ROW,
                         G_ADD_PRIVATE (HdyActionRow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_action_row_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_ACTIVATABLE_WIDGET,
  PROP_SUBTITLE,
  PROP_TITLE,
  PROP_USE_UNDERLINE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
row_activated_cb (HdyActionRow  *self,
                  GtkListBoxRow *row)
{
  /* No need to use GTK_LIST_BOX_ROW() for a pointer comparison. */
  if ((GtkListBoxRow *) self == row)
    hdy_action_row_activate (self);
}

static void
parent_cb (HdyActionRow *self)
{
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);
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
update_subtitle_visibility (HdyActionRow *self)
{
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->subtitle),
                          gtk_label_get_text (priv->subtitle) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->subtitle), "") != 0);
}

static void
hdy_action_row_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  HdyActionRow *self = HDY_ACTION_ROW (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, hdy_action_row_get_icon_name (self));
    break;
  case PROP_ACTIVATABLE_WIDGET:
    g_value_set_object (value, (GObject *) hdy_action_row_get_activatable_widget (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, hdy_action_row_get_subtitle (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, hdy_action_row_get_title (self));
    break;
  case PROP_USE_UNDERLINE:
    g_value_set_boolean (value, hdy_action_row_get_use_underline (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_action_row_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  HdyActionRow *self = HDY_ACTION_ROW (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    hdy_action_row_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_ACTIVATABLE_WIDGET:
    hdy_action_row_set_activatable_widget (self, (GtkWidget*) g_value_get_object (value));
    break;
  case PROP_SUBTITLE:
    hdy_action_row_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_TITLE:
    hdy_action_row_set_title (self, g_value_get_string (value));
    break;
  case PROP_USE_UNDERLINE:
    hdy_action_row_set_use_underline (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_action_row_dispose (GObject *object)
{
  HdyActionRow *self = HDY_ACTION_ROW (object);
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);

  if (priv->previous_parent != NULL) {
    g_signal_handlers_disconnect_by_func (priv->previous_parent, G_CALLBACK (row_activated_cb), self);
    priv->previous_parent = NULL;
  }

  G_OBJECT_CLASS (hdy_action_row_parent_class)->dispose (object);
}

static void
hdy_action_row_show_all (GtkWidget *widget)
{
  HdyActionRow *self = HDY_ACTION_ROW (widget);
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  gtk_container_foreach (GTK_CONTAINER (priv->prefixes),
                         (GtkCallback) gtk_widget_show_all,
                         NULL);
  GTK_WIDGET_CLASS (hdy_action_row_parent_class)->show_all (widget);
}

static void
hdy_action_row_destroy (GtkWidget *widget)
{
  HdyActionRow *self = HDY_ACTION_ROW (widget);
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);

  if (priv->box) {
    gtk_widget_destroy (GTK_WIDGET (priv->box));
    priv->box = NULL;
  }

  hdy_action_row_set_activatable_widget (self, NULL);

  priv->prefixes = NULL;
  priv->header = NULL;

  GTK_WIDGET_CLASS (hdy_action_row_parent_class)->destroy (widget);
}

static void
hdy_action_row_add (GtkContainer *container,
                    GtkWidget    *child)
{
  HdyActionRow *self = HDY_ACTION_ROW (container);
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);

  /* When constructing the widget, we want the box to be added as the child of
   * the GtkListBoxRow, as an implementation detail.
   */
  if (priv->box == NULL)
    GTK_CONTAINER_CLASS (hdy_action_row_parent_class)->add (container, child);
  else
    gtk_container_add (GTK_CONTAINER (priv->box), child);
}

typedef struct {
  HdyActionRow *row;
  GtkCallback callback;
  gpointer callback_data;
} ForallData;

static void
for_non_internal_child (GtkWidget *widget,
                        gpointer   callback_data)
{
  ForallData *data = callback_data;
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (data->row);

  if (widget != (GtkWidget *) priv->box &&
      widget != (GtkWidget *) priv->image &&
      widget != (GtkWidget *) priv->prefixes &&
      widget != (GtkWidget *) priv->title_box)
    data->callback (widget, data->callback_data);
}

static void
hdy_action_row_forall (GtkContainer *container,
                       gboolean      include_internals,
                       GtkCallback   callback,
                       gpointer      callback_data)
{
  HdyActionRow *self = HDY_ACTION_ROW (container);
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);
  ForallData data;

  if (include_internals) {
    GTK_CONTAINER_CLASS (hdy_action_row_parent_class)->forall (GTK_CONTAINER (self), include_internals, callback, callback_data);

    return;
  }

  data.row = self;
  data.callback = callback;
  data.callback_data = callback_data;

  if (priv->prefixes)
    GTK_CONTAINER_GET_CLASS (priv->prefixes)->forall (GTK_CONTAINER (priv->prefixes), include_internals, for_non_internal_child, &data);
  if (priv->header)
    GTK_CONTAINER_GET_CLASS (priv->header)->forall (GTK_CONTAINER (priv->header), include_internals, for_non_internal_child, &data);
  if (priv->box)
    GTK_CONTAINER_GET_CLASS (priv->box)->forall (GTK_CONTAINER (priv->box), include_internals, for_non_internal_child, &data);
}

static void
hdy_action_row_activate_real (HdyActionRow *self)
{
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);

  if (priv->activatable_widget)
    gtk_widget_mnemonic_activate (priv->activatable_widget, FALSE);
}

static void
hdy_action_row_class_init (HdyActionRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_action_row_get_property;
  object_class->set_property = hdy_action_row_set_property;
  object_class->dispose = hdy_action_row_dispose;

  widget_class->destroy = hdy_action_row_destroy;
  widget_class->show_all = hdy_action_row_show_all;

  container_class->add = hdy_action_row_add;
  container_class->forall = hdy_action_row_forall;

  klass->activate = hdy_action_row_activate_real;

  /**
   * HdyActionRow:icon-name:
   *
   * The icon name for this row.
   *
   * Since: 0.0.6
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         _("Icon name"),
                         _("Icon name"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyActionRow:activatable-widget:
   *
   * The activatable widget for this row.
   *
   * Since: 0.0.7
   */
  props[PROP_ACTIVATABLE_WIDGET] =
      g_param_spec_object ("activatable-widget",
                           _("Activatable widget"),
                           _("The widget to be activated when the row is activated"),
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE);

  /**
   * HdyActionRow:subtitle:
   *
   * The subtitle for this row.
   *
   * Since: 0.0.6
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         _("Subtitle"),
                         _("Subtitle"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyActionRow:title:
   *
   * The title for this row.
   *
   * Since: 0.0.6
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("Title"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyActionRow:use-underline:
   *
   * Whether an embedded underline in the text of the title and subtitle labels
   * indicates a mnemonic.
   *
   * Since: 0.0.6
   */
  props[PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline",
                          _("Use underline"),
                          _("If set, an underline in the text indicates the next character should be used for the mnemonic accelerator key"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-action-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, header);
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, prefixes);
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, subtitle);
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, title);
  gtk_widget_class_bind_template_child_private (widget_class, HdyActionRow, title_box);
}

static void
hdy_action_row_init (HdyActionRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  update_subtitle_visibility (self);

  g_signal_connect (self, "notify::parent", G_CALLBACK (parent_cb), NULL);

}

static void
hdy_action_row_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const gchar  *type)
{
  if (type && strcmp (type, "action") == 0)
    hdy_action_row_add_action (HDY_ACTION_ROW (buildable), GTK_WIDGET (child));
  else if (type && strcmp (type, "prefix") == 0)
    hdy_action_row_add_prefix (HDY_ACTION_ROW (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_action_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = hdy_action_row_buildable_add_child;
}

/**
 * hdy_action_row_new:
 *
 * Creates a new #HdyActionRow.
 *
 * Returns: a new #HdyActionRow
 *
 * Since: 0.0.6
 */
HdyActionRow *
hdy_action_row_new (void)
{
  return g_object_new (HDY_TYPE_ACTION_ROW, NULL);
}

/**
 * hdy_action_row_get_title:
 * @self: a #HdyActionRow
 *
 * Gets the title for @self.
 *
 * Returns: the title for @self.
 *
 * Since: 0.0.6
 */
const gchar *
hdy_action_row_get_title (HdyActionRow *self)
{
  HdyActionRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_ACTION_ROW (self), NULL);

  priv = hdy_action_row_get_instance_private (self);

  return gtk_label_get_text (priv->title);
}

/**
 * hdy_action_row_set_title:
 * @self: a #HdyActionRow
 * @title: the title
 *
 * Sets the title for @self.
 *
 * Since: 0.0.6
 */
void
hdy_action_row_set_title (HdyActionRow *self,
                          const gchar  *title)
{
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);
  hdy_preferences_row_set_title (HDY_PREFERENCES_ROW (self), title);

  if (g_strcmp0 (gtk_label_get_text (priv->title), title) == 0)
    return;

  gtk_label_set_text (priv->title, title);
  gtk_widget_set_visible (GTK_WIDGET (priv->title),
                          title != NULL && g_strcmp0 (title, "") != 0);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * hdy_action_row_get_subtitle:
 * @self: a #HdyActionRow
 *
 * Gets the subtitle for @self.
 *
 * Returns: the subtitle for @self.
 *
 * Since: 0.0.6
 */
const gchar *
hdy_action_row_get_subtitle (HdyActionRow *self)
{
  HdyActionRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_ACTION_ROW (self), NULL);

  priv = hdy_action_row_get_instance_private (self);

  return gtk_label_get_text (priv->subtitle);
}

/**
 * hdy_action_row_set_subtitle:
 * @self: a #HdyActionRow
 * @subtitle: the subtitle
 *
 * Sets the subtitle for @self.
 *
 * Since: 0.0.6
 */
void
hdy_action_row_set_subtitle (HdyActionRow *self,
                             const gchar  *subtitle)
{
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_text (priv->subtitle), subtitle) == 0)
    return;

  gtk_label_set_text (priv->subtitle, subtitle);
  gtk_widget_set_visible (GTK_WIDGET (priv->subtitle),
                          subtitle != NULL && g_strcmp0 (subtitle, "") != 0);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * hdy_action_row_get_icon_name:
 * @self: a #HdyActionRow
 *
 * Gets the icon name for @self.
 *
 * Returns: the icon name for @self.
 *
 * Since: 0.0.6
 */
const gchar *
hdy_action_row_get_icon_name (HdyActionRow *self)
{
  HdyActionRowPrivate *priv;
  const gchar *icon_name;

  g_return_val_if_fail (HDY_IS_ACTION_ROW (self), NULL);

  priv = hdy_action_row_get_instance_private (self);

  gtk_image_get_icon_name (priv->image, &icon_name, NULL);

  return icon_name;
}

/**
 * hdy_action_row_set_icon_name:
 * @self: a #HdyActionRow
 * @icon_name: the icon name
 *
 * Sets the icon name for @self.
 *
 * Since: 0.0.6
 */
void
hdy_action_row_set_icon_name (HdyActionRow *self,
                              const gchar  *icon_name)
{
  HdyActionRowPrivate *priv;
  const gchar *old_icon_name;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  gtk_image_get_icon_name (priv->image, &old_icon_name, NULL);
  if (g_strcmp0 (old_icon_name, icon_name) == 0)
    return;

  gtk_image_set_from_icon_name (priv->image, icon_name, GTK_ICON_SIZE_INVALID);
  gtk_widget_set_visible (GTK_WIDGET (priv->image),
                          icon_name != NULL && g_strcmp0 (icon_name, "") != 0);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * hdy_action_row_get_activatable_widget:
 * @self: a #HdyActionRow
 *
 * Gets the widget activated when @self is activated.
 *
 * Returns: (nullable) (transfer none): the widget activated when @self is
 *          activated, or %NULL if none has been set.
 *
 * Since: 0.0.7
 */
GtkWidget *
hdy_action_row_get_activatable_widget (HdyActionRow *self)
{
  HdyActionRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_ACTION_ROW (self), NULL);

  priv = hdy_action_row_get_instance_private (self);

  return priv->activatable_widget;
}

static void
activatable_widget_weak_notify (gpointer  data,
                                GObject  *where_the_object_was)
{
  HdyActionRow *self = HDY_ACTION_ROW (data);
  HdyActionRowPrivate *priv = hdy_action_row_get_instance_private (self);

  priv->activatable_widget = NULL;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATABLE_WIDGET]);
}

/**
 * hdy_action_row_set_activatable_widget:
 * @self: a #HdyActionRow
 * @widget: (nullable): the target #GtkWidget, or %NULL to unset
 *
 * Sets the widget to activate when @self is activated, either by clicking
 * on it, by calling hdy_action_row_activate(), or via mnemonics in the title or
 * the subtitle. See the “use_underline” property to enable mnemonics.
 *
 * The target widget will be activated by emitting the
 * GtkWidget::mnemonic-activate signal on it.
 *
 * Since: 0.0.7
 */
void
hdy_action_row_set_activatable_widget (HdyActionRow *self,
                                       GtkWidget    *widget)
{
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  if (priv->activatable_widget == widget)
    return;

  if (widget != NULL)
    g_return_if_fail (GTK_IS_WIDGET (widget));

  if (priv->activatable_widget)
    g_object_weak_unref (G_OBJECT (priv->activatable_widget),
                         activatable_widget_weak_notify,
                         self);

  priv->activatable_widget = widget;

  if (priv->activatable_widget != NULL)
    g_object_weak_ref (G_OBJECT (priv->activatable_widget),
                       activatable_widget_weak_notify,
                       self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVATABLE_WIDGET]);
}

/**
 * hdy_action_row_get_use_underline:
 * @self: a #HdyActionRow
 *
 * Gets whether an embedded underline in the text of the title and subtitle
 * labels indicates a mnemonic. See hdy_action_row_set_use_underline().
 *
 * Returns: %TRUE if an embedded underline in the title and subtitle labels
 *          indicates the mnemonic accelerator keys.
 *
 * Since: 0.0.6
 */
gboolean
hdy_action_row_get_use_underline (HdyActionRow *self)
{
  HdyActionRowPrivate *priv;

  g_return_val_if_fail (HDY_IS_ACTION_ROW (self), FALSE);

  priv = hdy_action_row_get_instance_private (self);

  return priv->use_underline;
}

/**
 * hdy_action_row_set_use_underline:
 * @self: a #HdyActionRow
 * @use_underline: %TRUE if underlines in the text indicate mnemonics
 *
 * If true, an underline in the text of the title and subtitle labels indicates
 * the next character should be used for the mnemonic accelerator key.
 *
 * Since: 0.0.6
 */
void
hdy_action_row_set_use_underline (HdyActionRow *self,
                                  gboolean      use_underline)
{
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  if (priv->use_underline == !!use_underline)
    return;

  priv->use_underline = !!use_underline;
  hdy_preferences_row_set_use_underline (HDY_PREFERENCES_ROW (self), priv->use_underline);
  gtk_label_set_use_underline (priv->title, priv->use_underline);
  gtk_label_set_use_underline (priv->subtitle, priv->use_underline);
  gtk_label_set_mnemonic_widget (priv->title, GTK_WIDGET (self));
  gtk_label_set_mnemonic_widget (priv->subtitle, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_USE_UNDERLINE]);
}

/**
 * hdy_action_row_add_action:
 * @self: a #HdyActionRow
 * @widget: (allow-none): the action widget
 *
 * Adds an action widget to @self.
 *
 * Since: 0.0.6
 */
void
hdy_action_row_add_action (HdyActionRow *self,
                           GtkWidget    *widget)
{
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  gtk_box_pack_end (priv->header, widget, FALSE, TRUE, 0);
}

/**
 * hdy_action_row_add_prefix:
 * @self: a #HdyActionRow
 * @widget: (allow-none): the prefix widget
 *
 * Adds a prefix widget to @self.
 *
 * Since: 0.0.6
 */
void
hdy_action_row_add_prefix (HdyActionRow *self,
                           GtkWidget    *widget)
{
  HdyActionRowPrivate *priv;

  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  priv = hdy_action_row_get_instance_private (self);

  gtk_box_pack_start (priv->prefixes, widget, FALSE, TRUE, 0);
  gtk_widget_show (GTK_WIDGET (priv->prefixes));
}

void
hdy_action_row_activate (HdyActionRow *self)
{
  g_return_if_fail (HDY_IS_ACTION_ROW (self));

  HDY_ACTION_ROW_GET_CLASS (self)->activate (self);
}
