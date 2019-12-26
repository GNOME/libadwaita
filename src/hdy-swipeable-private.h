/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <glib-object.h>
#include <hdy-swipeable.h>

G_BEGIN_DECLS

void hdy_swipeable_switch_child (HdySwipeable *self,
                                 guint         index,
                                 gint64        duration);
void hdy_swipeable_begin_swipe  (HdySwipeable           *self,
                                 HdyNavigationDirection  direction,
                                 gboolean                direct);
void hdy_swipeable_update_swipe (HdySwipeable *self,
                                 gdouble       value);
void hdy_swipeable_end_swipe    (HdySwipeable *self,
                                 gint64        duration,
                                 gdouble       to);

void hdy_swipeable_emit_switch_child (HdySwipeable *self,
                                      guint         index,
                                      gint64        duration);

gdouble  hdy_swipeable_get_distance        (HdySwipeable *self);
void     hdy_swipeable_get_range           (HdySwipeable *self,
                                            gdouble      *lower,
                                            gdouble      *upper);
gdouble *hdy_swipeable_get_snap_points     (HdySwipeable *self,
                                            gint         *n_snap_points);
gdouble  hdy_swipeable_get_progress        (HdySwipeable *self);
gdouble  hdy_swipeable_get_cancel_progress (HdySwipeable *self);

G_END_DECLS
