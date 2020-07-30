/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-enums.h"
#include "hdy-navigation-direction.h"

G_BEGIN_DECLS

#define HDY_TYPE_LEAFLET (hdy_leaflet_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyLeaflet, hdy_leaflet, HDY, LEAFLET, GtkContainer)

typedef enum {
  HDY_LEAFLET_TRANSITION_TYPE_OVER,
  HDY_LEAFLET_TRANSITION_TYPE_UNDER,
  HDY_LEAFLET_TRANSITION_TYPE_SLIDE,
} HdyLeafletTransitionType;

/**
 * HdyLeafletClass
 * @parent_class: The parent class
 */
struct _HdyLeafletClass
{
  GtkContainerClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_leaflet_new (void);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_get_folded (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_leaflet_get_visible_child (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_visible_child (HdyLeaflet *self,
                                                GtkWidget  *visible_child);
HDY_AVAILABLE_IN_ALL
const gchar     *hdy_leaflet_get_visible_child_name (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_visible_child_name (HdyLeaflet  *self,
                                                     const gchar *name);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_get_homogeneous (HdyLeaflet     *self,
                                              gboolean        folded,
                                              GtkOrientation  orientation);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_homogeneous (HdyLeaflet     *self,
                                              gboolean        folded,
                                              GtkOrientation  orientation,
                                              gboolean        homogeneous);
HDY_AVAILABLE_IN_ALL
HdyLeafletTransitionType hdy_leaflet_get_transition_type (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_transition_type (HdyLeaflet               *self,
                                                  HdyLeafletTransitionType  transition);

HDY_AVAILABLE_IN_ALL
guint            hdy_leaflet_get_mode_transition_duration (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_mode_transition_duration (HdyLeaflet *self,
                                                           guint       duration);

HDY_AVAILABLE_IN_ALL
guint            hdy_leaflet_get_child_transition_duration (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_child_transition_duration (HdyLeaflet *self,
                                                            guint       duration);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_get_child_transition_running (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_get_interpolate_size (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_interpolate_size (HdyLeaflet *self,
                                                   gboolean    interpolate_size);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_get_can_swipe_back (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_can_swipe_back (HdyLeaflet *self,
                                                 gboolean    can_swipe_back);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_get_can_swipe_forward (HdyLeaflet *self);
HDY_AVAILABLE_IN_ALL
void             hdy_leaflet_set_can_swipe_forward (HdyLeaflet *self,
                                                    gboolean    can_swipe_forward);

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_leaflet_get_adjacent_child (HdyLeaflet             *self,
                                                 HdyNavigationDirection  direction);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_leaflet_navigate (HdyLeaflet             *self,
                                       HdyNavigationDirection  direction);

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_leaflet_get_child_by_name (HdyLeaflet  *self,
                                                const gchar *name);

G_END_DECLS
