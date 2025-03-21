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

#include "adw-sidebar-item.h"

G_BEGIN_DECLS

typedef struct _AdwSidebarItem AdwSidebarItem;

#define ADW_TYPE_SIDEBAR_SECTION (adw_sidebar_section_get_type ())

ADW_AVAILABLE_IN_1_8
G_DECLARE_FINAL_TYPE (AdwSidebarSection, adw_sidebar_section, ADW, SIDEBAR_SECTION, GObject)

ADW_AVAILABLE_IN_1_8
AdwSidebarSection *adw_sidebar_section_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
const char *adw_sidebar_section_get_title (AdwSidebarSection *self);
ADW_AVAILABLE_IN_1_8
void        adw_sidebar_section_set_title (AdwSidebarSection *self,
                                           const char        *title);

ADW_AVAILABLE_IN_1_8
GListModel *adw_sidebar_section_get_items (AdwSidebarSection *self) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
AdwSidebarItem *adw_sidebar_section_get_item (AdwSidebarSection *self,
                                              guint              index);

ADW_AVAILABLE_IN_1_8
void adw_sidebar_section_append (AdwSidebarSection *self,
                                 AdwSidebarItem    *item);

ADW_AVAILABLE_IN_1_8
void adw_sidebar_section_remove (AdwSidebarSection *self,
                                 AdwSidebarItem    *item);

G_END_DECLS
