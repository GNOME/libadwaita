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
  gint css_width, css_height, min = 0, nat = 0;

  if (minimum)
    min = *minimum;

  if (natural)
    nat = *natural;

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
    min = MAX (min, css_height) +
          border.top + margin.top + padding.top +
          border.bottom + margin.bottom + padding.bottom;
    nat = MAX (nat, css_height) +
          border.top + margin.top + padding.top +
          border.bottom + margin.bottom + padding.bottom;
  } else {
    min = MAX (min, css_width) +
          border.left + margin.left + padding.left +
          border.right + margin.right + padding.right;
    nat = MAX (nat, css_width) +
          border.left + margin.left + padding.left +
          border.right + margin.right + padding.right;
  }

  if (minimum)
    *minimum = MAX (min, 0);

  if (natural)
    *natural = MAX (nat, 0);
}

void
hdy_css_size_allocate (GtkWidget     *widget,
                       GtkAllocation *allocation)
{
  hdy_css_size_allocate_self (widget, allocation);
  hdy_css_size_allocate_children (widget, allocation);
}

void
hdy_css_size_allocate_self (GtkWidget     *widget,
                            GtkAllocation *allocation)
{
  GtkStyleContext *style_context;
  GtkStateFlags state_flags;
  GtkBorder margin;

  /* Manually apply the border, the padding and the margin as we can't use the
   * private GtkGagdet.
   */
  style_context = gtk_widget_get_style_context (widget);
  state_flags = gtk_widget_get_state_flags (widget);

  gtk_style_context_get_margin (style_context, state_flags, &margin);

  allocation->width -= margin.left + margin.right;
  allocation->height -= margin.top + margin.bottom;
  allocation->x += margin.left;
  allocation->y += margin.top;
}

void
hdy_css_size_allocate_children (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  GtkStyleContext *style_context;
  GtkStateFlags state_flags;
  GtkBorder border, padding;

  /* Manually apply the border, the padding and the margin as we can't use the
   * private GtkGagdet.
   */
  style_context = gtk_widget_get_style_context (widget);
  state_flags = gtk_widget_get_state_flags (widget);

  gtk_style_context_get_border (style_context, state_flags, &border);
  gtk_style_context_get_padding (style_context, state_flags, &padding);

  allocation->width -= border.left + border.right +
                       padding.left + padding.right;
  allocation->height -= border.top + border.bottom +
                        padding.top + padding.bottom;
  allocation->x += border.left + padding.left;
  allocation->y += border.top + padding.top;
}

void
hdy_css_draw (GtkWidget *widget,
              cairo_t   *cr)
{
  gint width = gtk_widget_get_allocated_width (widget);
  gint height = gtk_widget_get_allocated_height (widget);
  GtkStyleContext *style_context;

  if (width <= 0 || height <= 0)
    return;

  /* Manually apply the border, the padding and the margin as we can't use the
   * private GtkGagdet.
   */
  style_context = gtk_widget_get_style_context (widget);

  gtk_render_background (style_context, cr, 0, 0, width, height);
  gtk_render_frame (style_context, cr, 0, 0, width, height);

  if (gtk_widget_has_visible_focus (widget)) {
    GtkStateFlags state_flags;
    GtkBorder border;

    state_flags = gtk_widget_get_state_flags (widget);

    gtk_style_context_get_border (style_context, state_flags, &border);

    gtk_render_focus (style_context, cr,
                      border.left, border.top,
                      width - border.left - border.right,
                      height - border.top - border.bottom);
  }
}
