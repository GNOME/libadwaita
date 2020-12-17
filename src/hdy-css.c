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

void
hdy_css_get_preferred_width (GtkWidget *widget,
                             gint      *minimum,
                             gint      *natural)
{
  GObjectClass *pclass = g_type_class_peek_parent (G_OBJECT_GET_CLASS (widget));

  g_assert (G_OBJECT_CLASS_TYPE (pclass) == GTK_TYPE_BIN);

  GTK_WIDGET_CLASS (pclass)->get_preferred_width (widget, minimum, natural);

  hdy_css_measure (widget, GTK_ORIENTATION_HORIZONTAL, minimum, natural);
}

void
hdy_css_get_preferred_width_for_height (GtkWidget *widget,
                                        gint       height,
                                        gint      *minimum,
                                        gint      *natural)
{
  GObjectClass *pclass = g_type_class_peek_parent (G_OBJECT_GET_CLASS (widget));

  g_assert (G_OBJECT_CLASS_TYPE (pclass) == GTK_TYPE_BIN);

  GTK_WIDGET_CLASS (pclass)->get_preferred_width_for_height (widget, height, minimum, natural);

  hdy_css_measure (widget, GTK_ORIENTATION_HORIZONTAL, minimum, natural);
}

void
hdy_css_get_preferred_height (GtkWidget *widget,
                              gint      *minimum,
                              gint      *natural)
{
  GObjectClass *pclass = g_type_class_peek_parent (G_OBJECT_GET_CLASS (widget));

  g_assert (G_OBJECT_CLASS_TYPE (pclass) == GTK_TYPE_BIN);

  GTK_WIDGET_CLASS (pclass)->get_preferred_height (widget, minimum, natural);

  hdy_css_measure (widget, GTK_ORIENTATION_VERTICAL, minimum, natural);
}

void
hdy_css_get_preferred_height_for_width (GtkWidget *widget,
                                        gint       width,
                                        gint      *minimum,
                                        gint      *natural)
{
  GObjectClass *pclass = g_type_class_peek_parent (G_OBJECT_GET_CLASS (widget));

  g_assert (G_OBJECT_CLASS_TYPE (pclass) == GTK_TYPE_BIN);

  GTK_WIDGET_CLASS (pclass)->get_preferred_height_for_width (widget, width, minimum, natural);

  hdy_css_measure (widget, GTK_ORIENTATION_VERTICAL, minimum, natural);
}

void
hdy_css_size_allocate_bin (GtkWidget     *widget,
                           GtkAllocation *allocation)
{
  GObjectClass *pclass = g_type_class_peek_parent (G_OBJECT_GET_CLASS (widget));
  GtkAllocation child_alloc;

  g_assert (G_OBJECT_CLASS_TYPE (pclass) == GTK_TYPE_BIN);

  hdy_css_size_allocate_self (widget, allocation);
  gtk_widget_set_allocation (widget, allocation);

  child_alloc = *allocation;
  hdy_css_size_allocate_children (widget, &child_alloc);
  gtk_widget_size_allocate (gtk_bin_get_child (GTK_BIN (widget)), &child_alloc);
}

gboolean
hdy_css_draw_bin (GtkWidget *widget,
                  cairo_t   *cr)
{
  GObjectClass *pclass = g_type_class_peek_parent (G_OBJECT_GET_CLASS (widget));

  g_assert (G_OBJECT_CLASS_TYPE (pclass) == GTK_TYPE_BIN);

  hdy_css_draw (widget, cr);

  return GTK_WIDGET_CLASS (pclass)->draw (widget, cr);
}
