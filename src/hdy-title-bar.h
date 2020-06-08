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

#define HDY_TYPE_TITLE_BAR (hdy_title_bar_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyTitleBar, hdy_title_bar, HDY, TITLE_BAR, GtkBin)

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_title_bar_new (void);

HDY_AVAILABLE_IN_ALL
gboolean hdy_title_bar_get_selection_mode (HdyTitleBar *self);
HDY_AVAILABLE_IN_ALL
void hdy_title_bar_set_selection_mode (HdyTitleBar *self,
                                       gboolean     selection_mode);

G_END_DECLS
