/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "hdy-clamp.h"

#include <glib/gi18n-lib.h>
#include <math.h>

#include "hdy-animation-private.h"

/**
 * SECTION:hdy-clamp
 * @short_description: A container letting its child grow up to a given width.
 * @Title: HdyClamp
 *
 * The #HdyClamp widget limits the size of the widget it contains to a given
 * maximum width. The expansion of the child from its minimum to its maximum
 * size is eased out for a smooth transition.
 *
 * If the child requires more than the requested maximum width, it will be
 * allocated the minimum width it can fit in instead.
 *
 * # CSS nodes
 *
 * #HdyClamp has a single CSS node with name clamp. The node will get the style
 * classes .wide when its child reached its maximum width, .narrow when the
 * clamp allocates its full width to its child, .medium in-between, or none if
 * it didn't compute its size yet.
 *
 * Since: 1.0
 */

#define HDY_EASE_OUT_TAN_CUBIC 3

enum {
  PROP_0,
  PROP_MAXIMUM_WIDTH,
  PROP_LINEAR_GROWTH_WIDTH,
  LAST_PROP,
};

struct _HdyClamp
{
  GtkBin parent_instance;

  gint maximum_width;
  gint linear_growth_width;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (HdyClamp, hdy_clamp, GTK_TYPE_BIN)

static void
hdy_clamp_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  HdyClamp *self = HDY_CLAMP (object);

  switch (prop_id) {
  case PROP_MAXIMUM_WIDTH:
    g_value_set_int (value, hdy_clamp_get_maximum_width (self));
    break;
  case PROP_LINEAR_GROWTH_WIDTH:
    g_value_set_int (value, hdy_clamp_get_linear_growth_width (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_clamp_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  HdyClamp *self = HDY_CLAMP (object);

  switch (prop_id) {
  case PROP_MAXIMUM_WIDTH:
    hdy_clamp_set_maximum_width (self, g_value_get_int (value));
    break;
  case PROP_LINEAR_GROWTH_WIDTH:
    hdy_clamp_set_linear_growth_width (self, g_value_get_int (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

/**
 * get_child_width:
 * @self: a #HdyClamp
 * @for_width: the width of the clamp
 * @child_minimum: the minimum width reachable by the child, and hence by @self
 * @child_maximum: the maximum width @self will ever allocate its child
 * @lower_threshold: the threshold below which @self will allocate its full width to its child
 * @upper_threshold: the threshold up from which @self will allocate its maximum size to its child
 *
 * Measures the child's extremes, the clamp's thresholds, and returns size to
 * allocate to the child.
 */
static gint
get_child_width (HdyClamp *self,
                 gint      for_width,
                 gint     *child_minimum,
                 gint     *child_maximum,
                 gint     *lower_threshold,
                 gint     *upper_threshold)
{
  GtkBin *bin = GTK_BIN (self);
  GtkWidget *child;
  gint min = 0, max = 0, lower = 0, upper = 0;
  gdouble amplitude, progress;

  child = gtk_bin_get_child (bin);
  if (child == NULL)
    return 0;

  if (gtk_widget_get_visible (child))
    gtk_widget_get_preferred_width (child, &min, NULL);

  lower = MAX (MIN (self->linear_growth_width, self->maximum_width), min);
  max = MAX (lower, self->maximum_width);
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

  if (for_width < 0)
    return 0;

  if (for_width <= lower)
    return for_width;

  if (for_width >= upper)
    return max;

  progress = (double) (for_width - lower) / (double) (upper - lower);

  return hdy_ease_out_cubic (progress) * amplitude + lower;
}

/* This private method is prefixed by the call name because it will be a virtual
 * method in GTK 4.
 */
static void
hdy_clamp_measure (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  GtkBin *bin = GTK_BIN (widget);
  GtkWidget *child;

  if (minimum)
    *minimum = 0;
  if (natural)
    *natural = 0;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;

  child = gtk_bin_get_child (bin);
  if (!(child && gtk_widget_get_visible (child)))
    return;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_get_preferred_width (child, minimum, natural);
  else {
    gint child_width = get_child_width (HDY_CLAMP (widget), for_size, NULL, NULL, NULL, NULL);

    gtk_widget_get_preferred_height_and_baseline_for_width (child,
                                                            child_width,
                                                            minimum,
                                                            natural,
                                                            minimum_baseline,
                                                            natural_baseline);
  }
}

static void
hdy_clamp_get_preferred_width (GtkWidget *widget,
                               gint      *minimum,
                               gint      *natural)
{
  hdy_clamp_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                     minimum, natural, NULL, NULL);
}

static void
hdy_clamp_get_preferred_height_and_baseline_for_width (GtkWidget *widget,
                                                       gint       width,
                                                       gint      *minimum,
                                                       gint      *natural,
                                                       gint      *minimum_baseline,
                                                       gint      *natural_baseline)
{
  hdy_clamp_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                     minimum, natural, minimum_baseline, natural_baseline);
}

static void
hdy_clamp_get_preferred_height (GtkWidget *widget,
                                gint      *minimum,
                                gint      *natural)
{
  hdy_clamp_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                     minimum, natural, NULL, NULL);
}

static void
hdy_clamp_size_allocate (GtkWidget     *widget,
                         GtkAllocation *allocation)
{
  HdyClamp *self = HDY_CLAMP (widget);
  GtkBin *bin = GTK_BIN (widget);
  GtkAllocation child_allocation;
  gint baseline;
  GtkWidget *child;
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  gint child_maximum = 0, lower_threshold = 0;

  gtk_widget_set_allocation (widget, allocation);

  child = gtk_bin_get_child (bin);
  if (!(child && gtk_widget_get_visible (child))) {
    gtk_style_context_remove_class (context, "narrow");
    gtk_style_context_remove_class (context, "medium");
    gtk_style_context_remove_class (context, "wide");

    return;
  }

  child_allocation.width = get_child_width (self, allocation->width, NULL, &child_maximum, &lower_threshold, NULL);
  child_allocation.height = allocation->height;

  if (child_allocation.width >= child_maximum) {
    gtk_style_context_remove_class (context, "narrow");
    gtk_style_context_remove_class (context, "medium");
    gtk_style_context_add_class (context, "wide");
  } else if (child_allocation.width <= lower_threshold) {
    gtk_style_context_add_class (context, "narrow");
    gtk_style_context_remove_class (context, "medium");
    gtk_style_context_remove_class (context, "wide");
  } else {
    gtk_style_context_remove_class (context, "narrow");
    gtk_style_context_add_class (context, "medium");
    gtk_style_context_remove_class (context, "wide");
  }

  if (!gtk_widget_get_has_window (widget)) {
    /* This allways center the child vertically. */
    child_allocation.x = allocation->x + (allocation->width - child_allocation.width) / 2;
    child_allocation.y = allocation->y;
  }
  else {
    child_allocation.x = 0;
    child_allocation.y = 0;
  }

  baseline = gtk_widget_get_allocated_baseline (widget);
  gtk_widget_size_allocate_with_baseline (child, &child_allocation, baseline);
}

static void
hdy_clamp_class_init (HdyClampClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_clamp_get_property;
  object_class->set_property = hdy_clamp_set_property;

  widget_class->get_preferred_width = hdy_clamp_get_preferred_width;
  widget_class->get_preferred_height = hdy_clamp_get_preferred_height;
  widget_class->get_preferred_height_and_baseline_for_width = hdy_clamp_get_preferred_height_and_baseline_for_width;
  widget_class->size_allocate = hdy_clamp_size_allocate;

  gtk_container_class_handle_border_width (container_class);

  /**
   * HdyClamp:maximum_width:
   *
   * The maximum width to allocate to the child.
   *
   * Since: 1.0
   */
  props[PROP_MAXIMUM_WIDTH] =
      g_param_spec_int ("maximum-width",
                        _("Maximum width"),
                        _("The maximum width allocated to the child"),
                        0, G_MAXINT, 0,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyClamp:linear_growth_width:
   *
   * The width up to which the child will be allocated all the width.
   *
   * Since: 1.0
   */
  props[PROP_LINEAR_GROWTH_WIDTH] =
      g_param_spec_int ("linear-growth-width",
                        _("Linear growth width"),
                        _("The width up to which the child will be allocated all the width"),
                        0, G_MAXINT, 0,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "clamp");
}

static void
hdy_clamp_init (HdyClamp *self)
{
}

/**
 * hdy_clamp_new:
 *
 * Creates a new #HdyClamp.
 *
 * Returns: a new #HdyClamp
 *
 * Since: 1.0
 */
GtkWidget *
hdy_clamp_new (void)
{
  return g_object_new (HDY_TYPE_CLAMP, NULL);
}

/**
 * hdy_clamp_get_maximum_width:
 * @self: a #HdyClamp
 *
 * Gets the maximum width to allocate to the contained child.
 *
 * Returns: the maximum width to allocate to the contained child.
 *
 * Since: 1.0
 */
gint
hdy_clamp_get_maximum_width (HdyClamp *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP (self), 0);

  return self->maximum_width;
}

/**
 * hdy_clamp_set_maximum_width:
 * @self: a #HdyClamp
 * @maximum_width: the maximum width
 *
 * Sets the maximum width to allocate to the contained child.
 *
 * Since: 1.0
 */
void
hdy_clamp_set_maximum_width (HdyClamp *self,
                             gint      maximum_width)
{
  g_return_if_fail (HDY_IS_CLAMP (self));

  if (self->maximum_width == maximum_width)
    return;

  self->maximum_width = maximum_width;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_WIDTH]);
}

/**
 * hdy_clamp_get_linear_growth_width:
 * @self: a #HdyClamp
 *
 * Gets the width up to which the child will be allocated all the available
 * width and starting from which it will be allocated a portion of the available
 * width. In bith cases the allocated width won't exceed the declared maximum.
 *
 * Returns: the width up to which the child will be allocated all the available
 * width.
 *
 * Since: 1.0
 */
gint
hdy_clamp_get_linear_growth_width (HdyClamp *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP (self), 0);

  return self->linear_growth_width;
}

/**
 * hdy_clamp_set_linear_growth_width:
 * @self: a #HdyClamp
 * @linear_growth_width: the linear growth width
 *
 * Sets the width up to which the child will be allocated all the available
 * width and starting from which it will be allocated a portion of the available
 * width. In bith cases the allocated width won't exceed the declared maximum.
 *
 * Since: 1.0
 */
void
hdy_clamp_set_linear_growth_width (HdyClamp *self,
                                   gint      linear_growth_width)
{
  g_return_if_fail (HDY_IS_CLAMP (self));

  if (self->linear_growth_width == linear_growth_width)
    return;

  self->linear_growth_width = linear_growth_width;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINEAR_GROWTH_WIDTH]);
}
