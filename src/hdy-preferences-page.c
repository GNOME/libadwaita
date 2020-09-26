/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-preferences-page-private.h"

#include "hdy-preferences-group-private.h"

/**
 * SECTION:hdy-preferences-page
 * @short_description: A page from the preferences window.
 * @Title: HdyPreferencesPage
 *
 * The #HdyPreferencesPage widget gathers preferences groups into a single page
 * of a preferences window.
 *
 * # CSS nodes
 *
 * #HdyPreferencesPage has a single CSS node with name preferencespage.
 *
 * Since: 0.0.10
 */

typedef struct
{
  GtkBox *box;
  GtkScrolledWindow *scrolled_window;

  gchar *icon_name;
  gchar *title;
} HdyPreferencesPagePrivate;

static void hdy_preferences_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyPreferencesPage, hdy_preferences_page, GTK_TYPE_WIDGET,
                         G_ADD_PRIVATE (HdyPreferencesPage)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_preferences_page_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TITLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void
hdy_preferences_page_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  HdyPreferencesPage *self = HDY_PREFERENCES_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, hdy_preferences_page_get_icon_name (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, hdy_preferences_page_get_title (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_preferences_page_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HdyPreferencesPage *self = HDY_PREFERENCES_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    hdy_preferences_page_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_TITLE:
    hdy_preferences_page_set_title (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_preferences_page_dispose (GObject *object)
{
  HdyPreferencesPage *self = HDY_PREFERENCES_PAGE (object);
  HdyPreferencesPagePrivate *priv = hdy_preferences_page_get_instance_private (self);

  g_clear_pointer ((GtkWidget **) &priv->scrolled_window, gtk_widget_unparent);

  G_OBJECT_CLASS (hdy_preferences_page_parent_class)->dispose (object);
}

static void
hdy_preferences_page_finalize (GObject *object)
{
  HdyPreferencesPage *self = HDY_PREFERENCES_PAGE (object);
  HdyPreferencesPagePrivate *priv = hdy_preferences_page_get_instance_private (self);

  g_clear_pointer (&priv->icon_name, g_free);
  g_clear_pointer (&priv->title, g_free);

  G_OBJECT_CLASS (hdy_preferences_page_parent_class)->finalize (object);
}

static void
hdy_preferences_page_class_init (HdyPreferencesPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_preferences_page_get_property;
  object_class->set_property = hdy_preferences_page_set_property;
  object_class->dispose = hdy_preferences_page_dispose;
  object_class->finalize = hdy_preferences_page_finalize;

  /**
   * HdyPreferencesPage:icon-name:
   *
   * The icon name for this page of preferences.
   *
   * Since: 0.0.10
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         _("Icon name"),
                         _("Icon name"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyPreferencesPage:title:
   *
   * The title for this page of preferences.
   *
   * Since: 0.0.10
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("Title"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-preferences-page.ui");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesPage, box);
  gtk_widget_class_bind_template_child_private (widget_class, HdyPreferencesPage, scrolled_window);

  gtk_widget_class_set_css_name (widget_class, "preferencespage");
}

static void
hdy_preferences_page_init (HdyPreferencesPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
hdy_preferences_page_buildable_add_child (GtkBuildable *buildable,
                                          GtkBuilder   *builder,
                                          GObject      *child,
                                          const gchar  *type)
{
  HdyPreferencesPage *self = HDY_PREFERENCES_PAGE (buildable);
  HdyPreferencesPagePrivate *priv = hdy_preferences_page_get_instance_private (self);

  if (priv->box && HDY_IS_PREFERENCES_GROUP (child))
    hdy_preferences_page_add (self, HDY_PREFERENCES_GROUP (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_preferences_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = hdy_preferences_page_buildable_add_child;
}

/**
 * hdy_preferences_page_new:
 *
 * Creates a new #HdyPreferencesPage.
 *
 * Returns: a new #HdyPreferencesPage
 *
 * Since: 0.0.10
 */
GtkWidget *
hdy_preferences_page_new (void)
{
  return g_object_new (HDY_TYPE_PREFERENCES_PAGE, NULL);
}

/**
 * hdy_preferences_page_get_icon_name:
 * @self: a #HdyPreferencesPage
 *
 * Gets the icon name for @self, or %NULL.
 *
 * Returns: (transfer none) (nullable): the icon name for @self, or %NULL.
 *
 * Since: 0.0.10
 */
const gchar *
hdy_preferences_page_get_icon_name (HdyPreferencesPage *self)
{
  HdyPreferencesPagePrivate *priv;

  g_return_val_if_fail (HDY_IS_PREFERENCES_PAGE (self), NULL);

  priv = hdy_preferences_page_get_instance_private (self);

  return priv->icon_name;
}

/**
 * hdy_preferences_page_set_icon_name:
 * @self: a #HdyPreferencesPage
 * @icon_name: (nullable): the icon name, or %NULL
 *
 * Sets the icon name for @self.
 *
 * Since: 0.0.10
 */
void
hdy_preferences_page_set_icon_name (HdyPreferencesPage *self,
                                    const gchar        *icon_name)
{
  HdyPreferencesPagePrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_PAGE (self));

  priv = hdy_preferences_page_get_instance_private (self);

  if (g_strcmp0 (priv->icon_name, icon_name) == 0)
    return;

  g_clear_pointer (&priv->icon_name, g_free);
  priv->icon_name = g_strdup (icon_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * hdy_preferences_page_get_title:
 * @self: a #HdyPreferencesPage
 *
 * Gets the title of @self, or %NULL.
 *
 * Returns: (transfer none) (nullable): the title of the @self, or %NULL.
 *
 * Since: 0.0.10
 */
const gchar *
hdy_preferences_page_get_title (HdyPreferencesPage *self)
{
  HdyPreferencesPagePrivate *priv;

  g_return_val_if_fail (HDY_IS_PREFERENCES_PAGE (self), NULL);

  priv = hdy_preferences_page_get_instance_private (self);

  return priv->title;
}

/**
 * hdy_preferences_page_set_title:
 * @self: a #HdyPreferencesPage
 * @title: (nullable): the title of the page, or %NULL
 *
 * Sets the title of @self.
 *
 * Since: 0.0.10
 */
void
hdy_preferences_page_set_title (HdyPreferencesPage *self,
                                const gchar        *title)
{
  HdyPreferencesPagePrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_PAGE (self));

  priv = hdy_preferences_page_get_instance_private (self);

  if (g_strcmp0 (priv->title, title) == 0)
    return;

  g_clear_pointer (&priv->title, g_free);
  priv->title = g_strdup (title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * hdy_preferences_page_get_rows:
 * @self: a #HdyPreferencesPage
 *
 * Returns a #GListModel that contains the rows of the page, and can be used to
 * keep an up-to-date view.
 *
 * Returns: (transfer full): a #GListModel for the page's rows
 */
GListModel *
hdy_preferences_page_get_rows (HdyPreferencesPage *self)
{
  HdyPreferencesPagePrivate *priv;
  GListModel *model;
  GtkExpression *expr;

  g_return_val_if_fail (HDY_IS_PREFERENCES_PAGE (self), NULL);

  priv = hdy_preferences_page_get_instance_private (self);

  expr = gtk_property_expression_new (GTK_TYPE_WIDGET, NULL, "visible");

  model = gtk_widget_observe_children (GTK_WIDGET (priv->box));
  model = G_LIST_MODEL (gtk_filter_list_model_new (model, GTK_FILTER (gtk_bool_filter_new (expr))));
  model = G_LIST_MODEL (gtk_map_list_model_new (model,
                                                (GtkMapListModelMapFunc) hdy_preferences_group_get_rows,
                                                NULL,
                                                NULL));

  return G_LIST_MODEL (gtk_flatten_list_model_new (model));
}

void
hdy_preferences_page_add (HdyPreferencesPage  *self,
                          HdyPreferencesGroup *group)
{
  HdyPreferencesPagePrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_PAGE (self));
  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (group));

  priv = hdy_preferences_page_get_instance_private (self);

  gtk_box_append (priv->box, GTK_WIDGET (group));
}

void
hdy_preferences_page_remove (HdyPreferencesPage  *self,
                             HdyPreferencesGroup *group)
{
  HdyPreferencesPagePrivate *priv;

  g_return_if_fail (HDY_IS_PREFERENCES_PAGE (self));
  g_return_if_fail (HDY_IS_PREFERENCES_GROUP (group));

  priv = hdy_preferences_page_get_instance_private (self);

  gtk_box_remove (priv->box, GTK_WIDGET (group));
}
