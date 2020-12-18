/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-status-page.h"

/**
 * SECTION:hdy-status-page
 * @short_description: A page used for empty/error states and similar use-cases.
 * @Title: HdyStatusPage
 *
 * The #HdyStatusPage widget can have an icon, a title, a description and a
 * custom widget which is displayed below them.
 *
 * # CSS nodes
 *
 * #HdyStatusPage has a main CSS node with name statuspage.
 *
 * Since: 1.1
 */

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_TITLE,
  PROP_DESCRIPTION,
  PROP_CHILD,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

struct _HdyStatusPage
{
  GtkWidget parent_instance;

  GtkWidget *scrolled_window;
  GtkBox *toplevel_box;
  GtkImage *image;
  gchar *icon_name;
  GtkLabel *title_label;
  GtkLabel *description_label;

  GtkWidget *user_widget;
};

static void hdy_status_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyStatusPage, hdy_status_page, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, hdy_status_page_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
update_title_visibility (HdyStatusPage *self)
{
  gtk_widget_set_visible (GTK_WIDGET (self->title_label),
                          gtk_label_get_text (self->title_label) != NULL &&
                          g_strcmp0 (gtk_label_get_text (self->title_label), "") != 0);
}

static void
update_description_visibility (HdyStatusPage *self)
{
  gtk_widget_set_visible (GTK_WIDGET (self->description_label),
                          gtk_label_get_text (self->description_label) != NULL &&
                          g_strcmp0 (gtk_label_get_text (self->description_label), "") != 0);
}

static void
hdy_status_page_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, hdy_status_page_get_icon_name (self));
    break;

  case PROP_TITLE:
    g_value_set_string (value, hdy_status_page_get_title (self));
    break;

  case PROP_DESCRIPTION:
    g_value_set_string (value, hdy_status_page_get_description (self));
    break;

  case PROP_CHILD:
    g_value_set_object (value, hdy_status_page_get_child (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_status_page_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    hdy_status_page_set_icon_name (self, g_value_get_string (value));
    break;

  case PROP_TITLE:
    hdy_status_page_set_title (self, g_value_get_string (value));
    break;

  case PROP_DESCRIPTION:
    hdy_status_page_set_description (self, g_value_get_string (value));
    break;

  case PROP_CHILD:
    hdy_status_page_set_child (self, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_status_page_dispose (GObject *object)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (object);

  hdy_status_page_set_child (self, NULL);

  g_clear_pointer (&self->scrolled_window, gtk_widget_unparent);
  self->toplevel_box = NULL;
  self->image = NULL;
  self->title_label = NULL;
  self->description_label = NULL;
  self->user_widget = NULL;

  G_OBJECT_CLASS (hdy_status_page_parent_class)->dispose (object);
}

static void
hdy_status_page_finalize (GObject *object)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (object);

  g_clear_pointer (&self->icon_name, g_free);

  G_OBJECT_CLASS (hdy_status_page_parent_class)->finalize (object);
}

static void
hdy_status_page_class_init (HdyStatusPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_status_page_get_property;
  object_class->set_property = hdy_status_page_set_property;
  object_class->dispose = hdy_status_page_dispose;
  object_class->finalize = hdy_status_page_finalize;

  /**
   * HdyStatusPage:icon-name:
   *
   * The name of the icon to be used.
   *
   * Since: 1.1
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         _("Icon name"),
                         _("The name of the icon to be used"),
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStatusPage:title:
   *
   * The title to be displayed below the icon.
   *
   * Since: 1.1
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("The title to be displayed below the icon"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStatusPage:description:
   *
   * The description to be displayed below the title.
   *
   * Since: 1.1
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description",
                         _("Description"),
                         _("The description to be displayed below the title"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CHILD] =
    g_param_spec_object ("child",
                         _("Child"),
                         _("The child widget"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-status-page.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, toplevel_box);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, image);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, title_label);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, description_label);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "statuspage");
}

static void
hdy_status_page_init (HdyStatusPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  update_title_visibility (self);
  update_description_visibility (self);
}

static void
hdy_status_page_buildable_add_child (GtkBuildable *buildable,
                                     GtkBuilder   *builder,
                                     GObject      *child,
                                     const gchar  *type)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (buildable);

  if (!self->scrolled_window && GTK_IS_WIDGET (child))
    gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (buildable));
  else if (GTK_IS_WIDGET (child))
    hdy_status_page_set_child (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_status_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = hdy_status_page_buildable_add_child;
}


/**
 * hdy_status_page_new:
 *
 * Creates a new #HdyStatusPage.
 *
 * Returns: a new #HdyStatusPage
 *
 * Since: 1.1
 */
GtkWidget *
hdy_status_page_new (void)
{
  return g_object_new (HDY_TYPE_STATUS_PAGE, NULL);
}

/**
 * hdy_status_page_get_icon_name:
 * @self: a #HdyStatusPage
 *
 * Gets the icon name for @self.
 *
 * Returns: (transfer none) (nullable): the icon name for @self.
 *
 * Since: 1.1
 */
const gchar *
hdy_status_page_get_icon_name (HdyStatusPage *self)
{
  return self->icon_name;
}

/**
 * hdy_status_page_set_icon_name:
 * @self: a #HdyStatusPage
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @self.
 *
 * Since: 1.1
 */
void
hdy_status_page_set_icon_name (HdyStatusPage *self,
                               const gchar   *icon_name)
{
  g_return_if_fail (HDY_IS_STATUS_PAGE (self));

  if (g_strcmp0 (self->icon_name, icon_name) == 0)
    return;

  g_free (self->icon_name);
  self->icon_name = g_strdup (icon_name);

  gtk_image_set_from_icon_name (self->image,
                                icon_name ? icon_name : "image-missing");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * hdy_status_page_get_title:
 * @self: a #HdyStatusPage
 *
 * Gets the title for @self.
 *
 * Returns: (transfer none) (nullable): the title for @self, or %NULL.
 *
 * Since: 1.1
 */
const gchar *
hdy_status_page_get_title (HdyStatusPage *self)
{
  g_return_val_if_fail (HDY_IS_STATUS_PAGE (self), NULL);

  return gtk_label_get_label (self->title_label);
}

/**
 * hdy_status_page_set_title:
 * @self: a #HdyStatusPage
 * @title: (nullable): the title
 *
 * Sets the title for @self.
 *
 * Since: 1.1
 */
void
hdy_status_page_set_title (HdyStatusPage *self,
                           const gchar   *title)
{
  g_return_if_fail (HDY_IS_STATUS_PAGE (self));

  if (g_strcmp0 (title, hdy_status_page_get_title (self)) == 0)
    return;

  gtk_label_set_label (self->title_label, title);
  update_title_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * hdy_status_page_get_description:
 * @self: a #HdyStatusPage
 *
 * Gets the description for @self.
 *
 * Returns: (transfer none) (nullable): the description for @self, or %NULL.
 *
 * Since: 1.1
 */
const gchar *
hdy_status_page_get_description (HdyStatusPage *self)
{
  g_return_val_if_fail (HDY_IS_STATUS_PAGE (self), NULL);

  return gtk_label_get_label (self->description_label);
}

/**
 * hdy_status_page_set_description:
 * @self: a #HdyStatusPage
 * @description: (nullable): the description
 *
 * Sets the description for @self.
 *
 * Since: 1.1
 */
void
hdy_status_page_set_description (HdyStatusPage *self,
                                 const gchar   *description)
{
  g_return_if_fail (HDY_IS_STATUS_PAGE (self));

  if (g_strcmp0 (description, hdy_status_page_get_description (self)) == 0)
    return;

  gtk_label_set_label (self->description_label, description);
  update_description_visibility (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

/**
 * hdy_status_page_get_child:
 * @self: a #HdyStatusPage
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
hdy_status_page_get_child (HdyStatusPage *self)
{
  g_return_val_if_fail (HDY_IS_STATUS_PAGE (self), NULL);

  return self->user_widget;
}

/**
 * hdy_status_page_set_child:
 * @self: a #HdyStatusPage
 * @child: (allow-none): the child widget
 *
 * Sets the child widget of @self.
 */
void
hdy_status_page_set_child (HdyStatusPage *self,
                           GtkWidget     *child)
{
  g_return_if_fail (HDY_IS_STATUS_PAGE (self));

  if (child == self->user_widget)
    return;

  if (self->user_widget)
    gtk_box_remove (self->toplevel_box, self->user_widget);

  self->user_widget = child;

  if (self->user_widget)
    gtk_box_append (self->toplevel_box, self->user_widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}
