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
  GtkBox *box;
  GtkLabel *description;
  GtkListBox *listbox;
  GtkBox *listbox_box;
  GtkLabel *title;
} HdyPreferencesGroupPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyPreferencesGroup, hdy_preferences_group, GTK_TYPE_BIN)

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

  /* Show the listbox only if it has children to avoid having showing the
   * listbox as an empty frame, parasiting the look of non-GtkListBoxRow
   * children.
   */
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
  g_autoptr(GList) children = NULL;

  /* We must wait until listob has been built and added. */
  if (priv->listbox == NULL)
    return;

  children = gtk_container_get_children (GTK_CONTAINER (priv->listbox));

  gtk_widget_set_visible (GTK_WIDGET (priv->listbox), children != NULL);
}

typedef struct {
  HdyPreferencesGroup *group;
  GtkCallback callback;
  gpointer callback_data;
} ForallData;

static void
for_non_internal_child (GtkWidget *widget,
                        gpointer   callback_data)
{
  ForallData *data = callback_data;
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (data->group);

  if (widget != (GtkWidget *) priv->listbox)
    data->callback (widget, data->callback_data);
}

static void
hdy_preferences_group_forall (GtkContainer *container,
                              gboolean      include_internals,
                              GtkCallback   callback,
                              gpointer      callback_data)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (container);
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);
  ForallData data;

  if (include_internals) {
    GTK_CONTAINER_CLASS (hdy_preferences_group_parent_class)->forall (GTK_CONTAINER (self), include_internals, callback, callback_data);

    return;
  }

  data.group = self;
  data.callback = callback;
  data.callback_data = callback_data;

  if (priv->listbox)
    GTK_CONTAINER_GET_CLASS (priv->listbox)->forall (GTK_CONTAINER (priv->listbox), include_internals, callback, callback_data);
  if (priv->listbox_box)
    GTK_CONTAINER_GET_CLASS (priv->listbox_box)->forall (GTK_CONTAINER (priv->listbox_box), include_internals, for_non_internal_child, &data);
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

  /*
   * Since we overload forall(), the inherited destroy() won't work as normal.
   * Remove internal widgets ourself.
   */
  g_clear_pointer ((GtkWidget **) &priv->description, gtk_widget_destroy);
  g_clear_pointer ((GtkWidget **) &priv->listbox, gtk_widget_destroy);
  g_clear_pointer ((GtkWidget **) &priv->listbox_box, gtk_widget_destroy);
  g_clear_pointer ((GtkWidget **) &priv->title, gtk_widget_destroy);

  G_OBJECT_CLASS (hdy_preferences_group_parent_class)->dispose (object);
}

static void
hdy_preferences_group_add (GtkContainer *container,
                           GtkWidget    *child)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (container);
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  if (priv->title == NULL || priv->description == NULL || priv->listbox_box == NULL) {
    GTK_CONTAINER_CLASS (hdy_preferences_group_parent_class)->add (container, child);

    return;
  }

  if (HDY_IS_PREFERENCES_ROW (child))
    gtk_container_add (GTK_CONTAINER (priv->listbox), child);
  else
    gtk_container_add (GTK_CONTAINER (priv->listbox_box), child);
}

static void
hdy_preferences_group_remove (GtkContainer *container,
                              GtkWidget    *child)
{
  HdyPreferencesGroup *self = HDY_PREFERENCES_GROUP (container);
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  if (child == GTK_WIDGET (priv->box))
    GTK_CONTAINER_CLASS (hdy_preferences_group_parent_class)->remove (container, child);
  else if (HDY_IS_PREFERENCES_ROW (child))
    gtk_container_remove (GTK_CONTAINER (priv->listbox), child);
  else if (child != GTK_WIDGET (priv->listbox))
    gtk_container_remove (GTK_CONTAINER (priv->listbox_box), child);
}

static void
hdy_preferences_group_class_init (HdyPreferencesGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_preferences_group_get_property;
  object_class->set_property = hdy_preferences_group_set_property;
  object_class->dispose = hdy_preferences_group_dispose;

  container_class->add = hdy_preferences_group_add;
  container_class->remove = hdy_preferences_group_remove;
  container_class->forall = hdy_preferences_group_forall;

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
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-preferences-group.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, description);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, listbox);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, listbox_box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesGroup, title);
  gtk_widget_class_bind_template_callback (widget_class, update_listbox_visibility);
}

static void
hdy_preferences_group_init (HdyPreferencesGroup *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  update_description_visibility (self);
  update_title_visibility (self);
  update_listbox_visibility (self);
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

static void
add_preferences_to_model (HdyPreferencesRow *row,
                          GListStore        *model)
{
  const gchar *title;

  g_assert (HDY_IS_PREFERENCES_ROW (row));
  g_assert (G_IS_LIST_STORE (model));

  if (!gtk_widget_get_visible (GTK_WIDGET (row)))
    return;

  title = hdy_preferences_row_get_title (row);

  if (!title || !*title)
    return;

  g_list_store_append (model, row);
}

/**
 * hdy_preferences_group_add_preferences_to_model: (skip)
 * @self: a #HdyPreferencesGroup
 * @model: the model
 *
 * Add preferences from @self to the model.
 *
 * Since: 0.0.10
 */
void
hdy_preferences_group_add_preferences_to_model (HdyPreferencesGroup *self,
                                                GListStore          *model)
{
  HdyPreferencesGroupPrivate *priv = hdy_preferences_group_get_instance_private (self);

  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (G_IS_LIST_STORE (model));

  if (!gtk_widget_get_visible (GTK_WIDGET (self)))
    return;

  gtk_container_foreach (GTK_CONTAINER (priv->listbox), (GtkCallback) add_preferences_to_model, model);
}
