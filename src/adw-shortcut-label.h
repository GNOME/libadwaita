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

G_BEGIN_DECLS

#define ADW_TYPE_SHORTCUT_LABEL (adw_shortcut_label_get_type())

ADW_AVAILABLE_IN_1_8
G_DECLARE_FINAL_TYPE (AdwShortcutLabel, adw_shortcut_label, ADW, SHORTCUT_LABEL, GtkWidget)

ADW_AVAILABLE_IN_1_8
GtkWidget *adw_shortcut_label_new (const char *accelerator) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
const char *adw_shortcut_label_get_accelerator (AdwShortcutLabel *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcut_label_set_accelerator (AdwShortcutLabel *self,
                                                const char       *accelerator);

ADW_AVAILABLE_IN_1_8
const char *adw_shortcut_label_get_disabled_text (AdwShortcutLabel *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcut_label_set_disabled_text (AdwShortcutLabel *self,
                                                  const char       *disabled_text);

G_END_DECLS
