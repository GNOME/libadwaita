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

#include "adw-sidebar-item.h"

G_BEGIN_DECLS

void adw_sidebar_item_set_section (AdwSidebarItem    *self,
                                   AdwSidebarSection *section);

void adw_sidebar_item_set_index (AdwSidebarItem *self,
                                 guint           index);

G_END_DECLS
