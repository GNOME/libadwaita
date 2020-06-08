/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-swipeable.h"

G_BEGIN_DECLS

#define HDY_TYPE_SWIPE_TRACKER (hdy_swipe_tracker_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdySwipeTracker, hdy_swipe_tracker, HDY, SWIPE_TRACKER, GObject)

HDY_AVAILABLE_IN_ALL
HdySwipeTracker *hdy_swipe_tracker_new (HdySwipeable *swipeable);

HDY_AVAILABLE_IN_ALL
HdySwipeable    *hdy_swipe_tracker_get_swipeable (HdySwipeTracker *self);

HDY_AVAILABLE_IN_ALL
gboolean         hdy_swipe_tracker_get_enabled (HdySwipeTracker *self);
HDY_AVAILABLE_IN_ALL
void             hdy_swipe_tracker_set_enabled (HdySwipeTracker *self,
                                                gboolean         enabled);

HDY_AVAILABLE_IN_ALL
gboolean         hdy_swipe_tracker_get_reversed (HdySwipeTracker *self);
HDY_AVAILABLE_IN_ALL
void             hdy_swipe_tracker_set_reversed (HdySwipeTracker *self,
                                                 gboolean         reversed);

HDY_AVAILABLE_IN_ALL
gboolean         hdy_swipe_tracker_get_allow_mouse_drag (HdySwipeTracker *self);
HDY_AVAILABLE_IN_ALL
void             hdy_swipe_tracker_set_allow_mouse_drag (HdySwipeTracker *self,
                                                         gboolean         allow_mouse_drag);

HDY_AVAILABLE_IN_ALL
void             hdy_swipe_tracker_shift_position (HdySwipeTracker *self,
                                                   gdouble          delta);

G_END_DECLS
