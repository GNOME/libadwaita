/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HDY_LEAFLET_H
#define HDY_LEAFLET_H

#if !defined(HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_LEAFLET_MODE_TRANSITION_TYPE (hdy_leaflet_mode_transition_type_get_type ())

#define HDY_TYPE_LEAFLET_CHILD_TRANSITION_TYPE (hdy_leaflet_child_transition_type_get_type ())

#define HDY_TYPE_LEAFLET (hdy_leaflet_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyLeaflet, hdy_leaflet, HDY, LEAFLET, GtkContainer)

typedef enum {
  HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE,
  HDY_LEAFLET_MODE_TRANSITION_TYPE_SLIDE,
} HdyLeafletModeTransitionType;

typedef enum {
  HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE,
  HDY_LEAFLET_CHILD_TRANSITION_TYPE_CROSSFADE,
  HDY_LEAFLET_CHILD_TRANSITION_TYPE_SLIDE,
  HDY_LEAFLET_CHILD_TRANSITION_TYPE_OVER,
} HdyLeafletChildTransitionType;

/**
 * HdyLeafletClass
 * @parent_class: The parent class
 * @todo: Class handler for the #HdyLeaflet::todos signal
 */
struct _HdyLeafletClass
{
  GtkContainerClass parent;

  /* Signals
   */
  void (*todo) (HdyLeaflet *self);
};

GType hdy_leaflet_mode_transition_type_get_type (void);
GType hdy_leaflet_child_transition_type_get_type (void);

GtkWidget       *hdy_leaflet_new (void);
gboolean         hdy_leaflet_get_folded (HdyLeaflet *self);
GtkWidget       *hdy_leaflet_get_visible_child (HdyLeaflet *self);
void             hdy_leaflet_set_visible_child (HdyLeaflet *self,
                                                GtkWidget  *visible_child);
const gchar     *hdy_leaflet_get_visible_child_name (HdyLeaflet *self);
void             hdy_leaflet_set_visible_child_name (HdyLeaflet  *self,
                                                     const gchar *name);
gboolean         hdy_leaflet_get_homogeneous_folded (HdyLeaflet *self);
void             hdy_leaflet_set_homogeneous_folded (HdyLeaflet *self,
                                                     gboolean    homogeneous);
gboolean         hdy_leaflet_get_hhomogeneous_folded (HdyLeaflet *self);
void             hdy_leaflet_set_hhomogeneous_folded (HdyLeaflet *self,
                                                      gboolean    hhomogeneous);
gboolean         hdy_leaflet_get_vhomogeneous_folded (HdyLeaflet *self);
void             hdy_leaflet_set_vhomogeneous_folded (HdyLeaflet *self,
                                                      gboolean    vhomogeneous);
gboolean         hdy_leaflet_get_homogeneous_unfolded (HdyLeaflet *self);
void             hdy_leaflet_set_homogeneous_unfolded (HdyLeaflet *self,
                                                       gboolean    homogeneous);
gboolean         hdy_leaflet_get_hhomogeneous_unfolded (HdyLeaflet *self);
void             hdy_leaflet_set_hhomogeneous_unfolded (HdyLeaflet *self,
                                                        gboolean    hhomogeneous);
gboolean         hdy_leaflet_get_vhomogeneous_unfolded (HdyLeaflet *self);
void             hdy_leaflet_set_vhomogeneous_unfolded (HdyLeaflet *self,
                                                        gboolean    vhomogeneous);
HdyLeafletModeTransitionType hdy_leaflet_get_mode_transition_type (HdyLeaflet *self);
void             hdy_leaflet_set_mode_transition_type (HdyLeaflet                   *self,
                                                       HdyLeafletModeTransitionType  transition);
guint            hdy_leaflet_get_mode_transition_duration (HdyLeaflet *self);
void             hdy_leaflet_set_mode_transition_duration (HdyLeaflet *self,
                                                           guint       duration);
HdyLeafletChildTransitionType hdy_leaflet_get_child_transition_type (HdyLeaflet *self);
void             hdy_leaflet_set_child_transition_type (HdyLeaflet                    *self,
                                                        HdyLeafletChildTransitionType  transition);
guint            hdy_leaflet_get_child_transition_duration (HdyLeaflet *self);
void             hdy_leaflet_set_child_transition_duration (HdyLeaflet *self,
                                                            guint       duration);
gboolean         hdy_leaflet_get_child_transition_running (HdyLeaflet *self);
gboolean         hdy_leaflet_get_interpolate_size (HdyLeaflet *self);
void             hdy_leaflet_set_interpolate_size (HdyLeaflet *self,
                                                   gboolean    interpolate_size);

G_END_DECLS

#endif /* HDY_LEAFLET_H */
