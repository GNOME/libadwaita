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

void            hdy_window_mixin_size_allocate (HdyWindowMixin *self,
                                                gint            width,
                                                gint            height,
                                                gint            baseline);

void            hdy_window_mixin_set_child (HdyWindowMixin *self,
                                            GtkWidget      *child);
GtkWidget      *hdy_window_mixin_get_child (HdyWindowMixin *self);

G_END_DECLS
