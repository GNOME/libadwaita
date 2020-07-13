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

#define HDY_TYPE_CAROUSEL_INDICATOR_LINES (hdy_carousel_indicator_lines_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyCarouselIndicatorLines, hdy_carousel_indicator_lines, HDY, CAROUSEL_INDICATOR_LINES, GtkDrawingArea)

HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_carousel_indicator_lines_new (void);

HDY_AVAILABLE_IN_ALL
HdyCarousel *hdy_carousel_indicator_lines_get_carousel (HdyCarouselIndicatorLines *self);
HDY_AVAILABLE_IN_ALL
void         hdy_carousel_indicator_lines_set_carousel (HdyCarouselIndicatorLines *self,
                                                        HdyCarousel               *carousel);

G_END_DECLS
