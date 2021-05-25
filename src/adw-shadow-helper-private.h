/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_SHADOW_HELPER (adw_shadow_helper_get_type())

G_DECLARE_FINAL_TYPE (AdwShadowHelper, adw_shadow_helper, ADW, SHADOW_HELPER, GObject)

AdwShadowHelper *adw_shadow_helper_new (GtkWidget *widget);

void adw_shadow_helper_size_allocate (AdwShadowHelper *self,
                                      int              width,
                                      int              height,
                                      int              baseline,
                                      int              x,
                                      int              y,
                                      double           progress,
                                      GtkPanDirection  direction);

void adw_shadow_helper_snapshot (AdwShadowHelper *self,
                                 GtkSnapshot     *snapshot);

G_END_DECLS
