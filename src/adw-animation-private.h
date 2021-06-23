/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "adw-version.h"
#include "adw-enums-private.h"

G_BEGIN_DECLS
typedef void   (*AdwAnimationTargetFunc) (double   value,
                                          gpointer user_data);

#define ADW_TYPE_ANIMATION_TARGET (adw_animation_target_get_type())

G_DECLARE_FINAL_TYPE (AdwAnimationTarget, adw_animation_target, ADW, ANIMATION_TARGET, GObject)


AdwAnimationTarget *adw_animation_target_new (AdwAnimationTargetFunc callback,
                                              gpointer               data);

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
  ADW_ANIMATION_STATUS_NONE,
  ADW_ANIMATION_STATUS_COMPLETED,
  ADW_ANIMATION_STATUS_RUNNING,
  ADW_ANIMATION_STATUS_PAUSED,
  ADW_ANIMATION_STATUS_CANCELED,
} AdwAnimationStatus;

/**
 * AdwAnimation
 * @parent_class: The parent class
 */
struct _AdwAnimationClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

AdwAnimation *adw_animation_new (GtkWidget                 *widget,
                                 double                     from,
                                 double                     to,
                                 gint64                     duration,
                                 AdwAnimationTargetFunc     value_cb,
                                 gpointer                   user_data) G_GNUC_WARN_UNUSED_RESULT;

void adw_animation_start (AdwAnimation *self);
void adw_animation_stop  (AdwAnimation *self);

GtkWidget *adw_animation_get_widget (AdwAnimation *self);
double     adw_animation_get_value  (AdwAnimation *self);

double                    adw_animation_get_value_from   (AdwAnimation *self);
double                    adw_animation_get_value_to     (AdwAnimation *self);
gint64                    adw_animation_get_duration     (AdwAnimation *self);
AdwAnimationInterpolator  adw_animation_get_interpolator (AdwAnimation *self);
AdwAnimationTarget       *adw_animation_get_target       (AdwAnimation *self);
AdwAnimationStatus        adw_animation_get_status       (AdwAnimation *self);

void adw_animation_set_value_from   (AdwAnimation             *self,
                                     double                    value);
void adw_animation_set_value_to     (AdwAnimation             *self,
                                     double                    value);
void adw_animation_set_duration     (AdwAnimation             *self,
                                     gint64                    duration);
void adw_animation_set_interpolator (AdwAnimation             *self,
                                     AdwAnimationInterpolator  interpolator);
void adw_animation_target_set_value (AdwAnimationTarget       *target,
                                     double                    value);
void adw_animation_set_status       (AdwAnimation             *self,
                                     AdwAnimationStatus        status);

G_END_DECLS
