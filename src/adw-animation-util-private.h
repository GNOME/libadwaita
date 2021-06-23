/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"
#include "adw-animation-util.h"

G_BEGIN_DECLS

double adw_ease_in_cubic     (double t);
double adw_ease_in_out_cubic (double t);
double adw_lerp (double a,
                 double b,
                 double t);

G_END_DECLS
