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

G_BEGIN_DECLS

#define HDY_TYPE_VIEW_SWITCHER (hdy_view_switcher_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyViewSwitcher, hdy_view_switcher, HDY, VIEW_SWITCHER, GtkBin)

typedef enum {
  HDY_VIEW_SWITCHER_POLICY_AUTO,
  HDY_VIEW_SWITCHER_POLICY_NARROW,
  HDY_VIEW_SWITCHER_POLICY_WIDE,
} HdyViewSwitcherPolicy;

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_view_switcher_new (void);

HDY_AVAILABLE_IN_ALL
HdyViewSwitcherPolicy hdy_view_switcher_get_policy (HdyViewSwitcher *self);
HDY_AVAILABLE_IN_ALL
void                  hdy_view_switcher_set_policy (HdyViewSwitcher       *self,
                                                    HdyViewSwitcherPolicy  policy);

HDY_AVAILABLE_IN_ALL
PangoEllipsizeMode hdy_view_switcher_get_narrow_ellipsize (HdyViewSwitcher *self);
HDY_AVAILABLE_IN_ALL
void               hdy_view_switcher_set_narrow_ellipsize (HdyViewSwitcher    *self,
                                                           PangoEllipsizeMode  mode);

HDY_AVAILABLE_IN_ALL
GtkStack *hdy_view_switcher_get_stack (HdyViewSwitcher *self);
HDY_AVAILABLE_IN_ALL
void      hdy_view_switcher_set_stack (HdyViewSwitcher *self,
                                       GtkStack        *stack);

G_END_DECLS
