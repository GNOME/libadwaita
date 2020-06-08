/*
 * Copyright (C) 2018 Purism SPC
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

#define HDY_TYPE_CLAMP (hdy_clamp_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyClamp, hdy_clamp, HDY, CLAMP, GtkBin)

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_clamp_new (void);
HDY_AVAILABLE_IN_ALL
gint hdy_clamp_get_maximum_size (HdyClamp *self);
HDY_AVAILABLE_IN_ALL
void hdy_clamp_set_maximum_size (HdyClamp *self,
                                 gint      maximum_size);
HDY_AVAILABLE_IN_ALL
gint hdy_clamp_get_tightening_threshold (HdyClamp *self);
HDY_AVAILABLE_IN_ALL
void hdy_clamp_set_tightening_threshold (HdyClamp *self,
                                         gint      tightening_threshold);

G_END_DECLS
