/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_CAROUSEL_BOX (adw_carousel_box_get_type())

G_DECLARE_FINAL_TYPE (AdwCarouselBox, adw_carousel_box, ADW, CAROUSEL_BOX, GtkWidget)

GtkWidget      *adw_carousel_box_new (void);

void            adw_carousel_box_insert (AdwCarouselBox *self,
                                         GtkWidget      *widget,
                                         int             position);
void            adw_carousel_box_reorder (AdwCarouselBox *self,
                                          GtkWidget      *widget,
                                          int             position);
void            adw_carousel_box_remove (AdwCarouselBox *self,
                                         GtkWidget      *widget);

gboolean        adw_carousel_box_is_animating (AdwCarouselBox *self);
void            adw_carousel_box_stop_animation (AdwCarouselBox *self);

void            adw_carousel_box_scroll_to (AdwCarouselBox *self,
                                            GtkWidget      *widget,
                                            gint64          duration);

guint           adw_carousel_box_get_n_pages (AdwCarouselBox *self);
gdouble         adw_carousel_box_get_distance (AdwCarouselBox *self);

gdouble         adw_carousel_box_get_position (AdwCarouselBox *self);
void            adw_carousel_box_set_position (AdwCarouselBox *self,
                                               gdouble         position);

guint           adw_carousel_box_get_spacing (AdwCarouselBox *self);
void            adw_carousel_box_set_spacing (AdwCarouselBox *self,
                                              guint           spacing);

guint           adw_carousel_box_get_reveal_duration (AdwCarouselBox *self);
void            adw_carousel_box_set_reveal_duration (AdwCarouselBox *self,
                                                      guint           reveal_duration);

GtkWidget      *adw_carousel_box_get_nth_child (AdwCarouselBox *self,
                                                guint           n);

gdouble        *adw_carousel_box_get_snap_points        (AdwCarouselBox *self,
                                                         int            *n_snap_points);
void            adw_carousel_box_get_range              (AdwCarouselBox *self,
                                                         gdouble        *lower,
                                                         gdouble        *upper);
gdouble         adw_carousel_box_get_closest_snap_point (AdwCarouselBox *self);
GtkWidget      *adw_carousel_box_get_page_at_position   (AdwCarouselBox *self,
                                                         gdouble         position);
int             adw_carousel_box_get_current_page_index (AdwCarouselBox *self);
int             adw_carousel_box_get_page_index         (AdwCarouselBox *self,
                                                         GtkWidget      *child);

G_END_DECLS
