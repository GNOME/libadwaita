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

#define HDY_TYPE_COLUMN (hdy_column_get_type())

G_DECLARE_FINAL_TYPE (HdyColumn, hdy_column, HDY, COLUMN, GtkBin)

HdyColumn *hdy_column_new (void);
gint hdy_column_get_maximum_width (HdyColumn *self);
void hdy_column_set_maximum_width (HdyColumn *self,
                                   gint       maximum_width);
gint hdy_column_get_linear_growth_width (HdyColumn *self);
void hdy_column_set_linear_growth_width (HdyColumn *self,
                                         gint       linear_growth_width);

G_END_DECLS
