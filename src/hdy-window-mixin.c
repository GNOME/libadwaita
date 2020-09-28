/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-focus-private.h"
#include "hdy-gizmo-private.h"
#include "hdy-window-mixin-private.h"

/**
 * PRIVATE:hdy-window-mixin
 * @short_description: A helper object for #HdyWindow and #HdyApplicationWindow
 * @title: HdyWindowMixin
 * @See_also: #HdyApplicationWindow, #HdyWindow
 * @stability: Private
 *
 * The HdyWindowMixin object contains the implementation of the HdyWindow and
 * HdyApplicationWindow classes, providing a way to make a GtkWindow subclass
 * that has masked window corners on all sides and no titlebar by default,
 * allowing for more freedom with how to handle the titlebar for applications.
 *
 * Since: 1.0
 */

struct _HdyWindowMixin
{
  GObject parent;

  GtkWindow *window;
  GtkWindowClass *klass;

  GtkWidget *titlebar;
  GtkWidget *contents;
  GtkWidget *outline;

  GtkWidget *child;
};

G_DEFINE_TYPE (HdyWindowMixin, hdy_window_mixin, G_TYPE_OBJECT)

static gboolean
outline_contains (HdyGizmo *gizmo,
                  gdouble   x,
                  gdouble   y)
{
  return FALSE;
}

void
hdy_window_mixin_size_allocate (HdyWindowMixin *self,
                                gint            width,
                                gint            height,
                                gint            baseline)
{
  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (self->window) != self->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for HdyWindow");

  if (gtk_window_get_child (self->window) != self->contents)
    g_error ("gtk_window_set_child() is not supported for HdyWindow");

  GTK_WIDGET_CLASS (self->klass)->size_allocate (GTK_WIDGET (self->window),
                                                 width,
                                                 height,
                                                 baseline);
}

static void
hdy_window_mixin_class_init (HdyWindowMixinClass *klass)
{
}

static void
hdy_window_mixin_init (HdyWindowMixin *self)
{
}

HdyWindowMixin *
hdy_window_mixin_new (GtkWindow      *window,
                      GtkWindowClass *klass)
{
  HdyWindowMixin *self;

  g_return_val_if_fail (GTK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (GTK_IS_WINDOW_CLASS (klass), NULL);
  g_return_val_if_fail (GTK_IS_BUILDABLE (window), NULL);

  self = g_object_new (HDY_TYPE_WINDOW_MIXIN, NULL);

  self->window = window;
  self->klass = klass;

  self->titlebar = hdy_gizmo_new ("nothing", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_hide (self->titlebar);
  gtk_window_set_titlebar (self->window, self->titlebar);

  self->contents = hdy_gizmo_new ("contents", NULL, NULL, NULL, NULL,
                                  (HdyGizmoFocusFunc) hdy_widget_focus_child,
                                  (HdyGizmoGrabFocusFunc) hdy_widget_grab_focus_child);
  gtk_widget_set_layout_manager (self->contents, gtk_bin_layout_new ());
  gtk_window_set_child (window, self->contents);

  self->outline = hdy_gizmo_new ("outline", NULL, NULL, NULL, outline_contains, NULL, NULL);
  gtk_widget_set_parent (self->outline, self->contents);

  gtk_widget_add_css_class (GTK_WIDGET (window), "unified");

  return self;
}

void
hdy_window_mixin_set_child (HdyWindowMixin *self,
                            GtkWidget      *child)
{
  g_clear_pointer (&self->child, gtk_widget_unparent);

  if (child) {
    self->child = child;
    gtk_widget_insert_before (child, self->contents, self->outline);
  }
}

GtkWidget *
hdy_window_mixin_get_child (HdyWindowMixin *self)
{
  return self->child;
}
