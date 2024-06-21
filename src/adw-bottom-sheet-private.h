/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-bottom-sheet.h"

G_BEGIN_DECLS

void adw_bottom_sheet_set_min_natural_width (AdwBottomSheet *self,
                                             int             min_natural_width);

GtkWidget *adw_bottom_sheet_get_sheet_bin (AdwBottomSheet *self);

void adw_bottom_sheet_set_callbacks (AdwBottomSheet *self,
                                     GFunc           closing_callback,
                                     GFunc           closed_callback,
                                     gpointer        user_data);

G_END_DECLS
