#include "adw-demo-transform-layout.h"

#include <glib/gi18n.h>

struct _AdwDemoTransformLayout
{
  GtkLayoutManager parent_instance;

  GskTransform *transform;
};

G_DEFINE_TYPE (AdwDemoTransformLayout, adw_demo_transform_layout, GTK_TYPE_LAYOUT_MANAGER)

enum {
  PROP_0,
  PROP_TRANSFORM,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
adw_demo_transform_layout_measure (GtkLayoutManager *layout,
                                   GtkWidget        *widget,
                                   GtkOrientation    orientation,
                                   gint              for_size,
                                   gint             *minimum,
                                   gint             *natural,
                                   gint             *minimum_baseline,
                                   gint             *natural_baseline)
{
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    gint child_min = 0;
    gint child_nat = 0;
    gint child_min_baseline = -1;
    gint child_nat_baseline = -1;

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
adw_demo_transform_layout_allocate (GtkLayoutManager *layout,
                                    GtkWidget        *widget,
                                    gint              width,
                                    gint              height,
                                    gint              baseline)
{
  AdwDemoTransformLayout *self = ADW_DEMO_TRANSFORM_LAYOUT (layout);
  GtkWidget *child;

  for (child = gtk_widget_get_first_child (widget);
       child != NULL;
       child = gtk_widget_get_next_sibling (child)) {
    GskTransform *transform;

    if (!child || !gtk_widget_should_layout (child))
      continue;

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (width / 2.0f, height / 2.0f));
    transform = gsk_transform_transform (transform, gsk_transform_ref (self->transform));
    transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (-width / 2.0f, -height / 2.0f));

    gtk_widget_allocate (child, width, height, baseline, transform);
  }
}

static void
adw_demo_transform_layout_dispose (GObject *object)
{
  AdwDemoTransformLayout *self = ADW_DEMO_TRANSFORM_LAYOUT (object);

  g_clear_pointer (&self->transform, gsk_transform_unref);

  G_OBJECT_CLASS (adw_demo_transform_layout_parent_class)->dispose (object);
}

static void
adw_demo_transform_layout_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  AdwDemoTransformLayout *self = ADW_DEMO_TRANSFORM_LAYOUT (object);

  switch (prop_id) {
  case PROP_TRANSFORM:
    g_value_set_boxed (value, adw_demo_transform_layout_get_transform (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_transform_layout_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  AdwDemoTransformLayout *self = ADW_DEMO_TRANSFORM_LAYOUT (object);

  switch (prop_id) {
  case PROP_TRANSFORM:
    adw_demo_transform_layout_set_transform (self, g_value_get_boxed (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_transform_layout_class_init (AdwDemoTransformLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkLayoutManagerClass *layout_class = GTK_LAYOUT_MANAGER_CLASS (klass);

  object_class->dispose = adw_demo_transform_layout_dispose;
  object_class->get_property = adw_demo_transform_layout_get_property;
  object_class->set_property = adw_demo_transform_layout_set_property;

  layout_class->measure = adw_demo_transform_layout_measure;
  layout_class->allocate = adw_demo_transform_layout_allocate;

  props[PROP_TRANSFORM] =
      g_param_spec_boxed ("transform",
                          _("Transform"),
                          _("Transform"),
                          GSK_TYPE_TRANSFORM,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_demo_transform_layout_init (AdwDemoTransformLayout *self)
{
}

GskTransform *
adw_demo_transform_layout_get_transform (AdwDemoTransformLayout *self)
{
  g_return_val_if_fail (ADW_IS_DEMO_TRANSFORM_LAYOUT (self), NULL);

  return self->transform;
}

void
adw_demo_transform_layout_set_transform (AdwDemoTransformLayout *self,
                                         GskTransform           *transform)
{
  g_return_if_fail (ADW_IS_DEMO_TRANSFORM_LAYOUT (self));

  if (transform == self->transform)
    return;

  gsk_transform_unref (self->transform);
  self->transform = gsk_transform_ref (transform);

  gtk_layout_manager_layout_changed (GTK_LAYOUT_MANAGER (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSFORM]);
}

void
adw_demo_transform_layout_take_transform (AdwDemoTransformLayout *self,
                                          GskTransform           *transform)
{
  g_return_if_fail (ADW_IS_DEMO_TRANSFORM_LAYOUT (self));

  adw_demo_transform_layout_set_transform (self, transform);
  gsk_transform_unref (transform);
}
