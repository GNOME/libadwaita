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
void hdy_swipeable_begin_swipe  (HdySwipeable *self,
                                 gint          direction);
void hdy_swipeable_update_swipe (HdySwipeable *self,
                                 gdouble       value);
void hdy_swipeable_end_swipe    (HdySwipeable *self,
                                 gint64        duration,
                                 gdouble       to);

void hdy_swipeable_emit_switch_child (HdySwipeable *self,
                                      guint         index,
                                      gint64        duration);

G_END_DECLS
