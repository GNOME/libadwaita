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

#include <glib-object.h>

#include "adw-sidebar.h"
#include "adw-sidebar-item.h"

G_BEGIN_DECLS

typedef struct _AdwSidebar AdwSidebar;
typedef struct _AdwSidebarItem AdwSidebarItem;

/**
 * AdwSidebarSectionCreateItemFunc:
 * @item: (type GObject): the item from the model for which to create an item for
 * @user_data: (closure): user data
 *
 * Called for sidebars that are bound to a [iface@Gio.ListModel] with
 * [method@SidebarSection.bind_model] for each
 * item that gets added to the model.
 *
 * Returns: (transfer full): an `AdwSidebarItem` that represents @item
 *
 * Since: 1.9
 */
typedef AdwSidebarItem * (*AdwSidebarSectionCreateItemFunc) (gpointer item,
                                                             gpointer user_data);

#define ADW_TYPE_SIDEBAR_SECTION (adw_sidebar_section_get_type ())

ADW_AVAILABLE_IN_1_9
G_DECLARE_FINAL_TYPE (AdwSidebarSection, adw_sidebar_section, ADW, SIDEBAR_SECTION, GObject)

ADW_AVAILABLE_IN_1_9
AdwSidebarSection *adw_sidebar_section_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_9
const char *adw_sidebar_section_get_title (AdwSidebarSection *self);
ADW_AVAILABLE_IN_1_9
void        adw_sidebar_section_set_title (AdwSidebarSection *self,
                                           const char        *title);

ADW_AVAILABLE_IN_1_9
GListModel *adw_sidebar_section_get_items (AdwSidebarSection *self) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_9
AdwSidebarItem *adw_sidebar_section_get_item (AdwSidebarSection *self,
                                              guint              index);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_section_append  (AdwSidebarSection *self,
                                  AdwSidebarItem    *item);
ADW_AVAILABLE_IN_1_9
void adw_sidebar_section_prepend (AdwSidebarSection *self,
                                  AdwSidebarItem    *item);
ADW_AVAILABLE_IN_1_9
void adw_sidebar_section_insert  (AdwSidebarSection *self,
                                  AdwSidebarItem    *item,
                                  int                position);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_section_remove (AdwSidebarSection *self,
                                 AdwSidebarItem    *item);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_section_remove_all (AdwSidebarSection *self);

ADW_AVAILABLE_IN_1_9
void adw_sidebar_section_bind_model (AdwSidebarSection               *self,
                                     GListModel                      *model,
                                     AdwSidebarSectionCreateItemFunc  create_item_func,
                                     gpointer                         user_data,
                                     GDestroyNotify                   user_data_free_func);

ADW_AVAILABLE_IN_1_9
AdwSidebar *adw_sidebar_section_get_sidebar (AdwSidebarSection *self);

G_END_DECLS
