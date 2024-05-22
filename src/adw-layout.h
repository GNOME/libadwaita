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

#define ADW_TYPE_LAYOUT (adw_layout_get_type())

ADW_AVAILABLE_IN_1_6
G_DECLARE_FINAL_TYPE (AdwLayout, adw_layout, ADW, LAYOUT, GObject)

ADW_AVAILABLE_IN_1_6
AdwLayout *adw_layout_new (GtkWidget *content) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_layout_get_content (AdwLayout *self);

ADW_AVAILABLE_IN_1_6
const char *adw_layout_get_name (AdwLayout *self);
ADW_AVAILABLE_IN_1_6
void        adw_layout_set_name (AdwLayout  *self,
                                 const char *name);

G_END_DECLS
