/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib.h>

#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_EASING_EASE_IN_CUBIC,
  ADW_EASING_EASE_OUT_CUBIC,
  ADW_EASING_EASE_IN_OUT_CUBIC,
} AdwEasing;

ADW_AVAILABLE_IN_ALL
double adw_easing_ease (AdwEasing self,
                        double    value);

G_END_DECLS
