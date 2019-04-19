/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

void hdy_gtk_window_toggle_maximized (GtkWindow *window);
GdkPixbuf *hdy_gtk_window_get_icon_for_size (GtkWindow *window,
                                             gint       size);

G_END_DECLS
