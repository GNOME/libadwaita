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

#define ADW_TYPE_BOTTOM_SHEET (adw_bottom_sheet_get_type())

ADW_AVAILABLE_IN_1_6
G_DECLARE_FINAL_TYPE (AdwBottomSheet, adw_bottom_sheet, ADW, BOTTOM_SHEET, GtkWidget)

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_bottom_sheet_new (void);

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_bottom_sheet_get_content (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void       adw_bottom_sheet_set_content (AdwBottomSheet *self,
                                         GtkWidget      *content);

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_bottom_sheet_get_sheet (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void       adw_bottom_sheet_set_sheet (AdwBottomSheet *self,
                                       GtkWidget      *sheet);

ADW_AVAILABLE_IN_1_6
GtkWidget *adw_bottom_sheet_get_bottom_bar (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void       adw_bottom_sheet_set_bottom_bar (AdwBottomSheet *self,
                                            GtkWidget      *bottom_bar);

ADW_AVAILABLE_IN_1_6
gboolean adw_bottom_sheet_get_open (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void     adw_bottom_sheet_set_open (AdwBottomSheet *self,
                                    gboolean        open);

ADW_AVAILABLE_IN_1_6
float adw_bottom_sheet_get_align (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void  adw_bottom_sheet_set_align (AdwBottomSheet *self,
                                  float           align);

ADW_AVAILABLE_IN_1_6
gboolean adw_bottom_sheet_get_full_width (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void     adw_bottom_sheet_set_full_width (AdwBottomSheet *self,
                                          gboolean        full_width);

ADW_AVAILABLE_IN_1_6
gboolean adw_bottom_sheet_get_show_drag_handle (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void     adw_bottom_sheet_set_show_drag_handle (AdwBottomSheet *self,
                                                gboolean        show_drag_handle);

ADW_AVAILABLE_IN_1_6
gboolean adw_bottom_sheet_get_modal (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void     adw_bottom_sheet_set_modal (AdwBottomSheet *self,
                                     gboolean        modal);

ADW_AVAILABLE_IN_1_6
gboolean adw_bottom_sheet_get_can_open (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void     adw_bottom_sheet_set_can_open (AdwBottomSheet *self,
                                        gboolean        can_open);

ADW_AVAILABLE_IN_1_6
gboolean adw_bottom_sheet_get_can_close (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
void     adw_bottom_sheet_set_can_close (AdwBottomSheet *self,
                                         gboolean        can_close);

ADW_AVAILABLE_IN_1_6
int adw_bottom_sheet_get_sheet_height (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_6
int adw_bottom_sheet_get_bottom_bar_height (AdwBottomSheet *self);

ADW_AVAILABLE_IN_1_7
gboolean adw_bottom_sheet_get_reveal_bottom_bar (AdwBottomSheet *self);
ADW_AVAILABLE_IN_1_7
void     adw_bottom_sheet_set_reveal_bottom_bar (AdwBottomSheet *self,
                                                 gboolean        reveal);

G_END_DECLS
