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

#include "adw-enums.h"
#include "adw-sidebar-section.h"

G_BEGIN_DECLS

typedef struct _AdwSidebarItem AdwSidebarItem;
typedef struct _AdwSidebarSection AdwSidebarSection;

typedef enum {
  ADW_SIDEBAR_MODE_SIDEBAR,
  ADW_SIDEBAR_MODE_PAGE,
} AdwSidebarMode;

#define ADW_TYPE_SIDEBAR (adw_sidebar_get_type ())

ADW_AVAILABLE_IN_1_9
G_DECLARE_FINAL_TYPE (AdwSidebar, adw_sidebar, ADW, SIDEBAR, GtkWidget)

ADW_AVAILABLE_IN_1_9
GtkWidget *adw_sidebar_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_9
AdwSidebarMode adw_sidebar_get_mode (AdwSidebar     *self);
ADW_AVAILABLE_IN_1_9
void           adw_sidebar_set_mode (AdwSidebar     *self,
                                     AdwSidebarMode  mode);

ADW_AVAILABLE_IN_1_9
guint adw_sidebar_get_selected (AdwSidebar *self);
ADW_AVAILABLE_IN_1_9
void  adw_sidebar_set_selected (AdwSidebar *self,
                                guint       selected);

ADW_AVAILABLE_IN_1_9
AdwSidebarItem *adw_sidebar_get_selected_item (AdwSidebar *self);

ADW_AVAILABLE_IN_1_9
GtkFilter *adw_sidebar_get_filter (AdwSidebar *self);
ADW_AVAILABLE_IN_1_9
void       adw_sidebar_set_filter (AdwSidebar *self,
                                   GtkFilter  *filter);

ADW_AVAILABLE_IN_1_9
GtkWidget *adw_sidebar_get_placeholder (AdwSidebar *self);
ADW_AVAILABLE_IN_1_9
void       adw_sidebar_set_placeholder (AdwSidebar *self,
                                        GtkWidget  *placeholder);

ADW_AVAILABLE_IN_1_9
GtkSelectionModel *adw_sidebar_get_items    (AdwSidebar *self);
ADW_AVAILABLE_IN_1_9
GListModel        *adw_sidebar_get_sections (AdwSidebar *self);

ADW_AVAILABLE_IN_1_9
AdwSidebarItem *adw_sidebar_get_item (AdwSidebar *self,
                                      guint       index);

ADW_AVAILABLE_IN_1_9
AdwSidebarSection *adw_sidebar_get_section (AdwSidebar *self,
                                            guint       index);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_append  (AdwSidebar        *self,
                          AdwSidebarSection *section);
ADW_AVAILABLE_IN_1_9
void adw_sidebar_prepend (AdwSidebar        *self,
                          AdwSidebarSection *section);
ADW_AVAILABLE_IN_1_9
void adw_sidebar_insert  (AdwSidebar        *self,
                          AdwSidebarSection *section,
                          int                position);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_remove (AdwSidebar        *self,
                         AdwSidebarSection *section);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_remove_all (AdwSidebar *self);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_setup_drop_target (AdwSidebar    *self,
                                    GdkDragAction  actions,
                                    GType         *types,
                                    gsize          n_types);

ADW_AVAILABLE_IN_1_9
gboolean adw_sidebar_get_drop_preload (AdwSidebar *self);
ADW_AVAILABLE_IN_1_9
void     adw_sidebar_set_drop_preload (AdwSidebar *self,
                                       gboolean    preload);

ADW_AVAILABLE_IN_1_9
GMenuModel *adw_sidebar_get_menu_model (AdwSidebar *self);
ADW_AVAILABLE_IN_1_9
void        adw_sidebar_set_menu_model (AdwSidebar *self,
                                        GMenuModel *menu_model);

G_END_DECLS
