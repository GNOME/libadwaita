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
#include "hdy-enums.h"
#include "hdy-navigation-direction.h"

G_BEGIN_DECLS

#define HDY_TYPE_LEAFLET (hdy_leaflet_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyLeaflet, hdy_leaflet, HDY, LEAFLET, GtkContainer)

typedef enum {
  HDY_LEAFLET_TRANSITION_TYPE_NONE,
  HDY_LEAFLET_TRANSITION_TYPE_SLIDE,
  HDY_LEAFLET_TRANSITION_TYPE_OVER,
  HDY_LEAFLET_TRANSITION_TYPE_UNDER,
} HdyLeafletTransitionType;

/**
 * HdyLeafletClass
 * @parent_class: The parent class
 */
struct _HdyLeafletClass
{
  GtkContainerClass parent_class;
};

GtkWidget       *hdy_leaflet_new (void);
gboolean         hdy_leaflet_get_folded (HdyLeaflet *self);
GtkWidget       *hdy_leaflet_get_visible_child (HdyLeaflet *self);
void             hdy_leaflet_set_visible_child (HdyLeaflet *self,
                                                GtkWidget  *visible_child);
const gchar     *hdy_leaflet_get_visible_child_name (HdyLeaflet *self);
void             hdy_leaflet_set_visible_child_name (HdyLeaflet  *self,
                                                     const gchar *name);
gboolean         hdy_leaflet_get_homogeneous (HdyLeaflet     *self,
                                              gboolean        folded,
                                              GtkOrientation  orientation);
void             hdy_leaflet_set_homogeneous (HdyLeaflet     *self,
                                              gboolean        folded,
                                              GtkOrientation  orientation,
                                              gboolean        homogeneous);
HdyLeafletTransitionType hdy_leaflet_get_transition_type (HdyLeaflet *self);
void             hdy_leaflet_set_transition_type (HdyLeaflet               *self,
                                                  HdyLeafletTransitionType  transition);

guint            hdy_leaflet_get_mode_transition_duration (HdyLeaflet *self);
void             hdy_leaflet_set_mode_transition_duration (HdyLeaflet *self,
                                                           guint       duration);

guint            hdy_leaflet_get_child_transition_duration (HdyLeaflet *self);
void             hdy_leaflet_set_child_transition_duration (HdyLeaflet *self,
                                                            guint       duration);
gboolean         hdy_leaflet_get_child_transition_running (HdyLeaflet *self);
gboolean         hdy_leaflet_get_interpolate_size (HdyLeaflet *self);
void             hdy_leaflet_set_interpolate_size (HdyLeaflet *self,
                                                   gboolean    interpolate_size);
gboolean         hdy_leaflet_get_can_swipe_back (HdyLeaflet *self);
void             hdy_leaflet_set_can_swipe_back (HdyLeaflet *self,
                                                 gboolean    can_swipe_back);
gboolean         hdy_leaflet_get_can_swipe_forward (HdyLeaflet *self);
void             hdy_leaflet_set_can_swipe_forward (HdyLeaflet *self,
                                                    gboolean    can_swipe_forward);

gboolean         hdy_leaflet_navigate (HdyLeaflet             *self,
                                       HdyNavigationDirection  direction);

G_END_DECLS
