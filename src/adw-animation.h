/*
 * Copyright (C) 2019 Purism SPC
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

ADW_AVAILABLE_IN_ALL
gboolean adw_get_enable_animations (GtkWidget *widget);

ADW_AVAILABLE_IN_ALL
double adw_ease_out_cubic (double t);

G_END_DECLS
