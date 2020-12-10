/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-swipe-tracker.h"

G_BEGIN_DECLS

#define HDY_SWIPE_BORDER 32

void hdy_swipe_tracker_emit_begin_swipe (HdySwipeTracker        *self,
                                         HdyNavigationDirection  direction,
                                         gboolean                direct);
void hdy_swipe_tracker_emit_update_swipe (HdySwipeTracker *self,
                                          gdouble          progress);
void hdy_swipe_tracker_emit_end_swipe (HdySwipeTracker *self,
                                       gint64           duration,
                                       gdouble          to);

G_END_DECLS
