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
 * @short_description: A widget constraining its child to a given size.
 * @Title: HdyClamp
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
  PROP_CHILD,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_TIGHTENING_THRESHOLD + 1,
};

struct _HdyClamp
{
  GtkWidget parent_instance;

  GtkWidget *child;
  gint maximum_size;
  gint tightening_threshold;

  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

static void hdy_clamp_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyClamp, hdy_clamp, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         hdy_clamp_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
set_orientation (HdyClamp       *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify (G_OBJECT (self), "orientation");
}

static void
hdy_clamp_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  HdyClamp *self = HDY_CLAMP (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, hdy_clamp_get_child (self));
    break;
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, hdy_clamp_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, hdy_clamp_get_tightening_threshold (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
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
  case PROP_CHILD:
    hdy_clamp_set_child (self, g_value_get_object (value));
    break;
  case PROP_MAXIMUM_SIZE:
    hdy_clamp_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    hdy_clamp_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_clamp_dispose (GObject *object)
{
  HdyClamp *self = HDY_CLAMP (object);

  g_clear_pointer (&self->child, gtk_widget_unparent);

  G_OBJECT_CLASS (hdy_clamp_parent_class)->dispose (object);
}

/**
 * get_child_size:
 * @self: a #HdyClamp
 * @for_size: the size of the clamp
 * @child_minimum: the minimum size reachable by the child, and hence by @self
 * @child_maximum: the maximum size @self will ever allocate its child
 * @lower_threshold: the threshold below which @self will allocate its full size to its child
 * @upper_threshold: the threshold up from which @self will allocate its maximum size to its child
 *
 * Measures the child's extremes, the clamp's thresholds, and returns size to
 * allocate to the child.
 *
 * If the clamp is horizontal, all values are widths, otherwise they are
 * heights.
 */
static gint
get_child_size (HdyClamp *self,
                gint      for_size,
                gint     *child_minimum,
                gint     *child_maximum,
                gint     *lower_threshold,
                gint     *upper_threshold)
{
  gint min = 0, max = 0, lower = 0, upper = 0;
  gdouble amplitude, progress;

  if (!self->child)
    return 0;

  if (gtk_widget_get_visible (self->child))
    gtk_widget_measure (self->child, self->orientation, -1, &min, NULL, NULL, NULL);

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
hdy_clamp_get_request_mode (GtkWidget *widget)
{
  HdyClamp *self = HDY_CLAMP (widget);

  return self->orientation == GTK_ORIENTATION_HORIZONTAL ?
    GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH :
    GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
hdy_clamp_measure (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  HdyClamp *self = HDY_CLAMP (widget);
  gint child_size = -1;

  if (self->child && gtk_widget_get_visible (self->child)) {
    if (self->orientation != orientation)
      child_size = get_child_size (HDY_CLAMP (widget), for_size,
                                   NULL, NULL, NULL, NULL);

    gtk_widget_measure (self->child, orientation,
                        child_size, minimum, natural,
                        minimum_baseline, natural_baseline);

    return;
  }

  if (minimum)
    *minimum = 0;
  if (natural)
    *natural = 0;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
hdy_clamp_size_allocate (GtkWidget *widget,
                         gint       width,
                         gint       height,
                         gint       baseline)
{
  HdyClamp *self = HDY_CLAMP (widget);
  GtkAllocation child_allocation;
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  gint child_maximum = 0, lower_threshold = 0;
  gint child_clamped_size;

  if (!(self->child && gtk_widget_get_visible (self->child))) {
    gtk_style_context_remove_class (context, "small");
    gtk_style_context_remove_class (context, "medium");
    gtk_style_context_remove_class (context, "large");

    return;
  }

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    child_allocation.width = get_child_size (self, width, NULL, &child_maximum, &lower_threshold, NULL);
    child_allocation.height = height;

    child_clamped_size = child_allocation.width;
  }
  else {
    child_allocation.width = width;
    child_allocation.height = get_child_size (self, height, NULL, &child_maximum, &lower_threshold, NULL);

    child_clamped_size = child_allocation.height;
  }

  if (child_clamped_size >= child_maximum) {
    gtk_style_context_remove_class (context, "small");
    gtk_style_context_remove_class (context, "medium");
    gtk_style_context_add_class (context, "large");
  } else if (child_clamped_size <= lower_threshold) {
    gtk_style_context_add_class (context, "small");
    gtk_style_context_remove_class (context, "medium");
    gtk_style_context_remove_class (context, "large");
  } else {
    gtk_style_context_remove_class (context, "small");
    gtk_style_context_add_class (context, "medium");
    gtk_style_context_remove_class (context, "large");
  }

  /* Always center the child on the side of the orientation. */
  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    child_allocation.x = (width - child_allocation.width) / 2;
    child_allocation.y = 0;
  } else {
    child_allocation.x = 0;
    child_allocation.y = (height - child_allocation.height) / 2;
  }

  gtk_widget_size_allocate (self->child, &child_allocation, baseline);
}

static void
hdy_clamp_class_init (HdyClampClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_clamp_get_property;
  object_class->set_property = hdy_clamp_set_property;
  object_class->dispose = hdy_clamp_dispose;

  widget_class->get_request_mode = hdy_clamp_get_request_mode;
  widget_class->measure = hdy_clamp_measure;
  widget_class->size_allocate = hdy_clamp_size_allocate;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  props[PROP_CHILD] =
      g_param_spec_object ("child",
                           _("Child"),
                           _("The child widget"),
                           GTK_TYPE_WIDGET,
                           G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyClamp:maximum-size:
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
   * HdyClamp:tightening-threshold:
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

  gtk_widget_class_set_css_name (widget_class, "clamp");
}

static void
hdy_clamp_init (HdyClamp *self)
{
  self->maximum_size = 600;
  self->tightening_threshold = 400;
}

static void
hdy_clamp_buildable_add_child (GtkBuildable *buildable,
                               GtkBuilder   *builder,
                               GObject      *child,
                               const char   *type)
{
  if (GTK_IS_WIDGET (child))
    hdy_clamp_set_child (HDY_CLAMP (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_clamp_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = hdy_clamp_buildable_add_child;
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
 * hdy_clamp_get_child:
 * @self: a #HdyClamp
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
hdy_clamp_get_child (HdyClamp  *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP (self), NULL);

  return self->child;
}

/**
 * hdy_clamp_set_child:
 * @self: a #HdyClamp
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
hdy_clamp_set_child (HdyClamp  *self,
                     GtkWidget *child)
{
  g_return_if_fail (HDY_IS_CLAMP (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (self->child == child)
    return;

  g_clear_pointer (&self->child, gtk_widget_unparent);

  self->child = child;

  if (child)
    gtk_widget_set_parent (child, GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

/**
 * hdy_clamp_get_maximum_size:
 * @self: a #HdyClamp
 *
 * Gets the maximum size to allocate to the contained child. It is the width if
 * @self is horizontal, or the height if it is vertical.
 *
 * Returns: the maximum width to allocate to the contained child.
 *
 * Since: 1.0
 */
gint
hdy_clamp_get_maximum_size (HdyClamp *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP (self), 0);

  return self->maximum_size;
}

/**
 * hdy_clamp_set_maximum_size:
 * @self: a #HdyClamp
 * @maximum_size: the maximum size
 *
 * Sets the maximum size to allocate to the contained child. It is the width if
 * @self is horizontal, or the height if it is vertical.
 *
 * Since: 1.0
 */
void
hdy_clamp_set_maximum_size (HdyClamp *self,
                            gint      maximum_size)
{
  g_return_if_fail (HDY_IS_CLAMP (self));

  if (self->maximum_size == maximum_size)
    return;

  self->maximum_size = maximum_size;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * hdy_clamp_get_tightening_threshold:
 * @self: a #HdyClamp
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
hdy_clamp_get_tightening_threshold (HdyClamp *self)
{
  g_return_val_if_fail (HDY_IS_CLAMP (self), 0);

  return self->tightening_threshold;
}

/**
 * hdy_clamp_set_tightening_threshold:
 * @self: a #HdyClamp
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size starting from which the clamp will tighten its grip on the
 * child.
 *
 * Since: 1.0
 */
void
hdy_clamp_set_tightening_threshold (HdyClamp *self,
                                    gint      tightening_threshold)
{
  g_return_if_fail (HDY_IS_CLAMP (self));

  if (self->tightening_threshold == tightening_threshold)
    return;

  self->tightening_threshold = tightening_threshold;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}
