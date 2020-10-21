/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_CLAMP_LAYOUT (hdy_clamp_layout_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyClampLayout, hdy_clamp_layout, HDY, CLAMP_LAYOUT, GtkLayoutManager)

HDY_AVAILABLE_IN_ALL
GtkLayoutManager *hdy_clamp_layout_new (void);

HDY_AVAILABLE_IN_ALL
gint hdy_clamp_layout_get_maximum_size (HdyClampLayout *self);
HDY_AVAILABLE_IN_ALL
void hdy_clamp_layout_set_maximum_size (HdyClampLayout *self,
                                        gint            maximum_size);

HDY_AVAILABLE_IN_ALL
gint hdy_clamp_layout_get_tightening_threshold (HdyClampLayout *self);
HDY_AVAILABLE_IN_ALL
void hdy_clamp_layout_set_tightening_threshold (HdyClampLayout *self,
                                                gint            tightening_threshold);

G_END_DECLS
