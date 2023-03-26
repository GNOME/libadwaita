/*
 * Copyright (C) 2023 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-accent-color.h"

G_BEGIN_DECLS

AdwAccentColor adw_accent_color_nearest_from_rgba (GdkRGBA *original_color);

G_END_DECLS
