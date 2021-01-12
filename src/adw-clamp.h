/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_CLAMP (adw_clamp_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwClamp, adw_clamp, ADW, CLAMP, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_clamp_new (void);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_clamp_get_child (AdwClamp  *self);
ADW_AVAILABLE_IN_ALL
void       adw_clamp_set_child (AdwClamp  *self,
                                GtkWidget *child);

ADW_AVAILABLE_IN_ALL
gint adw_clamp_get_maximum_size (AdwClamp *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_set_maximum_size (AdwClamp *self,
                                 gint      maximum_size);

ADW_AVAILABLE_IN_ALL
gint adw_clamp_get_tightening_threshold (AdwClamp *self);
ADW_AVAILABLE_IN_ALL
void adw_clamp_set_tightening_threshold (AdwClamp *self,
                                         gint      tightening_threshold);

G_END_DECLS
