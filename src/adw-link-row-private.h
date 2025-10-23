/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly"
#endif

#include "adw-version.h"

#include "adw-action-row.h"

G_BEGIN_DECLS

#define ADW_TYPE_LINK_ROW (adw_link_row_get_type ())

G_DECLARE_FINAL_TYPE (AdwLinkRow, adw_link_row, ADW, LINK_ROW, AdwActionRow)

GtkWidget *adw_link_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

const char *adw_link_row_get_uri (AdwLinkRow *self);
void        adw_link_row_set_uri (AdwLinkRow *self,
                                  const char *uri);

gboolean adw_link_row_get_visited (AdwLinkRow *self);
void     adw_link_row_set_visited (AdwLinkRow *self,
                                   gboolean    visited);

G_END_DECLS
