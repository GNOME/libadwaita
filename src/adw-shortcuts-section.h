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

#include "adw-shortcuts-item.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define ADW_TYPE_SHORTCUTS_SECTION (adw_shortcuts_section_get_type())

ADW_AVAILABLE_IN_1_8
G_DECLARE_FINAL_TYPE (AdwShortcutsSection, adw_shortcuts_section, ADW, SHORTCUTS_SECTION, GObject)

ADW_AVAILABLE_IN_1_8
AdwShortcutsSection *adw_shortcuts_section_new (const char *title) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
const char *adw_shortcuts_section_get_title (AdwShortcutsSection *self);
ADW_AVAILABLE_IN_1_8
void        adw_shortcuts_section_set_title (AdwShortcutsSection *self,
                                             const char          *title);

ADW_AVAILABLE_IN_1_8
void adw_shortcuts_section_add (AdwShortcutsSection *self,
                                AdwShortcutsItem    *item);

G_END_DECLS
