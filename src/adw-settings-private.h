/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <glib-object.h>
#include "adw-enums-private.h"

G_BEGIN_DECLS

typedef enum {
  ADW_SYSTEM_COLOR_SCHEME_DEFAULT,
  ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK,
  ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT,
} AdwSystemColorScheme;

#define ADW_TYPE_SETTINGS (adw_settings_get_type())

G_DECLARE_FINAL_TYPE (AdwSettings, adw_settings, ADW, SETTINGS, GObject)

AdwSettings *adw_settings_get_default (void);

gboolean             adw_settings_has_color_scheme (AdwSettings *self);
AdwSystemColorScheme adw_settings_get_color_scheme (AdwSettings *self);

gboolean adw_settings_get_high_contrast (AdwSettings *self);

G_END_DECLS
