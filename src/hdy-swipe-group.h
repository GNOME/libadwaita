/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <glib-object.h>
#include "hdy-swipeable.h"

G_BEGIN_DECLS

#define HDY_TYPE_SWIPE_GROUP (hdy_swipe_group_get_type())

G_DECLARE_FINAL_TYPE (HdySwipeGroup, hdy_swipe_group, HDY, SWIPE_GROUP, GObject)

HdySwipeGroup *hdy_swipe_group_new (void);

void           hdy_swipe_group_add_swipeable    (HdySwipeGroup *self,
                                                 HdySwipeable  *swipeable);
GSList *       hdy_swipe_group_get_swipeables   (HdySwipeGroup *self);
void           hdy_swipe_group_remove_swipeable (HdySwipeGroup *self,
                                                 HdySwipeable  *swipeable);

G_END_DECLS
