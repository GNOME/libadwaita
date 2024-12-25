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

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_BIN_LAYOUT (adw_bin_layout_get_type())

ADW_AVAILABLE_IN_1_7
G_DECLARE_FINAL_TYPE (AdwBinLayout, adw_bin_layout, ADW, BIN_LAYOUT, GtkLayoutManager)

ADW_AVAILABLE_IN_1_7
GtkLayoutManager *adw_bin_layout_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_7
GskTransform *adw_bin_layout_get_transform (AdwBinLayout *self);
ADW_AVAILABLE_IN_1_7
void          adw_bin_layout_set_transform (AdwBinLayout *self,
                                            GskTransform *transform);

ADW_AVAILABLE_IN_1_7
float adw_bin_layout_get_transform_origin_x (AdwBinLayout *self);
ADW_AVAILABLE_IN_1_7
void  adw_bin_layout_set_transform_origin_x (AdwBinLayout *self,
                                             float         origin);

ADW_AVAILABLE_IN_1_7
float adw_bin_layout_get_transform_origin_y (AdwBinLayout *self);
ADW_AVAILABLE_IN_1_7
void  adw_bin_layout_set_transform_origin_y (AdwBinLayout *self,
                                             float         origin);

G_END_DECLS
