/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
                                      GtkWindowClass *klass) G_GNUC_WARN_UNUSED_RESULT;

void adw_window_mixin_size_allocate (AdwWindowMixin *self,
                                     int             width,
                                     int             height,
                                     int             baseline);

GtkWidget *adw_window_mixin_get_content (AdwWindowMixin *self);
void       adw_window_mixin_set_content (AdwWindowMixin *self,
                                         GtkWidget      *content);

G_END_DECLS
