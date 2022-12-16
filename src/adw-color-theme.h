/*
 * Copyright (C) 2021 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-enums.h"

G_BEGIN_DECLS

typedef enum {
  ADW_COLOR_ACCENT_COLOR,

  ADW_COLOR_DESTRUCTIVE_COLOR,

  ADW_COLOR_SUCCESS_COLOR,
  ADW_COLOR_WARNING_COLOR,
  ADW_COLOR_ERROR_COLOR,

  ADW_COLOR_WINDOW_BG_COLOR,
  ADW_COLOR_WINDOW_FG_COLOR,

  ADW_COLOR_VIEW_BG_COLOR,
  ADW_COLOR_VIEW_FG_COLOR,

  ADW_COLOR_HEADERBAR_BG_COLOR,
  ADW_COLOR_HEADERBAR_FG_COLOR,
  ADW_COLOR_HEADERBAR_BORDER_COLOR,

  ADW_COLOR_CARD_BG_COLOR,
  ADW_COLOR_CARD_FG_COLOR,

  ADW_COLOR_POPOVER_BG_COLOR,
  ADW_COLOR_POPOVER_FG_COLOR,
} AdwColor;

#define ADW_TYPE_COLOR_THEME (adw_color_theme_get_type ())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwColorTheme, adw_color_theme, ADW, COLOR_THEME, GObject)

ADW_AVAILABLE_IN_ALL
AdwColorTheme *adw_color_theme_new_light (void) G_GNUC_WARN_UNUSED_RESULT;
ADW_AVAILABLE_IN_ALL
AdwColorTheme *adw_color_theme_new_dark  (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
void     adw_color_theme_set_color_from_rgba (AdwColorTheme *self,
                                              AdwColor       color,
                                              GdkRGBA       *rgba);
ADW_AVAILABLE_IN_ALL
GdkRGBA *adw_color_theme_get_color           (AdwColorTheme *self,
                                              AdwColor       color);

G_END_DECLS
