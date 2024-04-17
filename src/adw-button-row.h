/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
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

#include "adw-preferences-row.h"

G_BEGIN_DECLS

#define ADW_TYPE_BUTTON_ROW (adw_button_row_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwButtonRow, adw_button_row, ADW, BUTTON_ROW, AdwPreferencesRow)

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_button_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_6
const char *adw_button_row_get_start_icon_name (AdwButtonRow *self);
ADW_AVAILABLE_IN_1_6
void        adw_button_row_set_start_icon_name (AdwButtonRow *self,
                                                const char   *icon_name);

ADW_AVAILABLE_IN_1_6
const char *adw_button_row_get_end_icon_name (AdwButtonRow *self);
ADW_AVAILABLE_IN_1_6
void        adw_button_row_set_end_icon_name (AdwButtonRow *self,
                                              const char   *icon_name);

G_END_DECLS
