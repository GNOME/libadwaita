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

#include "adw-length-unit.h"

G_BEGIN_DECLS

#define ADW_TYPE_WRAP_LAYOUT (adw_wrap_layout_get_type())

ADW_AVAILABLE_IN_1_7
G_DECLARE_FINAL_TYPE (AdwWrapLayout, adw_wrap_layout, ADW, WRAP_LAYOUT, GtkLayoutManager)

typedef enum {
  ADW_JUSTIFY_NONE,
  ADW_JUSTIFY_FILL,
  ADW_JUSTIFY_SPREAD,
} AdwJustifyMode;

typedef enum {
  ADW_PACK_START_TO_END,
  ADW_PACK_END_TO_START,
} AdwPackDirection;

typedef enum {
  ADW_WRAP_MINIMUM,
  ADW_WRAP_NATURAL,
} AdwWrapPolicy;

ADW_AVAILABLE_IN_1_7
GtkLayoutManager *adw_wrap_layout_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_7
int  adw_wrap_layout_get_child_spacing (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void adw_wrap_layout_set_child_spacing (AdwWrapLayout *self,
                                        int            child_spacing);

ADW_AVAILABLE_IN_1_7
AdwLengthUnit adw_wrap_layout_get_child_spacing_unit (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_layout_set_child_spacing_unit (AdwWrapLayout *self,
                                                      AdwLengthUnit  unit);

ADW_AVAILABLE_IN_1_7
AdwPackDirection adw_wrap_layout_get_pack_direction (AdwWrapLayout    *self);
ADW_AVAILABLE_IN_1_7
void             adw_wrap_layout_set_pack_direction (AdwWrapLayout    *self,
                                                     AdwPackDirection  pack_direction);

ADW_AVAILABLE_IN_1_7
float adw_wrap_layout_get_align (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void  adw_wrap_layout_set_align (AdwWrapLayout *self,
                                 float          align);

ADW_AVAILABLE_IN_1_7
AdwJustifyMode adw_wrap_layout_get_justify (AdwWrapLayout  *self);
ADW_AVAILABLE_IN_1_7
void           adw_wrap_layout_set_justify (AdwWrapLayout  *self,
                                            AdwJustifyMode  justify);

ADW_AVAILABLE_IN_1_7
gboolean adw_wrap_layout_get_justify_last_line (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void     adw_wrap_layout_set_justify_last_line (AdwWrapLayout *self,
                                                gboolean       justify_last_line);

ADW_AVAILABLE_IN_1_7
int  adw_wrap_layout_get_line_spacing (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void adw_wrap_layout_set_line_spacing (AdwWrapLayout *self,
                                       int            line_spacing);

ADW_AVAILABLE_IN_1_7
AdwLengthUnit adw_wrap_layout_get_line_spacing_unit (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_layout_set_line_spacing_unit (AdwWrapLayout *self,
                                                     AdwLengthUnit  unit);

ADW_AVAILABLE_IN_1_7
gboolean adw_wrap_layout_get_line_homogeneous (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void     adw_wrap_layout_set_line_homogeneous (AdwWrapLayout *self,
                                               gboolean       homogeneous);

ADW_AVAILABLE_IN_1_7
int  adw_wrap_layout_get_natural_line_length (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void adw_wrap_layout_set_natural_line_length (AdwWrapLayout *self,
                                              int            natural_line_length);

ADW_AVAILABLE_IN_1_7
AdwLengthUnit adw_wrap_layout_get_natural_line_length_unit (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_layout_set_natural_line_length_unit (AdwWrapLayout *self,
                                                            AdwLengthUnit  unit);

ADW_AVAILABLE_IN_1_7
gboolean adw_wrap_layout_get_wrap_reverse (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void     adw_wrap_layout_set_wrap_reverse (AdwWrapLayout *self,
                                           gboolean       wrap_reverse);

ADW_AVAILABLE_IN_1_7
AdwWrapPolicy adw_wrap_layout_get_wrap_policy (AdwWrapLayout *self);
ADW_AVAILABLE_IN_1_7
void          adw_wrap_layout_set_wrap_policy (AdwWrapLayout *self,
                                               AdwWrapPolicy  wrap_policy);

G_END_DECLS
