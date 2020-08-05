/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>

#include "hdy-view-switcher.h"

G_BEGIN_DECLS

#define HDY_TYPE_VIEW_SWITCHER_BAR (hdy_view_switcher_bar_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyViewSwitcherBar, hdy_view_switcher_bar, HDY, VIEW_SWITCHER_BAR, GtkBin)

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_view_switcher_bar_new (void);

HDY_AVAILABLE_IN_ALL
HdyViewSwitcherPolicy hdy_view_switcher_bar_get_policy (HdyViewSwitcherBar *self);
HDY_AVAILABLE_IN_ALL
void                  hdy_view_switcher_bar_set_policy (HdyViewSwitcherBar    *self,
                                                        HdyViewSwitcherPolicy  policy);

HDY_AVAILABLE_IN_ALL
GtkStack *hdy_view_switcher_bar_get_stack (HdyViewSwitcherBar *self);
HDY_AVAILABLE_IN_ALL
void      hdy_view_switcher_bar_set_stack (HdyViewSwitcherBar *self,
                                           GtkStack           *stack);

HDY_AVAILABLE_IN_ALL
gboolean hdy_view_switcher_bar_get_reveal (HdyViewSwitcherBar *self);
HDY_AVAILABLE_IN_ALL
void     hdy_view_switcher_bar_set_reveal (HdyViewSwitcherBar *self,
                                           gboolean            reveal);

G_END_DECLS
