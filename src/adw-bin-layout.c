/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"
#include "adw-bin-layout.h"

/**
 * AdwBinLayout:
 *
 * TODO
 *
 * See also: [class@Bin].
 *
 * Since: 1.7
 */

enum {
  PROP_0,
  PROP_TRANSFORM,
  PROP_TRANSFORM_ORIGIN_X,
  PROP_TRANSFORM_ORIGIN_Y,
  LAST_PROP,
};

struct _AdwBinLayout
{
  GtkLayoutManager parent_instance;

  GskTransform *transform;
  float transform_origin_x;
  float transform_origin_y;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwBinLayout, adw_bin_layout, GTK_TYPE_LAYOUT_MANAGER)

static void
adw_bin_layout_measure (GtkLayoutManager *manager,
                        GtkWidget        *widget,
                        GtkOrientation    orientation,
                        int               for_size,
                        int              *minimum,
                        int              *natural,
                        int              *minimum_baseline,
                        int              *natural_baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    int child_min = 0, child_nat = 0;
    int child_min_baseline = -1, child_nat_baseline = -1;

    if (!gtk_widget_should_layout (child))
      continue;

    gtk_widget_measure (child, orientation, for_size,
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
adw_bin_layout_allocate (GtkLayoutManager *manager,
                         GtkWidget        *widget,
                         int               width,
                         int               height,
                         int               baseline)
{
  AdwBinLayout *self = ADW_BIN_LAYOUT (manager);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    GskTransform *transform = NULL;
    float x, y;

    if (!gtk_widget_should_layout (child))
      continue;

    x = width * self->transform_origin_x;
    y = height * self->transform_origin_y;

    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (x, y));
    transform = gsk_transform_transform (transform, self->transform);
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (-x, -y));

    gtk_widget_allocate (child, width, height, baseline, transform);
  }
}

static void
adw_bin_layout_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  AdwBinLayout *self = ADW_BIN_LAYOUT (object);

  switch (prop_id) {
  case PROP_TRANSFORM:
    g_value_set_boxed (value, adw_bin_layout_get_transform (self));
    break;
  case PROP_TRANSFORM_ORIGIN_X:
    g_value_set_float (value, adw_bin_layout_get_transform_origin_x (self));
    break;
  case PROP_TRANSFORM_ORIGIN_Y:
    g_value_set_float (value, adw_bin_layout_get_transform_origin_y (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_bin_layout_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  AdwBinLayout *self = ADW_BIN_LAYOUT (object);

  switch (prop_id) {
  case PROP_TRANSFORM:
    adw_bin_layout_set_transform (self, g_value_get_boxed (value));
    break;
  case PROP_TRANSFORM_ORIGIN_X:
    adw_bin_layout_set_transform_origin_x (self, g_value_get_float (value));
    break;
  case PROP_TRANSFORM_ORIGIN_Y:
    adw_bin_layout_set_transform_origin_y (self, g_value_get_float (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_bin_layout_dispose (GObject *object)
{
  AdwBinLayout *self = ADW_BIN_LAYOUT (object);

  g_clear_pointer (&self->transform, gsk_transform_unref);

  G_OBJECT_CLASS (adw_bin_layout_parent_class)->dispose (object);
}

static void
adw_bin_layout_class_init (AdwBinLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  object_class->get_property = adw_bin_layout_get_property;
  object_class->set_property = adw_bin_layout_set_property;
  object_class->dispose = adw_bin_layout_dispose;

  layout_manager_class->measure = adw_bin_layout_measure;
  layout_manager_class->allocate = adw_bin_layout_allocate;

  /**
   * AdwBinLayout:transform:
   *
   * The transform to apply to the children.
   *
   * Since: 1.7
   */
  props[PROP_TRANSFORM] =
    g_param_spec_boxed ("transform", NULL, NULL,
                        GSK_TYPE_TRANSFORM,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBinLayout:transform-origin-x:
   *
   * TODO
   *
   * Since: 1.7
   */
  props[PROP_TRANSFORM_ORIGIN_X] =
    g_param_spec_float ("transform-origin-x", NULL, NULL,
                        0.0, 1.0, 0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBinLayout:transform-origin-y:
   *
   * TODO
   *
   * Since: 1.7
   */
  props[PROP_TRANSFORM_ORIGIN_Y] =
    g_param_spec_float ("transform-origin-y", NULL, NULL,
                        0.0, 1.0, 0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_bin_layout_init (AdwBinLayout *self)
{
  self->transform_origin_x = 0.5;
  self->transform_origin_y = 0.5;
}

/**
 * adw_bin_layout_new:
 *
 * Creates a new `AdwBinLayout`.
 *
 * Returns: the newly created `AdwBinLayout`
 *
 * Since: 1.7
 */
GtkLayoutManager *
adw_bin_layout_new (void)
{
  return g_object_new (ADW_TYPE_BIN_LAYOUT, NULL);
}

/**
 * adw_bin_layout_get_transform:
 * @self: a bin
 *
 * Gets the child transform of @self.
 *
 * Returns: (nullable) (transfer none): the transform of @self
 *
 * Since: 1.7
 */
GskTransform *
adw_bin_layout_get_transform (AdwBinLayout *self)
{
  g_return_val_if_fail (ADW_IS_BIN_LAYOUT (self), NULL);

  return self->transform;
}

/**
 * adw_bin_layout_set_transform:
 * @self: a bin layout
 * @transform: (nullable): the child transform
 *
 * Sets the transform to apply to the children.
 *
 * Since: 1.7
 */
void
adw_bin_layout_set_transform (AdwBinLayout *self,
                              GskTransform *transform)
{
  g_return_if_fail (ADW_IS_BIN_LAYOUT (self));

  if (transform == self->transform)
    return;

  if (self->transform)
    gsk_transform_unref (self->transform);

  self->transform = transform;

  if (self->transform)
    gsk_transform_ref (self->transform);

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSFORM]);
}

/**
 * adw_bin_layout_get_transform_origin_x:
 * @self: a bin
 *
 * TODO
 *
 * Returns: the TODO of @self
 *
 * Since: 1.7
 */
float
adw_bin_layout_get_transform_origin_x (AdwBinLayout *self)
{
  g_return_val_if_fail (ADW_IS_BIN_LAYOUT (self), 0.0f);

  return self->transform_origin_x;
}

/**
 * adw_bin_layout_set_transform_origin_x:
 * @self: a bin layout
 * @origin: the TODO
 *
 * Sets the TODO of @self.
 *
 * Since: 1.7
 */
void
adw_bin_layout_set_transform_origin_x (AdwBinLayout *self,
                                       float         origin)
{
  g_return_if_fail (ADW_IS_BIN_LAYOUT (self));

  origin = CLAMP (origin, 0.0, 1.0);

  if (G_APPROX_VALUE (origin, self->transform_origin_x, FLT_EPSILON))
    return;

  self->transform_origin_x = origin;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSFORM_ORIGIN_X]);
}

/**
 * adw_bin_layout_get_transform_origin_y:
 * @self: a bin
 *
 * TODO
 *
 * Returns: the TODO of @self
 *
 * Since: 1.7
 */
float
adw_bin_layout_get_transform_origin_y (AdwBinLayout *self)
{
  g_return_val_if_fail (ADW_IS_BIN_LAYOUT (self), 0.0f);

  return self->transform_origin_y;
}

/**
 * adw_bin_layout_set_transform_origin_y:
 * @self: a bin layout
 * @origin: the TODO
 *
 * Sets the TODO of @self.
 *
 * Since: 1.7
 */
void
adw_bin_layout_set_transform_origin_y (AdwBinLayout *self,
                                       float         origin)
{
  g_return_if_fail (ADW_IS_BIN_LAYOUT (self));

  origin = CLAMP (origin, 0.0, 1.0);

  if (G_APPROX_VALUE (origin, self->transform_origin_y, FLT_EPSILON))
    return;

  self->transform_origin_y = origin;

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSFORM_ORIGIN_Y]);
}
