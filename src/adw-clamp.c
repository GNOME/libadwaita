/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include "adw-clamp.h"

#include <glib/gi18n-lib.h>

#include "adw-clamp-layout.h"

/**
 * SECTION:adw-clamp
 * @short_description: A widget constraining its child to a given size.
 * @Title: AdwClamp
 *
 * The #AdwClamp widget constraints the size of the widget it contains to a
 * given maximum size. It will constrain the width if it is horizontal, or the
 * height if it is vertical. The expansion of the child from its minimum to its
 * maximum size is eased out for a smooth transition.
 *
 * If the child requires more than the requested maximum size, it will be
 * allocated the minimum size it can fit in instead.
 *
 * # CSS nodes
 *
 * #AdwClamp has a single CSS node with name clamp. The node will get the style
 * classes .large when its child reached its maximum size, .small when the clamp
 * allocates its full size to its child, .medium in-between, or none if it
 * didn't compute its size yet.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_CHILD,
  PROP_MAXIMUM_SIZE,
  PROP_TIGHTENING_THRESHOLD,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_TIGHTENING_THRESHOLD + 1,
};

struct _AdwClamp
{
  GtkWidget parent_instance;

  GtkWidget *child;
  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

static void adw_clamp_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwClamp, adw_clamp, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                         adw_clamp_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
set_orientation (AdwClamp       *self,
                 GtkOrientation  orientation)
{
  GtkLayoutManager *layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));

  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  gtk_orientable_set_orientation (GTK_ORIENTABLE (layout), orientation);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adw_clamp_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  AdwClamp *self = ADW_CLAMP (object);

  switch (prop_id) {
  case PROP_CHILD:
    g_value_set_object (value, adw_clamp_get_child (self));
    break;
  case PROP_MAXIMUM_SIZE:
    g_value_set_int (value, adw_clamp_get_maximum_size (self));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    g_value_set_int (value, adw_clamp_get_tightening_threshold (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_clamp_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  AdwClamp *self = ADW_CLAMP (object);

  switch (prop_id) {
  case PROP_CHILD:
    adw_clamp_set_child (self, g_value_get_object (value));
    break;
  case PROP_MAXIMUM_SIZE:
    adw_clamp_set_maximum_size (self, g_value_get_int (value));
    break;
  case PROP_TIGHTENING_THRESHOLD:
    adw_clamp_set_tightening_threshold (self, g_value_get_int (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_clamp_dispose (GObject *object)
{
  AdwClamp *self = ADW_CLAMP (object);

  g_clear_pointer (&self->child, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_clamp_parent_class)->dispose (object);
}

static void
adw_clamp_class_init (AdwClampClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_clamp_get_property;
  object_class->set_property = adw_clamp_set_property;
  object_class->dispose = adw_clamp_dispose;

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
   * AdwClamp:maximum-size:
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
   * AdwClamp:tightening-threshold:
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

  gtk_widget_class_set_layout_manager_type (widget_class, ADW_TYPE_CLAMP_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "clamp");
}

static void
adw_clamp_init (AdwClamp *self)
{
}

static void
adw_clamp_buildable_add_child (GtkBuildable *buildable,
                               GtkBuilder   *builder,
                               GObject      *child,
                               const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_clamp_set_child (ADW_CLAMP (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_clamp_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_clamp_buildable_add_child;
}

/**
 * adw_clamp_new:
 *
 * Creates a new #AdwClamp.
 *
 * Returns: a new #AdwClamp
 *
 * Since: 1.0
 */
GtkWidget *
adw_clamp_new (void)
{
  return g_object_new (ADW_TYPE_CLAMP, NULL);
}

/**
 * adw_clamp_get_child:
 * @self: a #AdwClamp
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 */
GtkWidget *
adw_clamp_get_child (AdwClamp  *self)
{
  g_return_val_if_fail (ADW_IS_CLAMP (self), NULL);

  return self->child;
}

/**
 * adw_clamp_set_child:
 * @self: a #AdwClamp
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 */
void
adw_clamp_set_child (AdwClamp  *self,
                     GtkWidget *child)
{
  g_return_if_fail (ADW_IS_CLAMP (self));
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
 * adw_clamp_get_maximum_size:
 * @self: a #AdwClamp
 *
 * Gets the maximum size to allocate to the contained child. It is the width if
 * @self is horizontal, or the height if it is vertical.
 *
 * Returns: the maximum width to allocate to the contained child.
 *
 * Since: 1.0
 */
gint
adw_clamp_get_maximum_size (AdwClamp *self)
{
  AdwClampLayout *layout;

  g_return_val_if_fail (ADW_IS_CLAMP (self), 0);

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_clamp_layout_get_maximum_size (layout);
}

/**
 * adw_clamp_set_maximum_size:
 * @self: a #AdwClamp
 * @maximum_size: the maximum size
 *
 * Sets the maximum size to allocate to the contained child. It is the width if
 * @self is horizontal, or the height if it is vertical.
 *
 * Since: 1.0
 */
void
adw_clamp_set_maximum_size (AdwClamp *self,
                            gint      maximum_size)
{
  AdwClampLayout *layout;

  g_return_if_fail (ADW_IS_CLAMP (self));

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adw_clamp_layout_get_maximum_size (layout) == maximum_size)
    return;

  adw_clamp_layout_set_maximum_size (layout, maximum_size);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MAXIMUM_SIZE]);
}

/**
 * adw_clamp_get_tightening_threshold:
 * @self: a #AdwClamp
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
adw_clamp_get_tightening_threshold (AdwClamp *self)
{
  AdwClampLayout *layout;

  g_return_val_if_fail (ADW_IS_CLAMP (self), 0);

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_clamp_layout_get_tightening_threshold (layout);
}

/**
 * adw_clamp_set_tightening_threshold:
 * @self: a #AdwClamp
 * @tightening_threshold: the tightening threshold
 *
 * Sets the size starting from which the clamp will tighten its grip on the
 * child.
 *
 * Since: 1.0
 */
void
adw_clamp_set_tightening_threshold (AdwClamp *self,
                                    gint      tightening_threshold)
{
  AdwClampLayout *layout;

  g_return_if_fail (ADW_IS_CLAMP (self));

  layout = ADW_CLAMP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (adw_clamp_layout_get_tightening_threshold (layout) == tightening_threshold)
    return;

  adw_clamp_layout_set_tightening_threshold (layout, tightening_threshold);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIGHTENING_THRESHOLD]);
}
