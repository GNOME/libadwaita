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

#include "adw-sidebar-section.h"

G_BEGIN_DECLS

guint adw_sidebar_section_get_n_items (AdwSidebarSection *self);

guint adw_sidebar_section_get_first_index (AdwSidebarSection *self);
void  adw_sidebar_section_set_first_index (AdwSidebarSection *self,
                                           guint              index);

void adw_sidebar_section_set_sidebar (AdwSidebarSection *self,
                                      AdwSidebar        *sidebar);

G_END_DECLS
