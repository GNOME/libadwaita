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

HdyCarouselBox *hdy_carousel_box_new (void);

void            hdy_carousel_box_insert (HdyCarouselBox *self,
                                         GtkWidget      *child,
                                         gint            position);
void            hdy_carousel_box_reorder (HdyCarouselBox *self,
                                          GtkWidget      *child,
                                          gint            position);

void            hdy_carousel_box_animate (HdyCarouselBox *self,
                                          gdouble         position,
                                          gint64          duration);
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

GtkWidget      *hdy_carousel_box_get_nth_child (HdyCarouselBox *self,
                                                guint           n);

G_END_DECLS
