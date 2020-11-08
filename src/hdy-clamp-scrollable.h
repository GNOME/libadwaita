/*
 * Copyright (C) 2020 Purism SPC
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

#define HDY_TYPE_CLAMP_SCROLLABLE (hdy_clamp_scrollable_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyClampScrollable, hdy_clamp_scrollable, HDY, CLAMP_SCROLLABLE, GtkWidget)

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_clamp_scrollable_new (void);

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_clamp_scrollable_get_child (HdyClampScrollable *self);
HDY_AVAILABLE_IN_ALL
void       hdy_clamp_scrollable_set_child (HdyClampScrollable *self,
                                           GtkWidget          *child);

HDY_AVAILABLE_IN_ALL
gint hdy_clamp_scrollable_get_maximum_size (HdyClampScrollable *self);
HDY_AVAILABLE_IN_ALL
void hdy_clamp_scrollable_set_maximum_size (HdyClampScrollable *self,
                                            gint                maximum_size);

HDY_AVAILABLE_IN_ALL
gint hdy_clamp_scrollable_get_tightening_threshold (HdyClampScrollable *self);
HDY_AVAILABLE_IN_ALL
void hdy_clamp_scrollable_set_tightening_threshold (HdyClampScrollable *self,
                                                    gint                tightening_threshold);

G_END_DECLS
