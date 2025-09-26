/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-sidebar.h"
#include "adw-view-stack.h"

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER_SIDEBAR (adw_view_switcher_sidebar_get_type())

ADW_AVAILABLE_IN_1_9
G_DECLARE_FINAL_TYPE (AdwViewSwitcherSidebar, adw_view_switcher_sidebar, ADW, VIEW_SWITCHER_SIDEBAR, GtkWidget)

ADW_AVAILABLE_IN_1_9
GtkWidget *adw_view_switcher_sidebar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_9
AdwViewStack *adw_view_switcher_sidebar_get_stack (AdwViewSwitcherSidebar *self);
ADW_AVAILABLE_IN_1_9
void          adw_view_switcher_sidebar_set_stack (AdwViewSwitcherSidebar *self,
                                                   AdwViewStack           *stack);

ADW_AVAILABLE_IN_1_9
AdwSidebarMode adw_view_switcher_sidebar_get_mode (AdwViewSwitcherSidebar *self);
ADW_AVAILABLE_IN_1_9
void           adw_view_switcher_sidebar_set_mode (AdwViewSwitcherSidebar *self,
                                                   AdwSidebarMode          mode);

ADW_AVAILABLE_IN_1_9
GtkFilter *adw_view_switcher_sidebar_get_filter (AdwViewSwitcherSidebar *self);
ADW_AVAILABLE_IN_1_9
void       adw_view_switcher_sidebar_set_filter (AdwViewSwitcherSidebar *self,
                                                 GtkFilter              *filter);

ADW_AVAILABLE_IN_1_9
GtkWidget *adw_view_switcher_sidebar_get_placeholder (AdwViewSwitcherSidebar *self);
ADW_AVAILABLE_IN_1_9
void       adw_view_switcher_sidebar_set_placeholder (AdwViewSwitcherSidebar *self,
                                                      GtkWidget              *placeholder);

G_END_DECLS
