/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_SPRING_ANIMATION (adw_spring_animation_get_type())

typedef void   (*AdwAnimationValueCallback) (gdouble  value,
                                             gpointer user_data);
typedef void   (*AdwAnimationDoneCallback)  (gpointer user_data);

typedef struct _AdwSpringAnimation AdwSpringAnimation;

GType         adw_spring_animation_get_type  (void) G_GNUC_CONST;

AdwSpringAnimation *adw_spring_animation_new       (GtkWidget                 *widget,
                                                    gdouble                    from,
                                                    gdouble                    to,
                                                    gdouble                    velocity,
                                                    gdouble                    damping,
                                                    gdouble                    mass,
                                                    gdouble                    stiffness,
                                                    gdouble                    epsilon,
                                                    AdwAnimationValueCallback  value_cb,
                                                    AdwAnimationDoneCallback   done_cb,
                                                    gpointer                   user_data);

AdwSpringAnimation *adw_spring_animation_new_with_damping_ratio (GtkWidget                 *widget,
                                                                 gdouble                    from,
                                                                 gdouble                    to,
                                                                 gdouble                    velocity,
                                                                 gdouble                    damping_ratio,
                                                                 gdouble                    mass,
                                                                 gdouble                    stiffness,
                                                                 gdouble                    epsilon,
                                                                 AdwAnimationValueCallback  value_cb,
                                                                 AdwAnimationDoneCallback   done_cb,
                                                                 gpointer                   user_data);

AdwSpringAnimation *adw_spring_animation_ref       (AdwSpringAnimation *self);
void                adw_spring_animation_unref     (AdwSpringAnimation *self);

void                adw_spring_animation_start     (AdwSpringAnimation *self);
void                adw_spring_animation_stop      (AdwSpringAnimation *self);

gdouble             adw_spring_animation_get_value (AdwSpringAnimation *self);

gdouble             adw_spring_animation_get_estimated_duration (AdwSpringAnimation *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdwSpringAnimation, adw_spring_animation_unref)

G_END_DECLS
