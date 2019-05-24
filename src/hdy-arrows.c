/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-arrows.h"
#include "hdy-enums.h"
#include "gtkprogresstrackerprivate.h"

#define HDY_ARROWS_DEFAULT_THICKNESS 10

/**
 * SECTION:hdy-arrows
 * @short_description: Arrows indicating a swipe direction
 * @Title: HdyArrows
 *
 * The #HdyArrows widget displays arrows indicating a swiping direction.
 * An animation is run when the widget is mapped or then #hdy_arrows_animate()
 * is invoked.
 */

/**
 * HdyArrowsDirection:
 * @HDY_ARROWS_DIRECTION_UP: Arrows point upwards
 * @HDY_ARROWS_DIRECTION_DOWN: Arrows point to the left
 * @HDY_ARROWS_DIRECTION_LEFT: Arrows point to the right
 * @HDY_ARROWS_DIRECTION_RIGHT: Arrows point downwards
 */

typedef struct
{
  guint count;
  HdyArrowsDirection direction;

  struct {
    guint duration;   /* animation duration in ms */
    guint current;    /* current arrow being drawn */
    guint tick_id;
    GtkProgressTracker tracker;
  } animation;
} HdyArrowsPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyArrows, hdy_arrows, GTK_TYPE_DRAWING_AREA)

enum {
  PROP_0,
  PROP_COUNT,
  PROP_DIRECTION,
  PROP_DURATION,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

enum {
  STYLE_PROP_0,
  STYLE_PROP_THICKNESS,
  N_STYLE_PROPS
};
static GParamSpec *style_properties [N_STYLE_PROPS];

static gboolean
get_enable_animations (void)
{
  gboolean enable_animations;
  g_object_get (gtk_settings_get_default (),
                "gtk-enable-animations", &enable_animations,
                NULL);
  return enable_animations;
}


static void
schedule_draw (HdyArrows *self)
{
  gtk_widget_queue_draw (GTK_WIDGET (self));
}


static gboolean
animation_cb (GtkWidget     *widget,
              GdkFrameClock *frame_clock,
              gpointer       user_data)
{
  HdyArrows *self = HDY_ARROWS (widget);
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);

  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&priv->animation.tracker);

  gtk_progress_tracker_advance_frame (&priv->animation.tracker,
                                      gdk_frame_clock_get_frame_time (frame_clock));
  schedule_draw (self);

  if (gtk_progress_tracker_get_state (&priv->animation.tracker) == GTK_PROGRESS_STATE_AFTER) {
    priv->animation.tick_id = 0;
    return FALSE;
  }
  return TRUE;
}


static void
schedule_child_ticks (HdyArrows *self)
{
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);

  if (priv->animation.tick_id == 0) {
    priv->animation.tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self), animation_cb, self, NULL);
  }
}


static void
unschedule_child_ticks (HdyArrows *self)
{
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);

  if (priv->animation.tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), priv->animation.tick_id);
    priv->animation.tick_id = 0;
  }
}


static void
start_animation (HdyArrows                     *self)
{
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (self);

  if (gtk_widget_get_mapped (widget) &&
      get_enable_animations () &&
      priv->animation.duration > 0.0 &&
      /* Don't schedule an animation when already ongoing. */
      priv->animation.tick_id == 0) {
    gtk_progress_tracker_start (&priv->animation.tracker,
                                priv->animation.duration * 1000,
                                0,
                                1.0);
    schedule_child_ticks (self);
  } else {
    unschedule_child_ticks (self);
    gtk_progress_tracker_finish (&priv->animation.tracker);
  }

  schedule_draw (self);
}


static gboolean
map_cb (GtkWidget *widget,
        gpointer   unused)
{
  HdyArrows *self = HDY_ARROWS (widget);
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);

  if (priv->animation.tick_id == 0)
    start_animation (self);
  return TRUE;
}


static void
draw_arrow (cairo_t           *cr,
            gdouble            x,
            gdouble            y,
            gdouble            width,
            gdouble            height,
            gdouble            thickness,
            HdyArrowsDirection direction)
{
  gdouble th = thickness / 2.0;

  switch (direction) {
  case HDY_ARROWS_DIRECTION_UP:
    cairo_move_to(cr, x + th,          y + height - th);
    cairo_line_to(cr, x + width / 2.0, y + thickness);
    cairo_line_to(cr, x + width - th,  y + height - th);
    cairo_stroke (cr);
    break;
  case HDY_ARROWS_DIRECTION_DOWN:
    cairo_move_to(cr, x + th,          y + thickness);
    cairo_line_to(cr, x + width / 2.0, y + height - th);
    cairo_line_to(cr, x + width - th,  y + thickness);
    cairo_stroke (cr);
    break;
  case HDY_ARROWS_DIRECTION_LEFT:
    cairo_move_to(cr, x + width - th,  y + th);
    cairo_line_to(cr, x + thickness, y + height / 2.0);
    cairo_line_to(cr, x + width - th,  y + height - th);
    cairo_stroke (cr);
    break;
  case HDY_ARROWS_DIRECTION_RIGHT:
    cairo_move_to(cr, x + thickness,  y + th);
    cairo_line_to(cr, x + width - th, y + height / 2.0);
    cairo_line_to(cr, x + thickness,  y + height - th);
    cairo_stroke (cr);
    break;
  default:
    g_warning ("Unhandled arrow mode %d", direction);
  }
}


static guint
get_thickness (HdyArrows *self)
{
  guint thickness;

  gtk_widget_style_get (GTK_WIDGET (self), "thickness", &thickness, NULL);
  return thickness;
}


static gboolean
draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  HdyArrows *self = HDY_ARROWS (widget);
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);
  guint width, height;
  GdkRGBA color;
  GtkStyleContext *context;
  gdouble ah = 0.0, aw = 0.0; /* arrow width an height */
  gdouble xd = 0.0, yd = 0.0; /* offset between arrows */
  gdouble x = 0.0, y = 0.0;   /* arrow position */
  double iter;
  gint thickness = get_thickness (self);

  context = gtk_widget_get_style_context (widget);

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  gtk_render_background (context, cr, 0, 0, width, height);
  gtk_style_context_get_color (context,
                               gtk_style_context_get_state (context),
                               &color);
  gdk_cairo_set_source_rgba (cr, &color);

  if (priv->animation.tick_id) {
    iter = gtk_progress_tracker_get_iteration (&priv->animation.tracker);
    iter *= priv->count;
  } else { /* no animation */
    iter = priv->count;
  }

  switch (priv->direction) {
  case HDY_ARROWS_DIRECTION_UP:
    aw = width;
    ah = height / priv->count;
    xd = 0.0;
    yd = -1.0 * height / priv->count;
    x = 0.0;
    y = height - ah;
    break;
  case HDY_ARROWS_DIRECTION_DOWN:
    aw = width;
    ah = height / priv->count;
    xd = 0.0;
    yd = height / priv->count;
    x = 0.0;
    y = 0.0;
    break;
  case HDY_ARROWS_DIRECTION_LEFT:
    aw = width / priv->count;
    ah = height;
    xd = -1.0 * width / priv->count;
    yd = 0.0;
    x = width - aw;
    y = 0.0;
    break;
  case HDY_ARROWS_DIRECTION_RIGHT:
    aw = width / priv->count;
    ah = height;
    xd = 1.0 * width / priv->count;
    yd = 0.0;
    x = 0.0;
    y = 0.0;
    break;
  default:
    g_warning ("Unhandled arrow mode %d", priv->direction);
    return FALSE;
  }

  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_width (cr, thickness);
  for (int i = 1; i <= iter; i++) {
    draw_arrow (cr, x, y, aw, ah, thickness, priv->direction);
    y += yd;
    x += xd;
  }
  return FALSE;
}

static void
hdy_arrows_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  HdyArrows *self = HDY_ARROWS (object);

  switch (property_id) {
  case PROP_COUNT:
    hdy_arrows_set_count (self, g_value_get_uint (value));
    break;

  case PROP_DIRECTION:
    hdy_arrows_set_direction (self, g_value_get_enum (value));
    break;

  case PROP_DURATION:
    hdy_arrows_set_duration (self, g_value_get_uint (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_arrows_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  HdyArrows *self = HDY_ARROWS (object);
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);

  switch (property_id) {
  case PROP_COUNT:
    g_value_set_uint (value, priv->count);
    break;

  case PROP_DIRECTION:
    g_value_set_enum (value, priv->direction);
    break;

  case PROP_DURATION:
    g_value_set_enum (value, priv->animation.duration);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


/* This private method is prefixed by the call name because it will be a virtual
 * method in GTK 4.
 */
static void
hdy_arrows_measure (GtkWidget      *widget,
                    GtkOrientation  orientation,
                    int             for_size,
                    int            *minimum,
                    int            *natural,
                    int            *minimum_baseline,
                    int            *natural_baseline)
{
  HdyArrows *self = HDY_ARROWS (widget);
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);
  guint thickness, facter, size;

  thickness = get_thickness (self);
  facter = priv->direction == HDY_ARROWS_DIRECTION_LEFT ||
           priv->direction == HDY_ARROWS_DIRECTION_RIGHT ?
           2 : 3;

  size = thickness * priv->count * facter;

  if(minimum)
    *minimum = size;
  if(natural)
    *natural = size;
}


static void
get_preferred_width (GtkWidget *widget, gint *minimum, gint *natural)
{
  hdy_arrows_measure (widget, GTK_ORIENTATION_HORIZONTAL, 0,
                      minimum, natural, NULL, NULL);
}


static void
get_preferred_height (GtkWidget *widget, gint *minimum, gint *natural)
{
  hdy_arrows_measure (widget, GTK_ORIENTATION_HORIZONTAL, 0,
                      minimum, natural, NULL, NULL);
}


static void
hdy_arrows_constructed (GObject *object)
{

  HdyArrows *self = HDY_ARROWS (object);

  g_signal_connect (GTK_WIDGET (self),
                    "draw",
                    G_CALLBACK (draw_cb),
                    NULL);

  g_signal_connect (GTK_WIDGET (self),
                    "map",
                    G_CALLBACK (map_cb),
                    NULL);
}


static void
hdy_arrows_class_init (HdyArrowsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = hdy_arrows_constructed;

  object_class->set_property = hdy_arrows_set_property;
  object_class->get_property = hdy_arrows_get_property;

  widget_class->get_preferred_width = get_preferred_width;
  widget_class->get_preferred_height = get_preferred_height;

  props[PROP_COUNT] =
    g_param_spec_uint ("count",
                       _("Number of arrows"),
                       _("Number of arrows to display"),
                       1,
                       G_MAXUINT,
                       1,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_DIRECTION] =
    g_param_spec_enum ("direction",
                       _("Arrows Direction"),
                       _("Direction the arrows should point to"),
                       HDY_TYPE_ARROWS_DIRECTION,
                       HDY_ARROWS_DIRECTION_UP,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_DURATION] =
    g_param_spec_uint ("duration",
                       _("Arrow animation duration"),
                       _("The duration of the arrow animation in milliseconds"),
                       0,
                       G_MAXUINT,
                       1000,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  style_properties [STYLE_PROP_THICKNESS] =
    g_param_spec_uint ("thickness",
                       "Arrows thickness",
                       "Thickness of the arrows",
                       1,
                       G_MAXUINT,
                       10,
                       (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));
  gtk_widget_class_install_style_property (widget_class, style_properties [STYLE_PROP_THICKNESS]);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_ARROW);
  gtk_widget_class_set_css_name (widget_class, "hdyarrows");
}

/**
 * hdy_arrows_new:
 *
 * Create a new #HdyArrows widget.
 *
 * Returns: the newly created #HdyArrows widget
 */
GtkWidget *hdy_arrows_new (void)
{
  return g_object_new (HDY_TYPE_ARROWS, NULL);
}

static void
hdy_arrows_init (HdyArrows *self)
{
  HdyArrowsPrivate *priv = hdy_arrows_get_instance_private (self);

  priv->count = 1;
  priv->direction = HDY_ARROWS_DIRECTION_UP;
  priv->animation.duration = 1000;
}

/**
 * hdy_arrows_get_count:
 * @self: a #HdyArrows
 *
 * Get the number of arrows displayed in the widget.
 *
 * Returns: the current number of arrows
 */
guint
hdy_arrows_get_count (HdyArrows *self)
{
  HdyArrowsPrivate *priv;

  g_return_val_if_fail (HDY_IS_ARROWS (self), 1);

  priv = hdy_arrows_get_instance_private (self);
  return priv->count;
}

/**
 * hdy_arrows_set_count
 * @self: a #HdyArrows
 * @count: the number of arrows to display
 *
 * Set the number of arrows to display.
 *
 */
void
hdy_arrows_set_count (HdyArrows   *self, guint count)
{
  HdyArrowsPrivate *priv;

  g_return_if_fail (HDY_IS_ARROWS (self));
  g_return_if_fail (count >= 1);

  priv = hdy_arrows_get_instance_private (self);

  if (priv->count == count)
    return;

  priv->count = count;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COUNT]);
  hdy_arrows_animate (self);
}

/**
 * hdy_arrows_get_direction
 * @self: a #HdyArrows
 *
 * Get the direction the arrows point to
 *
 * Returns: the arrows direction
 */
HdyArrowsDirection
hdy_arrows_get_direction (HdyArrows *self)
{
  HdyArrowsPrivate *priv;

  g_return_val_if_fail (HDY_IS_ARROWS (self), FALSE);

  priv = hdy_arrows_get_instance_private (self);
  return priv->direction;
}


/**
 * hdy_arrows_set_direction
 * @self: a #HdyArrows
 * @direction: the arrows direction
 *
 * Set the direction the arrows should point to.
 *
 */
void
hdy_arrows_set_direction (HdyArrows *self,
                          HdyArrowsDirection direction)
{
  HdyArrowsPrivate *priv;

  g_return_if_fail (HDY_IS_ARROWS (self));
  g_return_if_fail (direction == HDY_ARROWS_DIRECTION_UP ||
                    direction == HDY_ARROWS_DIRECTION_DOWN ||
                    direction == HDY_ARROWS_DIRECTION_LEFT ||
                    direction == HDY_ARROWS_DIRECTION_RIGHT);

  priv = hdy_arrows_get_instance_private (self);
  switch (direction) {
  case HDY_ARROWS_DIRECTION_UP:
  case HDY_ARROWS_DIRECTION_DOWN:
  case HDY_ARROWS_DIRECTION_LEFT:
  case HDY_ARROWS_DIRECTION_RIGHT:
    break;
  default:
    direction = HDY_ARROWS_DIRECTION_UP;
  }

  if (priv->direction == direction)
    return;

  priv->direction = direction;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DIRECTION]);
  hdy_arrows_animate (self);
}

/**
 * hdy_arrows_get_duration
 * @self: a #HdyArrows
 *
 * Get the duration of the arrows animation.
 *
 * Returns: the duration of the animation in ms
 */
HdyArrowsDirection
hdy_arrows_get_duration (HdyArrows *self)
{
  HdyArrowsPrivate *priv;

  g_return_val_if_fail (HDY_IS_ARROWS (self), FALSE);

  priv = hdy_arrows_get_instance_private (self);
  return priv->animation.duration;
}


/**
 * hdy_arrows_set_duration
 * @self: a #HdyArrows
 * @duration: the duration of the animation in ms
 *
 * Set the duration of the arrow animation.
 *
 */
void
hdy_arrows_set_duration (HdyArrows *self,
                         guint duration)
{
  HdyArrowsPrivate *priv;

  g_return_if_fail (HDY_IS_ARROWS (self));

  priv = hdy_arrows_get_instance_private (self);

  if (priv->animation.duration == duration)
    return;

  priv->animation.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DURATION]);
  hdy_arrows_animate (self);
}

/**
 * hdy_arrows_animate
 * @self: a #HdyArrows
 *
 * Render the arrows animation.
 */
void
hdy_arrows_animate (HdyArrows *self)
{
  g_return_if_fail (HDY_IS_ARROWS (self));

  if (gtk_widget_get_mapped (GTK_WIDGET (self)))
    map_cb (GTK_WIDGET (self), NULL);
}
