/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "hdy-css-private.h"

void
hdy_css_measure (GtkWidget      *widget,
                 GtkOrientation  orientation,
                 gint           *minimum,
                 gint           *natural)
{
  GtkStyleContext *style_context = gtk_widget_get_style_context (widget);
  GtkStateFlags state_flags = gtk_widget_get_state_flags (widget);
  GtkBorder border, margin, padding;
  gint css_width, css_height;

  /* Manually apply minimum sizes, the border, the padding and the margin as we
   * can't use the private GtkGagdet.
   */
  gtk_style_context_get (style_context, state_flags,
                         "min-width", &css_width,
                         "min-height", &css_height,
                         NULL);
  gtk_style_context_get_border (style_context, state_flags, &border);
  gtk_style_context_get_margin (style_context, state_flags, &margin);
  gtk_style_context_get_padding (style_context, state_flags, &padding);
  if (orientation == GTK_ORIENTATION_VERTICAL) {
    *minimum = MAX (*minimum, css_height) +
               border.top + margin.top + padding.top +
               border.bottom + margin.bottom + padding.bottom;
    *natural = MAX (*natural, css_height) +
               border.top + margin.top + padding.top +
               border.bottom + margin.bottom + padding.bottom;
  } else {
    *minimum = MAX (*minimum, css_width) +
               border.left + margin.left + padding.left +
               border.right + margin.right + padding.right;
    *natural = MAX (*natural, css_width) +
               border.left + margin.left + padding.left +
               border.right + margin.right + padding.right;
  }
}

void
hdy_css_size_allocate (GtkWidget     *widget,
                       GtkAllocation *allocation)
{
  GtkStyleContext *style_context;
  GtkStateFlags state_flags;
  GtkBorder border, margin, padding;

  /* Manually apply the border, the padding and the margin as we can't use the
   * private GtkGagdet.
   */
  style_context = gtk_widget_get_style_context (widget);
  state_flags = gtk_widget_get_state_flags (widget);
  gtk_style_context_get_border (style_context, state_flags, &border);
  gtk_style_context_get_margin (style_context, state_flags, &margin);
  gtk_style_context_get_padding (style_context, state_flags, &padding);
  allocation->width -= border.left + border.right +
                       margin.left + margin.right +
                       padding.left + padding.right;
  allocation->height -= border.top + border.bottom +
                        margin.top + margin.bottom +
                        padding.top + padding.bottom;
  allocation->x += border.left + margin.left + padding.left;
  allocation->y += border.top + margin.top + padding.top;
}
