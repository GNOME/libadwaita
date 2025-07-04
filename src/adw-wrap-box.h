/*
 * Copyright (C) 2021 Purism SPC
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

#include "adw-wrap-layout.h"
#include "adw-length-unit.h"

G_BEGIN_DECLS

#define ADW_TYPE_WRAP_BOX (adw_wrap_box_get_type())

ADW_AVAILABLE_IN_1_7
G_DECLARE_FINAL_TYPE (AdwWrapBox, adw_wrap_box, ADW, WRAP_BOX, GtkWidget)

ADW_AVAILABLE_IN_1_7
GtkWidget *adw_wrap_box_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_7
int  adw_wrap_box_get_child_spacing (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void adw_wrap_box_set_child_spacing (AdwWrapBox *self,
                                     int         child_spacing);

ADW_AVAILABLE_IN_1_7
AdwLengthUnit adw_wrap_box_get_child_spacing_unit (AdwWrapBox    *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_box_set_child_spacing_unit (AdwWrapBox    *self,
                                                   AdwLengthUnit  unit);

ADW_AVAILABLE_IN_1_7
AdwPackDirection adw_wrap_box_get_pack_direction (AdwWrapBox       *self);
ADW_AVAILABLE_IN_1_7
void             adw_wrap_box_set_pack_direction (AdwWrapBox       *self,
                                                  AdwPackDirection  pack_direction);

ADW_AVAILABLE_IN_1_7
float adw_wrap_box_get_align (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void  adw_wrap_box_set_align (AdwWrapBox *self,
                              float       align);

ADW_AVAILABLE_IN_1_7
AdwJustifyMode adw_wrap_box_get_justify (AdwWrapBox     *self);
ADW_AVAILABLE_IN_1_7
void           adw_wrap_box_set_justify (AdwWrapBox     *self,
                                         AdwJustifyMode  justify);

ADW_AVAILABLE_IN_1_7
gboolean adw_wrap_box_get_justify_last_line (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void     adw_wrap_box_set_justify_last_line (AdwWrapBox *self,
                                             gboolean    justify_last_line);

ADW_AVAILABLE_IN_1_7
int  adw_wrap_box_get_line_spacing (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void adw_wrap_box_set_line_spacing (AdwWrapBox *self,
                                    int         line_spacing);

ADW_AVAILABLE_IN_1_7
AdwLengthUnit adw_wrap_box_get_line_spacing_unit (AdwWrapBox    *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_box_set_line_spacing_unit (AdwWrapBox    *self,
                                                  AdwLengthUnit  unit);

ADW_AVAILABLE_IN_1_7
gboolean adw_wrap_box_get_line_homogeneous (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void     adw_wrap_box_set_line_homogeneous (AdwWrapBox *self,
                                            gboolean    homogeneous);

ADW_AVAILABLE_IN_1_7
int  adw_wrap_box_get_natural_line_length (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void adw_wrap_box_set_natural_line_length (AdwWrapBox *self,
                                           int         natural_line_length);

ADW_AVAILABLE_IN_1_7
AdwLengthUnit adw_wrap_box_get_natural_line_length_unit (AdwWrapBox    *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_box_set_natural_line_length_unit (AdwWrapBox    *self,
                                                         AdwLengthUnit  unit);

ADW_AVAILABLE_IN_1_7
gboolean adw_wrap_box_get_wrap_reverse (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void     adw_wrap_box_set_wrap_reverse (AdwWrapBox *self,
                                        gboolean    wrap_reverse);

ADW_AVAILABLE_IN_1_7
AdwWrapPolicy adw_wrap_box_get_wrap_policy (AdwWrapBox *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_box_set_wrap_policy (AdwWrapBox    *self,
                                            AdwWrapPolicy  wrap_policy);

ADW_AVAILABLE_IN_1_7
void adw_wrap_box_insert_child_after  (AdwWrapBox *self,
                                       GtkWidget  *child,
                                       GtkWidget  *sibling);
ADW_AVAILABLE_IN_1_7
void adw_wrap_box_reorder_child_after (AdwWrapBox *self,
                                       GtkWidget  *child,
                                       GtkWidget  *sibling);

ADW_AVAILABLE_IN_1_7
void adw_wrap_box_append  (AdwWrapBox *self,
                           GtkWidget  *child);
ADW_AVAILABLE_IN_1_7
void adw_wrap_box_prepend (AdwWrapBox *self,
                           GtkWidget  *child);
ADW_AVAILABLE_IN_1_7
void adw_wrap_box_remove  (AdwWrapBox *self,
                           GtkWidget  *child);

ADW_AVAILABLE_IN_1_8
void adw_wrap_box_remove_all (AdwWrapBox *self);

G_END_DECLS
