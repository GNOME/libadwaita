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

#include <gtk/gtk.h>

#include "hdy-view-switcher.h"

G_BEGIN_DECLS

#define HDY_TYPE_VIEW_SWITCHER_BAR (hdy_view_switcher_bar_get_type())

struct _HdyViewSwitcherBarClass {
  GtkBinClass parent_class;
};

G_DECLARE_DERIVABLE_TYPE (HdyViewSwitcherBar, hdy_view_switcher_bar, HDY, VIEW_SWITCHER_BAR, GtkBin)

HdyViewSwitcherBar *hdy_view_switcher_bar_new (void);

HdyViewSwitcherPolicy hdy_view_switcher_bar_get_policy (HdyViewSwitcherBar *self);
void                  hdy_view_switcher_bar_set_policy (HdyViewSwitcherBar    *self,
                                                        HdyViewSwitcherPolicy  policy);

GtkIconSize hdy_view_switcher_bar_get_icon_size (HdyViewSwitcherBar *self);
void        hdy_view_switcher_bar_set_icon_size (HdyViewSwitcherBar *self,
                                                 GtkIconSize         icon_size);

GtkStack *hdy_view_switcher_bar_get_stack (HdyViewSwitcherBar *self);
void      hdy_view_switcher_bar_set_stack (HdyViewSwitcherBar *self,
                                           GtkStack           *stack);

gboolean hdy_view_switcher_bar_get_reveal (HdyViewSwitcherBar *self);
void     hdy_view_switcher_bar_set_reveal (HdyViewSwitcherBar *self,
                                           gboolean            reveal);

G_END_DECLS
