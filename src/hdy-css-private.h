/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

void hdy_css_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      gint           *minimum,
                      gint           *natural);

void hdy_css_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation);

void hdy_css_size_allocate_self (GtkWidget     *widget,
                                 GtkAllocation *allocation);

void hdy_css_size_allocate_children (GtkWidget     *widget,
                                     GtkAllocation *allocation);

void hdy_css_draw (GtkWidget *widget,
                   cairo_t   *cr);

void hdy_css_get_preferred_width (GtkWidget *widget,
                                  gint      *minimum,
                                  gint      *natural);

void hdy_css_get_preferred_width_for_height (GtkWidget *widget,
                                             gint       height,
                                             gint      *minimum,
                                             gint      *natural);

void hdy_css_get_preferred_height (GtkWidget *widget,
                                   gint      *minimum,
                                   gint      *natural);

void hdy_css_get_preferred_height_for_width (GtkWidget *widget,
                                             gint       width,
                                             gint      *minimum,
                                             gint      *natural);

void hdy_css_size_allocate_bin (GtkWidget     *widget,
                                GtkAllocation *allocation);

gboolean hdy_css_draw_bin (GtkWidget *widget,
                           cairo_t   *cr);

G_END_DECLS
