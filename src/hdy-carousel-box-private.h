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

G_BEGIN_DECLS

#define HDY_TYPE_CAROUSEL_BOX (hdy_carousel_box_get_type())

G_DECLARE_FINAL_TYPE (HdyCarouselBox, hdy_carousel_box, HDY, CAROUSEL_BOX, GtkContainer)

GtkWidget      *hdy_carousel_box_new (void);

void            hdy_carousel_box_insert (HdyCarouselBox *self,
                                         GtkWidget      *widget,
                                         gint            position);
void            hdy_carousel_box_reorder (HdyCarouselBox *self,
                                          GtkWidget      *widget,
                                          gint            position);

gboolean        hdy_carousel_box_is_animating (HdyCarouselBox *self);
void            hdy_carousel_box_stop_animation (HdyCarouselBox *self);

void            hdy_carousel_box_scroll_to (HdyCarouselBox *self,
                                            GtkWidget      *widget,
                                            gint64          duration);

guint           hdy_carousel_box_get_n_pages (HdyCarouselBox *self);
gdouble         hdy_carousel_box_get_distance (HdyCarouselBox *self);

gdouble         hdy_carousel_box_get_position (HdyCarouselBox *self);
void            hdy_carousel_box_set_position (HdyCarouselBox *self,
                                               gdouble         position);

guint           hdy_carousel_box_get_spacing (HdyCarouselBox *self);
void            hdy_carousel_box_set_spacing (HdyCarouselBox *self,
                                              guint           spacing);

guint           hdy_carousel_box_get_reveal_duration (HdyCarouselBox *self);
void            hdy_carousel_box_set_reveal_duration (HdyCarouselBox *self,
                                                      guint           reveal_duration);

GtkWidget      *hdy_carousel_box_get_nth_child (HdyCarouselBox *self,
                                                guint           n);

gdouble        *hdy_carousel_box_get_snap_points        (HdyCarouselBox *self,
                                                         gint           *n_snap_points);
void            hdy_carousel_box_get_range              (HdyCarouselBox *self,
                                                         gdouble        *lower,
                                                         gdouble        *upper);
gdouble         hdy_carousel_box_get_closest_snap_point (HdyCarouselBox *self);
GtkWidget      *hdy_carousel_box_get_page_at_position   (HdyCarouselBox *self,
                                                         gdouble         position);
gint            hdy_carousel_box_get_current_page_index (HdyCarouselBox *self);

G_END_DECLS
