/*
 * Copyright (C) 2018 Purism SPC
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-deck.h"
#include "hdy-stackable-box-private.h"
#include "hdy-swipeable.h"

/**
 * SECTION:hdy-deck
 * @short_description: A swipeable widget showing one of the visible children at a time.
 * @Title: HdyDeck
 *
 * The #HdyDeck widget displays one of the visible children, similar to a
 * #GtkStack. The children are strictly ordered and can be navigated using
 * swipe gestures.
 *
 * The “over” and “under” stack the children one on top of the other, while the
 * “slide” transition puts the children side by side. While navigating to a
 * child on the side or below can be performed by swiping the current child
 * away, navigating to an upper child requires dragging it from the edge where
 * it resides. This doesn't affect non-dragging swipes.
 *
 * The “over” and “under” transitions can draw their shadow on top of the
 * window's transparent areas, like the rounded corners. This is a side-effect
 * of allowing shadows to be drawn on top of OpenGL areas. It can be mitigated
 * by using #HdyWindow or #HdyApplicationWindow as they will crop anything drawn
 * beyond the rounded corners.
 *
 * # CSS nodes
 *
 * #HdyDeck has a single CSS node with name deck.
 *
 * Since: 1.0
 */

/**
 * HdyDeckTransitionType:
 * @HDY_DECK_TRANSITION_TYPE_OVER: Cover the old page or uncover the new page, sliding from or towards the end according to orientation, text direction and children order
 * @HDY_DECK_TRANSITION_TYPE_UNDER: Uncover the new page or cover the old page, sliding from or towards the start according to orientation, text direction and children order
 * @HDY_DECK_TRANSITION_TYPE_SLIDE: Slide from left, right, up or down according to the orientation, text direction and the children order
 *
 * This enumeration value describes the possible transitions between children
 * in a #HdyDeck widget.
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_HHOMOGENEOUS,
  PROP_VHOMOGENEOUS,
  PROP_VISIBLE_CHILD,
  PROP_VISIBLE_CHILD_NAME,
  PROP_TRANSITION_TYPE,
  PROP_TRANSITION_DURATION,
  PROP_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,
  PROP_CAN_SWIPE_BACK,
  PROP_CAN_SWIPE_FORWARD,

  /* orientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ORIENTATION,
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_NAME,
  LAST_CHILD_PROP,
};

typedef struct
{
  HdyStackableBox *box;
} HdyDeckPrivate;

static GParamSpec *props[LAST_PROP];
static GParamSpec *child_props[LAST_CHILD_PROP];

static void hdy_deck_swipeable_init (HdySwipeableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyDeck, hdy_deck, GTK_TYPE_CONTAINER,
                         G_ADD_PRIVATE (HdyDeck)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (HDY_TYPE_SWIPEABLE, hdy_deck_swipeable_init))

#define HDY_GET_HELPER(obj) (((HdyDeckPrivate *) hdy_deck_get_instance_private (HDY_DECK (obj)))->box)

/**
 * hdy_deck_set_homogeneous:
 * @self: a #HdyDeck
 * @orientation: the orientation
 * @homogeneous: %TRUE to make @self homogeneous
 *
 * Sets the #HdyDeck to be homogeneous or not for the given orientation.
 * If it is homogeneous, the #HdyDeck will request the same
 * width or height for all its children depending on the orientation.
 * If it isn't, the deck may change width or height when a different child
 * becomes visible.
 *
 * Since: 1.0
 */
void
hdy_deck_set_homogeneous (HdyDeck        *self,
                          GtkOrientation  orientation,
                          gboolean        homogeneous)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_homogeneous (HDY_GET_HELPER (self), TRUE, orientation, homogeneous);
}

/**
 * hdy_deck_get_homogeneous:
 * @self: a #HdyDeck
 * @orientation: the orientation
 *
 * Gets whether @self is homogeneous for the given orientation.
 * See hdy_deck_set_homogeneous().
 *
 * Returns: whether @self is homogeneous for the given orientation.
 *
 * Since: 1.0
 */
gboolean
hdy_deck_get_homogeneous (HdyDeck        *self,
                          GtkOrientation  orientation)
{
  g_return_val_if_fail (HDY_IS_DECK (self), FALSE);

  return hdy_stackable_box_get_homogeneous (HDY_GET_HELPER (self), TRUE, orientation);
}

/**
 * hdy_deck_get_transition_type:
 * @self: a #HdyDeck
 *
 * Gets the type of animation that will be used
 * for transitions between children in @self.
 *
 * Returns: the current transition type of @self
 *
 * Since: 1.0
 */
HdyDeckTransitionType
hdy_deck_get_transition_type (HdyDeck *self)
{
  HdyStackableBoxTransitionType type;

  g_return_val_if_fail (HDY_IS_DECK (self), HDY_DECK_TRANSITION_TYPE_OVER);

  type = hdy_stackable_box_get_transition_type (HDY_GET_HELPER (self));

  switch (type) {
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER:
    return HDY_DECK_TRANSITION_TYPE_OVER;

  case HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER:
    return HDY_DECK_TRANSITION_TYPE_UNDER;

  case HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE:
    return HDY_DECK_TRANSITION_TYPE_SLIDE;

  default:
    g_assert_not_reached ();
  }
}

/**
 * hdy_deck_set_transition_type:
 * @self: a #HdyDeck
 * @transition: the new transition type
 *
 * Sets the type of animation that will be used for transitions between children
 * in @self.
 *
 * The transition type can be changed without problems at runtime, so it is
 * possible to change the animation based on the child that is about to become
 * current.
 *
 * Since: 1.0
 */
void
hdy_deck_set_transition_type (HdyDeck               *self,
                              HdyDeckTransitionType  transition)
{
  HdyStackableBoxTransitionType type;

  g_return_if_fail (HDY_IS_DECK (self));
  g_return_if_fail (transition <= HDY_DECK_TRANSITION_TYPE_SLIDE);

  switch (transition) {
  case HDY_DECK_TRANSITION_TYPE_OVER:
    type = HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER;
    break;

  case HDY_DECK_TRANSITION_TYPE_UNDER:
    type = HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER;
    break;

  case HDY_DECK_TRANSITION_TYPE_SLIDE:
    type = HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE;
    break;

  default:
    g_assert_not_reached ();
  }

  hdy_stackable_box_set_transition_type (HDY_GET_HELPER (self), type);
}

/**
 * hdy_deck_get_transition_duration:
 * @self: a #HdyDeck
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between children in @self will take.
 *
 * Returns: the child transition duration
 *
 * Since: 1.0
 */
guint
hdy_deck_get_transition_duration (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), 0);

  return hdy_stackable_box_get_child_transition_duration (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_set_transition_duration:
 * @self: a #HdyDeck
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between children in @self
 * will take.
 *
 * Since: 1.0
 */
void
hdy_deck_set_transition_duration (HdyDeck *self,
                                  guint    duration)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_child_transition_duration (HDY_GET_HELPER (self), duration);
}

/**
 * hdy_deck_get_visible_child:
 * @self: a #HdyDeck
 *
 * Gets the visible child widget.
 *
 * Returns: (transfer none): the visible child widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_deck_get_visible_child (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), NULL);

  return hdy_stackable_box_get_visible_child (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_set_visible_child:
 * @self: a #HdyDeck
 * @visible_child: the new child
 *
 * Makes @visible_child visible using a transition determined by
 * HdyDeck:transition-type and HdyDeck:transition-duration. The transition can
 * be cancelled by the user, in which case visible child will change back to
 * the previously visible child.
 *
 * Since: 1.0
 */
void
hdy_deck_set_visible_child (HdyDeck   *self,
                            GtkWidget *visible_child)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_visible_child (HDY_GET_HELPER (self), visible_child);
}

/**
 * hdy_deck_get_visible_child_name:
 * @self: a #HdyDeck
 *
 * Gets the name of the currently visible child widget.
 *
 * Returns: (transfer none): the name of the visible child
 *
 * Since: 1.0
 */
const gchar *
hdy_deck_get_visible_child_name (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), NULL);

  return hdy_stackable_box_get_visible_child_name (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_set_visible_child_name:
 * @self: a #HdyDeck
 * @name: the name of a child
 *
 * Makes the child with the name @name visible.
 *
 * See hdy_deck_set_visible_child() for more details.
 *
 * Since: 1.0
 */
void
hdy_deck_set_visible_child_name (HdyDeck     *self,
                                 const gchar *name)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_visible_child_name (HDY_GET_HELPER (self), name);
}

/**
 * hdy_deck_get_transition_running:
 * @self: a #HdyDeck
 *
 * Returns whether @self is currently in a transition from one page to
 * another.
 *
 * Returns: %TRUE if the transition is currently running, %FALSE otherwise.
 *
 * Since: 1.0
 */
gboolean
hdy_deck_get_transition_running (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), FALSE);

  return hdy_stackable_box_get_child_transition_running (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_set_interpolate_size:
 * @self: a #HdyDeck
 * @interpolate_size: the new value
 *
 * Sets whether or not @self will interpolate its size when
 * changing the visible child. If the #HdyDeck:interpolate-size
 * property is set to %TRUE, @self will interpolate its size between
 * the current one and the one it'll take after changing the
 * visible child, according to the set transition duration.
 *
 * Since: 1.0
 */
void
hdy_deck_set_interpolate_size (HdyDeck  *self,
                               gboolean  interpolate_size)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_interpolate_size (HDY_GET_HELPER (self), interpolate_size);
}

/**
 * hdy_deck_get_interpolate_size:
 * @self: a #HdyDeck
 *
 * Returns whether the #HdyDeck is set up to interpolate between
 * the sizes of children on page switch.
 *
 * Returns: %TRUE if child sizes are interpolated
 *
 * Since: 1.0
 */
gboolean
hdy_deck_get_interpolate_size (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), FALSE);

  return hdy_stackable_box_get_interpolate_size (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_set_can_swipe_back:
 * @self: a #HdyDeck
 * @can_swipe_back: the new value
 *
 * Sets whether or not @self allows switching to the previous child via a swipe
 * gesture.
 *
 * Since: 1.0
 */
void
hdy_deck_set_can_swipe_back (HdyDeck  *self,
                             gboolean  can_swipe_back)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_can_swipe_back (HDY_GET_HELPER (self), can_swipe_back);
}

/**
 * hdy_deck_get_can_swipe_back
 * @self: a #HdyDeck
 *
 * Returns whether the #HdyDeck allows swiping to the previous child.
 *
 * Returns: %TRUE if back swipe is enabled.
 *
 * Since: 1.0
 */
gboolean
hdy_deck_get_can_swipe_back (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), FALSE);

  return hdy_stackable_box_get_can_swipe_back (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_set_can_swipe_forward:
 * @self: a #HdyDeck
 * @can_swipe_forward: the new value
 *
 * Sets whether or not @self allows switching to the next child via a swipe
 * gesture.
 *
 * Since: 1.0
 */
void
hdy_deck_set_can_swipe_forward (HdyDeck  *self,
                                gboolean  can_swipe_forward)
{
  g_return_if_fail (HDY_IS_DECK (self));

  hdy_stackable_box_set_can_swipe_forward (HDY_GET_HELPER (self), can_swipe_forward);
}

/**
 * hdy_deck_get_can_swipe_forward
 * @self: a #HdyDeck
 *
 * Returns whether the #HdyDeck allows swiping to the next child.
 *
 * Returns: %TRUE if forward swipe is enabled.
 *
 * Since: 1.0
 */
gboolean
hdy_deck_get_can_swipe_forward (HdyDeck *self)
{
  g_return_val_if_fail (HDY_IS_DECK (self), FALSE);

  return hdy_stackable_box_get_can_swipe_forward (HDY_GET_HELPER (self));
}

/**
 * hdy_deck_get_adjacent_child
 * @self: a #HdyDeck
 * @direction: the direction
 *
 * Gets the previous or next child, or %NULL if it doesn't exist. This will be
 * the same widget hdy_deck_navigate() will navigate to.
 *
 * Returns: (nullable) (transfer none): the previous or next child, or
 *   %NULL if it doesn't exist.
 *
 * Since: 1.0
 */
GtkWidget *
hdy_deck_get_adjacent_child (HdyDeck                *self,
                             HdyNavigationDirection  direction)
{
  g_return_val_if_fail (HDY_IS_DECK (self), NULL);

  return hdy_stackable_box_get_adjacent_child (HDY_GET_HELPER (self), direction);
}

/**
 * hdy_deck_navigate
 * @self: a #HdyDeck
 * @direction: the direction
 *
 * Switches to the previous or next child, similar to performing a swipe
 * gesture to go in @direction.
 *
 * Returns: %TRUE if visible child was changed, %FALSE otherwise.
 *
 * Since: 1.0
 */
gboolean
hdy_deck_navigate (HdyDeck                *self,
                   HdyNavigationDirection  direction)
{
  g_return_val_if_fail (HDY_IS_DECK (self), FALSE);

  return hdy_stackable_box_navigate (HDY_GET_HELPER (self), direction);
}

/**
 * hdy_deck_get_child_by_name:
 * @self: a #HdyDeck
 * @name: the name of the child to find
 *
 * Finds the child of @self with the name given as the argument. Returns %NULL
 * if there is no child with this name.
 *
 * Returns: (transfer none) (nullable): the requested child of @self
 *
 * Since: 1.0
 */
GtkWidget *
hdy_deck_get_child_by_name (HdyDeck     *self,
                            const gchar *name)
{
  g_return_val_if_fail (HDY_IS_DECK (self), NULL);

  return hdy_stackable_box_get_child_by_name (HDY_GET_HELPER (self), name);
}

/* This private method is prefixed by the call name because it will be a virtual
 * method in GTK 4.
 */
static void
hdy_deck_measure (GtkWidget      *widget,
                  GtkOrientation  orientation,
                  int             for_size,
                  int            *minimum,
                  int            *natural,
                  int            *minimum_baseline,
                  int            *natural_baseline)
{
  hdy_stackable_box_measure (HDY_GET_HELPER (widget),
                             orientation, for_size,
                             minimum, natural,
                             minimum_baseline, natural_baseline);
}

static void
hdy_deck_get_preferred_width (GtkWidget *widget,
                              gint      *minimum_width,
                              gint      *natural_width)
{
  hdy_deck_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                    minimum_width, natural_width, NULL, NULL);
}

static void
hdy_deck_get_preferred_height (GtkWidget *widget,
                               gint      *minimum_height,
                               gint      *natural_height)
{
  hdy_deck_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                    minimum_height, natural_height, NULL, NULL);
}

static void
hdy_deck_get_preferred_width_for_height (GtkWidget *widget,
                                         gint       height,
                                         gint      *minimum_width,
                                         gint      *natural_width)
{
  hdy_deck_measure (widget, GTK_ORIENTATION_HORIZONTAL, height,
                    minimum_width, natural_width, NULL, NULL);
}

static void
hdy_deck_get_preferred_height_for_width (GtkWidget *widget,
                                         gint       width,
                                         gint      *minimum_height,
                                         gint      *natural_height)
{
  hdy_deck_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                    minimum_height, natural_height, NULL, NULL);
}

static void
hdy_deck_size_allocate (GtkWidget     *widget,
                        GtkAllocation *allocation)
{
  hdy_stackable_box_size_allocate (HDY_GET_HELPER (widget), allocation);
}

static gboolean
hdy_deck_draw (GtkWidget *widget,
               cairo_t   *cr)
{
  return hdy_stackable_box_draw (HDY_GET_HELPER (widget), cr);
}

static void
hdy_deck_direction_changed (GtkWidget        *widget,
                            GtkTextDirection  previous_direction)
{
  hdy_stackable_box_direction_changed (HDY_GET_HELPER (widget), previous_direction);
}

static void
hdy_deck_add (GtkContainer *container,
              GtkWidget    *widget)
{
  hdy_stackable_box_add (HDY_GET_HELPER (container), widget);
}

static void
hdy_deck_remove (GtkContainer *container,
                 GtkWidget    *widget)
{
  hdy_stackable_box_remove (HDY_GET_HELPER (container), widget);
}

static void
hdy_deck_forall (GtkContainer *container,
                 gboolean      include_internals,
                 GtkCallback   callback,
                 gpointer      callback_data)
{
  hdy_stackable_box_forall (HDY_GET_HELPER (container), include_internals, callback, callback_data);
}

static void
hdy_deck_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  HdyDeck *self = HDY_DECK (object);

  switch (prop_id) {
  case PROP_HHOMOGENEOUS:
    g_value_set_boolean (value, hdy_deck_get_homogeneous (self, GTK_ORIENTATION_HORIZONTAL));
    break;
  case PROP_VHOMOGENEOUS:
    g_value_set_boolean (value, hdy_deck_get_homogeneous (self, GTK_ORIENTATION_VERTICAL));
    break;
  case PROP_VISIBLE_CHILD:
    g_value_set_object (value, hdy_deck_get_visible_child (self));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    g_value_set_string (value, hdy_deck_get_visible_child_name (self));
    break;
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, hdy_deck_get_transition_type (self));
    break;
  case PROP_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_deck_get_transition_duration (self));
    break;
  case PROP_TRANSITION_RUNNING:
    g_value_set_boolean (value, hdy_deck_get_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, hdy_deck_get_interpolate_size (self));
    break;
  case PROP_CAN_SWIPE_BACK:
    g_value_set_boolean (value, hdy_deck_get_can_swipe_back (self));
    break;
  case PROP_CAN_SWIPE_FORWARD:
    g_value_set_boolean (value, hdy_deck_get_can_swipe_forward (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, hdy_stackable_box_get_orientation (HDY_GET_HELPER (self)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_deck_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  HdyDeck *self = HDY_DECK (object);

  switch (prop_id) {
  case PROP_HHOMOGENEOUS:
    hdy_deck_set_homogeneous (self, GTK_ORIENTATION_HORIZONTAL, g_value_get_boolean (value));
    break;
  case PROP_VHOMOGENEOUS:
    hdy_deck_set_homogeneous (self, GTK_ORIENTATION_VERTICAL, g_value_get_boolean (value));
    break;
  case PROP_VISIBLE_CHILD:
    hdy_deck_set_visible_child (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    hdy_deck_set_visible_child_name (self, g_value_get_string (value));
    break;
  case PROP_TRANSITION_TYPE:
    hdy_deck_set_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_TRANSITION_DURATION:
    hdy_deck_set_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    hdy_deck_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_BACK:
    hdy_deck_set_can_swipe_back (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_FORWARD:
    hdy_deck_set_can_swipe_forward (self, g_value_get_boolean (value));
    break;
  case PROP_ORIENTATION:
    hdy_stackable_box_set_orientation (HDY_GET_HELPER (self), g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_deck_finalize (GObject *object)
{
  HdyDeck *self = HDY_DECK (object);
  HdyDeckPrivate *priv = hdy_deck_get_instance_private (self);

  g_clear_object (&priv->box);

  G_OBJECT_CLASS (hdy_deck_parent_class)->finalize (object);
}

static void
hdy_deck_get_child_property (GtkContainer *container,
                             GtkWidget    *widget,
                             guint         property_id,
                             GValue       *value,
                             GParamSpec   *pspec)
{
  switch (property_id) {
  case CHILD_PROP_NAME:
    g_value_set_string (value, hdy_stackable_box_get_child_name (HDY_GET_HELPER (container), widget));
    break;

  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_deck_set_child_property (GtkContainer *container,
                             GtkWidget    *widget,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  switch (property_id) {
  case CHILD_PROP_NAME:
    hdy_stackable_box_set_child_name (HDY_GET_HELPER (container), widget, g_value_get_string (value));
    gtk_container_child_notify_by_pspec (container, widget, pspec);
    break;

  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_deck_realize (GtkWidget *widget)
{
  hdy_stackable_box_realize (HDY_GET_HELPER (widget));
}

static void
hdy_deck_unrealize (GtkWidget *widget)
{
  hdy_stackable_box_unrealize (HDY_GET_HELPER (widget));
}

static void
hdy_deck_map (GtkWidget *widget)
{
  hdy_stackable_box_map (HDY_GET_HELPER (widget));
}

static void
hdy_deck_unmap (GtkWidget *widget)
{
  hdy_stackable_box_unmap (HDY_GET_HELPER (widget));
}

static void
hdy_deck_switch_child (HdySwipeable *swipeable,
                       guint         index,
                       gint64        duration)
{
  hdy_stackable_box_switch_child (HDY_GET_HELPER (swipeable), index, duration);
}

static HdySwipeTracker *
hdy_deck_get_swipe_tracker (HdySwipeable *swipeable)
{
  return hdy_stackable_box_get_swipe_tracker (HDY_GET_HELPER (swipeable));
}

static gdouble
hdy_deck_get_distance (HdySwipeable *swipeable)
{
  return hdy_stackable_box_get_distance (HDY_GET_HELPER (swipeable));
}

static gdouble *
hdy_deck_get_snap_points (HdySwipeable *swipeable,
                          gint         *n_snap_points)
{
  return hdy_stackable_box_get_snap_points (HDY_GET_HELPER (swipeable), n_snap_points);
}

static gdouble
hdy_deck_get_progress (HdySwipeable *swipeable)
{
  return hdy_stackable_box_get_progress (HDY_GET_HELPER (swipeable));
}

static gdouble
hdy_deck_get_cancel_progress (HdySwipeable *swipeable)
{
  return hdy_stackable_box_get_cancel_progress (HDY_GET_HELPER (swipeable));
}

static void
hdy_deck_get_swipe_area (HdySwipeable           *swipeable,
                         HdyNavigationDirection  navigation_direction,
                         gboolean                is_drag,
                         GdkRectangle           *rect)
{
  hdy_stackable_box_get_swipe_area (HDY_GET_HELPER (swipeable), navigation_direction, is_drag, rect);
}

static void
hdy_deck_class_init (HdyDeckClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;
  GtkContainerClass *container_class = (GtkContainerClass*) klass;

  object_class->get_property = hdy_deck_get_property;
  object_class->set_property = hdy_deck_set_property;
  object_class->finalize = hdy_deck_finalize;

  widget_class->realize = hdy_deck_realize;
  widget_class->unrealize = hdy_deck_unrealize;
  widget_class->map = hdy_deck_map;
  widget_class->unmap = hdy_deck_unmap;
  widget_class->get_preferred_width = hdy_deck_get_preferred_width;
  widget_class->get_preferred_height = hdy_deck_get_preferred_height;
  widget_class->get_preferred_width_for_height = hdy_deck_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = hdy_deck_get_preferred_height_for_width;
  widget_class->size_allocate = hdy_deck_size_allocate;
  widget_class->draw = hdy_deck_draw;
  widget_class->direction_changed = hdy_deck_direction_changed;

  container_class->add = hdy_deck_add;
  container_class->remove = hdy_deck_remove;
  container_class->forall = hdy_deck_forall;
  container_class->set_child_property = hdy_deck_set_child_property;
  container_class->get_child_property = hdy_deck_get_child_property;
  gtk_container_class_handle_border_width (container_class);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * HdyDeck:hhomogeneous:
   *
   * Horizontally homogeneous sizing.
   *
   * Since: 1.0
   */
  props[PROP_HHOMOGENEOUS] =
    g_param_spec_boolean ("hhomogeneous",
                          _("Horizontally homogeneous"),
                          _("Horizontally homogeneous sizing"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:vhomogeneous:
   *
   * Vertically homogeneous sizing.
   *
   * Since: 1.0
   */
  props[PROP_VHOMOGENEOUS] =
    g_param_spec_boolean ("vhomogeneous",
                          _("Vertically homogeneous"),
                          _("Vertically homogeneous sizing"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:visible-child:
   *
   * The widget currently visible.
   *
   * Since: 1.0
   */
  props[PROP_VISIBLE_CHILD] =
    g_param_spec_object ("visible-child",
                         _("Visible child"),
                         _("The widget currently visible"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:visible-child-name:
   *
   * The name of the widget currently visible.
   *
   * Since: 1.0
   */
  props[PROP_VISIBLE_CHILD_NAME] =
    g_param_spec_string ("visible-child-name",
                         _("Name of visible child"),
                         _("The name of the widget currently visible"),
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:transition-type:
   *
   * The type of animation that will be used for transitions between
   * children.
   *
   * The transition type can be changed without problems at runtime, so it is
   * possible to change the animation based on the child that is about
   * to become current.
   *
   * Since: 1.0
   */
  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type",
                       _("Transition type"),
                       _("The type of animation used to transition between children"),
                       HDY_TYPE_DECK_TRANSITION_TYPE, HDY_DECK_TRANSITION_TYPE_OVER,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:transition-duration:
   *
   * The transition animation duration, in milliseconds.
   *
   * Since: 1.0
   */
  props[PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration",
                       _("Transition duration"),
                       _("The transition animation duration, in milliseconds"),
                       0, G_MAXUINT, 200,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:transition-running:
   *
   * Whether or not the transition is currently running.
   *
   * Since: 1.0
   */
  props[PROP_TRANSITION_RUNNING] =
      g_param_spec_boolean ("transition-running",
                            _("Transition running"),
                            _("Whether or not the transition is currently running"),
                            FALSE,
                            G_PARAM_READABLE);

  /**
   * HdyDeck:interpolate-size:
   *
   * Whether or not the size should smoothly change when changing between
   * differently sized children.
   *
   * Since: 1.0
   */
  props[PROP_INTERPOLATE_SIZE] =
      g_param_spec_boolean ("interpolate-size",
                            _("Interpolate size"),
                            _("Whether or not the size should smoothly change when changing between differently sized children"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:can-swipe-back:
   *
   * Whether or not the deck allows switching to the previous child via a swipe
   * gesture.
   *
   * Since: 1.0
   */
  props[PROP_CAN_SWIPE_BACK] =
      g_param_spec_boolean ("can-swipe-back",
                            _("Can swipe back"),
                            _("Whether or not swipe gesture can be used to switch to the previous child"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyDeck:can-swipe-forward:
   *
   * Whether or not the deck allows switching to the next child via a swipe
   * gesture.
   *
   * Since: 1.0
   */
  props[PROP_CAN_SWIPE_FORWARD] =
      g_param_spec_boolean ("can-swipe-forward",
                            _("Can swipe forward"),
                            _("Whether or not swipe gesture can be used to switch to the next child"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  child_props[CHILD_PROP_NAME] =
    g_param_spec_string ("name",
                         _("Name"),
                         _("The name of the child page"),
                         NULL,
                         G_PARAM_READWRITE);

  gtk_container_class_install_child_properties (container_class, LAST_CHILD_PROP, child_props);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_PANEL);
  gtk_widget_class_set_css_name (widget_class, "deck");
}

GtkWidget *
hdy_deck_new (void)
{
  return g_object_new (HDY_TYPE_DECK, NULL);
}

#define NOTIFY(func, prop) \
static void \
func (HdyDeck *self) { \
  g_object_notify_by_pspec (G_OBJECT (self), props[prop]); \
}

NOTIFY (notify_hhomogeneous_folded_cb, PROP_HHOMOGENEOUS);
NOTIFY (notify_vhomogeneous_folded_cb, PROP_VHOMOGENEOUS);
NOTIFY (notify_visible_child_cb, PROP_VISIBLE_CHILD);
NOTIFY (notify_visible_child_name_cb, PROP_VISIBLE_CHILD_NAME);
NOTIFY (notify_transition_type_cb, PROP_TRANSITION_TYPE);
NOTIFY (notify_child_transition_duration_cb, PROP_TRANSITION_DURATION);
NOTIFY (notify_child_transition_running_cb, PROP_TRANSITION_RUNNING);
NOTIFY (notify_interpolate_size_cb, PROP_INTERPOLATE_SIZE);
NOTIFY (notify_can_swipe_back_cb, PROP_CAN_SWIPE_BACK);
NOTIFY (notify_can_swipe_forward_cb, PROP_CAN_SWIPE_FORWARD);

static void
notify_orientation_cb (HdyDeck *self)
{
  g_object_notify (G_OBJECT (self), "orientation");
}

static void
hdy_deck_init (HdyDeck *self)
{
  HdyDeckPrivate *priv = hdy_deck_get_instance_private (self);

  priv->box = hdy_stackable_box_new (GTK_CONTAINER (self),
                                     GTK_CONTAINER_CLASS (hdy_deck_parent_class),
                                     FALSE);

  g_signal_connect_object (priv->box, "notify::hhomogeneous-folded", G_CALLBACK (notify_hhomogeneous_folded_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::vhomogeneous-folded", G_CALLBACK (notify_vhomogeneous_folded_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::visible-child", G_CALLBACK (notify_visible_child_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::visible-child-name", G_CALLBACK (notify_visible_child_name_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::transition-type", G_CALLBACK (notify_transition_type_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::child-transition-duration", G_CALLBACK (notify_child_transition_duration_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::child-transition-running", G_CALLBACK (notify_child_transition_running_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::interpolate-size", G_CALLBACK (notify_interpolate_size_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::can-swipe-back", G_CALLBACK (notify_can_swipe_back_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::can-swipe-forward", G_CALLBACK (notify_can_swipe_forward_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->box, "notify::orientation", G_CALLBACK (notify_orientation_cb), self, G_CONNECT_SWAPPED);
}

static void
hdy_deck_swipeable_init (HdySwipeableInterface *iface)
{
  iface->switch_child = hdy_deck_switch_child;
  iface->get_swipe_tracker = hdy_deck_get_swipe_tracker;
  iface->get_distance = hdy_deck_get_distance;
  iface->get_snap_points = hdy_deck_get_snap_points;
  iface->get_progress = hdy_deck_get_progress;
  iface->get_cancel_progress = hdy_deck_get_cancel_progress;
  iface->get_swipe_area = hdy_deck_get_swipe_area;
}
