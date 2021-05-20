/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-preferences-group-private.h"

#include "adw-preferences-row.h"

/**
 * AdwPreferencesGroup:
 *
 * A group of preference rows.
 *
 * An `AdwPreferencesGroup` represents a group or tightly related preferences,
 * which in turn are represented by [class@Adw.PreferencesRow].
 *
 * To summarize the role of the preferences it gathers, a group can have both a
 * title and a description. The title will be used by
 * [class@Adw.PreferencesWindow] to let the user look for a preference.
 *
 * ## CSS nodes
 *
 * `AdwPreferencesGroup` has a single CSS node with name `preferencesgroup`.
 *
 * Since: 1.0
 */

typedef struct
{
  GtkWidget *box;
  GtkLabel *description;
  GtkListBox *listbox;
  GtkBox *listbox_box;
  GtkLabel *title;

  GListModel *rows;
} AdwPreferencesGroupPrivate;

static void adw_preferences_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwPreferencesGroup, adw_preferences_group, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (AdwPreferencesGroup)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adw_preferences_group_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_DESCRIPTION,
  PROP_TITLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
update_title_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->title),
                          gtk_label_get_text (priv->title) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->title), "") != 0);
}

static void
update_description_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->description),
                          gtk_label_get_text (priv->description) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->description), "") != 0);
}

static void
update_listbox_visibility (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  /* We must wait until the listbox has been built and added. */

  if (priv->rows == NULL)
    return;

  gtk_widget_set_visible (GTK_WIDGET (priv->listbox),
                          g_list_model_get_n_items (priv->rows) > 0);
}

static gboolean
listbox_keynav_failed_cb (AdwPreferencesGroup *self,
                          GtkDirectionType     direction)
{
  GtkWidget *toplevel = GTK_WIDGET (gtk_widget_get_root (GTK_WIDGET (self)));

  if (!toplevel)
    return FALSE;

  if (direction != GTK_DIR_UP && direction != GTK_DIR_DOWN)
    return FALSE;

  return gtk_widget_child_focus (toplevel, direction == GTK_DIR_UP ?
                                 GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD);
}

static void
adw_preferences_group_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (object);

  switch (prop_id) {
  case PROP_DESCRIPTION:
    g_value_set_string (value, adw_preferences_group_get_description (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, adw_preferences_group_get_title (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_group_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (object);

  switch (prop_id) {
  case PROP_DESCRIPTION:
    adw_preferences_group_set_description (self, g_value_get_string (value));
    break;
  case PROP_TITLE:
    adw_preferences_group_set_title (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_preferences_group_dispose (GObject *object)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (object);
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_unparent (priv->box);
  g_clear_object (&priv->rows);

  G_OBJECT_CLASS (adw_preferences_group_parent_class)->dispose (object);
}

static void
adw_preferences_group_class_init (AdwPreferencesGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_preferences_group_get_property;
  object_class->set_property = adw_preferences_group_set_property;
  object_class->dispose = adw_preferences_group_dispose;

  /**
   * AdwPreferencesGroup:description: (attributes org.gtk.Property.get=adw_preferences_group_get_description org.gtk.Property.set=adw_preferences_group_set_description)
   *
   * The description for this group of preferences.
   *
   * Since: 1.0
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description",
                         "Description",
                         "The description for this group of preferences",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwPreferencesGroup:title: (attributes org.gtk.Property.get=adw_preferences_group_get_title org.gtk.Property.set=adw_preferences_group_set_title)
   *
   * The title for this group of preferences.
   *
   * Since: 1.0
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "The title for this group of preferences",
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "preferencesgroup");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-preferences-group.ui");
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, box);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, description);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, listbox);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, listbox_box);
  gtk_widget_class_bind_template_child_private (widget_class, AdwPreferencesGroup, title);
  gtk_widget_class_bind_template_callback (widget_class, listbox_keynav_failed_cb);
}

static void
adw_preferences_group_init (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  update_description_visibility (self);
  update_title_visibility (self);
  update_listbox_visibility (self);

  priv->rows = gtk_widget_observe_children (GTK_WIDGET (priv->listbox));

  g_signal_connect_object (priv->rows, "items-changed",
                           G_CALLBACK (update_listbox_visibility), self,
                           G_CONNECT_SWAPPED);
}

static void
adw_preferences_group_buildable_add_child (GtkBuildable *buildable,
                                           GtkBuilder   *builder,
                                           GObject      *child,
                                           const char   *type)
{
  AdwPreferencesGroup *self = ADW_PREFERENCES_GROUP (buildable);
  AdwPreferencesGroupPrivate *priv = adw_preferences_group_get_instance_private (self);

  if (priv->box && GTK_IS_WIDGET (child))
    adw_preferences_group_add (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_preferences_group_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adw_preferences_group_buildable_add_child;
}

/**
 * adw_preferences_group_new:
 *
 * Creates a new `AdwPreferencesGroup`.
 *
 * Returns: the newly created `AdwPreferencesGroup`
 *
 * Since: 1.0
 */
GtkWidget *
adw_preferences_group_new (void)
{
  return g_object_new (ADW_TYPE_PREFERENCES_GROUP, NULL);
}

/**
 * adw_preferences_group_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a `AdwPreferencesGroup`
 *
 * Gets the title of @self.
 *
 * Returns: (nullable): the title of @self
 *
 * Since: 1.0
 */
const char *
adw_preferences_group_get_title (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  return gtk_label_get_text (priv->title);
}

/**
 * adw_preferences_group_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a `AdwPreferencesGroup`
 * @title: (nullable): the title
 *
 * Sets the title for @self.
 *
 * Since: 1.0
 */
void
adw_preferences_group_set_title (AdwPreferencesGroup *self,
                                 const char          *title)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));

  priv = adw_preferences_group_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->title), title) == 0)
    return;

  gtk_label_set_label (priv->title, title);
  update_title_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_preferences_group_get_description: (attributes org.gtk.Method.get_property=description)
 * @self: a `AdwPreferencesGroup`
 *
 * Gets the description of @self.
 *
 * Returns: (nullable): the description of @self
 *
 * Since: 1.0
 */
const char *
adw_preferences_group_get_description (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  return gtk_label_get_text (priv->description);
}

/**
 * adw_preferences_group_set_description: (attributes org.gtk.Method.set_property=description)
 * @self: a `AdwPreferencesGroup`
 * @description: (nullable): the description
 *
 * Sets the description for @self.
 *
 * Since: 1.0
 */
void
adw_preferences_group_set_description (AdwPreferencesGroup *self,
                                       const char          *description)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));

  priv = adw_preferences_group_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->description), description) == 0)
    return;

  gtk_label_set_label (priv->description, description);
  update_description_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

static gboolean
row_has_title (AdwPreferencesRow *row,
               gpointer           user_data)
{
  const char *title;

  g_assert (ADW_IS_PREFERENCES_ROW (row));

  if (!gtk_widget_get_visible (GTK_WIDGET (row)))
    return FALSE;

  title = adw_preferences_row_get_title (row);

  return title && *title;
}

/**
 * adw_preferences_group_get_rows:
 * @self: a `AdwPreferencesGroup`
 *
 * Gets a `GListModel` that contains the rows of the group.
 *
 * This can be used to keep an up-to-date view.
 *
 * Returns: (transfer full): a `GListModel` for the group's rows
 *
 * Since: 1.0
 */
GListModel *
adw_preferences_group_get_rows (AdwPreferencesGroup *self)
{
  AdwPreferencesGroupPrivate *priv;
  GtkCustomFilter *filter;
  GListModel *model;

  g_return_val_if_fail (ADW_IS_PREFERENCES_GROUP (self), NULL);

  priv = adw_preferences_group_get_instance_private (self);

  filter = gtk_custom_filter_new ((GtkCustomFilterFunc) row_has_title, NULL, NULL);
  model = gtk_widget_observe_children (GTK_WIDGET (priv->listbox));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (filter)));

  return model;
}

/**
 * adw_preferences_group_add:
 * @self: a `AdwPreferencesGroup`
 * @child: the widget to add
 *
 * Adds a child to @self.
 *
 * Since: 1.0
 */
void
adw_preferences_group_add (AdwPreferencesGroup *self,
                           GtkWidget           *child)
{
  AdwPreferencesGroupPrivate *priv;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adw_preferences_group_get_instance_private (self);

  if (ADW_IS_PREFERENCES_ROW (child))
    gtk_list_box_append (priv->listbox, child);
  else
    gtk_box_append (priv->listbox_box, child);
}

/**
 * adw_preferences_group_remove:
 * @self: a `AdwPreferencesGroup`
 * @child: the child to remove
 *
 * Removes a child from @self.
 *
 * Since: 1.0
 */
void
adw_preferences_group_remove (AdwPreferencesGroup *self,
                              GtkWidget           *child)
{
  AdwPreferencesGroupPrivate *priv;
  GtkWidget *parent;

  g_return_if_fail (ADW_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = adw_preferences_group_get_instance_private (self);

  parent = gtk_widget_get_parent (child);

  if (parent == GTK_WIDGET (priv->listbox))
    gtk_list_box_remove (priv->listbox, child);
  else if (parent == GTK_WIDGET (priv->listbox_box))
    gtk_box_remove (priv->listbox_box, child);
}
