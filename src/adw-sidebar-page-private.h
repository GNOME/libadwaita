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

#include "adw-preferences-page.h"
#include "adw-sidebar.h"

G_BEGIN_DECLS

#define ADW_TYPE_SIDEBAR_PAGE (adw_sidebar_page_get_type ())

G_DECLARE_FINAL_TYPE (AdwSidebarPage, adw_sidebar_page, ADW, SIDEBAR_PAGE, AdwPreferencesPage)

GtkWidget *adw_sidebar_page_new (AdwSidebar *sidebar) G_GNUC_WARN_UNUSED_RESULT;

void adw_sidebar_page_unparent_children (AdwSidebarPage *self);

G_END_DECLS
