/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "adw-clamp-layout.h"

#include <math.h>

#include "adw-animation-private.h"

/**
 * AdwClampLayout:
 *
 * A layout manager constraining its children to a given size.
 *
 * `AdwClampLayout` constraints the size of the widgets it contains to a given
 * maximum size. It will constrain the width if it is horizontal, or the height
 * if it is vertical. The expansion of the children from their minimum to their
 * maximum size is eased out for a smooth transition.
 *
 * If a child requires more than the requested maximum size, it will be
 * allocated the minimum size it can fit in instead.
 *
 * Each child will get the style  classes .large when it reached its maximum
 * size, .small when it's allocated the full size, .medium in-between, or none
 * if it hasn't been allocated yet.
 *
 * Since: 1.0
 */

#define ADW_EASE_OUT_TAN_CUBIC 3

enum {
  PROP_0,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_TIGHTENING_THRESHOLD + 1,
};

struct _AdwClampLayout
{
  GtkLayoutManager parent_instance;

  int maximum_size;
  int tightening_threshold;

  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE (AdwClampLayout, adw_clamp_layout, GTK_TYPE_LAYOUT_MANAGER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static void
set_orientation (AdwClampLayout *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adw_clamp_layout_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwClampLayout *self = ADW_CLAMP_LAYOUT (object);

  switch (prop_id) {
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, adw_clamp_layout_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, adw_clamp_layout_get_tightening_threshold (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_clamp_layout_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwClampLayout *self = ADW_CLAMP_LAYOUT (object);

  switch (prop_id) {
  case PROP_MAXIMUM_SIZE:
    adw_clamp_layout_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    adw_clamp_layout_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static int
get_child_size (AdwClampLayout *self,
                GtkWidget      *child,
                int             for_size,
                int            *child_minimum,
                int            *child_maximum,
                int            *lower_threshold,
                int            *upper_threshold)
{
  int min = 0, max = 0, lower = 0, upper = 0;
  double amplitude, progress;

  if (gtk_widget_get_visible (child))
    gtk_widget_measure (child, self->orientation, -1, &min, NULL, NULL, NULL);

  lower = MAX (MIN (self->tightening_threshold, self->maximum_size), min);
  max = MAX (lower, self->maximum_size);
  amplitude = max - lower;
  upper = ADW_EASE_OUT_TAN_CUBIC * amplitude + lower;

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

  return adw_ease_out_cubic (progress) * amplitude + lower;
}

static GtkSizeRequestMode
adw_clamp_layout_get_request_mode (GtkLayoutManager *manager,
                                   GtkWidget        *widget)
{
  AdwClampLayout *self = ADW_CLAMP_LAYOUT (manager);

  return self->orientation == GTK_ORIENTATION_HORIZONTAL ?
    GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH :
    GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
adw_clamp_layout_measure (GtkLayoutManager *manager,
                          GtkWidget        *widget,
                          GtkOrientation    orientation,
                          int               for_size,
                          int              *minimum,
                          int              *natural,
                          int              *minimum_baseline,
                          int              *natural_baseline)
{
  AdwClampLayout *self = ADW_CLAMP_LAYOUT (manager);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    int child_size = -1;
    int child_min = 0;
    int child_nat = 0;
    int child_min_baseline = -1;
    int child_nat_baseline = -1;

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
adw_clamp_layout_allocate (GtkLayoutManager *manager,
                           GtkWidget        *widget,
                           int               width,
                           int               height,
                           int               baseline)
{
  AdwClampLayout *self = ADW_CLAMP_LAYOUT (manager);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    GtkAllocation child_allocation;
    int child_maximum = 0, lower_threshold = 0;
    int child_clamped_size;

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
adw_clamp_layout_class_init (AdwClampLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  object_class->get_property = adw_clamp_layout_get_property;
  object_class->set_property = adw_clamp_layout_set_property;

  layout_manager_class->get_request_mode = adw_clamp_layout_get_request_mode;
  layout_manager_class->measure = adw_clamp_layout_measure;
  layout_manager_class->allocate = adw_clamp_layout_allocate;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdwClampLayout:maximum-size: (attributes org.gtk.Property.get=adw_clamp_layout_get_maximum_size org.gtk.Property.set=adw_clamp_layout_set_maximum_size)
   *
   * The maximum size to allocate to the children. It is the width if the
   * layout is horizontal, or the height if it is vertical.
   *
   * Since: 1.0
   */
  props[PROP_MAXIMUM_SIZE] =
      g_param_spec_int ("maximum-size",
                        "Maximum size",
                        "The maximum size allocated to the child",
                        0, G_MAXINT, 600,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwClampLayout:tightening-threshold: (attributes org.gtk.Property.get=adw_clamp_layout_get_tightening_threshold org.gtk.Property.set=adw_clamp_layout_set_tightening_threshold)
   *
   * The size above which the child is clamped.
   *
   * Starting from this size, the layout will tighten its grip on the children,
   * slowly allocating less and less of the available size up to the maximum
   * allocated size. Below that threshold and below the maximum width, the
   * children will be allocated all the available size.
   *
   * If the threshold is greater than the maximum size to allocate to the
   * children, they will be allocated the whole size up to the maximum. If the
   * threshold is lower than the minimum size to allocate to the children, that
   * size will be used as the tightening threshold.
   *
   * Effectively, tightening the grip on a child before it reaches its maximum
   * size makes transitions to and from the maximum size smoother when resizing.
   *
   * Since: 1.0
   */
  props[PROP_TIGHTENING_THRESHOLD] =
      g_param_spec_int ("tightening-threshold",
                        "Tightening threshold",
                        "The size above which the children are clamped",
                        0, G_MAXINT, 400,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_clamp_layout_init (AdwClampLayout *self)
{
  self->maximum_size = 600;
  self->tightening_threshold = 400;
}

/**
 * adw_clamp_layout_new:
 *
 * Creates a new `AdwClampLayout`.
 *
 * Returns: the newly created `AdwClampLayout`
 *
 * Since: 1.0
 */
GtkLayoutManager *
adw_clamp_layout_new (void)
{
  return g_object_new (ADW_TYPE_CLAMP_LAYOUT, NULL);
}

/**
 * adw_clamp_layout_get_maximum_size: (attributes org.gtk.Method.get_property=maximum-size)
 * @self: a `AdwClampLayout`
 *
 * Gets the maximum size allocated to the children.
 *
 * Returns: the maximum size to allocate to the children
 *
 * Since: 1.0
 */
int
adw_clamp_layout_get_maximum_size (AdwClampLayout *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP_LAYOUT (self), 0);

  return self->maximum_size;
}

/**
 * adw_clamp_layout_set_maximum_size: (attributes org.gtk.Method.set_property=maximum-size)
 * @self: a `AdwClampLayout`
 * @maximum_size: the maximum size
 *
 * Sets the maximum size allocated to the children.
 *
 * Since: 1.0
 */
void
adw_clamp_layout_set_maximum_size (AdwClampLayout *self,
                                   int             maximum_size)
{
  g_return_if_fail (ADW_IS_CLAMP_LAYOUT (self));

  if (self->maximum_size == maximum_size)
    return;

  self->maximum_size = maximum_size;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * adw_clamp_layout_get_tightening_threshold: (attributes org.gtk.Method.get_property=tightening-threshold)
 * @self: a `AdwClampLayout`
 *
 * Gets the size above which the children are clamped.
 *
 * Returns: the size above which the children are clamped
 *
 * Since: 1.0
 */
int
adw_clamp_layout_get_tightening_threshold (AdwClampLayout *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP_LAYOUT (self), 0);

  return self->tightening_threshold;
}

/**
 * adw_clamp_layout_set_tightening_threshold: (attributes org.gtk.Method.set_property=tightening-threshold)
 * @self: a `AdwClampLayout`
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size above which the children are clamped.
 *
 * Since: 1.0
 */
void
adw_clamp_layout_set_tightening_threshold (AdwClampLayout *self,
                                           int             tightening_threshold)
{
  g_return_if_fail (ADW_IS_CLAMP_LAYOUT (self));

  if (self->tightening_threshold == tightening_threshold)
    return;

  self->tightening_threshold = tightening_threshold;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}
