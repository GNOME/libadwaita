/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-window-handle.h"
#include "hdy-window-handle-controller-private.h"

/**
 * SECTION:hdy-window-handle
 * @short_description: A bin that acts like a titlebar.
 * @Title: HdyWindowHandle
 * @See_also: #HdyApplicationWindow, #HdyHeaderBar, #HdyWindow
 *
 * HdyWindowHandle is a #GtkBin subclass that can be dragged to move its
 * #GtkWindow, and handles right click, middle click and double click as
 * expected from a titlebar. This is particularly useful with #HdyWindow or
 * #HdyApplicationWindow.
 *
 * It isn't necessary to use #HdyWindowHandle if you use #HdyHeaderBar.
 *
 * It can be safely nested or used in the actual window titlebar.
 *
 * # CSS nodes
 *
 * #HdyWindowHandle has a single CSS node with name windowhandle.
 *
 * Since: 1.0
 */

struct _HdyWindowHandle
{
  GtkEventBox parent_instance;

  HdyWindowHandleController *controller;
};

G_DEFINE_TYPE (HdyWindowHandle, hdy_window_handle, GTK_TYPE_EVENT_BOX)

static void
hdy_window_handle_finalize (GObject *object)
{
  HdyWindowHandle *self = (HdyWindowHandle *)object;

  g_clear_object (&self->controller);

  G_OBJECT_CLASS (hdy_window_handle_parent_class)->finalize (object);
}

static void
hdy_window_handle_class_init (HdyWindowHandleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hdy_window_handle_finalize;

  gtk_widget_class_set_css_name (widget_class, "windowhandle");
}

static void
hdy_window_handle_init (HdyWindowHandle *self)
{
  self->controller = hdy_window_handle_controller_new (GTK_WIDGET (self));
}

/**
 * hdy_window_handle_new:
 *
 * Creates a new #HdyWindowHandle.
 *
 * Returns: (transfer full): a newly created #HdyWindowHandle
 *
 * Since: 1.0
 */
GtkWidget *
hdy_window_handle_new (void)
{
  return g_object_new (HDY_TYPE_WINDOW_HANDLE, NULL);
}
