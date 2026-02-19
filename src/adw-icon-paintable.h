/*
 * Copyright (C) 2026 GNOME Foundation Inc.
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

#define ADW_TYPE_ICON_PAINTABLE (adw_icon_paintable_get_type ())

ADW_AVAILABLE_IN_1_9
G_DECLARE_FINAL_TYPE (AdwIconPaintable, adw_icon_paintable, ADW, ICON_PAINTABLE, GObject)

ADW_AVAILABLE_IN_1_9
AdwIconPaintable *adw_icon_paintable_new (const char *icon_name,
                                          GtkWidget  *widget) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_9
const char *adw_icon_paintable_get_icon_name (AdwIconPaintable *self);
ADW_AVAILABLE_IN_1_9
void        adw_icon_paintable_set_icon_name (AdwIconPaintable *self,
                                              const char       *icon_name);

ADW_AVAILABLE_IN_1_9
guint adw_icon_paintable_get_state (AdwIconPaintable *self);
ADW_AVAILABLE_IN_1_9
void  adw_icon_paintable_set_state (AdwIconPaintable *self,
                                    guint             state);

ADW_AVAILABLE_IN_1_9
gboolean adw_icon_paintable_get_animate_in (AdwIconPaintable *self);
ADW_AVAILABLE_IN_1_9
void     adw_icon_paintable_set_animate_in (AdwIconPaintable *self,
                                            gboolean          animate_in);

ADW_AVAILABLE_IN_1_9
GtkWidget *adw_icon_paintable_get_widget (AdwIconPaintable *self);
ADW_AVAILABLE_IN_1_9
void       adw_icon_paintable_set_widget (AdwIconPaintable *self,
                                          GtkWidget        *widget);

G_END_DECLS
