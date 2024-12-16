/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-clamp-layout.h"

#include <math.h>

#include "adw-animation-util.h"
#include "adw-easing.h"
#include "adw-enums.h"
#include "adw-length-unit.h"

/**
 * AdwClampLayout:
 *
 * A layout manager constraining its children to a given size.
 *
 * <picture>
 *   <source srcset="clamp-wide-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="clamp-wide.png" alt="clamp-wide">
 * </picture>
 * <picture>
 *   <source srcset="clamp-narrow-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="clamp-narrow.png" alt="clamp-narrow">
 * </picture>
 *
 * `AdwClampLayout` constraints the size of the widgets it contains to a given
 * maximum size. It will constrain the width if it is horizontal, or the height
 * if it is vertical. The expansion of the children from their minimum to their
 * maximum size is eased out for a smooth transition.
 *
 * If a child requires more than the requested maximum size, it will be
 * allocated the minimum size it can fit in instead.
 *
 * `AdwClampLayout` can scale with the text scale factor, use the
 * [property@ClampLayout:unit] property to enable that behavior.
 *
 * See also: [class@Clamp], [class@ClampScrollable].
 */

#define ADW_EASE_OUT_TAN_CUBIC 3

enum {
  PROP_0,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,
  PROP_UNIT,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_UNIT + 1,
};

struct _AdwClampLayout
{
  GtkLayoutManager parent_instance;

  int maximum_size;
  int tightening_threshold;
  AdwLengthUnit unit;

  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwClampLayout, adw_clamp_layout, GTK_TYPE_LAYOUT_MANAGER,
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
  case PROP_UNIT:
    g_value_set_enum (value, adw_clamp_layout_get_unit (self));
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
  case PROP_UNIT:
    adw_clamp_layout_set_unit (self, g_value_get_enum (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

/* Given adw_lerp (a, b, t) -> r, find t. In other words: we know what sizes
 * we're interpolating between and the current size; find the current
 * interpolation progress.
 *
 * Note that this is different from the similarly named inverse_lerp function
 * in adw-view-stack.c. That one finds b given a, r, and t, while this one
 * finds t given a, b, and r.
 */
static inline double
inverse_lerp (double a,
              double b,
              double r)
{
  return (r - a) / (b - a);
}

static int
clamp_size_from_child (AdwClampLayout *self,
                       GtkSettings    *settings,
                       GtkWidget      *child,
                       int             child_size)
{
  double max, lower, upper, progress, maximum_size, tightening_threshold;
  int min, nat;

  maximum_size = adw_length_unit_to_px (self->unit,
                                        self->maximum_size,
                                        settings);
  tightening_threshold = adw_length_unit_to_px (self->unit,
                                                self->tightening_threshold,
                                                settings);

  gtk_widget_measure (child, self->orientation, -1, &min, &nat, NULL, NULL);

  lower = MAX (MIN (tightening_threshold, maximum_size), min);
  max = MAX (lower, maximum_size);
  upper = lower + ADW_EASE_OUT_TAN_CUBIC * (max - lower);

  g_assert (child_size != -1);

  if (child_size <= lower)
    return child_size;
  else if (child_size >= max)
    return upper;
  else {
    double ease = inverse_lerp (lower, max, child_size);

    progress = 1 + cbrt (ease - 1); // inverse ease out cubic
  }

  return ceil (adw_lerp (lower, upper, progress));
}

static int
child_size_from_clamp (AdwClampLayout *self,
                       GtkSettings    *settings,
                       GtkWidget      *child,
                       int             for_size,
                       int            *child_maximum,
                       int            *lower_threshold)
{
  double lower, upper, max, progress, maximum_size, tightening_threshold;
  int min = 0, nat = 0;

  maximum_size = adw_length_unit_to_px (self->unit,
                                        self->maximum_size,
                                        settings);
  tightening_threshold = adw_length_unit_to_px (self->unit,
                                                self->tightening_threshold,
                                                settings);

  gtk_widget_measure (child, self->orientation, -1, &min, &nat, NULL, NULL);

  lower = MAX (MIN (tightening_threshold, maximum_size), min);
  max = MAX (lower, maximum_size);
  upper = lower + ADW_EASE_OUT_TAN_CUBIC * (max - lower);

  if (child_maximum)
    *child_maximum = ceil (max);
  if (lower_threshold)
    *lower_threshold = ceil (lower);

  if (for_size < 0)
    return MIN (nat, ceil (max));

  if (for_size <= lower)
    return for_size;

  if (for_size >= upper)
    return ceil (max);

  progress = inverse_lerp (lower, upper, for_size);

  return floor (adw_lerp (lower, max, adw_easing_ease (ADW_EASE_OUT_CUBIC, progress)));
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
  GtkSettings *settings = gtk_widget_get_settings (widget);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    int clamp_min = 0;
    int clamp_nat = 0;
    int child_min = 0;
    int child_nat = 0;
    int min_baseline = -1;
    int nat_baseline = -1;

    if (!gtk_widget_should_layout (child))
      continue;

    if (self->orientation == orientation) {
      /* We pass all of for_size to the child, but we have to post-process
       * its measurements.
       */
      gtk_widget_measure (child, orientation, for_size,
                          &child_min, &child_nat,
                          &min_baseline, &nat_baseline);

      clamp_min = clamp_size_from_child (self, settings, child, child_min);
      clamp_nat = clamp_size_from_child (self, settings, child, child_nat);
      if (min_baseline > -1)
        min_baseline += (clamp_min - child_min) / 2;
      if (nat_baseline > -1)
        nat_baseline += (clamp_nat - child_nat) / 2;
    } else {
      /* We clamp for_size, but after that, child's minimum and natural
       * sizes in this orientation are what we report.
       */
      int child_maximum = 0;
      int child_size = child_size_from_clamp (self, settings, child,
                                              for_size, &child_maximum,
                                              NULL);

      gtk_widget_measure (child, orientation, child_size,
                          &child_min, &child_nat,
                          &min_baseline, &nat_baseline);

      if (for_size == -1 && child_size < child_maximum) {
        /* For any specific for_size, we have a definite child_size that
         * we measure the child for. When passed for_size = -1 however,
         * we're supposed to report overall minimum size, for any
         * potential value of for_size. For that, we have to measure the
         * child at the maximum size we could give it. Note that we
         * should not use this measurement for the natural size.
         */
        gtk_widget_measure (child, orientation, child_maximum,
                            &child_min, NULL,
                            &min_baseline, NULL);
      }

      clamp_min = child_min;
      clamp_nat = child_nat;
    }

    *minimum = MAX (*minimum, clamp_min);
    *natural = MAX (*natural, clamp_nat);

    if (min_baseline > -1)
      *minimum_baseline = MAX (*minimum_baseline, min_baseline);
    if (nat_baseline > -1)
      *natural_baseline = MAX (*natural_baseline, nat_baseline);
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
  GtkSettings *settings = gtk_widget_get_settings (widget);
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

      continue;
    }

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      child_allocation.width = child_size_from_clamp (self, settings,
                                                      child, width,
                                                      &child_maximum,
                                                      &lower_threshold);
      child_allocation.height = height;

      child_clamped_size = child_allocation.width;
    } else {
      child_allocation.width = width;
      child_allocation.height = child_size_from_clamp (self, settings,
                                                       child, height,
                                                       &child_maximum,
                                                       &lower_threshold);

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

  layout_manager_class->measure = adw_clamp_layout_measure;
  layout_manager_class->allocate = adw_clamp_layout_allocate;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdwClampLayout:maximum-size:
   *
   * The maximum size to allocate to the children.
   *
   * It is the width if the layout is horizontal, or the height if it is
   * vertical.
   */
  props[PROP_MAXIMUM_SIZE] =
    g_param_spec_int ("maximum-size", NULL, NULL,
                      0, G_MAXINT, 600,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwClampLayout:tightening-threshold:
   *
   * The size above which the children are clamped.
   *
   * Starting from this size, the layout will tighten its grip on the children,
   * slowly allocating less and less of the available size up to the maximum
   * allocated size. Below that threshold and below the maximum size, the
   * children will be allocated all the available size.
   *
   * If the threshold is greater than the maximum size to allocate to the
   * children, they will be allocated the whole size up to the maximum. If the
   * threshold is lower than the minimum size to allocate to the children, that
   * size will be used as the tightening threshold.
   *
   * Effectively, tightening the grip on a child before it reaches its maximum
   * size makes transitions to and from the maximum size smoother when resizing.
   */
  props[PROP_TIGHTENING_THRESHOLD] =
    g_param_spec_int ("tightening-threshold", NULL, NULL,
                      0, G_MAXINT, 400,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwClampLayout:unit:
   *
   * The length unit for maximum size and tightening threshold.
   *
   * Allows the sizes to vary depending on the text scale factor.
   *
   * Since: 1.4
   */
  props[PROP_UNIT] =
    g_param_spec_enum ("unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_SP,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_clamp_layout_init (AdwClampLayout *self)
{
  self->maximum_size = 600;
  self->tightening_threshold = 400;
  self->unit = ADW_LENGTH_UNIT_SP;
}

/**
 * adw_clamp_layout_new:
 *
 * Creates a new `AdwClampLayout`.
 *
 * Returns: the newly created `AdwClampLayout`
 */
GtkLayoutManager *
adw_clamp_layout_new (void)
{
  return g_object_new (ADW_TYPE_CLAMP_LAYOUT, NULL);
}

/**
 * adw_clamp_layout_get_maximum_size:
 * @self: a clamp layout
 *
 * Gets the maximum size allocated to the children.
 *
 * Returns: the maximum size to allocate to the children
 */
int
adw_clamp_layout_get_maximum_size (AdwClampLayout *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP_LAYOUT (self), 0);

  return self->maximum_size;
}

/**
 * adw_clamp_layout_set_maximum_size:
 * @self: a clamp layout
 * @maximum_size: the maximum size
 *
 * Sets the maximum size allocated to the children.
 *
 * It is the width if the layout is horizontal, or the height if it is vertical.
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
 * adw_clamp_layout_get_tightening_threshold:
 * @self: a clamp layout
 *
 * Gets the size above which the children are clamped.
 *
 * Returns: the size above which the children are clamped
 */
int
adw_clamp_layout_get_tightening_threshold (AdwClampLayout *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP_LAYOUT (self), 0);

  return self->tightening_threshold;
}

/**
 * adw_clamp_layout_set_tightening_threshold:
 * @self: a clamp layout
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size above which the children are clamped.
 *
 * Starting from this size, the layout will tighten its grip on the children,
 * slowly allocating less and less of the available size up to the maximum
 * allocated size. Below that threshold and below the maximum size, the children
 * will be allocated all the available size.
 *
 * If the threshold is greater than the maximum size to allocate to the
 * children, they will be allocated the whole size up to the maximum. If the
 * threshold is lower than the minimum size to allocate to the children, that
 * size will be used as the tightening threshold.
 *
 * Effectively, tightening the grip on a child before it reaches its maximum
 * size makes transitions to and from the maximum size smoother when resizing.
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

/**
 * adw_clamp_layout_get_unit:
 * @self: a clamp layout
 *
 * Gets the length unit for maximum size and tightening threshold.
 *
 * Returns: the length unit
 *
 * Since: 1.4
 */
AdwLengthUnit
adw_clamp_layout_get_unit (AdwClampLayout *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP_LAYOUT (self), ADW_LENGTH_UNIT_PX);

  return self->unit;
}

/**
 * adw_clamp_layout_set_unit:
 * @self: a clamp layout
 * @unit: the length unit
 *
 * Sets the length unit for maximum size and tightening threshold.
 *
 * Allows the sizes to vary depending on the text scale factor.
 *
 * Since: 1.4
 */
void
adw_clamp_layout_set_unit (AdwClampLayout *self,
                           AdwLengthUnit   unit)
{
  g_return_if_fail (ADW_IS_CLAMP_LAYOUT (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  if (unit == self->unit)
    return;

  self->unit = unit;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UNIT]);
}
