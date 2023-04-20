/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-breakpoint-bin.h"

G_BEGIN_DECLS

void adw_breakpoint_bin_set_warning_widget (AdwBreakpointBin *self,
                                            GtkWidget        *warning_widget);

G_END_DECLS
