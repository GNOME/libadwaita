/*
 * Copyright (C) 2023 Christopher Davis <christopherdavis@gnome.org>
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

typedef enum {
  ADW_ACCENT_COLOR_BLUE,
  ADW_ACCENT_COLOR_TEAL,
  ADW_ACCENT_COLOR_GREEN,
  ADW_ACCENT_COLOR_YELLOW,
  ADW_ACCENT_COLOR_ORANGE,
  ADW_ACCENT_COLOR_RED,
  ADW_ACCENT_COLOR_PINK,
  ADW_ACCENT_COLOR_PURPLE,
  ADW_ACCENT_COLOR_SLATE,
} AdwAccentColor;

ADW_AVAILABLE_IN_1_6
void adw_accent_color_to_rgba (AdwAccentColor  self,
                               GdkRGBA        *rgba);

ADW_AVAILABLE_IN_1_6
void adw_accent_color_to_standalone_rgba (AdwAccentColor  self,
                                          gboolean        dark,
                                          GdkRGBA        *rgba);

ADW_AVAILABLE_IN_1_6
void adw_rgba_to_standalone (const GdkRGBA *rgba,
                             gboolean       dark,
                             GdkRGBA       *standalone_rgba);

G_END_DECLS
