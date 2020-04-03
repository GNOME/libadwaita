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

#define HDY_TYPE_CLAMP (hdy_clamp_get_type())

G_DECLARE_FINAL_TYPE (HdyClamp, hdy_clamp, HDY, CLAMP, GtkBin)

GtkWidget *hdy_clamp_new (void);
gint hdy_clamp_get_maximum_width (HdyClamp *self);
void hdy_clamp_set_maximum_width (HdyClamp *self,
                                  gint      maximum_width);
gint hdy_clamp_get_linear_growth_width (HdyClamp *self);
void hdy_clamp_set_linear_growth_width (HdyClamp *self,
                                        gint      linear_growth_width);

G_END_DECLS
