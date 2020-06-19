/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "hdy-navigation-direction.h"

G_BEGIN_DECLS

#define HDY_TYPE_SWIPEABLE (hdy_swipeable_get_type ())

G_DECLARE_INTERFACE (HdySwipeable, hdy_swipeable, HDY, SWIPEABLE, GtkWidget)

/**
 * HdySwipeableInterface:
 * @parent: The parent interface.
 * @switch_child: Switches visible child.
 * @begin_swipe: Starts a swipe gesture.
 * @update_swipe: Updates swipe progress value.
 * @end_swipe: Ends a swipe gesture.
 * @get_distance: Gets the swipe distance.
 * @get_range: Gets the range of progress values.
 * @get_snap_points: Gets the snap points
 * @get_progress: Gets the current progress.
 * @get_cancel_progress: Gets the cancel progress.
 *
 * An interface for swipeable widgets.
 *
 * Since: 0.0.12
 **/
struct _HdySwipeableInterface
{
  GTypeInterface parent;

  void (*switch_child) (HdySwipeable *self,
                        guint         index,
                        gint64        duration);
  void (*begin_swipe)  (HdySwipeable           *self,
                        HdyNavigationDirection  direction,
                        gboolean                direct);
  void (*update_swipe) (HdySwipeable *self,
                        gdouble       value);
  void (*end_swipe)    (HdySwipeable *self,
                        gint64        duration,
                        gdouble       to);

  gdouble   (*get_distance)        (HdySwipeable *self);
  void      (*get_range)           (HdySwipeable *self,
                                    gdouble      *lower,
                                    gdouble      *upper);
  gdouble * (*get_snap_points)     (HdySwipeable *self,
                                    gint         *n_snap_points);
  gdouble   (*get_progress)        (HdySwipeable *self);
  gdouble   (*get_cancel_progress) (HdySwipeable *self);
};

G_END_DECLS
