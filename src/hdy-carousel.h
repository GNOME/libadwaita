/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_CAROUSEL (hdy_carousel_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyCarousel, hdy_carousel, HDY, CAROUSEL, GtkEventBox)

HDY_AVAILABLE_IN_ALL
GtkWidget      *hdy_carousel_new (void);

HDY_AVAILABLE_IN_ALL
void            hdy_carousel_prepend (HdyCarousel *self,
                                      GtkWidget   *child);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_insert (HdyCarousel *self,
                                     GtkWidget   *child,
                                     gint         position);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_reorder (HdyCarousel *self,
                                      GtkWidget   *child,
                                      gint         position);

HDY_AVAILABLE_IN_ALL
void            hdy_carousel_scroll_to (HdyCarousel *self,
                                        GtkWidget   *widget);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_scroll_to_full (HdyCarousel *self,
                                             GtkWidget   *widget,
                                             gint64       duration);

HDY_AVAILABLE_IN_ALL
guint           hdy_carousel_get_n_pages (HdyCarousel *self);
HDY_AVAILABLE_IN_ALL
gdouble         hdy_carousel_get_position (HdyCarousel *self);

HDY_AVAILABLE_IN_ALL
gboolean        hdy_carousel_get_interactive (HdyCarousel *self);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_set_interactive (HdyCarousel *self,
                                              gboolean     interactive);

HDY_AVAILABLE_IN_ALL
guint           hdy_carousel_get_spacing (HdyCarousel *self);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_set_spacing (HdyCarousel *self,
                                          guint        spacing);

HDY_AVAILABLE_IN_ALL
guint           hdy_carousel_get_animation_duration (HdyCarousel *self);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_set_animation_duration (HdyCarousel *self,
                                                     guint        duration);

HDY_AVAILABLE_IN_ALL
gboolean        hdy_carousel_get_allow_mouse_drag (HdyCarousel *self);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_set_allow_mouse_drag (HdyCarousel *self,
                                                   gboolean     allow_mouse_drag);

HDY_AVAILABLE_IN_ALL
guint           hdy_carousel_get_reveal_duration (HdyCarousel *self);
HDY_AVAILABLE_IN_ALL
void            hdy_carousel_set_reveal_duration (HdyCarousel *self,
                                                  guint        reveal_duration);
G_END_DECLS
