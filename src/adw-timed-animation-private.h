/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-animation-private.h"
#include "adw-enums.h"

G_BEGIN_DECLS

#define ADW_TYPE_TIMED_ANIMATION (adw_timed_animation_get_type())

GDK_DECLARE_INTERNAL_TYPE (AdwTimedAnimation, adw_timed_animation, ADW, TIMED_ANIMATION, AdwAnimation)

typedef enum {
  ADW_ANIMATION_INTERPOLATOR_EASE_IN,
  ADW_ANIMATION_INTERPOLATOR_EASE_OUT,
  ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT,
} AdwAnimationInterpolator;

AdwAnimation *adw_timed_animation_new (GtkWidget          *widget,
                                       double              from,
                                       double              to,
                                       guint               duration,
                                       AdwAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

double adw_timed_animation_get_value_from (AdwTimedAnimation *self);
void   adw_timed_animation_set_value_from (AdwTimedAnimation *self,
                                           double             value);

double adw_timed_animation_get_value_to (AdwTimedAnimation *self);
void   adw_timed_animation_set_value_to (AdwTimedAnimation *self,
                                         double             value);

guint adw_timed_animation_get_duration (AdwTimedAnimation *self);
void  adw_timed_animation_set_duration (AdwTimedAnimation *self,
                                        guint              duration);

AdwAnimationInterpolator adw_timed_animation_get_interpolator (AdwTimedAnimation        *self);
void                     adw_timed_animation_set_interpolator (AdwTimedAnimation        *self,
                                                               AdwAnimationInterpolator  interpolator);

guint adw_timed_animation_get_repeat_count (AdwTimedAnimation *self);
void  adw_timed_animation_set_repeat_count (AdwTimedAnimation *self,
                                            guint              repeat_count);

gboolean adw_timed_animation_get_reverse (AdwTimedAnimation *self);
void     adw_timed_animation_set_reverse (AdwTimedAnimation *self,
                                          gboolean           reverse);

gboolean adw_timed_animation_get_alternate (AdwTimedAnimation *self);
void     adw_timed_animation_set_alternate (AdwTimedAnimation *self,
                                            gboolean           alternate);

G_END_DECLS
