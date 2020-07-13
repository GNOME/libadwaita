/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-carousel.h"

G_BEGIN_DECLS

#define HDY_TYPE_CAROUSEL_INDICATOR_DOTS (hdy_carousel_indicator_dots_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyCarouselIndicatorDots, hdy_carousel_indicator_dots, HDY, CAROUSEL_INDICATOR_DOTS, GtkDrawingArea)

HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_carousel_indicator_dots_new (void);

HDY_AVAILABLE_IN_ALL
HdyCarousel *hdy_carousel_indicator_dots_get_carousel (HdyCarouselIndicatorDots *self);
HDY_AVAILABLE_IN_ALL
void         hdy_carousel_indicator_dots_set_carousel (HdyCarouselIndicatorDots *self,
                                                       HdyCarousel              *carousel);

G_END_DECLS
