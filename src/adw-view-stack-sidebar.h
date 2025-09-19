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

#define ADW_TYPE_VIEW_STACK_SIDEBAR (adw_view_stack_sidebar_get_type())

ADW_AVAILABLE_IN_1_9
G_DECLARE_FINAL_TYPE (AdwViewStackSidebar, adw_view_stack_sidebar, ADW, VIEW_STACK_SIDEBAR, GtkWidget)

ADW_AVAILABLE_IN_1_9
GtkWidget *adw_view_stack_sidebar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_9
AdwViewStack *adw_view_stack_sidebar_get_stack (AdwViewStackSidebar *self);
ADW_AVAILABLE_IN_1_9
void          adw_view_stack_sidebar_set_stack (AdwViewStackSidebar *self,
                                                AdwViewStack        *stack);

ADW_AVAILABLE_IN_1_9
AdwSidebarMode adw_view_stack_sidebar_get_mode (AdwViewStackSidebar *self);
ADW_AVAILABLE_IN_1_9
void           adw_view_stack_sidebar_set_mode (AdwViewStackSidebar *self,
                                                AdwSidebarMode       mode);

G_END_DECLS
