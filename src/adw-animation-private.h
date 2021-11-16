/*
 * Copyright (C) 2019 Purism SPC
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

#include "adw-enums-private.h"
#include "adw-animation-target-private.h"

G_BEGIN_DECLS

#define ADW_DURATION_INFINITE G_MAXUINT

#define ADW_TYPE_ANIMATION (adw_animation_get_type())

G_DECLARE_DERIVABLE_TYPE (AdwAnimation, adw_animation, ADW, ANIMATION, GObject)

typedef void   (*AdwAnimationDoneCallback)  (gpointer user_data);
typedef double (*AdwAnimationEasingFunc)    (double   t);

typedef enum {
  ADW_ANIMATION_INTERPOLATOR_EASE_IN,
  ADW_ANIMATION_INTERPOLATOR_EASE_OUT,
  ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT,
} AdwAnimationInterpolator;

typedef enum {
  ADW_ANIMATION_IDLE,
  ADW_ANIMATION_PAUSED,
  ADW_ANIMATION_PLAYING,
  ADW_ANIMATION_FINISHED,
} AdwAnimationState;

struct _AdwAnimationClass
{
  GObjectClass parent_class;

  guint (*estimate_duration) (AdwAnimation *self);

  double (*calculate_value) (AdwAnimation *self,
                             guint         t);
};

AdwAnimation *adw_animation_new (GtkWidget          *widget,
                                 double              from,
                                 double              to,
                                 guint               duration,
                                 AdwAnimationTarget *target) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_animation_get_widget (AdwAnimation *self);

AdwAnimationTarget *adw_animation_get_target (AdwAnimation *self);

double adw_animation_get_value (AdwAnimation *self);

AdwAnimationState adw_animation_get_state (AdwAnimation *self);

void adw_animation_play   (AdwAnimation *self);
void adw_animation_pause  (AdwAnimation *self);
void adw_animation_resume (AdwAnimation *self);
void adw_animation_reset  (AdwAnimation *self);
void adw_animation_skip   (AdwAnimation *self);

double adw_animation_get_value_from (AdwAnimation *self);
void   adw_animation_set_value_from (AdwAnimation *self,
                                     double        value);

double adw_animation_get_value_to (AdwAnimation *self);
void   adw_animation_set_value_to (AdwAnimation *self,
                                   double        value);

guint adw_animation_get_duration (AdwAnimation *self);
void  adw_animation_set_duration (AdwAnimation *self,
                                  guint         duration);

AdwAnimationInterpolator adw_animation_get_interpolator (AdwAnimation             *self);
void                     adw_animation_set_interpolator (AdwAnimation             *self,
                                                         AdwAnimationInterpolator  interpolator);

G_END_DECLS
