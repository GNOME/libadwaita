/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-preferences-group-private.h"

#include "hdy-preferences-row.h"

/**
 * SECTION:hdy-preferences-group
 * @short_description: A group gathering preferences rows.
 * @Title: HdyPreferencesGroup
 *
 * A #HdyPreferencesGroup represents a group or tightly related preferences,
 * which in turn are represented by HdyPreferencesRow.
 *
 * To summarize the role of the preferences it gathers, a group can have both a
 * title and a description. The title will be used by #HdyPreferencesWindow to
 * let the user look for a preference.
 *
 * # CSS nodes
 *
 * #HdyPreferencesGroup has a single CSS node with name preferencesgroup.
 *
 * Since: 0.0.10
 */

typedef struct
{
  GtkWidget *box;
  GtkLabel *description;
  GtkListBox *listbox;
  GtkBox *listbox_box;
  GtkLabel *title;

  GListModel *rows;
} HdyPreferencesGroupPrivate;

static void hdy_preferences_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyPreferencesGroup, hdy_preferences_group, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (HdyPreferencesGroup)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_preferences_group_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_DESCRIPTION,
  PROP_TITLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
update_title_visibility (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->title),
                          gtk_label_get_text (priv->title) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->title), "") != 0);
}

static void
update_description_visibility (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  gtk_widget_set_visible (GTK_WIDGET (priv->description),
                          gtk_label_get_text (priv->description) != NULL &&
                          g_strcmp0 (gtk_label_get_text (priv->description), "") != 0);
}

static void
update_listbox_visibility (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  /* We must wait until the listbox has been built and added. */

  if (priv->rows == NULL)
    return;

  gtk_widget_set_visible (GTK_WIDGET (priv->listbox),
                          g_list_model_get_n_items (priv->rows) > 0);
}

static gboolean
listbox_keynav_failed_cb (HdyPreferencesGroup *self,
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
hdy_preferences_group_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (object);

  switch (prop_id) {
  case PROP_DESCRIPTION:
    g_value_set_string (value, hdy_preferences_group_get_description (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, hdy_preferences_group_get_title (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_preferences_group_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (object);

  switch (prop_id) {
  case PROP_DESCRIPTION:
    hdy_preferences_group_set_description (self, g_value_get_string (value));
    break;
  case PROP_TITLE:
    hdy_preferences_group_set_title (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_preferences_group_dispose (GObject *object)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (object);
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  g_clear_pointer (&priv->box, gtk_widget_unparent);
  priv->description = NULL;
  priv->listbox = NULL;
  priv->listbox_box = NULL;
  priv->title = NULL;
  g_clear_object (&priv->rows);

  G_OBJECT_CLASS (hdy_preferences_group_parent_class)->dispose (object);
}

static void
hdy_preferences_group_class_init (HdyPreferencesGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_preferences_group_get_property;
  object_class->set_property = hdy_preferences_group_set_property;
  object_class->dispose = hdy_preferences_group_dispose;

  /**
   * HdyPreferencesGroup:description:
   *
   * The description for this group of preferences.
   *
   * Since: 0.0.10
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description",
                         _("Description"),
                         _("Description"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyPreferencesGroup:title:
   *
   * The title for this group of preferences.
   *
   * Since: 0.0.10
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("Title"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "preferencesgroup");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-preferences-group.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, description);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, listbox);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, listbox_box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, title);
  gtk_widget_class_bind_template_callback (widget_class, listbox_keynav_failed_cb);
}

static void
hdy_preferences_group_init (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

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
hdy_preferences_group_buildable_add_child (GtkBuildable *buildable,
                                           GtkBuilder   *builder,
                                           GObject      *child,
                                           const gchar  *type)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (buildable);
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  if (priv->box && GTK_IS_WIDGET (child))
    hdy_preferences_group_add (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_preferences_group_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = hdy_preferences_group_buildable_add_child;
}

/**
 * hdy_preferences_group_new:
 *
 * Creates a new #HdyPreferencesGroup.
 *
 * Returns: a new #HdyPreferencesGroup
 *
 * Since: 0.0.10
 */
GtkWidget *
hdy_preferences_group_new (void)
{
  return g_object_new (HDY_TYPE_PREFERENCES_GROUP, NULL);
}

/**
 * hdy_preferences_group_get_title:
 * @self: a #HdyPreferencesGroup
 *
 * Gets the title of @self.
 *
 * Returns: the title of @self.
 *
 * Since: 0.0.10
 */
const gchar *
hdy_preferences_group_get_title (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv;

  g_return_val_if_fail (HDY_IS_PREFERENCES_GROUP (self), NULL);

  priv = hdy_preferences_group_get_instance_private (self);

  return gtk_label_get_text (priv->title);
}

/**
 * hdy_preferences_group_set_title:
 * @self: a #HdyPreferencesGroup
 * @title: the title
 *
 * Sets the title for @self.
 *
 * Since: 0.0.10
 */
void
hdy_preferences_group_set_title (HdyPreferencesGroup *self,
                                 const gchar         *title)
{
  HdyPreferencesGroupPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (self));

  priv = hdy_preferences_group_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->title), title) == 0)
    return;

  gtk_label_set_label (priv->title, title);
  update_title_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * hdy_preferences_group_get_description:
 * @self: a #HdyPreferencesGroup
 *
 *
 * Returns: the description of @self.
 *
 * Since: 0.0.10
 */
const gchar *
hdy_preferences_group_get_description (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv;

  g_return_val_if_fail (HDY_IS_PREFERENCES_GROUP (self), NULL);

  priv = hdy_preferences_group_get_instance_private (self);

  return gtk_label_get_text (priv->description);
}

/**
 * hdy_preferences_group_set_description:
 * @self: a #HdyPreferencesGroup
 * @description: the description
 *
 * Sets the description for @self.
 *
 * Since: 0.0.10
 */
void
hdy_preferences_group_set_description (HdyPreferencesGroup *self,
                                       const gchar         *description)
{
  HdyPreferencesGroupPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (self));

  priv = hdy_preferences_group_get_instance_private (self);

  if (g_strcmp0 (gtk_label_get_label (priv->description), description) == 0)
    return;

  gtk_label_set_label (priv->description, description);
  update_description_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

static gboolean
row_has_title (HdyPreferencesRow *row,
               gpointer           user_data)
{
  const gchar *title;

  g_assert (HDY_IS_PREFERENCES_ROW (row));

  if (!gtk_widget_get_visible (GTK_WIDGET (row)))
    return FALSE;

  title = hdy_preferences_row_get_title (row);

  return title && *title;
}

/**
 * hdy_preferences_group_get_rows:
 * @self: a #HdyPreferencesGroup
 *
 * Returns a #GListModel that contains the rows of the group, and can be used to
 * keep an up-to-date view.
 *
 * Returns: (transfer full): a #GListModel for the page's rows
 */
GListModel *
hdy_preferences_group_get_rows (HdyPreferencesGroup *self)
{
  HdyPreferencesGroupPrivate *priv;
  GtkCustomFilter *filter;
  GListModel *model;

  g_return_val_if_fail (HDY_IS_PREFERENCES_GROUP (self), NULL);

  priv = hdy_preferences_group_get_instance_private (self);

  filter = gtk_custom_filter_new ((GtkCustomFilterFunc) row_has_title, NULL, NULL);
  model = gtk_widget_observe_children (GTK_WIDGET (priv->listbox));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (filter)));

  return model;
}

void
hdy_preferences_group_add (HdyPreferencesGroup *self,
                           GtkWidget           *child)
{
  HdyPreferencesGroupPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = hdy_preferences_group_get_instance_private (self);

  if (HDY_IS_PREFERENCES_ROW (child))
    gtk_list_box_append (priv->listbox, child);
  else
    gtk_box_append (priv->listbox_box, child);
}

void
hdy_preferences_group_remove (HdyPreferencesGroup *self,
                              GtkWidget           *child)
{
  HdyPreferencesGroupPrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  priv = hdy_preferences_group_get_instance_private (self);

  if (HDY_IS_PREFERENCES_ROW (child))
    gtk_list_box_remove (priv->listbox, child);
  else if (child != GTK_WIDGET (priv->listbox))
    gtk_box_remove (priv->listbox_box, child);
}
