/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-gizmo-private.h"
#include "adw-macros-private.h"
#include "adw-widget-utils-private.h"
#include "adw-window-mixin-private.h"

struct _AdwWindowMixin
{
  GObject parent;

  GtkWindow *window;
  GtkWindowClass *klass;

  GtkWidget *titlebar;
  GtkWidget *child;

  GtkWidget *content;
};

G_DEFINE_FINAL_TYPE (AdwWindowMixin, adw_window_mixin, G_TYPE_OBJECT)

void
adw_window_mixin_size_allocate (AdwWindowMixin *self,
                                int             width,
                                int             height,
                                int             baseline)
{
  /* We don't want to allow any other titlebar */
  if (gtk_window_get_titlebar (self->window) != self->titlebar)
    g_error ("gtk_window_set_titlebar() is not supported for AdwWindow");

  if (gtk_window_get_child (self->window) != self->child)
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

  self->titlebar = adw_gizmo_new_with_role ("nothing", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                            NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_hide (self->titlebar);
  gtk_window_set_titlebar (self->window, self->titlebar);

  self->child = adw_gizmo_new_with_role ("contents", GTK_ACCESSIBLE_ROLE_GROUP,
                                         NULL, NULL, NULL, NULL,
                                         (AdwGizmoFocusFunc) adw_widget_focus_child,
                                         (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child);
  gtk_widget_set_layout_manager (self->child, gtk_bin_layout_new ());
  gtk_window_set_child (window, self->child);

  return self;
}

void
adw_window_mixin_set_content (AdwWindowMixin *self,
                              GtkWidget      *content)
{
  g_clear_pointer (&self->content, gtk_widget_unparent);

  if (content) {
    self->content = content;
    gtk_widget_set_parent (content, self->child);
  }
}

GtkWidget *
adw_window_mixin_get_content (AdwWindowMixin *self)
{
  return self->content;
}
