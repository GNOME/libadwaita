/*
 * Copyright (C) 2021 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-enums.h"
#include "adw-wrap-layout.h"

#include <math.h>

/**
 * AdwWrapLayout:
 *
 * A box-like layout that can wrap into multiple lines.
 *
 * <picture>
 *   <source srcset="wrap-box-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="wrap-box.png" alt="wrap-box">
 * </picture>
 *
 * `AdwWrapLayout` is similar to [class@Gtk.BoxLayout], but can wrap lines when
 * the widgets cannot fit otherwise. Unlike [class@Gtk.FlowBox], the children
 * aren't arranged into a grid and behave more like words in a wrapping label.
 *
 * Like `GtkBoxLayout`, `AdwWrapLayout` is orientable and has spacing:
 *
 * - [property@WrapLayout:child-spacing] between children in the same line;
 * - [property@WrapLayout:line-spacing] between lines.
 *
 * ::: note
 *     Unlike `GtkBoxLayout`, `AdwWrapLayout` cannot follow the CSS
 *     `border-spacing` property.
 *
 * Use the [property@WrapLayout:natural-line-length] property to determine the
 * layout's natural size, e.g. when using it in a [class@Gtk.Popover].
 *
 * Normally, a horizontal `AdwWrapLayout` wraps left to right and top to bottom
 * for left-to-right languages. Both of these directions can be reversed, using
 * the [property@WrapLayout:pack-direction] and
 * [property@WrapLayout:wrap-reverse] properties. Additionally, the alignment
 * of each line can be controlled with the [property@WrapLayout:align] property.
 *
 * Lines can be justified using the [property@WrapLayout:justify] property,
 * filling the entire line by either increasing child size or spacing depending
 * on the value. Set [property@WrapLayout:justify-last-line] to justify the last
 * line as well.
 *
 * By default, `AdwWrapLayout` wraps as soon as the previous line cannot fit
 * any more children without shrinking them past their natural size. Set
 * [property@WrapLayout:wrap-policy] to [enum@Adw.WrapPolicy.MINIMUM] to only
 * wrap once all the children in the previous line have been shrunk to their
 * minimum size.
 *
 * To make each line take the same amount of space, set
 * [property@WrapLayout:line-homogeneous] to `TRUE`.
 *
 * Spacing and natural line length can scale with the text scale factor, use the
 * [property@WrapLayout:child-spacing-unit],
 * [property@WrapLayout:line-spacing-unit] and/or
 * [property@WrapLayout:natural-line-length-unit] properties to enable that
 * behavior.
 *
 * See [class@WrapBox].
 *
 * Since: 1.7
 */

/**
 * AdwJustifyMode:
 * @ADW_JUSTIFY_NONE: Don't justify children within a line.
 * @ADW_JUSTIFY_FILL: Stretch each child within the line, keeping consistent
 *     spacing, so that the line fills the entire length.
 * @ADW_JUSTIFY_SPREAD: Increase spacing between children, moving the children
 *     so that the first and last child are aligned with the beginning and end
 *     of the line. If the line only contains a single widget, it will be
 *     stretched regardless.
 *
 * Describes line justify behaviors in a [class@WrapLayout] or [class@WrapBox].
 *
 * See [property@WrapLayout:justify] and [property@WrapBox:justify].
 *
 * Since: 1.7
 */

/**
 * AdwPackDirection:
 * @ADW_PACK_START_TO_END: Pack children from left to right for LTR languages,
 *     or top to bottom vertically.
 * @ADW_PACK_END_TO_START: Pack children from right to left for LTR languages,
 *     or bottom to top vertically.
 *
 * Describes child packing behavior in a [class@WrapLayout] or [class@WrapBox].
 *
 * See [property@WrapLayout:pack-direction] and
 * [property@WrapBox:pack-direction].
 *
 * Since: 1.7
 */

/**
 * AdwWrapPolicy:
 * @ADW_WRAP_NATURAL: Wrap to the next line as soon as the previous line cannot
 *     fit any more children without shrinking them past their natural size.
 * @ADW_WRAP_MINIMUM: Fit as many children into each line as possible, shrinking
 *     them down to their minimum size before wrapping to the next line.
 *
 * Describes line wrapping behavior in a [class@WrapLayout] or [class@WrapBox].
 *
 * See [property@WrapLayout:wrap-policy] and [property@WrapBox:wrap-policy].
 *
 * Since: 1.7
 */

struct _AdwWrapLayout
{
  GtkLayoutManager parent_instance;

  int child_spacing;
  AdwLengthUnit child_spacing_unit;
  AdwPackDirection pack_direction;
  float align;
  AdwJustifyMode justify;
  gboolean justify_last_line;

  int line_spacing;
  AdwLengthUnit line_spacing_unit;
  gboolean line_homogeneous;
  int natural_line_length;
  AdwLengthUnit natural_line_length_unit;
  gboolean wrap_reverse;
  AdwWrapPolicy wrap_policy;

  GtkOrientation orientation;
};

enum {
  PROP_0,
  PROP_CHILD_SPACING,
  PROP_CHILD_SPACING_UNIT,
  PROP_PACK_DIRECTION,
  PROP_ALIGN,
  PROP_JUSTIFY,
  PROP_JUSTIFY_LAST_LINE,
  PROP_LINE_SPACING,
  PROP_LINE_SPACING_UNIT,
  PROP_LINE_HOMOGENEOUS,
  PROP_NATURAL_LINE_LENGTH,
  PROP_NATURAL_LINE_LENGTH_UNIT,
  PROP_WRAP_REVERSE,
  PROP_WRAP_POLICY,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_WRAP_POLICY + 1,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE (AdwWrapLayout, adw_wrap_layout, GTK_TYPE_LAYOUT_MANAGER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static void
set_orientation (AdwWrapLayout  *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify (G_OBJECT (self), "orientation");
}

typedef struct _AllocationData AllocationData;

struct _AllocationData {
  // Provided values
  int minimum_size;
  int natural_size;
  gboolean expand;

  // Computed values
  int available_size;
  int allocated_size;

  // Context, a widget for children and a line data for lines
  union {
    GtkWidget *widget;
    struct {
      AllocationData *children;
      int n_children;
    } line;
  } data;
};

static int
count_line_children (AdwWrapLayout  *self,
                     int             for_size,
                     int             spacing,
                     AllocationData *child_data,
                     int             n_children)
{
  int remaining_space = for_size + spacing;
  int n_line_children = 0;

  if (for_size < 0)
    return n_children;

  /* Count how many widgets can fit into this line */
  while (TRUE) {
    int delta;

    if (n_line_children >= n_children)
      break;

    switch (self->wrap_policy) {
    case ADW_WRAP_MINIMUM:
      delta = child_data[n_line_children].minimum_size;
      break;
    case ADW_WRAP_NATURAL:
      delta = child_data[n_line_children].natural_size;
      break;
    default:
      g_assert_not_reached ();
    }

    delta += spacing;

    if (remaining_space < delta)
      break;

    remaining_space -= delta;
    n_line_children++;
  }

  return n_line_children;
}

static int
count_lines (AdwWrapLayout  *self,
             int             for_size,
             int             child_spacing,
             AllocationData *child_data,
             int             n_children)
{
  int n_lines = 0;

  while (n_children > 0) {
    int n_line_children = count_line_children (self, for_size, child_spacing,
                                               child_data, n_children);

    if (n_line_children == 0)
      n_line_children++;

    n_children -= n_line_children;
    child_data = &child_data[n_line_children];
    n_lines++;
  }

  return n_lines;
}

/* child_data may represent both lines within the box and children within the
 * line, the function is used in both orientations */
static void
box_allocate (AllocationData *child_data,
              int             n_children,
              int             for_size,
              int             spacing,
              AdwJustifyMode  justify)
{
  GtkRequestedSize *sizes = g_new (GtkRequestedSize, n_children);
  int extra_space;
  int n_expand = 0;
  int size_given_to_child = 0;
  int n_extra_widgets = 0;
  int children_minimum_size = 0;
  int i;

  g_assert (for_size >= 0);

  for (i = 0; i < n_children; i++) {
    if (child_data[i].expand)
      n_expand++;

    children_minimum_size += child_data[i].minimum_size;
  }

  extra_space = for_size - (n_children - 1) * spacing;
  g_assert (extra_space >= 0);

  for (i = 0; i < n_children; i++) {
    sizes[i].minimum_size = child_data[i].minimum_size;
    sizes[i].natural_size = child_data[i].natural_size;
  }

  /* Bring children up to size first */
  extra_space -= children_minimum_size;
  extra_space = MAX (0, extra_space);
  extra_space = gtk_distribute_natural_allocation (extra_space, n_children, sizes);

  /* Calculate space which hasn't been distributed yet,
   * and is available for expanding children.
   */
  if (n_expand > 0) {
    size_given_to_child = extra_space / n_expand;
    n_extra_widgets = extra_space % n_expand;
  } else if (justify != ADW_JUSTIFY_NONE) {
    size_given_to_child = extra_space / n_children;
    n_extra_widgets = extra_space % n_children;
  }

  /* Allocate sizes */
  for (i = 0; i < n_children; i++) {
    int available_size = sizes[i].minimum_size;

    if (child_data[i].expand ||
        (n_expand == 0 && justify != ADW_JUSTIFY_NONE)) {
      available_size += size_given_to_child;

      if (n_extra_widgets > 0) {
        available_size++;
        n_extra_widgets--;
      }
    }

    child_data[i].available_size = available_size;

    if (n_expand == 0 && n_children > 1 && justify == ADW_JUSTIFY_SPREAD)
      child_data[i].allocated_size = sizes[i].minimum_size;
    else
      child_data[i].allocated_size = available_size;
  }

  g_free (sizes);
}

static void
box_allocate_homogeneous (AllocationData *child_data,
                          int             n_children,
                          int             for_size,
                          int             spacing)
{
  int child_size, i;

  if (n_children == 0)
    return;

  g_assert (for_size >= 0);

  for_size -= (n_children - 1) * spacing;

  child_size = for_size / n_children;

  for (i = 0; i < n_children; i++)
    child_data[i].available_size = child_data[i].allocated_size = child_size;
}

static int
compute_line (AdwWrapLayout  *self,
              int             for_size,
              int             spacing,
              AllocationData *child_data,
              int             n_children,
              gboolean        last_line)
{
  AdwJustifyMode justify;
  int n_line_children;

  g_assert (n_children > 0);

  /* Count how many widgets can fit into this line */
  n_line_children = count_line_children (self, for_size, spacing, child_data, n_children);

  if (for_size < 0)
    return n_line_children;

  /* Even one widget doesn't fit. Since we can't have a line with 0 widgets,
   * we take the first one and allocate it out of bounds. This can happen
   * when wrap policy is ADW_WRAP_NATURAL, but we're being allocated less
   * than the child's natural size. */
  if (n_line_children == 0) {
    child_data[0].allocated_size = MAX (for_size, child_data[0].minimum_size);

    return 1;
  }

  if (last_line && !self->justify_last_line)
    justify = ADW_JUSTIFY_NONE;
  else
    justify = self->justify;

  /* All widgets fit, we can calculate their exact sizes within the line. */
  box_allocate (child_data, n_line_children, for_size, spacing, justify);

  return n_line_children;
}

static AllocationData *
compute_sizes (AdwWrapLayout   *self,
               GtkWidget       *widget,
               int              for_size,
               int              child_spacing,
               int             *n_lines,
               AllocationData **out_child_data)
{
  AllocationData *child_data, *line_data, *line_start;
  GtkWidget *child;
  int n_visible_children = 0;
  int i = 0, j;
  GtkOrientation opposite_orientation;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    opposite_orientation = GTK_ORIENTATION_VERTICAL;
  else
    opposite_orientation = GTK_ORIENTATION_HORIZONTAL;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    if (!gtk_widget_should_layout (child))
      continue;

    n_visible_children++;
  }

  child_data = g_new0 (AllocationData, n_visible_children);

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    if (!gtk_widget_should_layout (child))
      continue;

    gtk_widget_measure (child, self->orientation, -1,
                        &child_data[i].minimum_size,
                        &child_data[i].natural_size,
                        NULL, NULL);

    child_data[i].expand = gtk_widget_compute_expand (child, self->orientation);
    child_data[i].data.widget = child;
    i++;
  }

  *n_lines = count_lines (self, for_size, child_spacing, child_data, n_visible_children);
  line_data = g_new0 (AllocationData, *n_lines);
  line_start = child_data;

  for (i = 0; i < *n_lines; i++) {
    int line_min = 0, line_nat = 0;
    int n_line_children;
    gboolean expand = FALSE;

    n_line_children = compute_line (self, for_size, child_spacing, line_start,
                                    n_visible_children, i == *n_lines - 1);

    g_assert (n_line_children > 0);

    for (j = 0; j < n_line_children; j++) {
      int child_min = 0, child_nat = 0;

      gtk_widget_measure (line_start[j].data.widget,
                          opposite_orientation,
                          for_size >= 0 ? line_start[j].allocated_size : -1,
                          &child_min, &child_nat, NULL, NULL);

      expand |= gtk_widget_compute_expand (line_start[j].data.widget,
                                           opposite_orientation);

      line_min = MAX (line_min, child_min);
      line_nat = MAX (line_nat, child_nat);
    }

    line_data[i].minimum_size = line_min;
    line_data[i].natural_size = line_nat;
    line_data[i].expand = expand;
    line_data[i].data.line.children = line_start;
    line_data[i].data.line.n_children = n_line_children;

    n_visible_children -= n_line_children;
    line_start = &line_start[n_line_children];
  }

  *out_child_data = child_data;

  return line_data;
}

static int search_for_min_size (AdwWrapLayout   *self,
                                GtkWidget       *widget,
                                int              for_size,
                                int              minimum,
                                int              natural);

static void
adw_wrap_layout_measure (GtkLayoutManager *manager,
                         GtkWidget        *widget,
                         GtkOrientation    orientation,
                         int               for_size,
                         int              *minimum,
                         int              *natural,
                         int              *minimum_baseline,
                         int              *natural_baseline)
{
  AdwWrapLayout *self = ADW_WRAP_LAYOUT (manager);
  GtkWidget *child, *visible_child = NULL;
  gboolean multiple_visible_children = FALSE;
  int min = 0, nat = 0, line_spacing, child_spacing, natural_line_length = -1;
  GtkSettings *settings = gtk_widget_get_settings (widget);

  /* Handle the trivial cases. */
  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    if (!gtk_widget_should_layout (child))
      continue;

    if (visible_child) {
      multiple_visible_children = TRUE;
      break;
    }
    visible_child = child;
  }

  if (!multiple_visible_children) {
    if (visible_child) {
      /* Passthrough the measurement directly. */
      gtk_widget_measure (visible_child, orientation, for_size,
                          minimum, natural, minimum_baseline, natural_baseline);
    } else {
      /* Empty. */
      if (minimum)
        *minimum = 0;
      if (natural)
        *natural = 0;
      if (minimum_baseline)
        *minimum_baseline = -1;
      if (natural_baseline)
        *natural_baseline = -1;
    }
    return;
  }

  line_spacing = adw_length_unit_to_px (self->line_spacing_unit,
                                        self->line_spacing,
                                        settings);

  child_spacing = adw_length_unit_to_px (self->child_spacing_unit,
                                         self->child_spacing,
                                         settings);

  if (self->natural_line_length >= 0) {
    natural_line_length = adw_length_unit_to_px (self->natural_line_length_unit,
                                                 self->natural_line_length,
                                                 settings);
  }

  if (self->orientation == orientation) {
    for (child = gtk_widget_get_first_child (widget);
         child != NULL;
         child = gtk_widget_get_next_sibling (child)) {
      int child_min, child_nat;

      if (!gtk_widget_should_layout (child))
        continue;

      gtk_widget_measure (child, orientation, -1,
                          &child_min, &child_nat, NULL, NULL);

      if (for_size != -1 && natural_line_length < 0) {
        gtk_widget_measure (child, orientation, for_size,
                            NULL, &child_nat, NULL, NULL);
      }

      /* Minimum is with one child per line. */
      min = MAX (min, child_min);
      /* Natural is with all children on the same line. */
      nat += child_nat + child_spacing;
    }
    /* No space after the last child. */
    nat -= child_spacing;

    if (natural_line_length >= 0)
      nat = MAX (min, natural_line_length);

    /* If the available size in the opposite orientation is constrained,
     * search for the minimum size that would fit.
     */
    if (for_size >= 0) {
      min = search_for_min_size (self, widget, for_size, min, nat);
      nat = MAX (nat, min);
    }
  } else {
    AllocationData *line_data, *child_data;
    int i, n_lines;

    if (for_size == -1)
      for_size = natural_line_length;

    line_data = compute_sizes (self, widget, for_size, child_spacing, &n_lines, &child_data);

    if (self->line_homogeneous) {
      for (i = 0; i < n_lines; i++) {
        min = MAX (min, line_data[i].minimum_size);
        nat = MAX (nat, line_data[i].natural_size);
      }

      min *= n_lines;
      nat *= n_lines;
    } else {
      for (i = 0; i < n_lines; i++) {
        min += line_data[i].minimum_size;
        nat += line_data[i].natural_size;
      }
    }

    min += line_spacing * (n_lines - 1);
    nat += line_spacing * (n_lines - 1);

    g_free (child_data);
    g_free (line_data);
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static int
search_for_min_size (AdwWrapLayout   *self,
                     GtkWidget       *widget,
                     int              for_size,
                     int              minimum,
                     int              natural)
{
  int min = minimum;
  int max = G_MAXINT;
  int min_opposite;
  GtkOrientation opposite_orientation;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    opposite_orientation = GTK_ORIENTATION_VERTICAL;
  else
    opposite_orientation = GTK_ORIENTATION_HORIZONTAL;

  while (min < max) {
    int test;

    /* We're likely to be measured for a size that matches our minimum or
     * natural size in the opposite orientation, so start by checking around
     * those sizes.
     */
    if (min == minimum + 1 && max == natural)
      test = max - 1;
    else if (max != G_MAXINT)
      test = (min + max) / 2;
    else if (min == minimum)
      test = min;
    else if (min == minimum + 1 && natural >= min)
      test = natural;
    else
      test = min * 2;

    adw_wrap_layout_measure (GTK_LAYOUT_MANAGER (self), widget,
                             opposite_orientation, test,
                             &min_opposite, NULL, NULL, NULL);

    if (min_opposite > for_size)
      min = test + 1;
    else
      max = test;
  }

  return min;
}

static void
allocate_line (AdwWrapLayout  *self,
               int             available_length,
               int             spacing,
               gboolean        is_rtl,
               gboolean        horiz,
               AllocationData *line_child_data,
               int             n_children,
               int             line_size,
               int             line_offset,
               gboolean        last_line)
{
  int i, widget_offset = 0;
  int allocated_length;
  gboolean justify_line = self->justify != ADW_JUSTIFY_NONE &&
                          (!last_line || self->justify_last_line);
  gboolean reverse_line = self->pack_direction == ADW_PACK_END_TO_START;

  if (is_rtl && horiz)
    widget_offset = available_length + spacing;

  if (!justify_line || reverse_line) {
    allocated_length = spacing * (n_children - 1);

    for (i = 0; i < n_children; i++)
      allocated_length += line_child_data[i].allocated_size;

    if (!justify_line) {
      int length_delta = available_length - allocated_length;

      if (is_rtl && horiz)
        widget_offset -= roundf ((float) length_delta * self->align);
      else
        widget_offset += roundf ((float) length_delta * self->align);
    }
  }

  if (reverse_line) {
    if (horiz && is_rtl)
      widget_offset -= allocated_length + spacing;
    else
      widget_offset += allocated_length + spacing;
  }

  for (i = 0; i < n_children; i++) {
    GtkWidget *widget = line_child_data[i].data.widget;
    int available_size = line_child_data[i].available_size;
    int allocated_size = line_child_data[i].allocated_size;
    int size_delta = available_size - allocated_size;
    GskTransform *transform;
    int x, y, w, h;

    if ((is_rtl && horiz) != reverse_line)
      widget_offset -= allocated_size + spacing;

    if (horiz) {
      x = widget_offset;
      y = line_offset;
      w = allocated_size;
      h = line_size;

      if (size_delta > 0) {
        if (is_rtl != reverse_line)
          x += round (size_delta * (1 - i / (double) (n_children - 1)));
        else
          x += round (size_delta * i / (double) (n_children - 1));
      }
    } else {
      x = line_offset;
      y = widget_offset;
      w = line_size;
      h = allocated_size;

      if (size_delta > 0) {
        if (reverse_line)
          y += round (size_delta * (1 - i / (double) (n_children - 1)));
        else
          y += round (size_delta * i / (double) (n_children - 1));
      }
    }

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y));
    gtk_widget_allocate (widget, w, h, -1, transform);

    if ((!is_rtl || !horiz) != reverse_line)
      widget_offset += allocated_size + spacing;
  }
}

static void
adw_wrap_layout_allocate (GtkLayoutManager *manager,
                          GtkWidget        *widget,
                          int               width,
                          int               height,
                          int               baseline)
{
  AdwWrapLayout *self = ADW_WRAP_LAYOUT (manager);
  AllocationData *line_data, *child_data;
  GtkSettings *settings = gtk_widget_get_settings (widget);
  gboolean horiz = self->orientation == GTK_ORIENTATION_HORIZONTAL;
  gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  int length = horiz ? width : height;
  int i, line_pos = 0;
  int n_lines;
  gboolean reverse = self->wrap_reverse != (!horiz && is_rtl);
  int line_spacing, child_spacing;

  line_spacing = adw_length_unit_to_px (self->line_spacing_unit,
                                        self->line_spacing,
                                        settings);

  child_spacing = adw_length_unit_to_px (self->child_spacing_unit,
                                         self->child_spacing,
                                         settings);

  if (reverse)
    line_pos = (horiz ? height : width) + line_spacing;

  line_data = compute_sizes (self, widget, length, child_spacing, &n_lines, &child_data);

  if (self->line_homogeneous) {
    box_allocate_homogeneous (line_data, n_lines, horiz ? height : width,
                              line_spacing);
  } else {
    box_allocate (line_data, n_lines, horiz ? height : width,
                  line_spacing, ADW_JUSTIFY_NONE);
  }

  for (i = 0; i < n_lines; i++) {
    if (reverse)
      line_pos -= line_data[i].allocated_size + line_spacing;

    allocate_line (self, length, child_spacing, is_rtl, horiz,
                   line_data[i].data.line.children,
                   line_data[i].data.line.n_children,
                   line_data[i].allocated_size, line_pos,
                   i == n_lines - 1);

    if (!reverse)
      line_pos += line_data[i].allocated_size + line_spacing;
  }

  g_free (child_data);
  g_free (line_data);
}

static GtkSizeRequestMode
adw_wrap_layout_get_request_mode (GtkLayoutManager *manager,
                                  GtkWidget        *widget)
{
  AdwWrapLayout *self = ADW_WRAP_LAYOUT (manager);
  GtkWidget *child, *visible_child = NULL;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    if (!gtk_widget_should_layout (child))
      continue;

    if (visible_child) {
      /* Multiple visible children. */
      if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
        return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;

      return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
    }

    visible_child = child;
  }

  /* Passthrough request mode if there's just a single child. */
  if (visible_child)
    return gtk_widget_get_request_mode (visible_child);

  return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
adw_wrap_layout_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdwWrapLayout *self = ADW_WRAP_LAYOUT (object);

  switch (prop_id) {
  case PROP_CHILD_SPACING:
    g_value_set_int (value, adw_wrap_layout_get_child_spacing (self));
    break;
  case PROP_CHILD_SPACING_UNIT:
    g_value_set_enum (value, adw_wrap_layout_get_child_spacing_unit (self));
    break;
  case PROP_PACK_DIRECTION:
    g_value_set_enum (value, adw_wrap_layout_get_pack_direction (self));
    break;
  case PROP_ALIGN:
    g_value_set_float (value, adw_wrap_layout_get_align (self));
    break;
  case PROP_JUSTIFY:
    g_value_set_enum (value, adw_wrap_layout_get_justify (self));
    break;
  case PROP_JUSTIFY_LAST_LINE:
    g_value_set_boolean (value, adw_wrap_layout_get_justify_last_line (self));
    break;
  case PROP_LINE_SPACING:
    g_value_set_int (value, adw_wrap_layout_get_line_spacing (self));
    break;
  case PROP_LINE_SPACING_UNIT:
    g_value_set_enum (value, adw_wrap_layout_get_line_spacing_unit (self));
    break;
  case PROP_LINE_HOMOGENEOUS:
    g_value_set_boolean (value, adw_wrap_layout_get_line_homogeneous (self));
    break;
  case PROP_NATURAL_LINE_LENGTH:
    g_value_set_int (value, adw_wrap_layout_get_natural_line_length (self));
    break;
  case PROP_NATURAL_LINE_LENGTH_UNIT:
    g_value_set_enum (value, adw_wrap_layout_get_natural_line_length_unit (self));
    break;
  case PROP_WRAP_REVERSE:
    g_value_set_boolean (value, adw_wrap_layout_get_wrap_reverse (self));
    break;
  case PROP_WRAP_POLICY:
    g_value_set_enum (value, adw_wrap_layout_get_wrap_policy (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_wrap_layout_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdwWrapLayout *self = ADW_WRAP_LAYOUT (object);

  switch (prop_id) {
  case PROP_CHILD_SPACING:
    adw_wrap_layout_set_child_spacing (self, g_value_get_int (value));
    break;
  case PROP_CHILD_SPACING_UNIT:
    adw_wrap_layout_set_child_spacing_unit (self, g_value_get_enum (value));
    break;
  case PROP_PACK_DIRECTION:
    adw_wrap_layout_set_pack_direction (self, g_value_get_enum (value));
    break;
  case PROP_ALIGN:
    adw_wrap_layout_set_align (self, g_value_get_float (value));
    break;
  case PROP_JUSTIFY:
    adw_wrap_layout_set_justify (self, g_value_get_enum (value));
    break;
  case PROP_JUSTIFY_LAST_LINE:
    adw_wrap_layout_set_justify_last_line (self, g_value_get_boolean (value));
    break;
  case PROP_LINE_SPACING:
    adw_wrap_layout_set_line_spacing (self, g_value_get_int (value));
    break;
  case PROP_LINE_SPACING_UNIT:
    adw_wrap_layout_set_line_spacing_unit (self, g_value_get_enum (value));
    break;
  case PROP_LINE_HOMOGENEOUS:
    adw_wrap_layout_set_line_homogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_NATURAL_LINE_LENGTH:
    adw_wrap_layout_set_natural_line_length (self, g_value_get_int (value));
    break;
  case PROP_NATURAL_LINE_LENGTH_UNIT:
    adw_wrap_layout_set_natural_line_length_unit (self, g_value_get_enum (value));
    break;
  case PROP_WRAP_REVERSE:
    adw_wrap_layout_set_wrap_reverse (self, g_value_get_boolean (value));
    break;
  case PROP_WRAP_POLICY:
    adw_wrap_layout_set_wrap_policy (self, g_value_get_enum (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_wrap_layout_class_init (AdwWrapLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  object_class->get_property = adw_wrap_layout_get_property;
  object_class->set_property = adw_wrap_layout_set_property;

  layout_manager_class->measure = adw_wrap_layout_measure;
  layout_manager_class->allocate = adw_wrap_layout_allocate;
  layout_manager_class->get_request_mode = adw_wrap_layout_get_request_mode;

  /**
   * AdwWrapLayout:child-spacing:
   *
   * The spacing between widgets on the same line.
   *
   * See [property@WrapLayout:child-spacing-unit].
   *
   * Since: 1.7
   */
  props[PROP_CHILD_SPACING] =
    g_param_spec_int ("child-spacing", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:child-spacing-unit:
   *
   * The length unit for child spacing.
   *
   * Allows the spacing to vary depending on the text scale factor.
   *
   * See [property@WrapLayout:child-spacing].
   *
   * Since: 1.7
   */
  props[PROP_CHILD_SPACING_UNIT] =
    g_param_spec_enum ("child-spacing-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_PX,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:pack-direction:
   *
   * The direction children are packed in each line.
   *
   * Since: 1.7
   */
  props[PROP_PACK_DIRECTION] =
    g_param_spec_enum ("pack-direction", NULL, NULL,
                       ADW_TYPE_PACK_DIRECTION,
                       ADW_PACK_START_TO_END,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:align:
   *
   * The alignment of the children within each line.
   *
   * 0 means the children are placed at the start of the line, 1 means they are
   * placed at the end of the line. 0.5 means they are placed in the middle of
   * the line.
   *
   * Alignment is only used when [property@WrapLayout:justify] is set to
   * `ADW_JUSTIFY_NONE`, or on the last line when the
   * [property@WrapLayout:justify-last-line] is `FALSE`.
   *
   * Since: 1.7
   */
  props[PROP_ALIGN] =
    g_param_spec_float ("align", NULL, NULL,
                        0, 1, 0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:justify:
   *
   * Determines whether and how each complete line should be stretched to fill
   * the entire widget.
   *
   * If set to `ADW_JUSTIFY_FILL`, each widget in the line will be stretched,
   * keeping consistent spacing, so that the line fills the entire widget.
   *
   * If set to `ADW_JUSTIFY_SPREAD`, the spacing between widgets will be
   * increased, keeping widget sizes intact. The first and last widget will be
   * aligned with the beginning and end of the line. If the line only contains a
   * single widget, it will be stretched regardless.
   *
   * If set to `ADW_JUSTIFY_NONE`, the line will not be stretched and the
   * children will be placed together within the line, according to
   * [property@WrapLayout:align].
   *
   * By default this doesn't affect the last line, as it will be incomplete. Use
   * [property@WrapLayout:justify-last-line] to justify it as well.
   *
   * Since: 1.7
   */
  props[PROP_JUSTIFY] =
    g_param_spec_enum ("justify", NULL, NULL,
                       ADW_TYPE_JUSTIFY_MODE,
                       ADW_JUSTIFY_NONE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:justify-last-line:
   *
   * Whether the last line should be stretched to fill the entire widget.
   *
   * See [property@WrapLayout:justify].
   *
   * Since: 1.7
   */
  props[PROP_JUSTIFY_LAST_LINE] =
    g_param_spec_boolean ("justify-last-line", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:line-spacing:
   *
   * The spacing between lines.
   *
   * See [property@WrapLayout:line-spacing-unit].
   *
   * Since: 1.7
   */
  props[PROP_LINE_SPACING] =
    g_param_spec_int ("line-spacing", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:line-spacing-unit:
   *
   * The length unit for line spacing.
   *
   * Allows the spacing to vary depending on the text scale factor.
   *
   * See [property@WrapLayout:line-spacing].
   *
   * Since: 1.7
   */
  props[PROP_LINE_SPACING_UNIT] =
    g_param_spec_enum ("line-spacing-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_PX,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:line-homogeneous:
   *
   * Whether all lines should take the same amount of space.
   *
   * Since: 1.7
   */
  props[PROP_LINE_HOMOGENEOUS] =
    g_param_spec_boolean ("line-homogeneous", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:natural-line-length:
   *
   * Determines the natural size for each line.
   *
   * It should be used to limit the line lengths, for example when used in
   * popovers.
   *
   * See [property@WrapLayout:natural-line-length-unit].
   *
   * Since: 1.7
   */
  props[PROP_NATURAL_LINE_LENGTH] =
    g_param_spec_int ("natural-line-length", NULL, NULL,
                      -1, G_MAXINT, -1,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:natural-line-length-unit:
   *
   * The length unit for natural line length.
   *
   * Allows the length to vary depending on the text scale factor.
   *
   * See [property@WrapLayout:natural-line-length].
   *
   * Since: 1.7
   */
  props[PROP_NATURAL_LINE_LENGTH_UNIT] =
    g_param_spec_enum ("natural-line-length-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_PX,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:wrap-reverse:
   *
   * Whether wrap direction should be reversed.
   *
   * By default, lines wrap downwards in a horizontal box, and towards the end
   * in a vertical box. If set to `TRUE`, they wrap upwards or towards the start
   * respectively.
   *
   * Since: 1.7
   */
  props[PROP_WRAP_REVERSE] =
    g_param_spec_boolean ("wrap-reverse", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapLayout:wrap-policy:
   *
   * The policy for line wrapping.
   *
   * If set to `ADW_WRAP_NATURAL`, the box will wrap to the next line as soon as
   * the previous line cannot fit any more children without shrinking them past
   * their natural size.
   *
   * If set to `ADW_WRAP_MINIMUM`, the box will try to fit as many children into
   * each line as possible, shrinking them down to their minimum size before
   * wrapping to the next line.
   *
   * Since: 1.7
   */
  props[PROP_WRAP_POLICY] =
    g_param_spec_enum ("wrap-policy", NULL, NULL,
                       ADW_TYPE_WRAP_POLICY,
                       ADW_WRAP_NATURAL,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  g_object_class_override_property (object_class, PROP_ORIENTATION, "orientation");
}

static void
adw_wrap_layout_init (AdwWrapLayout *self)
{
  self->child_spacing_unit = ADW_LENGTH_UNIT_PX;
  self->pack_direction = ADW_PACK_START_TO_END;
  self->align = 0;
  self->justify = ADW_JUSTIFY_NONE;
  self->line_spacing_unit = ADW_LENGTH_UNIT_PX;
  self->natural_line_length = -1;
  self->natural_line_length_unit = ADW_LENGTH_UNIT_PX;
  self->wrap_policy = ADW_WRAP_NATURAL;
}

/**
 * adw_wrap_layout_new:
 *
 * Creates a new `AdwWrapLayout`.
 *
 * Returns: the newly created `AdwWrapLayout`
 *
 * Since: 1.7
 */
GtkLayoutManager *
adw_wrap_layout_new (void)
{
  return g_object_new (ADW_TYPE_WRAP_LAYOUT, NULL);
}

/**
 * adw_wrap_layout_get_child_spacing:
 * @self: a wrap layout
 *
 * Gets spacing between widgets on the same line.
 *
 * Returns: spacing between widgets on the same line
 *
 * Since: 1.7
 */
int
adw_wrap_layout_get_child_spacing (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), 0);

  return self->child_spacing;
}

/**
 * adw_wrap_layout_set_child_spacing:
 * @self: a wrap layout
 * @child_spacing: the child spacing
 *
 * Sets the spacing between widgets on the same line.
 *
 * See [property@WrapLayout:child-spacing-unit].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_child_spacing (AdwWrapLayout *self,
                                   int            child_spacing)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  if (child_spacing < 0)
    child_spacing = 0;

  if (child_spacing == self->child_spacing)
    return;

  self->child_spacing = child_spacing;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_SPACING]);
}

/**
 * adw_wrap_layout_get_child_spacing_unit:
 * @self: a wrap layout
 *
 * Gets the length unit for child spacing.
 *
 * Returns: the length unit
 *
 * Since: 1.7
 */
AdwLengthUnit
adw_wrap_layout_get_child_spacing_unit (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), ADW_LENGTH_UNIT_PX);

  return self->child_spacing_unit;
}

/**
 * adw_wrap_layout_set_child_spacing_unit:
 * @self: a wrap layout
 * @unit: the length unit
 *
 * Sets the length unit for child spacing.
 *
 * Allows the spacing to vary depending on the text scale factor.
 *
 * See [property@WrapLayout:child-spacing].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_child_spacing_unit (AdwWrapLayout *self,
                                        AdwLengthUnit  unit)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  if (unit == self->child_spacing_unit)
    return;

  self->child_spacing_unit = unit;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_SPACING_UNIT]);
}

/**
 * adw_wrap_layout_get_pack_direction:
 * @self: a wrap layout
 *
 * Gets the direction children are packed in each line.
 *
 * Returns: the line direction
 *
 * Since: 1.7
 */
AdwPackDirection
adw_wrap_layout_get_pack_direction (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), ADW_PACK_START_TO_END);

  return self->pack_direction;
}

/**
 * adw_wrap_layout_set_pack_direction:
 * @self: a wrap layout
 * @pack_direction: the new line direction
 *
 * Sets the direction children are packed in each line.
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_pack_direction (AdwWrapLayout    *self,
                                    AdwPackDirection  pack_direction)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));
  g_return_if_fail (pack_direction >= ADW_PACK_START_TO_END);
  g_return_if_fail (pack_direction <= ADW_PACK_END_TO_START);

  if (self->pack_direction == pack_direction)
    return;

  self->pack_direction = pack_direction;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PACK_DIRECTION]);
}

/**
 * adw_wrap_layout_get_align:
 * @self: a wrap layout
 *
 * Gets the alignment of the children within each line.
 *
 * Returns: the child alignment
 *
 * Since: 1.7
 */
float
adw_wrap_layout_get_align (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), 0.0f);

  return self->align;
}

/**
 * adw_wrap_layout_set_align:
 * @self: a wrap layout
 * @align: the child alignment
 *
 * Sets the alignment of the children within each line.
 *
 * 0 means the children are placed at the start of the line, 1 means they are
 * placed at the end of the line. 0.5 means they are placed in the middle of the
 * line.
 *
 * Alignment is only used when [property@WrapLayout:justify] is set to
 * `ADW_JUSTIFY_NONE`, or on the last line when the
 * [property@WrapLayout:justify-last-line] is `FALSE`.
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_align (AdwWrapLayout *self,
                           float          align)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  if (G_APPROX_VALUE (align, self->align, FLT_EPSILON))
    return;

  self->align = align;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALIGN]);
}

/**
 * adw_wrap_layout_get_justify:
 * @self: a wrap layout
 *
 * Gets whether and how each complete line is stretched to fill the entire widget.
 *
 * Returns: the justify mode
 *
 * Since: 1.7
 */
AdwJustifyMode
adw_wrap_layout_get_justify (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), ADW_JUSTIFY_NONE);

  return self->justify;
}

/**
 * adw_wrap_layout_set_justify:
 * @self: a wrap layout
 * @justify: the justify mode
 *
 * Sets whether and how each complete line should be stretched to fill the
 * entire widget.
 *
 * If set to `ADW_JUSTIFY_FILL`, each widget in the line will be stretched,
 * keeping consistent spacing, so that the line fills the entire widget.
 *
 * If set to `ADW_JUSTIFY_SPREAD`, the spacing between widgets will be
 * increased, keeping widget sizes intact. The first and last widget will be
 * aligned with the beginning and end of the line. If the line only contains a
 * single widget, it will be stretched regardless.
 *
 * If set to `ADW_JUSTIFY_NONE`, the line will not be stretched and the children
 * will be placed together within the line, according to
 * [property@WrapLayout:align].
 *
 * By default this doesn't affect the last line, as it will be incomplete. Use
 * [property@WrapLayout:justify-last-line] to justify it as well.
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_justify (AdwWrapLayout  *self,
                             AdwJustifyMode  justify)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));
  g_return_if_fail (justify >= ADW_JUSTIFY_NONE);
  g_return_if_fail (justify <= ADW_JUSTIFY_SPREAD);

  if (self->justify == justify)
    return;

  self->justify = justify;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_JUSTIFY]);
}

/**
 * adw_wrap_layout_get_justify_last_line:
 * @self: a wrap layout
 *
 * Gets whether the last line should be stretched to fill the entire widget.
 *
 * Returns: whether the last line is justified
 *
 * Since: 1.7
 */
gboolean
adw_wrap_layout_get_justify_last_line (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), FALSE);

  return self->justify_last_line;
}

/**
 * adw_wrap_layout_set_justify_last_line:
 * @self: a wrap layout
 * @justify_last_line: whether to justify the last line
 *
 * Sets whether the last line should be stretched to fill the entire widget.
 *
 * See [property@WrapLayout:justify].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_justify_last_line (AdwWrapLayout *self,
                                       gboolean       justify_last_line)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  justify_last_line = !!justify_last_line;

  if (self->justify_last_line == justify_last_line)
    return;

  self->justify_last_line = justify_last_line;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_JUSTIFY_LAST_LINE]);
}

/**
 * adw_wrap_layout_get_line_spacing:
 * @self: a wrap layout
 *
 * Gets the spacing between lines.
 *
 * Returns: the line spacing
 *
 * Since: 1.7
 */
int
adw_wrap_layout_get_line_spacing (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), 0);

  return self->line_spacing;
}

/**
 * adw_wrap_layout_set_line_spacing:
 * @self: a wrap layout
 * @line_spacing: the line spacing
 *
 * Sets the spacing between lines.
 *
 * See [property@WrapLayout:line-spacing-unit].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_line_spacing (AdwWrapLayout *self,
                                  int            line_spacing)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  if (line_spacing < 0)
    line_spacing = 0;

  if (line_spacing == self->line_spacing)
    return;

  self->line_spacing = line_spacing;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINE_SPACING]);
}

/**
 * adw_wrap_layout_get_line_spacing_unit:
 * @self: a wrap layout
 *
 * Gets the length unit for line spacing.
 *
 * Returns: the length unit
 *
 * Since: 1.7
 */
AdwLengthUnit
adw_wrap_layout_get_line_spacing_unit (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), ADW_LENGTH_UNIT_PX);

  return self->line_spacing_unit;
}

/**
 * adw_wrap_layout_set_line_spacing_unit:
 * @self: a wrap layout
 * @unit: the length unit
 *
 * Sets the length unit for line spacing.
 *
 * Allows the spacing to vary depending on the text scale factor.
 *
 * See [property@WrapLayout:line-spacing].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_line_spacing_unit (AdwWrapLayout *self,
                                       AdwLengthUnit  unit)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  if (unit == self->line_spacing_unit)
    return;

  self->line_spacing_unit = unit;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINE_SPACING_UNIT]);
}

/**
 * adw_wrap_layout_get_line_homogeneous:
 * @self: a wrap layout
 *
 * Gets whether all lines should take the same amount of space.
 *
 * Returns: whether lines should be homogeneous
 *
 * Since: 1.7
 */
gboolean
adw_wrap_layout_get_line_homogeneous (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), FALSE);

  return self->line_homogeneous;
}

/**
 * adw_wrap_layout_set_line_homogeneous:
 * @self: a wrap layout
 * @homogeneous: whether lines should be homogeneous
 *
 * Sets whether all lines should take the same amount of space.
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_line_homogeneous (AdwWrapLayout *self,
                                       gboolean       homogeneous)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  homogeneous = !!homogeneous;

  if (self->line_homogeneous == homogeneous)
    return;

  self->line_homogeneous = homogeneous;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINE_HOMOGENEOUS]);
}

/**
 * adw_wrap_layout_get_natural_line_length:
 * @self: a wrap layout
 *
 * Gets the natural size for each line.
 *
 * Returns: the natural length
 *
 * Since: 1.7
 */
int
adw_wrap_layout_get_natural_line_length (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), 0);

  return self->natural_line_length;
}

/**
 * adw_wrap_layout_set_natural_line_length:
 * @self: a wrap layout
 * @natural_line_length: the natural length
 *
 * Sets the natural size for each line.
 *
 * It should be used to limit the line lengths, for example when used in
 * popovers.
 *
 * See [property@WrapLayout:natural-line-length-unit].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_natural_line_length (AdwWrapLayout *self,
                                         int            natural_line_length)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  if (natural_line_length < -1)
    natural_line_length = -1;

  if (natural_line_length == self->natural_line_length)
    return;

  self->natural_line_length = natural_line_length;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NATURAL_LINE_LENGTH]);
}

/**
 * adw_wrap_layout_get_natural_line_length_unit:
 * @self: a wrap layout
 *
 * Gets the length unit for line spacing.
 *
 * Returns: the length unit
 *
 * Since: 1.7
 */
AdwLengthUnit
adw_wrap_layout_get_natural_line_length_unit (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), ADW_LENGTH_UNIT_PX);

  return self->natural_line_length_unit;
}

/**
 * adw_wrap_layout_set_natural_line_length_unit:
 * @self: a wrap layout
 * @unit: the length unit
 *
 * Sets the length unit for natural line length.
 *
 * Allows the length to vary depending on the text scale factor.
 *
 * See [property@WrapLayout:natural-line-length].
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_natural_line_length_unit (AdwWrapLayout *self,
                                              AdwLengthUnit  unit)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  if (unit == self->natural_line_length_unit)
    return;

  self->natural_line_length_unit = unit;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NATURAL_LINE_LENGTH_UNIT]);
}

/**
 * adw_wrap_layout_get_wrap_reverse:
 * @self: a wrap layout
 *
 * Gets whether wrap direction is reversed.
 *
 * Returns: whether wrap direction is reversed
 *
 * Since: 1.7
 */
gboolean
adw_wrap_layout_get_wrap_reverse (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), FALSE);

  return self->wrap_reverse;
}

/**
 * adw_wrap_layout_set_wrap_reverse:
 * @self: a wrap layout
 * @wrap_reverse: whether to reverse wrap direction
 *
 * Sets whether wrap direction should be reversed.
 *
 * By default, lines wrap downwards in a horizontal box, and towards the end
 * in a vertical box. If set to `TRUE`, they wrap upwards or towards the start
 * respectively.
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_wrap_reverse (AdwWrapLayout *self,
                                  gboolean       wrap_reverse)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));

  wrap_reverse = !!wrap_reverse;

  if (self->wrap_reverse == wrap_reverse)
    return;

  self->wrap_reverse = wrap_reverse;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WRAP_REVERSE]);
}

/**
 * adw_wrap_layout_get_wrap_policy:
 * @self: a wrap layout
 *
 * Gets the policy for line wrapping.
 *
 * Returns: the wrap policy
 *
 * Since: 1.7
 */
AdwWrapPolicy
adw_wrap_layout_get_wrap_policy (AdwWrapLayout *self)
{
  g_return_val_if_fail (ADW_IS_WRAP_LAYOUT (self), ADW_WRAP_MINIMUM);

  return self->wrap_policy;
}

/**
 * adw_wrap_layout_set_wrap_policy:
 * @self: a wrap layout
 * @wrap_policy: the new wrap policy
 *
 * Sets the policy for line wrapping.
 *
 * If set to `ADW_WRAP_NATURAL`, the box will wrap to the next line as soon as
 * the previous line cannot fit any more children without shrinking them past
 * their natural size.
 *
 * If set to `ADW_WRAP_MINIMUM`, the box will try to fit as many children into
 * each line as possible, shrinking them down to their minimum size before
 * wrapping to the next line.
 *
 * Since: 1.7
 */
void
adw_wrap_layout_set_wrap_policy (AdwWrapLayout *self,
                                 AdwWrapPolicy  wrap_policy)
{
  g_return_if_fail (ADW_IS_WRAP_LAYOUT (self));
  g_return_if_fail (wrap_policy >= ADW_WRAP_MINIMUM);
  g_return_if_fail (wrap_policy <= ADW_WRAP_NATURAL);

  if (self->wrap_policy == wrap_policy)
    return;

  self->wrap_policy = wrap_policy;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WRAP_POLICY]);
}
