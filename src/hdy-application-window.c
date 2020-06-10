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

#define HDY_GET_WINDOW_MIXIN(obj) (((HdyApplicationWindowPrivate *) hdy_application_window_get_instance_private (HDY_APPLICATION_WINDOW (obj)))->mixin)

static void
hdy_application_window_add (GtkContainer *container,
                            GtkWidget    *widget)
{
  hdy_window_mixin_add (HDY_GET_WINDOW_MIXIN (container), widget);
}

static void
hdy_application_window_remove (GtkContainer *container,
                               GtkWidget    *widget)
{
  hdy_window_mixin_remove (HDY_GET_WINDOW_MIXIN (container), widget);
}

static void
hdy_application_window_forall (GtkContainer *container,
                               gboolean      include_internals,
                               GtkCallback   callback,
                               gpointer      callback_data)
{
  hdy_window_mixin_forall (HDY_GET_WINDOW_MIXIN (container),
                           include_internals,
                           callback,
                           callback_data);
}

static gboolean
hdy_application_window_draw (GtkWidget *widget,
                             cairo_t   *cr)
{
  return hdy_window_mixin_draw (HDY_GET_WINDOW_MIXIN (widget), cr);
}

static void
hdy_application_window_destroy (GtkWidget *widget)
{
  hdy_window_mixin_destroy (HDY_GET_WINDOW_MIXIN (widget));
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
hdy_application_window_class_init (HdyApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = hdy_application_window_finalize;
  widget_class->draw = hdy_application_window_draw;
  widget_class->destroy = hdy_application_window_destroy;
  container_class->add = hdy_application_window_add;
  container_class->remove = hdy_application_window_remove;
  container_class->forall = hdy_application_window_forall;
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
  hdy_window_mixin_buildable_add_child (HDY_GET_WINDOW_MIXIN (buildable),
                                        builder,
                                        child,
                                        type);
}

static void
hdy_application_window_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = hdy_application_window_buildable_add_child;
}

/**
 * hdy_application_window_new:
 *
 * Creates a new #HdyApplicationWindow.
 *
 * Returns: (transfer full): a newly created #HdyApplicationWindow
 *
 * Since: 1.0
 */
GtkWidget *
hdy_application_window_new (void)
{
  return g_object_new (HDY_TYPE_APPLICATION_WINDOW,
                       NULL);
}
