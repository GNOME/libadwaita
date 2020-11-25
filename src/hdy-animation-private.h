/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-animation.h"

G_BEGIN_DECLS

#define HDY_TYPE_ANIMATION (hdy_animation_get_type())

typedef struct _HdyAnimation HdyAnimation;

typedef void   (*HdyAnimationValueCallback) (gdouble  value,
                                             gpointer user_data);
typedef void   (*HdyAnimationDoneCallback)  (gpointer user_data);
typedef double (*HdyAnimationEasingFunc)    (gdouble  t);

GType         hdy_animation_get_type  (void) G_GNUC_CONST;

HdyAnimation *hdy_animation_new       (GtkWidget                 *widget,
                                       gdouble                    from,
                                       gdouble                    to,
                                       gint64                     duration,
                                       HdyAnimationEasingFunc     easing_func,
                                       HdyAnimationValueCallback  value_cb,
                                       HdyAnimationDoneCallback   done_cb,
                                       gpointer                   user_data);

HdyAnimation *hdy_animation_ref       (HdyAnimation *self);
void          hdy_animation_unref     (HdyAnimation *self);

void          hdy_animation_start     (HdyAnimation *self);
void          hdy_animation_stop      (HdyAnimation *self);

gdouble       hdy_animation_get_value (HdyAnimation *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (HdyAnimation, hdy_animation_unref)

gdouble hdy_lerp (gdouble a, gdouble b, gdouble t);

G_END_DECLS
