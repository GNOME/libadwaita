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

#include "adw-preferences-row.h"
#include "adw-shortcuts-item.h"

G_BEGIN_DECLS

#define ADW_TYPE_SHORTCUT_ROW (adw_shortcut_row_get_type())

G_DECLARE_FINAL_TYPE (AdwShortcutRow, adw_shortcut_row, ADW, SHORTCUT_ROW, AdwPreferencesRow)

GtkWidget *adw_shortcut_row_new (AdwShortcutsItem *item) G_GNUC_WARN_UNUSED_RESULT;

AdwShortcutsItem *adw_shortcut_row_get_item (AdwShortcutRow *self);

G_END_DECLS
