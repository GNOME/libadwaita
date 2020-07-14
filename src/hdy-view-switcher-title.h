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

#define HDY_TYPE_VIEW_SWITCHER_TITLE (hdy_view_switcher_title_get_type())

G_DECLARE_FINAL_TYPE (HdyViewSwitcherTitle, hdy_view_switcher_title, HDY, VIEW_SWITCHER_TITLE, GtkBin)

HdyViewSwitcherTitle *hdy_view_switcher_title_new (void);

HdyViewSwitcherPolicy hdy_view_switcher_title_get_policy (HdyViewSwitcherTitle *self);
void                  hdy_view_switcher_title_set_policy (HdyViewSwitcherTitle  *self,
                                                          HdyViewSwitcherPolicy  policy);

GtkIconSize hdy_view_switcher_title_get_icon_size (HdyViewSwitcherTitle *self);
void        hdy_view_switcher_title_set_icon_size (HdyViewSwitcherTitle *self,
                                                   GtkIconSize           icon_size);

GtkStack *hdy_view_switcher_title_get_stack (HdyViewSwitcherTitle *self);
void      hdy_view_switcher_title_set_stack (HdyViewSwitcherTitle *self,
                                             GtkStack             *stack);

const gchar *hdy_view_switcher_title_get_title (HdyViewSwitcherTitle *self);
void         hdy_view_switcher_title_set_title (HdyViewSwitcherTitle *self,
                                                const gchar          *title);

const gchar *hdy_view_switcher_title_get_subtitle (HdyViewSwitcherTitle *self);
void         hdy_view_switcher_title_set_subtitle (HdyViewSwitcherTitle *self,
                                                   const gchar          *subtitle);

gboolean hdy_view_switcher_title_get_view_switcher_enabled (HdyViewSwitcherTitle *self);
void     hdy_view_switcher_title_set_view_switcher_enabled (HdyViewSwitcherTitle *self,
                                                            gboolean              enabled);

gboolean hdy_view_switcher_title_get_title_visible (HdyViewSwitcherTitle *self);

G_END_DECLS
