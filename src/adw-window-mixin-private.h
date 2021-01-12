/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_WINDOW_MIXIN (adw_window_mixin_get_type())

G_DECLARE_FINAL_TYPE (AdwWindowMixin, adw_window_mixin, ADW, WINDOW_MIXIN, GObject)

AdwWindowMixin *adw_window_mixin_new (GtkWindow      *window,
                                      GtkWindowClass *klass);

void            adw_window_mixin_size_allocate (AdwWindowMixin *self,
                                                gint            width,
                                                gint            height,
                                                gint            baseline);

void            adw_window_mixin_set_child (AdwWindowMixin *self,
                                            GtkWidget      *child);
GtkWidget      *adw_window_mixin_get_child (AdwWindowMixin *self);

G_END_DECLS
