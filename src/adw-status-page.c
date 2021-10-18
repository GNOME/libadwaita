/*
 * Copyright (C) 2020 Andrei Lișiță <andreii.lisita@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-status-page.h"

#include "adw-widget-utils-private.h"

/**
 * AdwStatusPage:
 *
 * A page used for empty/error states and similar use-cases.
 *
 * The `AdwStatusPage` widget can have an icon, a title, a description and a
 * custom widget which is displayed below them.
 *
 * ## CSS nodes
 *
 * `AdwStatusPage` has a main CSS node with name `statuspage`.
 *
 * `AdwStatusPage` can use the `.compact` style class for when it needs to fit
 * into a small space such a sidebar or a popover.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_PAINTABLE,
  PROP_TITLE,
  PROP_DESCRIPTION,
  PROP_CHILD,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

struct _AdwStatusPage
{
  GtkWidget parent_instance;

  GtkWidget *scrolled_window;
  GtkBox *toplevel_box;
  GtkImage *image;
  char *icon_name;
  GdkPaintable *paintable;
  GtkLabel *title_label;
  GtkLabel *description_label;

  GtkWidget *user_widget;
};

static void adw_status_page_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwStatusPage, adw_status_page, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_status_page_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static gboolean
has_image (AdwStatusPage *self,
           const char    *icon_name,
           GdkPaintable  *paintable)
{
  return paintable || (icon_name && icon_name[0]);
}

static gboolean
string_is_not_empty (AdwStatusPage *self,
                     const char    *string)
{
  return string && string[0];
}

static void
adw_status_page_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdwStatusPage *self = ADW_STATUS_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_status_page_get_icon_name (self));
    break;

  case PROP_PAINTABLE:
    g_value_set_object (value, adw_status_page_get_paintable (self));
    break;

  case PROP_TITLE:
    g_value_set_string (value, adw_status_page_get_title (self));
    break;

  case PROP_DESCRIPTION:
    g_value_set_string (value, adw_status_page_get_description (self));
    break;

  case PROP_CHILD:
    g_value_set_object (value, adw_status_page_get_child (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_status_page_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdwStatusPage *self = ADW_STATUS_PAGE (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adw_status_page_set_icon_name (self, g_value_get_string (value));
    break;

  case PROP_PAINTABLE:
    adw_status_page_set_paintable (self, g_value_get_object (value));
    break;

  case PROP_TITLE:
    adw_status_page_set_title (self, g_value_get_string (value));
    break;

  case PROP_DESCRIPTION:
    adw_status_page_set_description (self, g_value_get_string (value));
    break;

  case PROP_CHILD:
    adw_status_page_set_child (self, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_status_page_dispose (GObject *object)
{
  AdwStatusPage *self = ADW_STATUS_PAGE (object);

  adw_status_page_set_child (self, NULL);

  gtk_widget_unparent (self->scrolled_window);
  self->user_widget = NULL;

  G_OBJECT_CLASS (adw_status_page_parent_class)->dispose (object);
}

static void
adw_status_page_finalize (GObject *object)
{
  AdwStatusPage *self = ADW_STATUS_PAGE (object);

  g_clear_pointer (&self->icon_name, g_free);
  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (adw_status_page_parent_class)->finalize (object);
}

static void
adw_status_page_class_init (AdwStatusPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_status_page_get_property;
  object_class->set_property = adw_status_page_set_property;
  object_class->dispose = adw_status_page_dispose;
  object_class->finalize = adw_status_page_finalize;

  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwStatusPage:icon-name: (attributes org.gtk.Property.get=adw_status_page_get_icon_name org.gtk.Property.set=adw_status_page_set_icon_name)
   *
   * The name of the icon to be used.
   *
   * Changing this will clear [property@Adw.StatusPage:paintable] out.
   *
   * Since: 1.0
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon name",
                         "The name of the icon to be used",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwStatusPage:paintable: (attributes org.gtk.Property.get=adw_status_page_get_paintable org.gtk.Property.set=adw_status_page_set_paintable)
   *
   * The @GdkPaintable to be used.
   *
   * Changing this will clear [property@Adw.StatusPage:icon-name] out.
   *
   * Since: 1.0
   */
  props[PROP_PAINTABLE] =
    g_param_spec_object ("paintable",
                         "Paintable",
                         "The GdkPaintable to be used",
                         GDK_TYPE_PAINTABLE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwStatusPage:title: (attributes org.gtk.Property.get=adw_status_page_get_title org.gtk.Property.set=adw_status_page_set_title)
   *
   * The title to be displayed below the icon.
   *
   * Since: 1.0
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "The title to be displayed below the icon",
                         "",
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwStatusPage:description: (attributes org.gtk.Property.get=adw_status_page_get_description org.gtk.Property.set=adw_status_page_set_description)
   *
   * The description to be displayed below the title.
   *
   * Since: 1.0
   */
  props[PROP_DESCRIPTION] =
    g_param_spec_string ("description",
                         "Description",
                         "The description to be displayed below the title",
                         "",
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwStatusPage:child: (attributes org.gtk.Property.get=adw_status_page_get_child org.gtk.Property.set=adw_status_page_set_child)
   *
   * The child widget.
   *
   * Since: 1.0
   */
  props[PROP_CHILD] =
    g_param_spec_object ("child",
                         "Child",
                         "The child widget",
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-status-page.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwStatusPage, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, AdwStatusPage, toplevel_box);
  gtk_widget_class_bind_template_child (widget_class, AdwStatusPage, image);
  gtk_widget_class_bind_template_child (widget_class, AdwStatusPage, title_label);
  gtk_widget_class_bind_template_child (widget_class, AdwStatusPage, description_label);
  gtk_widget_class_bind_template_callback (widget_class, has_image);
  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "statuspage");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_status_page_init (AdwStatusPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
adw_status_page_buildable_add_child (GtkBuildable *buildable,
                                     GtkBuilder   *builder,
                                     GObject      *child,
                                     const char   *type)
{
  AdwStatusPage *self = ADW_STATUS_PAGE (buildable);

  if (!self->scrolled_window && GTK_IS_WIDGET (child))
    gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (buildable));
  else if (GTK_IS_WIDGET (child))
    adw_status_page_set_child (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_status_page_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_status_page_buildable_add_child;
}


/**
 * adw_status_page_new:
 *
 * Creates a new `AdwStatusPage`.
 *
 * Returns: the newly created `AdwStatusPage`
 *
 * Since: 1.0
 */
GtkWidget *
adw_status_page_new (void)
{
  return g_object_new (ADW_TYPE_STATUS_PAGE, NULL);
}

/**
 * adw_status_page_get_icon_name: (attributes org.gtk.Method.get_property=icon-name)
 * @self: a `AdwStatusPage`
 *
 * Gets the icon name for @self.
 *
 * Returns: (nullable): the icon name
 *
 * Since: 1.0
 */
const char *
adw_status_page_get_icon_name (AdwStatusPage *self)
{
  g_return_val_if_fail (ADW_IS_STATUS_PAGE (self), NULL);

  return self->icon_name;
}

/**
 * adw_status_page_set_icon_name: (attributes org.gtk.Method.set_property=icon-name)
 * @self: a `AdwStatusPage`
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name for @self.
 *
 * Since: 1.0
 */
void
adw_status_page_set_icon_name (AdwStatusPage *self,
                               const char    *icon_name)
{
  g_return_if_fail (ADW_IS_STATUS_PAGE (self));

  if (g_strcmp0 (self->icon_name, icon_name) == 0)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (self->paintable) {
    g_clear_object (&self->paintable);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAINTABLE]);
  }

  g_free (self->icon_name);
  self->icon_name = g_strdup (icon_name);
  gtk_image_set_from_icon_name (self->image, self->icon_name);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_status_page_get_paintable: (attributes org.gtk.Method.get_property=paintable)
 * @self: a `AdwStatusPage`
 *
 * Gets the paintable for @self.
 *
 * Returns: (nullable) (transfer none): the paintable
 *
 * Since: 1.0
 */
GdkPaintable *
adw_status_page_get_paintable (AdwStatusPage *self)
{
  g_return_val_if_fail (ADW_IS_STATUS_PAGE (self), NULL);

  return self->paintable;
}

/**
 * adw_status_page_set_paintable: (attributes org.gtk.Method.set_property=paintable)
 * @self: a `AdwStatusPage`
 * @paintable: (nullable): the paintable
 *
 * Sets the paintable for @self.
 *
 * Since: 1.0
 */
void
adw_status_page_set_paintable (AdwStatusPage *self,
                               GdkPaintable  *paintable)
{
  g_return_if_fail (ADW_IS_STATUS_PAGE (self));
  g_return_if_fail (paintable == NULL || GDK_IS_PAINTABLE (paintable));

  if (self->paintable == paintable)
    return;

  g_object_freeze_notify (G_OBJECT (self));

  if (self->icon_name) {
    g_clear_pointer (&self->icon_name, g_free);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
  }

  g_set_object (&self->paintable, paintable);
  gtk_image_set_from_paintable (self->image, self->paintable);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAINTABLE]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_status_page_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a `AdwStatusPage`
 *
 * Gets the title for @self.
 *
 * Returns: the title
 *
 * Since: 1.0
 */
const char *
adw_status_page_get_title (AdwStatusPage *self)
{
  g_return_val_if_fail (ADW_IS_STATUS_PAGE (self), NULL);

  return gtk_label_get_label (self->title_label);
}

/**
 * adw_status_page_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a `AdwStatusPage`
 * @title: the title
 *
 * Sets the title for @self.
 *
 * Since: 1.0
 */
void
adw_status_page_set_title (AdwStatusPage *self,
                           const char    *title)
{
  g_return_if_fail (ADW_IS_STATUS_PAGE (self));

  if (g_strcmp0 (title, adw_status_page_get_title (self)) == 0)
    return;

  gtk_label_set_label (self->title_label, title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_status_page_get_description: (attributes org.gtk.Method.get_property=description)
 * @self: a `AdwStatusPage`
 *
 * Gets the description for @self.
 *
 * Returns: (nullable): the description
 *
 * Since: 1.0
 */
const char *
adw_status_page_get_description (AdwStatusPage *self)
{
  g_return_val_if_fail (ADW_IS_STATUS_PAGE (self), NULL);

  return gtk_label_get_label (self->description_label);
}

/**
 * adw_status_page_set_description: (attributes org.gtk.Method.set_property=description)
 * @self: a `AdwStatusPage`
 * @description: (nullable): the description
 *
 * Sets the description for @self.
 *
 * Since: 1.0
 */
void
adw_status_page_set_description (AdwStatusPage *self,
                                 const char    *description)
{
  g_return_if_fail (ADW_IS_STATUS_PAGE (self));

  if (g_strcmp0 (description, adw_status_page_get_description (self)) == 0)
    return;

  gtk_label_set_label (self->description_label, description);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}

/**
 * adw_status_page_get_child: (attributes org.gtk.Method.get_property=child)
 * @self: a `AdwStatusPage`
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.0
 */
GtkWidget *
adw_status_page_get_child (AdwStatusPage *self)
{
  g_return_val_if_fail (ADW_IS_STATUS_PAGE (self), NULL);

  return self->user_widget;
}

/**
 * adw_status_page_set_child: (attributes org.gtk.Method.set_property=child)
 * @self: a `AdwStatusPage`
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.0
 */
void
adw_status_page_set_child (AdwStatusPage *self,
                           GtkWidget     *child)
{
  g_return_if_fail (ADW_IS_STATUS_PAGE (self));

  if (child == self->user_widget)
    return;

  if (self->user_widget)
    gtk_box_remove (self->toplevel_box, self->user_widget);

  self->user_widget = child;

  if (self->user_widget)
    gtk_box_append (self->toplevel_box, self->user_widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DESCRIPTION]);
}
