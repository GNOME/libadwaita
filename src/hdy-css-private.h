/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

void hdy_css_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      gint           *minimum,
                      gint           *natural);

void hdy_css_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation);

G_END_DECLS
