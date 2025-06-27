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

#define ADW_TYPE_SHORTCUTS_ITEM (adw_shortcuts_item_get_type())

ADW_AVAILABLE_IN_1_8
G_DECLARE_FINAL_TYPE (AdwShortcutsItem, adw_shortcuts_item, ADW, SHORTCUTS_ITEM, GObject)

ADW_AVAILABLE_IN_1_8
AdwShortcutsItem *adw_shortcuts_item_new (const char *title,
                                          const char *accelerator) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
AdwShortcutsItem *adw_shortcuts_item_new_from_action (const char *title,
                                                      const char *action_name) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
const char *adw_shortcuts_item_get_title (AdwShortcutsItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcuts_item_set_title (AdwShortcutsItem *self,
                                          const char       *title);

ADW_AVAILABLE_IN_1_8
const char *adw_shortcuts_item_get_subtitle (AdwShortcutsItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcuts_item_set_subtitle (AdwShortcutsItem *self,
                                             const char       *subtitle);

ADW_AVAILABLE_IN_1_8
const char *adw_shortcuts_item_get_accelerator (AdwShortcutsItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcuts_item_set_accelerator (AdwShortcutsItem *self,
                                                const char       *accelerator);

ADW_AVAILABLE_IN_1_8
const char *adw_shortcuts_item_get_action_name (AdwShortcutsItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcuts_item_set_action_name (AdwShortcutsItem *self,
                                                const char       *action_name);

ADW_AVAILABLE_IN_1_8
GtkTextDirection adw_shortcuts_item_get_direction (AdwShortcutsItem  *self);
ADW_AVAILABLE_IN_1_8
void             adw_shortcuts_item_set_direction (AdwShortcutsItem  *self,
                                                   GtkTextDirection   direction);

G_END_DECLS
