/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "hdy-navigation-direction.h"
#include "hdy-swipe-tracker.h"

G_BEGIN_DECLS

#define HDY_TYPE_STACKABLE_BOX (hdy_stackable_box_get_type())

G_DECLARE_FINAL_TYPE (HdyStackableBox, hdy_stackable_box, HDY, STACKABLE_BOX, GObject)

typedef enum {
  HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER,
  HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER,
  HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE,
} HdyStackableBoxTransitionType;

HdyStackableBox *hdy_stackable_box_new (GtkContainer      *container,
                                        GtkContainerClass *klass,
                                        gboolean           can_unfold);
gboolean         hdy_stackable_box_get_folded (HdyStackableBox *self);
GtkWidget       *hdy_stackable_box_get_visible_child (HdyStackableBox *self);
void             hdy_stackable_box_set_visible_child (HdyStackableBox *self,
                                                      GtkWidget       *visible_child);
const gchar     *hdy_stackable_box_get_visible_child_name (HdyStackableBox *self);
void             hdy_stackable_box_set_visible_child_name (HdyStackableBox *self,
                                                           const gchar     *name);
gboolean         hdy_stackable_box_get_homogeneous (HdyStackableBox *self,
                                                    gboolean         folded,
                                                    GtkOrientation   orientation);
void             hdy_stackable_box_set_homogeneous (HdyStackableBox *self,
                                                    gboolean         folded,
                                                    GtkOrientation   orientation,
                                                    gboolean         homogeneous);
HdyStackableBoxTransitionType hdy_stackable_box_get_transition_type (HdyStackableBox *self);
void             hdy_stackable_box_set_transition_type (HdyStackableBox               *self,
                                                        HdyStackableBoxTransitionType  transition);

guint            hdy_stackable_box_get_mode_transition_duration (HdyStackableBox *self);
void             hdy_stackable_box_set_mode_transition_duration (HdyStackableBox *self,
                                                                 guint            duration);

guint            hdy_stackable_box_get_child_transition_duration (HdyStackableBox *self);
void             hdy_stackable_box_set_child_transition_duration (HdyStackableBox *self,
                                                                  guint            duration);
gboolean         hdy_stackable_box_get_child_transition_running (HdyStackableBox *self);
gboolean         hdy_stackable_box_get_interpolate_size (HdyStackableBox *self);
void             hdy_stackable_box_set_interpolate_size (HdyStackableBox *self,
                                                         gboolean         interpolate_size);
gboolean         hdy_stackable_box_get_can_swipe_back (HdyStackableBox *self);
void             hdy_stackable_box_set_can_swipe_back (HdyStackableBox *self,
                                                       gboolean         can_swipe_back);
gboolean         hdy_stackable_box_get_can_swipe_forward (HdyStackableBox *self);
void             hdy_stackable_box_set_can_swipe_forward (HdyStackableBox *self,
                                                          gboolean         can_swipe_forward);

GtkWidget       *hdy_stackable_box_get_adjacent_child (HdyStackableBox        *self,
                                                       HdyNavigationDirection  direction);
gboolean         hdy_stackable_box_navigate (HdyStackableBox        *self,
                                             HdyNavigationDirection  direction);

GtkWidget       *hdy_stackable_box_get_child_by_name (HdyStackableBox *self,
                                                      const gchar     *name);

GtkOrientation   hdy_stackable_box_get_orientation (HdyStackableBox *self);
void             hdy_stackable_box_set_orientation (HdyStackableBox *self,
                                                    GtkOrientation   orientation);

const gchar     *hdy_stackable_box_get_child_name (HdyStackableBox *self,
                                                   GtkWidget       *widget);
void             hdy_stackable_box_set_child_name (HdyStackableBox *self,
                                                   GtkWidget       *widget,
                                                   const gchar     *name);
gboolean         hdy_stackable_box_get_child_navigatable (HdyStackableBox *self,
                                                          GtkWidget       *widget);
void             hdy_stackable_box_set_child_navigatable (HdyStackableBox *self,
                                                          GtkWidget       *widget,
                                                          gboolean         navigatable);

void             hdy_stackable_box_switch_child (HdyStackableBox *self,
                                                 guint            index,
                                                 gint64           duration);

HdySwipeTracker *hdy_stackable_box_get_swipe_tracker (HdyStackableBox *self);
gdouble          hdy_stackable_box_get_distance (HdyStackableBox *self);
gdouble         *hdy_stackable_box_get_snap_points (HdyStackableBox *self,
                                                    gint            *n_snap_points);
gdouble          hdy_stackable_box_get_progress (HdyStackableBox *self);
gdouble          hdy_stackable_box_get_cancel_progress (HdyStackableBox *self);
void             hdy_stackable_box_get_swipe_area (HdyStackableBox        *self,
                                                   HdyNavigationDirection  navigation_direction,
                                                   gboolean                is_drag,
                                                   GdkRectangle           *rect);

void             hdy_stackable_box_add (HdyStackableBox *self,
                                        GtkWidget       *widget);
void             hdy_stackable_box_remove (HdyStackableBox *self,
                                           GtkWidget       *widget);
void             hdy_stackable_box_forall (HdyStackableBox *self,
                                           gboolean         include_internals,
                                           GtkCallback      callback,
                                           gpointer         callback_data);

void             hdy_stackable_box_measure (HdyStackableBox *self,
                                            GtkOrientation   orientation,
                                            int              for_size,
                                            int             *minimum,
                                            int             *natural,
                                            int             *minimum_baseline,
                                            int             *natural_baseline);
void             hdy_stackable_box_size_allocate (HdyStackableBox *self,
                                                  GtkAllocation   *allocation);
gboolean         hdy_stackable_box_draw (HdyStackableBox *self,
                                         cairo_t         *cr);
void             hdy_stackable_box_realize (HdyStackableBox *self);
void             hdy_stackable_box_unrealize (HdyStackableBox *self);
void             hdy_stackable_box_map (HdyStackableBox *self);
void             hdy_stackable_box_unmap (HdyStackableBox *self);
void             hdy_stackable_box_direction_changed (HdyStackableBox  *self,
                                                      GtkTextDirection  previous_direction);

G_END_DECLS
