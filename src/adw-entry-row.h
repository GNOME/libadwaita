/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@protonmail.com>
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include "adw-preferences-row.h"

G_BEGIN_DECLS

#define ADW_TYPE_ENTRY_ROW (adw_entry_row_get_type())

ADW_AVAILABLE_IN_1_2
G_DECLARE_DERIVABLE_TYPE (AdwEntryRow, adw_entry_row, ADW, ENTRY_ROW, AdwPreferencesRow)

/**
 * AdwEntryRowClass
 * @parent_class: The parent class
 */
struct _AdwEntryRowClass
{
  AdwPreferencesRowClass parent_class;
};

ADW_AVAILABLE_IN_1_2
GtkWidget *adw_entry_row_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_2
void adw_entry_row_add_prefix (AdwEntryRow *self,
                               GtkWidget   *widget);
ADW_AVAILABLE_IN_1_2
void adw_entry_row_add_suffix (AdwEntryRow *self,
                               GtkWidget   *widget);
ADW_AVAILABLE_IN_1_2
void adw_entry_row_remove     (AdwEntryRow *self,
                               GtkWidget   *widget);

ADW_AVAILABLE_IN_1_2
gboolean adw_entry_row_get_show_apply_button (AdwEntryRow *self);
ADW_AVAILABLE_IN_1_2
void     adw_entry_row_set_show_apply_button (AdwEntryRow *self,
                                              gboolean     show_apply_button);

G_END_DECLS
