/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-swipeable.h"

/**
 * SECTION:hdy-swipeable
 * @short_description: An interface for swipeable widgets.
 * @title: HdySwipeable
 * @See_also: #HdyCarousel, #HdyDeck, #HdyLeaflet, #HdySwipeGroup
 *
 * The #HdySwipeable interface is implemented by all swipeable widgets. They
 * can be synced using #HdySwipeGroup.
 *
 * See #HdySwipeTracker for details about implementing it.
 *
 * Since: 0.0.12
 */

G_DEFINE_INTERFACE (HdySwipeable, hdy_swipeable, GTK_TYPE_WIDGET)

enum {
  SIGNAL_CHILD_SWITCHED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
hdy_swipeable_default_init (HdySwipeableInterface *iface)
{
  /**
   * HdySwipeable::child-switched:
   * @self: The #HdySwipeable instance
   * @index: the index of the child to switch to
   * @duration: Animation duration in milliseconds
   *
   * This signal should be emitted when the widget's visible child is changed.
   *
   * @duration can be 0 if the child is switched without animation.
   *
   * This is used by #HdySwipeGroup, applications should not connect to it.
   *
   * Since: 1.0
   */
  signals[SIGNAL_CHILD_SWITCHED] =
    g_signal_new ("child-switched",
                  G_TYPE_FROM_INTERFACE (iface),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_UINT, G_TYPE_INT64);
}

/**
 * hdy_swipeable_switch_child:
 * @self: a #HdySwipeable
 * @index: the index of the child to switch to
 * @duration: Animation duration in milliseconds
 *
 * See HdySwipeable::child-switched.
 *
 * Since: 1.0
 */
void
hdy_swipeable_switch_child (HdySwipeable *self,
                            guint         index,
                            gint64        duration)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_if_fail (iface->switch_child != NULL);

  iface->switch_child (self, index, duration);
}

/**
 * hdy_swipeable_emit_child_switched:
 * @self: a #HdySwipeable
 * @index: the index of the child to switch to
 * @duration: Animation duration in milliseconds
 *
 * Emits HdySwipeable::child-switched signal. This should be called when the
 * widget switches visible child widget.
 *
 * @duration can be 0 if the child is switched without animation.
 *
 * Since: 1.0
 */
void
hdy_swipeable_emit_child_switched (HdySwipeable *self,
                                   guint         index,
                                   gint64        duration)
{
  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  g_signal_emit (self, signals[SIGNAL_CHILD_SWITCHED], 0, index, duration);
}

/**
 * hdy_swipeable_get_swipe_tracker:
 * @self: a #HdySwipeable
 *
 * Gets the #HdySwipeTracker used by this swipeable widget.
 *
 * Returns: (transfer none): the swipe tracker
 *
 * Since: 1.0
 */
HdySwipeTracker *
hdy_swipeable_get_swipe_tracker (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), NULL);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_swipe_tracker != NULL, NULL);

  return iface->get_swipe_tracker (self);
}

/**
 * hdy_swipeable_get_distance:
 * @self: a #HdySwipeable
 *
 * Gets the swipe distance of @self. This corresponds to how many pixels
 * 1 unit represents.
 *
 * Returns: the swipe distance in pixels
 *
 * Since: 1.0
 */
gdouble
hdy_swipeable_get_distance (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), 0);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_distance != NULL, 0);

  return iface->get_distance (self);
}

/**
 * hdy_swipeable_get_snap_points: (virtual get_snap_points)
 * @self: a #HdySwipeable
 * @n_snap_points: (out): location to return the number of the snap points
 *
 * Gets the snap points of @self. Each snap point represents a progress value
 * that is considered acceptable to end the swipe on.
 *
 * Returns: (array length=n_snap_points) (transfer full): the snap points of
 *     @self. The array must be freed with g_free().
 *
 * Since: 1.0
 */
gdouble *
hdy_swipeable_get_snap_points (HdySwipeable *self,
                               gint         *n_snap_points)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), NULL);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_snap_points != NULL, NULL);

  return iface->get_snap_points (self, n_snap_points);
}

/**
 * hdy_swipeable_get_progress:
 * @self: a #HdySwipeable
 *
 * Gets the current progress of @self
 *
 * Returns: the current progress, unitless
 *
 * Since: 1.0
 */
gdouble
hdy_swipeable_get_progress (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), 0);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_progress != NULL, 0);

  return iface->get_progress (self);
}

/**
 * hdy_swipeable_get_cancel_progress:
 * @self: a #HdySwipeable
 *
 * Gets the progress @self will snap back to after the gesture is canceled.
 *
 * Returns: the cancel progress, unitless
 *
 * Since: 1.0
 */
gdouble
hdy_swipeable_get_cancel_progress (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), 0);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_cancel_progress != NULL, 0);

  return iface->get_cancel_progress (self);
}

/**
 * hdy_swipeable_get_swipe_area:
 * @self: a #HdySwipeable
 * @navigation_direction: the direction of the swipe
 * @is_drag: whether the swipe is caused by a dragging gesture
 * @rect: (out): a pointer to a #GdkRectangle to store the swipe area
 *
 * Gets the area @self can start a swipe from for the given direction and
 * gesture type.
 * This can be used to restrict swipes to only be possible from a certain area,
 * for example, to only allow edge swipes, or to have a draggable element and
 * ignore swipes elsewhere.
 *
 * Swipe area is only considered for direct swipes (as in, not initiated by
 * #HdySwipeGroup).
 *
 * If not implemented, the default implementation returns the allocation of
 * @self, allowing swipes from anywhere.
 *
 * Since: 1.0
 */
void
hdy_swipeable_get_swipe_area (HdySwipeable           *self,
                              HdyNavigationDirection  navigation_direction,
                              gboolean                is_drag,
                              GdkRectangle           *rect)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));
  g_return_if_fail (rect != NULL);

  iface = HDY_SWIPEABLE_GET_IFACE (self);

  if (iface->get_swipe_area) {
    iface->get_swipe_area (self, navigation_direction, is_drag, rect);
    return;
  }

  rect->x = 0;
  rect->y = 0;
  rect->width = gtk_widget_get_allocated_width (GTK_WIDGET (self));
  rect->height = gtk_widget_get_allocated_height (GTK_WIDGET (self));
}
