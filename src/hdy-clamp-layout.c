/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-clamp-layout.h"

#include <glib/gi18n-lib.h>
#include <math.h>

#include "hdy-animation-private.h"

/**
 * SECTION:hdy-clamp-layout
 * @short_description: A layout manager constraining its children to a given size.
 * @Title: HdyClampLayout
 *
 * The #HdyClamp widget constraints the size of the widget it contains to a
 * given maximum size. It will constrain the width if it is horizontal, or the
 * height if it is vertical. The expansion of the child from its minimum to its
 * maximum size is eased out for a smooth transition.
 *
 * If the child requires more than the requested maximum size, it will be
 * allocated the minimum size it can fit in instead.
 *
 * # CSS nodes
 *
 * #HdyClamp has a single CSS node with name clamp. The node will get the style
 * classes .large when its child reached its maximum size, .small when the clamp
 * allocates its full size to its child, .medium in-between, or none if it
 * didn't compute its size yet.
 *
 * Since: 1.0
 */

#define HDY_EASE_OUT_TAN_CUBIC 3

enum {
  PROP_0,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_TIGHTENING_THRESHOLD + 1,
};

struct _HdyClampLayout
{
  GtkLayoutManager parent_instance;

  gint maximum_size;
  gint tightening_threshold;

  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE (HdyClampLayout, hdy_clamp_layout, GTK_TYPE_LAYOUT_MANAGER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static void
set_orientation (HdyClampLayout *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
hdy_clamp_layout_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  HdyClampLayout *self = HDY_CLAMP_LAYOUT (object);

  switch (prop_id) {
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, hdy_clamp_layout_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, hdy_clamp_layout_get_tightening_threshold (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_clamp_layout_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HdyClampLayout *self = HDY_CLAMP_LAYOUT (object);

  switch (prop_id) {
  case PROP_MAXIMUM_SIZE:
    hdy_clamp_layout_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    hdy_clamp_layout_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

/**
 * get_child_size:
 * @self: a #HdyClampLayout
 * @child: a #GtkWidget
 * @for_size: the size of the clamp
 * @child_minimum: the minimum size reachable by the child, and hence by @self
 * @child_maximum: the maximum size @self will ever allocate its child
 * @lower_threshold: the threshold below which @self will allocate its full size to its child
 * @upper_threshold: the threshold up from which @self will allocate its maximum size to its child
 *
 * Measures @child's extremes, the clamp's thresholds, and returns size to
 * allocate to the child.
 *
 * If the clamp is horizontal, all values are widths, otherwise they are
 * heights.
 */
static gint
get_child_size (HdyClampLayout *self,
                GtkWidget      *child,
                gint            for_size,
                gint           *child_minimum,
                gint           *child_maximum,
                gint           *lower_threshold,
                gint           *upper_threshold)
{
  gint min = 0, max = 0, lower = 0, upper = 0;
  gdouble amplitude, progress;

  if (gtk_widget_get_visible (child))
    gtk_widget_measure (child, self->orientation, -1, &min, NULL, NULL, NULL);

  lower = MAX (MIN (self->tightening_threshold, self->maximum_size), min);
  max = MAX (lower, self->maximum_size);
  amplitude = max - lower;
  upper = HDY_EASE_OUT_TAN_CUBIC * amplitude + lower;

  if (child_minimum)
    *child_minimum = min;
  if (child_maximum)
    *child_maximum = max;
  if (lower_threshold)
    *lower_threshold = lower;
  if (upper_threshold)
    *upper_threshold = upper;

  if (for_size < 0)
    return 0;

  if (for_size <= lower)
    return for_size;

  if (for_size >= upper)
    return max;

  progress = (double) (for_size - lower) / (double) (upper - lower);

  return hdy_ease_out_cubic (progress) * amplitude + lower;
}

static GtkSizeRequestMode
hdy_clamp_layout_get_request_mode (GtkLayoutManager *manager,
                                   GtkWidget        *widget)
{
  HdyClampLayout *self = HDY_CLAMP_LAYOUT (manager);

  return self->orientation == GTK_ORIENTATION_HORIZONTAL ?
    GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH :
    GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
hdy_clamp_layout_measure (GtkLayoutManager *manager,
                          GtkWidget        *widget,
                          GtkOrientation    orientation,
                          int               for_size,
                          int              *minimum,
                          int              *natural,
                          int              *minimum_baseline,
                          int              *natural_baseline)
{
  HdyClampLayout *self = HDY_CLAMP_LAYOUT (manager);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    gint child_size = -1;
    gint child_min = 0;
    gint child_nat = 0;
    gint child_min_baseline = -1;
    gint child_nat_baseline = -1;

    if (!gtk_widget_should_layout (child))
      continue;

    if (self->orientation != orientation)
      child_size = get_child_size (self, child, for_size,
                                   NULL, NULL, NULL, NULL);

    gtk_widget_measure (child, orientation, child_size,
                        &child_min, &child_nat,
                        &child_min_baseline, &child_nat_baseline);

    *minimum = MAX (*minimum, child_min);
    *natural = MAX (*natural, child_nat);

    if (child_min_baseline > -1)
      *minimum_baseline = MAX (*minimum_baseline, child_min_baseline);
    if (child_nat_baseline > -1)
      *natural_baseline = MAX (*natural_baseline, child_nat_baseline);
  }
}

static void
hdy_clamp_layout_allocate (GtkLayoutManager *manager,
                           GtkWidget        *widget,
                           gint              width,
                           gint              height,
                           gint              baseline)
{
  HdyClampLayout *self = HDY_CLAMP_LAYOUT (manager);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    GtkAllocation child_allocation;
    gint child_maximum = 0, lower_threshold = 0;
    gint child_clamped_size;

    if (!gtk_widget_should_layout (child)) {
      gtk_widget_remove_css_class (child, "small");
      gtk_widget_remove_css_class (child, "medium");
      gtk_widget_remove_css_class (child, "large");

      return;
    }

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      child_allocation.width = get_child_size (self, child, width,
                                               NULL, &child_maximum,
                                               &lower_threshold, NULL);
      child_allocation.height = height;

      child_clamped_size = child_allocation.width;
    }
    else {
      child_allocation.width = width;
      child_allocation.height = get_child_size (self, child, height,
                                                NULL, &child_maximum,
                                                &lower_threshold, NULL);

      child_clamped_size = child_allocation.height;
    }

    if (child_clamped_size >= child_maximum) {
      gtk_widget_remove_css_class (child, "small");
      gtk_widget_remove_css_class (child, "medium");
      gtk_widget_add_css_class (child, "large");
    } else if (child_clamped_size <= lower_threshold) {
      gtk_widget_add_css_class (child, "small");
      gtk_widget_remove_css_class (child, "medium");
      gtk_widget_remove_css_class (child, "large");
    } else {
      gtk_widget_remove_css_class (child, "small");
      gtk_widget_add_css_class (child, "medium");
      gtk_widget_remove_css_class (child, "large");
    }

    /* Always center the child on the side of the orientation. */
    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      child_allocation.x = (width - child_allocation.width) / 2;
      child_allocation.y = 0;
    } else {
      child_allocation.x = 0;
      child_allocation.y = (height - child_allocation.height) / 2;
    }

    gtk_widget_size_allocate (child, &child_allocation, baseline);
  }
}

static void
hdy_clamp_layout_class_init (HdyClampLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  object_class->get_property = hdy_clamp_layout_get_property;
  object_class->set_property = hdy_clamp_layout_set_property;

  layout_manager_class->get_request_mode = hdy_clamp_layout_get_request_mode;
  layout_manager_class->measure = hdy_clamp_layout_measure;
  layout_manager_class->allocate = hdy_clamp_layout_allocate;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * HdyClampLayout:maximum-size:
   *
   * The maximum size to allocate to the child. It is the width if the clamp is
   * horizontal, or the height if it is vertical.
   *
   * Since: 1.0
   */
  props[PROP_MAXIMUM_SIZE] =
      g_param_spec_int ("maximum-size",
                        _("Maximum size"),
                        _("The maximum size allocated to the child"),
                        0, G_MAXINT, 600,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyClampLayout:tightening-threshold:
   *
   * The size starting from which the clamp will tighten its grip on the child,
   * slowly allocating less and less of the available size up to the maximum
   * allocated size. Below that threshold and below the maximum width, the child
   * will be allocated all the available size.
   *
   * If the threshold is greater than the maximum size to allocate to the child,
   * the child will be allocated all the width up to the maximum.
   * If the threshold is lower than the minimum size to allocate to the child,
   * that size will be used as the tightening threshold.
   *
   * Effectively, tightening the grip on the child before it reaches its maximum
   * size makes transitions to and from the maximum size smoother when resizing.
   *
   * Since: 1.0
   */
  props[PROP_TIGHTENING_THRESHOLD] =
      g_param_spec_int ("tightening-threshold",
                        _("Tightening threshold"),
                        _("The size from which the clamp will tighten its grip on the child"),
                        0, G_MAXINT, 400,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
hdy_clamp_layout_init (HdyClampLayout *self)
{
  self->maximum_size = 600;
  self->tightening_threshold = 400;
}

/**
 * hdy_clamp_layout_new:
 *
 * Creates a new #HdyClampLayout.
 *
 * Returns: a new #HdyClampLayout
 *
 * Since: 1.0
 */
GtkLayoutManager *
hdy_clamp_layout_new (void)
{
  return g_object_new (HDY_TYPE_CLAMP_LAYOUT, NULL);
}

/**
 * hdy_clamp_layout_get_maximum_size:
 * @self: a #HdyClampLayout
 *
 * Gets the maximum size to allocate to the contained child. It is the width if
 * @self is horizontal, or the height if it is vertical.
 *
 * Returns: the maximum width to allocate to the contained child.
 *
 * Since: 1.0
 */
gint
hdy_clamp_layout_get_maximum_size (HdyClampLayout *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP_LAYOUT (self), 0);

  return self->maximum_size;
}

/**
 * hdy_clamp_layout_set_maximum_size:
 * @self: a #HdyClampLayout
 * @maximum_size: the maximum size
 *
 * Sets the maximum size to allocate to the contained child. It is the width if
 * @self is horizontal, or the height if it is vertical.
 *
 * Since: 1.0
 */
void
hdy_clamp_layout_set_maximum_size (HdyClampLayout *self,
                                   gint            maximum_size)
{
  g_return_if_fail (HDY_IS_CLAMP_LAYOUT (self));

  if (self->maximum_size == maximum_size)
    return;

  self->maximum_size = maximum_size;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * hdy_clamp_layout_get_tightening_threshold:
 * @self: a #HdyClampLayout
 *
 * Gets the size starting from which the clamp will tighten its grip on the
 * child.
 *
 * Returns: the size starting from which the clamp will tighten its grip on the
 * child.
 *
 * Since: 1.0
 */
gint
hdy_clamp_layout_get_tightening_threshold (HdyClampLayout *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP_LAYOUT (self), 0);

  return self->tightening_threshold;
}

/**
 * hdy_clamp_layout_set_tightening_threshold:
 * @self: a #HdyClampLayout
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size starting from which the clamp will tighten its grip on the
 * child.
 *
 * Since: 1.0
 */
void
hdy_clamp_layout_set_tightening_threshold (HdyClampLayout *self,
                                           gint            tightening_threshold)
{
  g_return_if_fail (HDY_IS_CLAMP_LAYOUT (self));

  if (self->tightening_threshold == tightening_threshold)
    return;

  self->tightening_threshold = tightening_threshold;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}
