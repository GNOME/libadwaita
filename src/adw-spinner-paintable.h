/*
 * Copyright (C) 2024 Alice Mikhaylenko <alicem@gnome.org>
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

#define ADW_TYPE_SPINNER_PAINTABLE (adw_spinner_paintable_get_type ())

ADW_AVAILABLE_IN_1_6
G_DECLARE_FINAL_TYPE (AdwSpinnerPaintable, adw_spinner_paintable, ADW, SPINNER_PAINTABLE, GObject)

ADW_AVAILABLE_IN_1_6
AdwSpinnerPaintable *adw_spinner_paintable_new (GtkWidget *widget) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_spinner_paintable_get_widget (AdwSpinnerPaintable *self);
ADW_AVAILABLE_IN_1_6
void       adw_spinner_paintable_set_widget (AdwSpinnerPaintable *self,
                                             GtkWidget           *widget);

G_END_DECLS
