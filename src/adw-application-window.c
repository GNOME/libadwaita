/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-application-window.h"
#include "adw-window-mixin-private.h"

/**
 * SECTION:adw-application-window
 * @short_description: A freeform application window.
 * @title: AdwApplicationWindow
 * @See_also: #AdwWindow
 *
 * AdwApplicationWindow is a #GtkApplicationWindow subclass providing the same
 * features as #AdwWindow.
 *
 * See #AdwWindow for details.
 *
 * Using gtk_application_set_app_menu() and gtk_application_set_menubar() is
 * not supported and may result in visual glitches.
 *
 * Since: 1.0
 */

typedef struct
{
  AdwWindowMixin *mixin;
} AdwApplicationWindowPrivate;

static void adw_application_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwApplicationWindow, adw_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (AdwApplicationWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_application_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  LAST_PROP = PROP_0,
};

#define ADW_GET_WINDOW_MIXIN(obj) (((AdwApplicationWindowPrivate *) adw_application_window_get_instance_private (ADW_APPLICATION_WINDOW (obj)))->mixin)

static void
adw_application_window_size_allocate (GtkWidget *widget,
                                      gint       width,
                                      gint       height,
                                      gint       baseline)
{
  adw_window_mixin_size_allocate (ADW_GET_WINDOW_MIXIN (widget),
                                  width,
                                  height,
                                  baseline);
}

static void
adw_application_window_finalize (GObject *object)
{
  AdwApplicationWindow *self = (AdwApplicationWindow *)object;
  AdwApplicationWindowPrivate *priv = adw_application_window_get_instance_private (self);

  g_clear_object (&priv->mixin);

  G_OBJECT_CLASS (adw_application_window_parent_class)->finalize (object);
}

static void
adw_application_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_application_window_get_child (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_application_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AdwApplicationWindow *self = ADW_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_application_window_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_application_window_class_init (AdwApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = adw_application_window_finalize;
  object_class->get_property = adw_application_window_get_property;
  object_class->set_property = adw_application_window_set_property;
  widget_class->size_allocate = adw_application_window_size_allocate;

  g_object_class_override_property (object_class, PROP_CHILD, "child");
}

static void
adw_application_window_init (AdwApplicationWindow *self)
{
  AdwApplicationWindowPrivate *priv = adw_application_window_get_instance_private (self);

  priv->mixin = adw_window_mixin_new (GTK_WINDOW (self),
                                      GTK_WINDOW_CLASS (adw_application_window_parent_class));

  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), FALSE);
}

static void
adw_application_window_buildable_add_child (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const gchar  *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    adw_application_window_set_child (ADW_APPLICATION_WINDOW (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_application_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_application_window_buildable_add_child;
}

/**
 * adw_application_window_new:
 * @app: a #GtkApplication
 *
 * Creates a new #AdwApplicationWindow for @app.
 *
 * Returns: (transfer full): a newly created #AdwApplicationWindow
 *
 * Since: 1.0
 */
GtkWidget *
adw_application_window_new (GtkApplication *app)
{
  return g_object_new (ADW_TYPE_APPLICATION_WINDOW,
                       "application", app,
                       NULL);
}

/**
 * adw_application_window_set_child:
 * @self: a #AdwApplicationWindow
 * @child: (allow-none): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.0
 */
void
adw_application_window_set_child (AdwApplicationWindow *self,
                                  GtkWidget            *child)
{
  g_return_if_fail (ADW_IS_APPLICATION_WINDOW (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  adw_window_mixin_set_child (ADW_GET_WINDOW_MIXIN (self), child);

  g_object_notify (G_OBJECT (self), "child");
}

/**
 * adw_application_window_get_child:
 * @self: a #AdwApplicationWindow
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.0
 */
GtkWidget *
adw_application_window_get_child (AdwApplicationWindow *self)
{
  g_return_val_if_fail (ADW_IS_APPLICATION_WINDOW (self), NULL);

  return adw_window_mixin_get_child (ADW_GET_WINDOW_MIXIN (self));
}
