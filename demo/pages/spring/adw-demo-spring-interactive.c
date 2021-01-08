#include "adw-demo-spring-interactive.h"

#include <glib/gi18n.h>

#include "adw-demo-transform-layout.h"
#include "adw-spring-animation-private.h"

struct _AdwDemoSpringInteractive
{
  AdwBin parent_instance;

  gdouble damping;
  gdouble mass;
  gdouble stiffness;
  gdouble precision;

  AdwDemoTransformLayout *layout;
  GtkWidget *handle;
  GtkGesture *drag_gesture;
  GtkGesture *swipe_gesture;
  GdkCursor *grab_cursor;
  GdkCursor *grabbing_cursor;

  AdwSpringAnimation *animation_x;
  AdwSpringAnimation *animation_y;

  gdouble start_x;
  gdouble start_y;
  gdouble last_x;
  gdouble last_y;
};

G_DEFINE_TYPE (AdwDemoSpringInteractive, adw_demo_spring_interactive, ADW_TYPE_BIN)

enum {
  PROP_0,
  PROP_DAMPING,
  PROP_MASS,
  PROP_STIFFNESS,
  PROP_PRECISION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
set_translation (AdwDemoSpringInteractive *self,
                 gdouble                   x,
                 gdouble                   y)
{
  self->last_x = x;
  self->last_y = y;

  adw_demo_transform_layout_take_transform (self->layout,
                                            gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, y)));
}

static void
x_value_cb (gdouble                   value,
            AdwDemoSpringInteractive *self)
{
  set_translation (self, value, self->last_y);
}

static void
x_done_cb (AdwDemoSpringInteractive *self)
{
  g_clear_pointer (&self->animation_x, adw_spring_animation_unref);
}

static void
y_value_cb (gdouble                   value,
            AdwDemoSpringInteractive *self)
{
  set_translation (self, self->last_x, value);
}

static void
y_done_cb (AdwDemoSpringInteractive *self)
{
  g_clear_pointer (&self->animation_y, adw_spring_animation_unref);
}

static void
animate (AdwDemoSpringInteractive *self,
         gdouble                   velocity_x,
         gdouble                   velocity_y)
{
  gtk_widget_set_cursor (self->handle,
                         self->grab_cursor);

  self->animation_x = adw_spring_animation_new (self->handle,
                                                self->last_x,
                                                0,
                                                velocity_x,
                                                self->damping,
                                                self->mass,
                                                self->stiffness,
                                                self->precision,
                                                (AdwAnimationValueCallback) x_value_cb,
                                                (AdwAnimationDoneCallback) x_done_cb,
                                                self);

  self->animation_y = adw_spring_animation_new (self->handle,
                                                self->last_y,
                                                0,
                                                velocity_y,
                                                self->damping,
                                                self->mass,
                                                self->stiffness,
                                                self->precision,
                                                (AdwAnimationValueCallback) y_value_cb,
                                                (AdwAnimationDoneCallback) y_done_cb,
                                                self);

  adw_spring_animation_start (self->animation_x);
  adw_spring_animation_start (self->animation_y);
}

static void
drag_begin_cb (AdwDemoSpringInteractive *self,
               gdouble                   start_x,
               gdouble                   start_y)
{
  if (gtk_widget_pick (GTK_WIDGET (self), start_x, start_y, GTK_PICK_DEFAULT) != self->handle) {
    gtk_gesture_set_state (self->drag_gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  gtk_gesture_set_state (self->drag_gesture, GTK_EVENT_SEQUENCE_CLAIMED);

  self->start_x = self->last_x;
  self->start_y = self->last_y;

  if (self->animation_x)
    adw_spring_animation_stop (self->animation_x);

  if (self->animation_y)
    adw_spring_animation_stop (self->animation_y);

  set_translation (self, self->start_x, self->start_y);

  gtk_widget_set_cursor (self->handle,
                         self->grabbing_cursor);
}

static void
drag_update_cb (AdwDemoSpringInteractive *self,
                gdouble                   offset_x,
                gdouble                   offset_y)
{
  set_translation (self, offset_x + self->start_x, offset_y + self->start_y);
}

static void
drag_cancel_cb (AdwDemoSpringInteractive *self)
{
  if (self->animation_x)
    adw_spring_animation_stop (self->animation_x);

  if (self->animation_y)
    adw_spring_animation_stop (self->animation_y);

  animate (self, 0, 0);
}

static void
adw_demo_spring_interactive_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  AdwDemoSpringInteractive *self = ADW_DEMO_SPRING_INTERACTIVE (object);

  switch (prop_id) {
  case PROP_DAMPING:
    g_value_set_double (value, self->damping);
    break;
  case PROP_MASS:
    g_value_set_double (value, self->mass);
    break;
  case PROP_STIFFNESS:
    g_value_set_double (value, self->stiffness);
    break;
  case PROP_PRECISION:
    g_value_set_double (value, self->precision);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_spring_interactive_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  AdwDemoSpringInteractive *self = ADW_DEMO_SPRING_INTERACTIVE (object);

  switch (prop_id) {
  case PROP_DAMPING:
    self->damping = g_value_get_double (value);
    break;
  case PROP_MASS:
    self->mass = g_value_get_double (value);
    break;
  case PROP_STIFFNESS:
    self->stiffness = g_value_get_double (value);
    break;
  case PROP_PRECISION:
    self->precision = g_value_get_double (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_spring_interactive_class_init (AdwDemoSpringInteractiveClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_demo_spring_interactive_get_property;
  object_class->set_property = adw_demo_spring_interactive_set_property;

  props[PROP_DAMPING] =
    g_param_spec_double ("damping",
                         _("Damping"),
                         _("Damping"),
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE);

  props[PROP_MASS] =
    g_param_spec_double ("mass",
                         _("Mass"),
                         _("Mass"),
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE);

  props[PROP_STIFFNESS] =
    g_param_spec_double ("stiffness",
                         _("Stiffness"),
                         _("Stiffness"),
                         0, G_MAXDOUBLE, 0,
                         G_PARAM_READWRITE);

  props[PROP_PRECISION] =
    g_param_spec_double ("precision",
                         _("Precision"),
                         _("Precision"),
                         0, 1, 0,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/spring/adw-demo-spring-interactive.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringInteractive, layout);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringInteractive, handle);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringInteractive, drag_gesture);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringInteractive, swipe_gesture);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringInteractive, grab_cursor);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringInteractive, grabbing_cursor);
  gtk_widget_class_bind_template_callback (widget_class, drag_begin_cb);
  gtk_widget_class_bind_template_callback (widget_class, drag_update_cb);
  gtk_widget_class_bind_template_callback (widget_class, drag_cancel_cb);
  gtk_widget_class_bind_template_callback (widget_class, animate);
}

static void
adw_demo_spring_interactive_init (AdwDemoSpringInteractive *self)
{
  g_type_ensure (ADW_TYPE_DEMO_TRANSFORM_LAYOUT);

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_gesture_group (self->drag_gesture, self->swipe_gesture);
}

void
adw_demo_spring_interactive_reset (AdwDemoSpringInteractive *self)
{
  if (self->animation_x)
    adw_spring_animation_stop (self->animation_x);

  if (self->animation_y)
    adw_spring_animation_stop (self->animation_y);

  set_translation (self, 0, 0);
}
