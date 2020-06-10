/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_WINDOW_MIXIN (hdy_window_mixin_get_type())

G_DECLARE_FINAL_TYPE (HdyWindowMixin, hdy_window_mixin, HDY, WINDOW_MIXIN, GObject)

HdyWindowMixin *hdy_window_mixin_new (GtkWindow      *window,
                                      GtkWindowClass *klass);

void            hdy_window_mixin_add (HdyWindowMixin *self,
                                      GtkWidget      *widget);
void            hdy_window_mixin_remove (HdyWindowMixin *self,
                                         GtkWidget      *widget);
void            hdy_window_mixin_forall (HdyWindowMixin *self,
                                         gboolean        include_internals,
                                         GtkCallback     callback,
                                         gpointer        callback_data);

gboolean        hdy_window_mixin_draw (HdyWindowMixin *self,
                                       cairo_t        *cr);
void            hdy_window_mixin_destroy (HdyWindowMixin *self);

void            hdy_window_mixin_buildable_add_child (HdyWindowMixin *self,
                                                      GtkBuilder     *builder,
                                                      GObject        *child,
                                                      const gchar    *type);

G_END_DECLS
