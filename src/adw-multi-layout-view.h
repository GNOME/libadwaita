/*
 * Copyright (C) 2023 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-layout.h"

G_BEGIN_DECLS

#define ADW_TYPE_MULTI_LAYOUT_VIEW (adw_multi_layout_view_get_type())

ADW_AVAILABLE_IN_1_6
G_DECLARE_FINAL_TYPE (AdwMultiLayoutView, adw_multi_layout_view, ADW, MULTI_LAYOUT_VIEW, GtkWidget)

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_multi_layout_view_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_6
AdwLayout *adw_multi_layout_view_get_layout (AdwMultiLayoutView *self);
ADW_AVAILABLE_IN_1_6
void       adw_multi_layout_view_set_layout (AdwMultiLayoutView *self,
                                             AdwLayout          *layout);

ADW_AVAILABLE_IN_1_6
const char *adw_multi_layout_view_get_layout_name (AdwMultiLayoutView *self);
ADW_AVAILABLE_IN_1_6
void        adw_multi_layout_view_set_layout_name (AdwMultiLayoutView *self,
                                                   const char         *name);

ADW_AVAILABLE_IN_1_6
void adw_multi_layout_view_add_layout (AdwMultiLayoutView *self,
                                       AdwLayout          *layout);

ADW_AVAILABLE_IN_1_6
void adw_multi_layout_view_remove_layout (AdwMultiLayoutView *self,
                                          AdwLayout          *layout);
ADW_AVAILABLE_IN_1_6
AdwLayout *adw_multi_layout_view_get_layout_by_name (AdwMultiLayoutView *self,
                                                     const char         *name);

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_multi_layout_view_get_child (AdwMultiLayoutView *self,
                                            const char         *id);
ADW_AVAILABLE_IN_1_6
void       adw_multi_layout_view_set_child (AdwMultiLayoutView *self,
                                            const char         *id,
                                            GtkWidget          *child);

G_END_DECLS
