/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-application-window.h"
#include "hdy-window-mixin-private.h"

/**
 * SECTION:hdy-application-window
 * @short_description: A freeform application window.
 * @title: HdyApplicationWindow
 * @See_also: #HdyHeaderBar, #HdyWindow, #HdyWindowHandle
 *
 * HdyApplicationWindow is a #GtkApplicationWindow subclass providing the same
 * features as #HdyWindow.
 *
 * See #HdyWindow for details.
 *
 * Using gtk_application_set_app_menu() and gtk_application_set_menubar() is
 * not supported and may result in visual glitches.
 *
 * Since: 1.0
 */

typedef struct
{
  HdyWindowMixin *mixin;
} HdyApplicationWindowPrivate;

static void hdy_application_window_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyApplicationWindow, hdy_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (HdyApplicationWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, hdy_application_window_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CHILD,
  LAST_PROP = PROP_0,
};

#define HDY_GET_WINDOW_MIXIN(obj) (((HdyApplicationWindowPrivate *) hdy_application_window_get_instance_private (HDY_APPLICATION_WINDOW (obj)))->mixin)

static void
hdy_application_window_size_allocate (GtkWidget *widget,
                                      gint       width,
                                      gint       height,
                                      gint       baseline)
{
  hdy_window_mixin_size_allocate (HDY_GET_WINDOW_MIXIN (widget),
                                  width,
                                  height,
                                  baseline);
}

static void
hdy_application_window_finalize (GObject *object)
{
  HdyApplicationWindow *self = (HdyApplicationWindow *)object;
  HdyApplicationWindowPrivate *priv = hdy_application_window_get_instance_private (self);

  g_clear_object (&priv->mixin);

  G_OBJECT_CLASS (hdy_application_window_parent_class)->finalize (object);
}

static void
hdy_application_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  HdyApplicationWindow *self = HDY_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, hdy_application_window_get_child (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_application_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  HdyApplicationWindow *self = HDY_APPLICATION_WINDOW (object);

  switch (prop_id) {
  case PROP_CHILD:
    hdy_application_window_set_child (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_application_window_class_init (HdyApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hdy_application_window_finalize;
  object_class->get_property = hdy_application_window_get_property;
  object_class->set_property = hdy_application_window_set_property;
  widget_class->size_allocate = hdy_application_window_size_allocate;

  g_object_class_override_property (object_class, PROP_CHILD, "child");
}

static void
hdy_application_window_init (HdyApplicationWindow *self)
{
  HdyApplicationWindowPrivate *priv = hdy_application_window_get_instance_private (self);

  priv->mixin = hdy_window_mixin_new (GTK_WINDOW (self),
                                      GTK_WINDOW_CLASS (hdy_application_window_parent_class));

  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (self), FALSE);
}

static void
hdy_application_window_buildable_add_child (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const gchar  *type)
{
  if (!g_strcmp0 (type, "titlebar"))
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (buildable, type);
  else if (GTK_IS_WIDGET (child))
    hdy_application_window_set_child (HDY_APPLICATION_WINDOW (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_application_window_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = hdy_application_window_buildable_add_child;
}

/**
 * hdy_application_window_new:
 * @app: a #GtkApplication
 *
 * Creates a new #HdyApplicationWindow for @app.
 *
 * Returns: (transfer full): a newly created #HdyApplicationWindow
 *
 * Since: 1.0
 */
GtkWidget *
hdy_application_window_new (GtkApplication *app)
{
  return g_object_new (HDY_TYPE_APPLICATION_WINDOW,
                       "application", app,
                       NULL);
}

/**
 * hdy_application_window_set_child:
 * @self: a #HdyApplicaitonWindow
 * @child: (allow-none): the child widget
 *
 * Sets the child widget of @self.
 */
void
hdy_application_window_set_child (HdyApplicationWindow *self,
                                  GtkWidget            *child)
{
  g_return_if_fail (HDY_IS_APPLICATION_WINDOW (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  hdy_window_mixin_set_child (HDY_GET_WINDOW_MIXIN (self), child);

  g_object_notify (G_OBJECT (self), "child");
}

/**
 * hdy_application_window_get_child:
 * @self: a #HdyApplicationWindow
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
hdy_application_window_get_child (HdyApplicationWindow *self)
{
  g_return_val_if_fail (HDY_IS_APPLICATION_WINDOW (self), NULL);

  return hdy_window_mixin_get_child (HDY_GET_WINDOW_MIXIN (self));
}
