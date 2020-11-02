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
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

struct _HdyStatusPage
{
  GtkBin parent_instance;

  GtkWidget *scrolled_window;
  GtkBox *toplevel_box;
  GtkImage *image;
  gchar *icon_name;
  GtkLabel *title_label;
  GtkLabel *description_label;

  GtkWidget *user_widget;
};

G_DEFINE_TYPE (HdyStatusPage, hdy_status_page, GTK_TYPE_BIN)

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

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_status_page_finalize (GObject *object)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (object);

  g_clear_pointer (&self->icon_name, g_free);

  G_OBJECT_CLASS (hdy_status_page_parent_class)->finalize (object);
}

static void
hdy_status_page_destroy (GtkWidget *widget)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (widget);

  if (self->scrolled_window) {
    gtk_container_remove (GTK_CONTAINER (self), self->scrolled_window);
    self->toplevel_box = NULL;
    self->image = NULL;
    self->title_label = NULL;
    self->description_label = NULL;
    self->user_widget = NULL;
  }

  GTK_WIDGET_CLASS (hdy_status_page_parent_class)->destroy (widget);
}

static void
hdy_status_page_add (GtkContainer *container,
                     GtkWidget    *child)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (container);

  if (!self->scrolled_window) {
    GTK_CONTAINER_CLASS (hdy_status_page_parent_class)->add (container, child);
  } else if (!self->user_widget) {
    gtk_container_add (GTK_CONTAINER (self->toplevel_box), child);
    self->user_widget = child;
  } else {
    g_warning ("Attempting to add a second child to a HdyStatusPage, but a HdyStatusPage can only have one child");
  }
}

static void
hdy_status_page_remove (GtkContainer *container,
                        GtkWidget    *child)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (container);

  if (child == self->scrolled_window) {
    GTK_CONTAINER_CLASS (hdy_status_page_parent_class)->remove (container, child);
  } else if (child == self->user_widget) {
    gtk_container_remove (GTK_CONTAINER (self->toplevel_box), child);
    self->user_widget = NULL;
  } else {
    g_return_if_reached ();
  }
}

static void
hdy_status_page_forall (GtkContainer *container,
                        gboolean      include_internals,
                        GtkCallback   callback,
                        gpointer      callback_data)
{
  HdyStatusPage *self = HDY_STATUS_PAGE (container);

  if (include_internals)
    GTK_CONTAINER_CLASS (hdy_status_page_parent_class)->forall (container,
                                                                include_internals,
                                                                callback,
                                                                callback_data);
  else if (self->user_widget)
    callback (self->user_widget, callback_data);
}

static void
hdy_status_page_class_init (HdyStatusPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_status_page_get_property;
  object_class->set_property = hdy_status_page_set_property;
  object_class->finalize = hdy_status_page_finalize;
  widget_class->destroy = hdy_status_page_destroy;
  container_class->add = hdy_status_page_add;
  container_class->remove = hdy_status_page_remove;
  container_class->forall = hdy_status_page_forall;

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

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-status-page.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, toplevel_box);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, image);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, title_label);
  gtk_widget_class_bind_template_child (widget_class, HdyStatusPage, description_label);

  gtk_widget_class_set_css_name (widget_class, "statuspage");
}

static void
hdy_status_page_init (HdyStatusPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
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

  if (!icon_name)
    g_object_set (G_OBJECT (self->image), "icon-name", "image-missing", NULL);
  else
    g_object_set (G_OBJECT (self->image), "icon-name", icon_name, NULL);

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

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}
