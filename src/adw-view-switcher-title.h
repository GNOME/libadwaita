/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-view-switcher.h"

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER_TITLE (adw_view_switcher_title_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwViewSwitcherTitle, adw_view_switcher_title, ADW, VIEW_SWITCHER_TITLE, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_view_switcher_title_new (void);

ADW_AVAILABLE_IN_ALL
AdwViewSwitcherPolicy adw_view_switcher_title_get_policy (AdwViewSwitcherTitle  *self);
ADW_AVAILABLE_IN_ALL
void                  adw_view_switcher_title_set_policy (AdwViewSwitcherTitle  *self,
                                                          AdwViewSwitcherPolicy  policy);

ADW_AVAILABLE_IN_ALL
GtkStack *adw_view_switcher_title_get_stack (AdwViewSwitcherTitle *self);
ADW_AVAILABLE_IN_ALL
void      adw_view_switcher_title_set_stack (AdwViewSwitcherTitle *self,
                                             GtkStack             *stack);

ADW_AVAILABLE_IN_ALL
const char *adw_view_switcher_title_get_title (AdwViewSwitcherTitle *self);
ADW_AVAILABLE_IN_ALL
void        adw_view_switcher_title_set_title (AdwViewSwitcherTitle *self,
                                               const char           *title);

ADW_AVAILABLE_IN_ALL
const char *adw_view_switcher_title_get_subtitle (AdwViewSwitcherTitle *self);
ADW_AVAILABLE_IN_ALL
void        adw_view_switcher_title_set_subtitle (AdwViewSwitcherTitle *self,
                                                  const char           *subtitle);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_switcher_title_get_view_switcher_enabled (AdwViewSwitcherTitle *self);
ADW_AVAILABLE_IN_ALL
void     adw_view_switcher_title_set_view_switcher_enabled (AdwViewSwitcherTitle *self,
                                                            gboolean              enabled);

ADW_AVAILABLE_IN_ALL
gboolean adw_view_switcher_title_get_title_visible (AdwViewSwitcherTitle *self);

G_END_DECLS
