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
#include "hdy-navigation-direction.h"
#include "hdy-types.h"

G_BEGIN_DECLS

#define HDY_TYPE_SWIPEABLE (hdy_swipeable_get_type ())

HDY_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (HdySwipeable, hdy_swipeable, HDY, SWIPEABLE, GtkWidget)

/**
 * HdySwipeableInterface:
 * @parent: The parent interface.
 * @switch_child: Switches visible child.
 * @get_swipe_tracker: Gets the swipe tracker.
 * @get_distance: Gets the swipe distance.
 * @get_snap_points: Gets the snap points
 * @get_progress: Gets the current progress.
 * @get_cancel_progress: Gets the cancel progress.
 * @get_swipe_area: Gets the swipeable rectangle.
 *
 * An interface for swipeable widgets.
 *
 * Since: 1.0
 **/
struct _HdySwipeableInterface
{
  GTypeInterface parent;

  void (*switch_child) (HdySwipeable *self,
                        guint         index,
                        gint64        duration);

  HdySwipeTracker * (*get_swipe_tracker)   (HdySwipeable *self);
  gdouble           (*get_distance)        (HdySwipeable *self);
  gdouble *         (*get_snap_points)     (HdySwipeable *self,
                                            gint         *n_snap_points);
  gdouble           (*get_progress)        (HdySwipeable *self);
  gdouble           (*get_cancel_progress) (HdySwipeable *self);
  void              (*get_swipe_area)      (HdySwipeable           *self,
                                            HdyNavigationDirection  navigation_direction,
                                            gboolean                is_drag,
                                            GdkRectangle           *rect);

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
void hdy_swipeable_switch_child (HdySwipeable *self,
                                 guint         index,
                                 gint64        duration);

HDY_AVAILABLE_IN_ALL
void hdy_swipeable_emit_child_switched (HdySwipeable *self,
                                        guint         index,
                                        gint64        duration);

HDY_AVAILABLE_IN_ALL
HdySwipeTracker *hdy_swipeable_get_swipe_tracker   (HdySwipeable *self);
HDY_AVAILABLE_IN_ALL
gdouble          hdy_swipeable_get_distance        (HdySwipeable *self);
HDY_AVAILABLE_IN_ALL
gdouble         *hdy_swipeable_get_snap_points     (HdySwipeable *self,
                                                    gint         *n_snap_points);
HDY_AVAILABLE_IN_ALL
gdouble          hdy_swipeable_get_progress        (HdySwipeable *self);
HDY_AVAILABLE_IN_ALL
gdouble          hdy_swipeable_get_cancel_progress (HdySwipeable *self);
HDY_AVAILABLE_IN_ALL
void             hdy_swipeable_get_swipe_area      (HdySwipeable           *self,
                                                    HdyNavigationDirection  navigation_direction,
                                                    gboolean                is_drag,
                                                    GdkRectangle           *rect);

G_END_DECLS
