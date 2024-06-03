/*
 * Copyright (C) 2023 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_LAYOUT_SLOT (adw_layout_slot_get_type())

ADW_AVAILABLE_IN_1_6
G_DECLARE_FINAL_TYPE (AdwLayoutSlot, adw_layout_slot, ADW, LAYOUT_SLOT, GtkWidget)

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_layout_slot_new (const char *id);

ADW_AVAILABLE_IN_1_6
const char *adw_layout_slot_get_slot_id (AdwLayoutSlot *self);

G_END_DECLS
