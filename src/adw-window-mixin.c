/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-focus-private.h"
#include "adw-gizmo-private.h"
#include "adw-window-mixin-private.h"

/**
 * PRIVATE:adw-window-mixin
 * @short_description: A helper object for #AdwWindow and #AdwApplicationWindow
 * @title: AdwWindowMixin
 * @See_also: #AdwApplicationWindow, #AdwWindow
 * @stability: Private
 *
 * The AdwWindowMixin object contains the implementation of the AdwWindow and
 * AdwApplicationWindow classes, providing a way to make a GtkWindow subclass
 * that has masked window corners on all sides and no titlebar by default,
 * allowing for more freedom with how to handle the titlebar for applications.
 *
 * Since: 1.0
 */

struct _AdwWindowMixin
{
  GObject parent;

  GtkWindow *window;
  GtkWindowClass *klass;

  GtkWidget *titlebar;
  GtkWidget *contents;
  GtkWidget *outline;

  GtkWidget *child;
};

G_DEFINE_TYPE (AdwWindowMixin, adw_window_mixin, G_TYPE_OBJECT)

static gboolean
outline_contains (AdwGizmo *gizmo,
                  gdouble   x,
                  gdouble   y)
{
  return FALSE;
}

void
adw_window_mixin_size_allocate (AdwWindowMixin *self,
                                gint            width,
                                gint            height,
                                gint            baseline)
{
  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (self->window) != self->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for AdwWindow");

  if (gtk_window_get_child (self->window) != self->contents)
    g_error ("gtk_window_set_child() is not supported for AdwWindow");

  GTK_WIDGET_CLASS (self->klass)->size_allocate (GTK_WIDGET (self->window),
                                                 width,
                                                 height,
                                                 baseline);
}

static void
adw_window_mixin_class_init (AdwWindowMixinClass *klass)
{
}

static void
adw_window_mixin_init (AdwWindowMixin *self)
{
}

AdwWindowMixin *
adw_window_mixin_new (GtkWindow      *window,
                      GtkWindowClass *klass)
{
  AdwWindowMixin *self;

  g_return_val_if_fail (GTK_IS_WINDOW (window), NULL);
  g_return_val_if_fail (GTK_IS_WINDOW_CLASS (klass), NULL);
  g_return_val_if_fail (GTK_IS_BUILDABLE (window), NULL);

  self = g_object_new (ADW_TYPE_WINDOW_MIXIN, NULL);

  self->window = window;
  self->klass = klass;

  self->titlebar = adw_gizmo_new ("nothing", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_hide (self->titlebar);
  gtk_window_set_titlebar (self->window, self->titlebar);

  self->contents = adw_gizmo_new ("contents", NULL, NULL, NULL, NULL,
                                  (AdwGizmoFocusFunc) adw_widget_focus_child,
                                  (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child);
  gtk_widget_set_layout_manager (self->contents, gtk_bin_layout_new ());
  gtk_window_set_child (window, self->contents);

  self->outline = adw_gizmo_new ("outline", NULL, NULL, NULL, outline_contains, NULL, NULL);
  gtk_widget_set_parent (self->outline, self->contents);

  gtk_widget_add_css_class (GTK_WIDGET (window), "unified");

  return self;
}

void
adw_window_mixin_set_child (AdwWindowMixin *self,
                            GtkWidget      *child)
{
  g_clear_pointer (&self->child, gtk_widget_unparent);

  if (child) {
    self->child = child;
    gtk_widget_insert_before (child, self->contents, self->outline);
  }
}

GtkWidget *
adw_window_mixin_get_child (AdwWindowMixin *self)
{
  return self->child;
}
