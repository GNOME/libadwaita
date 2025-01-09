/*
 * Copyright (C) 2024 GNOME Foundation Inc.
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

#define ADW_TYPE_ADAPTIVE_PREVIEW (adw_adaptive_preview_get_type())

G_DECLARE_FINAL_TYPE (AdwAdaptivePreview, adw_adaptive_preview, ADW, ADAPTIVE_PREVIEW, GtkWidget)

GtkWidget *adw_adaptive_preview_new (void) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_adaptive_preview_get_child (AdwAdaptivePreview *self);
void       adw_adaptive_preview_set_child (AdwAdaptivePreview *self,
                                           GtkWidget          *child);

gboolean adw_adaptive_preview_get_window_controls (AdwAdaptivePreview *self);
void     adw_adaptive_preview_set_window_controls (AdwAdaptivePreview *self,
                                                   gboolean            window_controls);

gboolean adw_adaptive_preview_get_scale_to_fit (AdwAdaptivePreview *self);
void     adw_adaptive_preview_set_scale_to_fit (AdwAdaptivePreview *self,
                                                gboolean            scale_to_fit);

gboolean adw_adaptive_preview_get_highlight_bezel (AdwAdaptivePreview *self);
void     adw_adaptive_preview_set_highlight_bezel (AdwAdaptivePreview *self,
                                                   gboolean            highlight_bezel);

GtkWidget *adw_adaptive_preview_get_screen (AdwAdaptivePreview *self);

G_END_DECLS
