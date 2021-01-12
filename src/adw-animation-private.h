/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-animation.h"

G_BEGIN_DECLS

#define ADW_TYPE_ANIMATION (adw_animation_get_type())

typedef struct _AdwAnimation AdwAnimation;

typedef void   (*AdwAnimationValueCallback) (gdouble  value,
                                             gpointer user_data);
typedef void   (*AdwAnimationDoneCallback)  (gpointer user_data);
typedef double (*AdwAnimationEasingFunc)    (gdouble  t);

GType         adw_animation_get_type  (void) G_GNUC_CONST;

AdwAnimation *adw_animation_new       (GtkWidget                 *widget,
                                       gdouble                    from,
                                       gdouble                    to,
                                       gint64                     duration,
                                       AdwAnimationEasingFunc     easing_func,
                                       AdwAnimationValueCallback  value_cb,
                                       AdwAnimationDoneCallback   done_cb,
                                       gpointer                   user_data);

AdwAnimation *adw_animation_ref       (AdwAnimation *self);
void          adw_animation_unref     (AdwAnimation *self);

void          adw_animation_start     (AdwAnimation *self);
void          adw_animation_stop      (AdwAnimation *self);

gdouble       adw_animation_get_value (AdwAnimation *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (AdwAnimation, adw_animation_unref)

gdouble adw_lerp (gdouble a, gdouble b, gdouble t);

G_END_DECLS
