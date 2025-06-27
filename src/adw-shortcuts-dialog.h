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

#include "adw-dialog.h"
#include "adw-shortcuts-section.h"

G_BEGIN_DECLS

#define ADW_TYPE_SHORTCUTS_DIALOG (adw_shortcuts_dialog_get_type())

ADW_AVAILABLE_IN_1_8
G_DECLARE_FINAL_TYPE (AdwShortcutsDialog, adw_shortcuts_dialog, ADW, SHORTCUTS_DIALOG, AdwDialog)

ADW_AVAILABLE_IN_1_8
AdwDialog *adw_shortcuts_dialog_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
void adw_shortcuts_dialog_add (AdwShortcutsDialog  *self,
                               AdwShortcutsSection *section);

G_END_DECLS
