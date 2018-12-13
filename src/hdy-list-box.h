/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

void
hdy_list_box_separator_header (GtkListBoxRow *row,
                               GtkListBoxRow *before,
                               gpointer       unused_user_data);

G_END_DECLS
