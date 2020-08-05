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

#define HDY_TYPE_VIEW_SWITCHER_TITLE (hdy_view_switcher_title_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyViewSwitcherTitle, hdy_view_switcher_title, HDY, VIEW_SWITCHER_TITLE, GtkBin)

HDY_AVAILABLE_IN_ALL
HdyViewSwitcherTitle *hdy_view_switcher_title_new (void);

HDY_AVAILABLE_IN_ALL
HdyViewSwitcherPolicy hdy_view_switcher_title_get_policy (HdyViewSwitcherTitle *self);
HDY_AVAILABLE_IN_ALL
void                  hdy_view_switcher_title_set_policy (HdyViewSwitcherTitle  *self,
                                                          HdyViewSwitcherPolicy  policy);

HDY_AVAILABLE_IN_ALL
GtkStack *hdy_view_switcher_title_get_stack (HdyViewSwitcherTitle *self);
HDY_AVAILABLE_IN_ALL
void      hdy_view_switcher_title_set_stack (HdyViewSwitcherTitle *self,
                                             GtkStack             *stack);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_view_switcher_title_get_title (HdyViewSwitcherTitle *self);
HDY_AVAILABLE_IN_ALL
void         hdy_view_switcher_title_set_title (HdyViewSwitcherTitle *self,
                                                const gchar          *title);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_view_switcher_title_get_subtitle (HdyViewSwitcherTitle *self);
HDY_AVAILABLE_IN_ALL
void         hdy_view_switcher_title_set_subtitle (HdyViewSwitcherTitle *self,
                                                   const gchar          *subtitle);

HDY_AVAILABLE_IN_ALL
gboolean hdy_view_switcher_title_get_view_switcher_enabled (HdyViewSwitcherTitle *self);
HDY_AVAILABLE_IN_ALL
void     hdy_view_switcher_title_set_view_switcher_enabled (HdyViewSwitcherTitle *self,
                                                            gboolean              enabled);

HDY_AVAILABLE_IN_ALL
gboolean hdy_view_switcher_title_get_title_visible (HdyViewSwitcherTitle *self);

G_END_DECLS
