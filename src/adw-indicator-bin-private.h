/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_INDICATOR_BIN (adw_indicator_bin_get_type())

G_DECLARE_FINAL_TYPE (AdwIndicatorBin, adw_indicator_bin, ADW, INDICATOR_BIN, GtkWidget)

GtkWidget *adw_indicator_bin_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_indicator_bin_get_child (AdwIndicatorBin *self);
void       adw_indicator_bin_set_child (AdwIndicatorBin *self,
                                        GtkWidget       *child);

gboolean adw_indicator_bin_get_needs_attention (AdwIndicatorBin *self);
void     adw_indicator_bin_set_needs_attention (AdwIndicatorBin *self,
                                                gboolean         needs_attention);

gboolean adw_indicator_bin_get_contained (AdwIndicatorBin *self);
void     adw_indicator_bin_set_contained (AdwIndicatorBin *self,
                                          gboolean         contained);

G_END_DECLS
