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
#include "hdy-enums.h"

G_BEGIN_DECLS

#define HDY_TYPE_CAROUSEL (hdy_carousel_get_type())

G_DECLARE_FINAL_TYPE (HdyCarousel, hdy_carousel, HDY, CAROUSEL, GtkEventBox)

typedef enum {
  HDY_CAROUSEL_INDICATOR_STYLE_NONE,
  HDY_CAROUSEL_INDICATOR_STYLE_DOTS,
  HDY_CAROUSEL_INDICATOR_STYLE_LINES,
} HdyCarouselIndicatorStyle;

GtkWidget    *hdy_carousel_new (void);

void            hdy_carousel_prepend (HdyCarousel *self,
                                      GtkWidget   *child);
void            hdy_carousel_insert (HdyCarousel *self,
                                     GtkWidget   *child,
                                     gint         position);
void            hdy_carousel_reorder (HdyCarousel *self,
                                      GtkWidget   *child,
                                      gint         position);

void            hdy_carousel_scroll_to (HdyCarousel *self,
                                        GtkWidget   *widget);
void            hdy_carousel_scroll_to_full (HdyCarousel *self,
                                             GtkWidget   *widget,
                                             gint64       duration);

guint           hdy_carousel_get_n_pages (HdyCarousel *self);
gdouble         hdy_carousel_get_position (HdyCarousel *self);

gboolean        hdy_carousel_get_interactive (HdyCarousel *self);
void            hdy_carousel_set_interactive (HdyCarousel *self,
                                              gboolean     interactive);

HdyCarouselIndicatorStyle hdy_carousel_get_indicator_style (HdyCarousel *self);
void            hdy_carousel_set_indicator_style (HdyCarousel               *self,
                                                  HdyCarouselIndicatorStyle  style);

guint           hdy_carousel_get_indicator_spacing (HdyCarousel *self);
void            hdy_carousel_set_indicator_spacing (HdyCarousel *self,
                                                    guint        spacing);

gboolean        hdy_carousel_get_center_content (HdyCarousel *self);
void            hdy_carousel_set_center_content (HdyCarousel *self,
                                                 gboolean     center_content);

guint           hdy_carousel_get_spacing (HdyCarousel *self);
void            hdy_carousel_set_spacing (HdyCarousel *self,
                                          guint        spacing);

guint           hdy_carousel_get_animation_duration (HdyCarousel *self);
void            hdy_carousel_set_animation_duration (HdyCarousel *self,
                                                     guint        duration);

gboolean        hdy_carousel_get_allow_mouse_drag (HdyCarousel *self);
void            hdy_carousel_set_allow_mouse_drag (HdyCarousel *self,
                                                   gboolean     allow_mouse_drag);

G_END_DECLS
