/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_SPINNER (adw_spinner_get_type ())

ADW_AVAILABLE_IN_1_6
G_DECLARE_FINAL_TYPE (AdwSpinner, adw_spinner, ADW, SPINNER, GtkWidget)

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_spinner_new (void) G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS
