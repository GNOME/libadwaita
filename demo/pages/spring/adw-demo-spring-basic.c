#include "adw-demo-spring-basic.h"

#include <glib/gi18n.h>

#include "adw-demo-transform-layout.h"
#include "adw-spring-animation-private.h"

#include <math.h>

#define GRAPH_PADDING 24

typedef struct {
  gint64 time;
  gdouble value;
} GraphPoint;

struct _AdwDemoSpringBasic
{
  AdwBin parent_instance;

  gdouble damping;
  gdouble mass;
  gdouble stiffness;
  gdouble precision;

  AdwDemoTransformLayout *scale_layout;
  AdwDemoTransformLayout *htranslate_layout;
  AdwDemoTransformLayout *rotate_layout;
  AdwDemoTransformLayout *vtranslate_layout;
  GtkAdjustment *velocity;
  GtkToggleButton *info_btn;
  GtkStack *stack;
  GtkDrawingArea *darea;
  GtkWidget *label_box;
  GtkLabel *duration_label;
  GtkLabel *min_label;
  GtkLabel *max_label;

  AdwSpringAnimation *animation;
  gboolean invert;

  AdwSpringAnimation *graph_animation;
  GArray *points;
  gint64 start_time;
  gint64 duration;
  gdouble min;
  gdouble max;
};

G_DEFINE_TYPE (AdwDemoSpringBasic, adw_demo_spring_basic, ADW_TYPE_BIN)

enum {
  PROP_0,
  PROP_DAMPING,
  PROP_MASS,
  PROP_STIFFNESS,
  PROP_PRECISION,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static inline gfloat
lerp (gfloat a, gfloat b, gfloat t)
{
  return a * (1.0 - t) + b * t;
}

static void
set_value (AdwDemoSpringBasic *self,
           gdouble             value)
{
  gfloat x = lerp (-30, 30, value);
  gfloat y = lerp (30, -30, value);
  gfloat s = MAX (0, lerp (3, 1, value));
  gfloat r = lerp (0, 90, value);

  adw_demo_transform_layout_take_transform (self->htranslate_layout,
                                            gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (x, 0)));
  adw_demo_transform_layout_take_transform (self->vtranslate_layout,
                                            gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, y)));
  adw_demo_transform_layout_take_transform (self->scale_layout,
                                            gsk_transform_scale (NULL, s, s));
  adw_demo_transform_layout_take_transform (self->rotate_layout,
                                            gsk_transform_rotate (NULL, r));
}

static void
set_min (AdwDemoSpringBasic *self,
         gdouble             min)
{
  g_autofree gchar *label = g_strdup_printf (_("Min: %.2lf"), min);

  self->min = min;

  gtk_label_set_label (self->min_label, label);
}

static void
set_max (AdwDemoSpringBasic *self,
         gdouble             max)
{
  g_autofree gchar *label = g_strdup_printf (_("Max: %.2lf"), max);

  self->max = max;

  gtk_label_set_label (self->max_label, label);
}

static void
add_plot_point (AdwDemoSpringBasic *self,
                gint64              time,
                gdouble             value)
{
  GraphPoint point = { time - self->start_time, value };

  g_array_append_val (self->points, point);

  gtk_widget_queue_draw (GTK_WIDGET (self->darea));

  if (value < self->min)
    set_min (self, value);

  if (value > self->max)
    set_max (self, value);
}

static void
active_changed_cb (AdwDemoSpringBasic *self)
{
  const gchar *name = gtk_toggle_button_get_active (self->info_btn) ? "info" : "basic";

  gtk_stack_set_visible_child_name (self->stack, name);
}

static void
graph_value_cb (gdouble             value,
                AdwDemoSpringBasic *self)
{
  gint64 frame_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (GTK_WIDGET (self)));

  add_plot_point (self, frame_time, value);
}

static void
graph_done_cb (AdwDemoSpringBasic *self)
{
  g_clear_pointer (&self->graph_animation, adw_spring_animation_unref);
}

static void
run_graph (AdwDemoSpringBasic *self)
{
  gdouble velocity = gtk_adjustment_get_value (self->velocity);
  gdouble duration;

  if (self->graph_animation)
    adw_spring_animation_stop (self->graph_animation);

  g_clear_pointer (&self->points, g_array_unref);
  self->points = g_array_new (FALSE, FALSE, sizeof (GraphPoint));

  self->start_time = gdk_frame_clock_get_frame_time (gtk_widget_get_frame_clock (GTK_WIDGET (self)));

  set_min (self, 0);
  set_max (self, 1);
  add_plot_point (self, self->start_time, 0);

  self->graph_animation = adw_spring_animation_new (GTK_WIDGET (self),
                                                    0,
                                                    1,
                                                    velocity,
                                                    self->damping,
                                                    self->mass,
                                                    self->stiffness,
                                                    self->precision,
                                                    (AdwAnimationValueCallback) graph_value_cb,
                                                    (AdwAnimationDoneCallback) graph_done_cb,
                                                    self);

  duration = adw_spring_animation_get_estimated_duration (self->graph_animation);

  if (isinf (duration)) {
    self->duration = -1;

    gtk_label_set_label (self->duration_label, _("Duration: ∞"));
  } else {
    g_autofree gchar *label = g_strdup_printf (_("Duration: %.0lfms"), duration * 1000);

    self->duration = (gint64) (duration * 1000000);
    gtk_label_set_label (self->duration_label, label);
  }

  adw_spring_animation_start (self->graph_animation);

  gtk_widget_set_opacity (self->label_box, 1);
}

static void
basic_value_cb (gdouble             value,
                AdwDemoSpringBasic *self)
{
  set_value (self, value);
}

static void
basic_done_cb (AdwDemoSpringBasic *self)
{
  g_clear_pointer (&self->animation, adw_spring_animation_unref);
}

static void
run_basic (AdwDemoSpringBasic *self)
{
  gdouble value = self->invert ? 1 : 0;
  gdouble velocity = gtk_adjustment_get_value (self->velocity);

  if (self->animation) {
    value = adw_spring_animation_get_value (self->animation);
    adw_spring_animation_stop (self->animation);
  }

  self->animation = adw_spring_animation_new (GTK_WIDGET (self),
                                              value,
                                              self->invert ? 0 : 1,
                                              velocity,
                                              self->damping,
                                              self->mass,
                                              self->stiffness,
                                              self->precision,
                                              (AdwAnimationValueCallback) basic_value_cb,
                                              (AdwAnimationDoneCallback) basic_done_cb,
                                              self);

  adw_spring_animation_start (self->animation);

  self->invert = !self->invert;
}

static void
run_cb (AdwDemoSpringBasic *self)
{
  if (gtk_toggle_button_get_active (self->info_btn))
    run_graph (self);
  else
    run_basic (self);
}

static inline void
set_color_from_css (AdwDemoSpringBasic *self,
                    cairo_t            *cr,
                    const gchar        *name,
                    gdouble             alpha_multiplier)
{
  GdkRGBA rgba = { 0, 0, 0, 1 };

  gtk_style_context_lookup_color (gtk_widget_get_style_context (GTK_WIDGET (self->darea)), name, &rgba);

  cairo_set_source_rgba (cr, rgba.red, rgba.green, rgba.blue, rgba.alpha * alpha_multiplier);
}

static inline double
transform_y (AdwDemoSpringBasic *self,
             gdouble             height,
             gdouble             y)
{
  gdouble bottom_padding = gtk_widget_get_allocated_height (self->label_box);

  return height - (bottom_padding + (y - self->min) * (height - GRAPH_PADDING - bottom_padding) / (self->max - self->min));
}

static void
draw_cb (GtkDrawingArea     *darea,
         cairo_t            *cr,
         gint                width,
         gint                height,
         AdwDemoSpringBasic *self)
{
  gdouble x = 0;
  gdouble y1 = transform_y (self, height, 0);
  gdouble y2 = transform_y (self, height, 1);
  gdouble dashes[2] = { 4, 2 };
  gint64 d;
  guint i;
  cairo_path_t *path;

  cairo_save (cr);

  cairo_set_line_width (cr, 1);
  cairo_set_dash (cr, dashes, 2, 0);
  cairo_translate(cr, 0, 0.5);

  set_color_from_css (self, cr, "borders", 1);

  cairo_move_to (cr, 0, y1);
  cairo_line_to (cr, width, y1);

  cairo_move_to (cr, 0, y2);
  cairo_line_to (cr, width, y2);

  cairo_stroke (cr);

  cairo_restore (cr);

  if (!self->points)
    return;

  cairo_new_path (cr);

  d = self->duration < 0 ? 10000000 : self->duration;

  for (i = 0; i < self->points->len; i++) {
    GraphPoint *point = &g_array_index (self->points, GraphPoint, i);

    x = (double) point->time * width / d;

    cairo_line_to (cr, x, transform_y (self, height, point->value));
  }

  path = cairo_copy_path (cr);

  set_color_from_css (self, cr, "yellow_1", 0.5);

  cairo_line_to (cr, x, height);
  cairo_line_to (cr, -1, height);
  cairo_close_path (cr);
  cairo_fill (cr);

  cairo_append_path (cr, path);

  cairo_set_line_width (cr, 2);
  set_color_from_css (self, cr, "yellow_5", 1);
  cairo_stroke (cr);

  cairo_path_destroy (path);
}

static void
adw_demo_spring_basic_finalize (GObject *object)
{
  AdwDemoSpringBasic *self = ADW_DEMO_SPRING_BASIC (object);

  g_clear_pointer (&self->points, g_array_unref);

  G_OBJECT_CLASS (adw_demo_spring_basic_parent_class)->finalize (object);
}

static void
adw_demo_spring_basic_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwDemoSpringBasic *self = ADW_DEMO_SPRING_BASIC (object);

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
adw_demo_spring_basic_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwDemoSpringBasic *self = ADW_DEMO_SPRING_BASIC (object);

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
adw_demo_spring_basic_class_init (AdwDemoSpringBasicClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = adw_demo_spring_basic_finalize;
  object_class->get_property = adw_demo_spring_basic_get_property;
  object_class->set_property = adw_demo_spring_basic_set_property;

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

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/spring/adw-demo-spring-basic.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, scale_layout);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, htranslate_layout);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, rotate_layout);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, vtranslate_layout);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, velocity);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, info_btn);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, darea);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, label_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, duration_label);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, min_label);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoSpringBasic, max_label);
  gtk_widget_class_bind_template_callback (widget_class, active_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, run_cb);
  gtk_widget_class_bind_template_callback (widget_class, adw_demo_spring_basic_reset);
}

static void
adw_demo_spring_basic_init (AdwDemoSpringBasic *self)
{
  g_type_ensure (ADW_TYPE_DEMO_TRANSFORM_LAYOUT);

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_drawing_area_set_draw_func (self->darea, (GtkDrawingAreaDrawFunc) draw_cb, self, NULL);

  set_value (self, 0);
  adw_demo_spring_basic_reset (self);
}

void
adw_demo_spring_basic_reset (AdwDemoSpringBasic *self)
{
  if (self->animation)
    adw_spring_animation_stop (self->animation);

  if (self->graph_animation)
    adw_spring_animation_stop (self->graph_animation);

  g_clear_pointer (&self->points, g_array_unref);
  self->points = NULL;
  self->min = 0;
  self->max = 1;

  gtk_widget_set_opacity (self->label_box, 0);
  gtk_widget_queue_draw (GTK_WIDGET (self->darea));
}
