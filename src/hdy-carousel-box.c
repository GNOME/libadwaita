/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-animation-private.h"
#include "hdy-cairo-private.h"
#include "hdy-carousel-box-private.h"

#include <math.h>

/**
 * PRIVATE:hdy-carousel-box
 * @short_description: Scrolling box used in #HdyCarousel
 * @title: HdyCarouselBox
 * @See_also: #HdyCarousel
 * @stability: Private
 *
 * The #HdyCarouselBox object is meant to be used exclusively as part of the
 * #HdyCarousel implementation.
 *
 * Since: 1.0
 */

typedef struct _HdyCarouselBoxChildInfo HdyCarouselBoxChildInfo;

struct _HdyCarouselBoxChildInfo
{
  GtkWidget *widget;
  GdkWindow *window;
  gint position;
  gboolean visible;
  cairo_surface_t *surface;
  cairo_region_t *dirty_region;
};

struct _HdyCarouselBox
{
  GtkContainer parent_instance;

  struct {
    guint tick_cb_id;
    gint64 start_time;
    gint64 end_time;
    gdouble start_position;
    gdouble end_position;
  } animation_data;
  GList *children;

  gint child_width;
  gint child_height;

  gdouble distance;
  gdouble position;
  guint spacing;
  GtkOrientation orientation;
};

G_DEFINE_TYPE_WITH_CODE (HdyCarouselBox, hdy_carousel_box, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL));

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_POSITION,
  PROP_SPACING,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_SPACING + 1,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_ANIMATION_STOPPED,
  SIGNAL_POSITION_SHIFTED,
  SIGNAL_LAST_SIGNAL,
};
static guint signals[SIGNAL_LAST_SIGNAL];

static HdyCarouselBoxChildInfo *
find_child_info (HdyCarouselBox *self,
                 GtkWidget      *widget)
{
  GList *l;

  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    if (widget == info->widget)
      return info;
  }

  return NULL;
}

static gint
find_child_index (HdyCarouselBox *self,
                  GtkWidget      *widget)
{
  GList *l;
  gint i;

  i = 0;
  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    if (widget == info->widget)
      return i;

    i++;
  }

  return -1;
}

static HdyCarouselBoxChildInfo *
find_child_info_by_window (HdyCarouselBox *self,
                           GdkWindow      *window)
{
  GList *l;

  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    if (window == info->window)
      return info;
  }

  return NULL;
}

static void
free_child_info (HdyCarouselBoxChildInfo *info)
{
  if (info->surface)
    cairo_surface_destroy (info->surface);
  if (info->dirty_region)
    cairo_region_destroy (info->dirty_region);
  g_free (info);
}

static void
invalidate_handler_cb (GdkWindow      *window,
                       cairo_region_t *region)
{
  gpointer user_data;
  HdyCarouselBox *self;
  HdyCarouselBoxChildInfo *info;

  gdk_window_get_user_data (window, &user_data);
  g_assert (HDY_IS_CAROUSEL_BOX (user_data));
  self = HDY_CAROUSEL_BOX (user_data);

  info = find_child_info_by_window (self, window);

  if (!info->dirty_region)
    info->dirty_region = cairo_region_create ();

  cairo_region_union (info->dirty_region, region);
}

static void
register_window (HdyCarouselBoxChildInfo *info,
                 HdyCarouselBox          *self)
{
  GtkWidget *widget;
  GdkWindow *window;
  GdkWindowAttr attributes;
  GtkAllocation allocation;
  gint attributes_mask;

  widget = GTK_WIDGET (self);
  gtk_widget_get_allocation (info->widget, &allocation);

  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

  window = gdk_window_new (gtk_widget_get_parent_window (widget),
                           &attributes, attributes_mask);
  gtk_widget_register_window (widget, window);
  gtk_widget_set_parent_window (info->widget, window);

  gdk_window_set_user_data (window, self);

  gdk_window_show (window);

  info->window = window;

  gdk_window_set_invalidate_handler (window, invalidate_handler_cb);
}

static void
unregister_window (HdyCarouselBoxChildInfo *info,
                   HdyCarouselBox          *self)
{
  gtk_widget_set_parent_window (info->widget, NULL);
  gtk_widget_unregister_window (GTK_WIDGET (self), info->window);
  gdk_window_destroy (info->window);
  info->window = NULL;
}

static gboolean
animation_cb (GtkWidget     *widget,
              GdkFrameClock *frame_clock,
              gpointer       user_data)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);
  gint64 frame_time, duration;
  gdouble position;
  gdouble t;

  g_assert (hdy_carousel_box_is_animating (self));

  frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;
  frame_time = MIN (frame_time, self->animation_data.end_time);

  duration = self->animation_data.end_time - self->animation_data.start_time;
  position = (gdouble) (frame_time - self->animation_data.start_time) / duration;

  t = hdy_ease_out_cubic (position);
  hdy_carousel_box_set_position (self,
                                 hdy_lerp (self->animation_data.start_position,
                                           self->animation_data.end_position, 1 - t));

  if (frame_time == self->animation_data.end_time) {
    self->animation_data.tick_cb_id = 0;
    g_signal_emit (self, signals[SIGNAL_ANIMATION_STOPPED], 0);
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static gboolean
hdy_carousel_box_draw (GtkWidget *widget,
                       cairo_t   *cr)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);
  GList *l;

  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    if (!info->visible)
      continue;

    if (info->dirty_region) {
      g_autoptr (cairo_t) surface_cr = NULL;
      GtkAllocation child_alloc;

      if (!info->surface) {
        gint width, height;

        width = gdk_window_get_width (info->window);
        height = gdk_window_get_height (info->window);

        info->surface = gdk_window_create_similar_surface (info->window,
                                                           CAIRO_CONTENT_COLOR_ALPHA,
                                                           width, height);
      }

      gtk_widget_get_allocation (info->widget, &child_alloc);

      surface_cr = cairo_create (info->surface);

      gdk_cairo_region (surface_cr, info->dirty_region);
      cairo_clip (surface_cr);

      if (self->orientation == GTK_ORIENTATION_VERTICAL)
        cairo_translate (surface_cr, 0, -info->position);
      else
        cairo_translate (surface_cr, -info->position, 0);

      cairo_save (surface_cr);
      cairo_set_source_rgba (surface_cr, 0, 0, 0, 0);
      cairo_set_operator (surface_cr, CAIRO_OPERATOR_SOURCE);
      cairo_paint (surface_cr);
      cairo_restore (surface_cr);

      gtk_container_propagate_draw (GTK_CONTAINER (self), info->widget, surface_cr);

      cairo_region_destroy (info->dirty_region);
      info->dirty_region = NULL;
    }

    if (!info->surface)
      continue;

    if (self->orientation == GTK_ORIENTATION_VERTICAL)
      cairo_set_source_surface (cr, info->surface, 0, info->position);
    else
      cairo_set_source_surface (cr, info->surface, info->position, 0);
    cairo_paint (cr);
  }

  return GDK_EVENT_PROPAGATE;
}

static void
measure (GtkWidget      *widget,
         GtkOrientation  orientation,
         gint            for_size,
         gint           *minimum,
         gint           *natural,
         gint           *minimum_baseline,
         gint           *natural_baseline)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);
  GList *children;

  if (minimum)
    *minimum = 0;
  if (natural)
    *natural = 0;

  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;

  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;
    GtkWidget *child = child_info->widget;
    gint child_min, child_nat;

    if (!gtk_widget_get_visible (child))
      continue;

    if (orientation == GTK_ORIENTATION_VERTICAL) {
      if (for_size < 0)
        gtk_widget_get_preferred_height (child, &child_min, &child_nat);
      else
        gtk_widget_get_preferred_height_for_width (child, for_size, &child_min, &child_nat);
    } else {
      if (for_size < 0)
        gtk_widget_get_preferred_width (child, &child_min, &child_nat);
      else
        gtk_widget_get_preferred_width_for_height (child, for_size, &child_min, &child_nat);
    }

    if (minimum)
      *minimum = MAX (*minimum, child_min);
    if (natural)
      *natural = MAX (*natural, child_nat);
  }
}

static void
hdy_carousel_box_get_preferred_width (GtkWidget *widget,
                                      gint      *minimum_width,
                                      gint      *natural_width)
{
  measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
           minimum_width, natural_width, NULL, NULL);
}

static void
hdy_carousel_box_get_preferred_height (GtkWidget *widget,
                                       gint      *minimum_height,
                                       gint      *natural_height)
{
  measure (widget, GTK_ORIENTATION_VERTICAL, -1,
           minimum_height, natural_height, NULL, NULL);
}

static void
hdy_carousel_box_get_preferred_width_for_height (GtkWidget *widget,
                                                 gint       for_height,
                                                 gint      *minimum_width,
                                                 gint      *natural_width)
{
  measure (widget, GTK_ORIENTATION_HORIZONTAL, for_height,
           minimum_width, natural_width, NULL, NULL);
}

static void
hdy_carousel_box_get_preferred_height_for_width (GtkWidget *widget,
                                                 gint       for_width,
                                                 gint      *minimum_height,
                                                 gint      *natural_height)
{
  measure (widget, GTK_ORIENTATION_VERTICAL, for_width,
           minimum_height, natural_height, NULL, NULL);
}

static void
invalidate_cache_for_child (HdyCarouselBox          *self,
                            HdyCarouselBoxChildInfo *child)
{
  cairo_rectangle_int_t rect;

  rect.x = 0;
  rect.y = 0;
  rect.width = self->child_width;
  rect.height = self->child_height;

  if (child->surface)
    g_clear_pointer (&child->surface, cairo_surface_destroy);

  if (child->dirty_region)
    cairo_region_destroy (child->dirty_region);
  child->dirty_region = cairo_region_create_rectangle (&rect);
}

static void
invalidate_drawing_cache (HdyCarouselBox *self)
{
  GList *l;

  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *child_info = l->data;

    invalidate_cache_for_child (self, child_info);
  }
}

static void
update_windows (HdyCarouselBox *self)
{
  GList *children;
  GtkAllocation alloc;
  gint x, y, offset;
  gboolean is_rtl;

  if (!gtk_widget_get_realized (GTK_WIDGET (self)))
    return;

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  x = alloc.x;
  y = alloc.y;

  is_rtl = (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL);

  if (self->orientation == GTK_ORIENTATION_VERTICAL)
    offset = (self->distance * self->position) - (alloc.height - self->child_height) / 2.0;
  else if (is_rtl)
    offset = -(self->distance * self->position) + (alloc.width - self->child_width) / 2.0;
  else
    offset = (self->distance * self->position) - (alloc.width - self->child_width) / 2.0;

  if (self->orientation == GTK_ORIENTATION_VERTICAL)
    y -= offset;
  else
    x -= offset;

  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;

    if (!gtk_widget_get_visible (child_info->widget))
      continue;

    if (self->orientation == GTK_ORIENTATION_VERTICAL) {
      child_info->position = y;
      child_info->visible = child_info->position < alloc.height &&
                            child_info->position + self->child_height > 0;
      gdk_window_move (child_info->window, alloc.x, alloc.y + child_info->position);
    } else {
      child_info->position = x;
      child_info->visible = child_info->position < alloc.width &&
                            child_info->position + self->child_width > 0;
      gdk_window_move (child_info->window, alloc.x + child_info->position, alloc.y);
    }

    if (!child_info->visible)
      invalidate_cache_for_child (self, child_info);

    if (self->orientation == GTK_ORIENTATION_VERTICAL)
      y += self->distance;
    else if (is_rtl)
      x -= self->distance;
    else
      x += self->distance;
  }
}

static void
hdy_carousel_box_map (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (hdy_carousel_box_parent_class)->map (widget);

  gtk_widget_queue_draw (GTK_WIDGET (widget));
}

static void
hdy_carousel_box_realize (GtkWidget *widget)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);

  GTK_WIDGET_CLASS (hdy_carousel_box_parent_class)->realize (widget);

  g_list_foreach (self->children, (GFunc) register_window, self);

  gtk_widget_queue_allocate (widget);
}

static void
hdy_carousel_box_unrealize (GtkWidget *widget)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);

  g_list_foreach (self->children, (GFunc) unregister_window, self);

  GTK_WIDGET_CLASS (hdy_carousel_box_parent_class)->unrealize (widget);
}

static void
hdy_carousel_box_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);
  gint size, width, height;
  GList *children;

  gtk_widget_set_allocation (widget, allocation);

  size = 0;
  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;
    GtkWidget *child = child_info->widget;
    gint min, nat;
    gint child_size;

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      gtk_widget_get_preferred_width_for_height (child, allocation->height,
                                                 &min, &nat);
      if (gtk_widget_get_hexpand (child))
        child_size = MAX (min, allocation->width);
      else
        child_size = MAX (min, nat);
    } else {
      gtk_widget_get_preferred_height_for_width (child, allocation->width,
                                                 &min, &nat);
      if (gtk_widget_get_vexpand (child))
        child_size = MAX (min, allocation->height);
      else
        child_size = MAX (min, nat);
    }

    size = MAX (size, child_size);
  }

  self->distance = size + self->spacing;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    width = size;
    height = allocation->height;
  } else {
    width = allocation->width;
    height = size;
  }

  if (width != self->child_width || height != self->child_height)
    invalidate_drawing_cache (self);

  self->child_width = width;
  self->child_height = height;

  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;

    if (!gtk_widget_get_visible (child_info->widget))
      continue;

    if (!gtk_widget_get_realized (GTK_WIDGET (self)))
      continue;

    gdk_window_resize (child_info->window, width, height);
  }

  update_windows (self);

  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;
    GtkWidget *child = child_info->widget;
    GtkAllocation alloc;

    if (!gtk_widget_get_visible (child))
      continue;

    alloc.x = 0;
    alloc.y = 0;
    alloc.width = width;
    alloc.height = height;
    gtk_widget_size_allocate (child, &alloc);
  }

  invalidate_drawing_cache (self);
  gtk_widget_set_clip (widget, allocation);
}

static void
hdy_carousel_box_add (GtkContainer *container,
                      GtkWidget    *widget)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (container);

  hdy_carousel_box_insert (self, widget, -1);
}

static void
shift_position (HdyCarouselBox *self,
                gdouble         delta)
{
  hdy_carousel_box_set_position (self, self->position + delta);
  g_signal_emit (self, signals[SIGNAL_POSITION_SHIFTED], 0, delta);
}

static void
hdy_carousel_box_remove (GtkContainer *container,
                         GtkWidget    *widget)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (container);
  gint index;
  gdouble closest_point;
  HdyCarouselBoxChildInfo *info;

  info = find_child_info (self, widget);
  if (!info)
    return;

  closest_point = hdy_carousel_box_get_closest_snap_point (self);

  gtk_widget_unparent (widget);
  index = g_list_index (self->children, info);
  self->children = g_list_remove (self->children, info);

  if (gtk_widget_get_realized (GTK_WIDGET (container)))
    unregister_window (info, self);

  if (closest_point >= index)
    shift_position (self, -1);
  else
    gtk_widget_queue_allocate (GTK_WIDGET (self));

  free_child_info (info);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static void
hdy_carousel_box_forall (GtkContainer *container,
                         gboolean      include_internals,
                         GtkCallback   callback,
                         gpointer      callback_data)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (container);
  GList *list;
  GtkWidget *child;

  list = self->children;
  while (list) {
    HdyCarouselBoxChildInfo *child_info = list->data;
    child = child_info->widget;
    list = list->next;

    (* callback) (child, callback_data);
  }
}

static void
hdy_carousel_box_finalize (GObject *object)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (object);

  hdy_carousel_box_stop_animation (self);

  g_list_free_full (self->children, (GDestroyNotify) free_child_info);

  G_OBJECT_CLASS (hdy_carousel_box_parent_class)->finalize (object);
}

static void
hdy_carousel_box_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (object);

  switch (prop_id) {
  case PROP_N_PAGES:
    g_value_set_uint (value, hdy_carousel_box_get_n_pages (self));
    break;

  case PROP_POSITION:
    g_value_set_double (value, hdy_carousel_box_get_position (self));
    break;

  case PROP_SPACING:
    g_value_set_uint (value, hdy_carousel_box_get_spacing (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_carousel_box_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (object);

  switch (prop_id) {
  case PROP_POSITION:
    hdy_carousel_box_set_position (self, g_value_get_double (value));
    break;

  case PROP_SPACING:
    hdy_carousel_box_set_spacing (self, g_value_get_uint (value));
    break;

  case PROP_ORIENTATION:
    {
      GtkOrientation orientation = g_value_get_enum (value);
      if (orientation != self->orientation) {
        self->orientation = orientation;
        gtk_widget_queue_resize (GTK_WIDGET (self));
        g_object_notify (G_OBJECT (self), "orientation");
      }
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_carousel_box_class_init (HdyCarouselBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = hdy_carousel_box_finalize;
  object_class->get_property = hdy_carousel_box_get_property;
  object_class->set_property = hdy_carousel_box_set_property;
  widget_class->draw = hdy_carousel_box_draw;
  widget_class->get_preferred_width = hdy_carousel_box_get_preferred_width;
  widget_class->get_preferred_height = hdy_carousel_box_get_preferred_height;
  widget_class->get_preferred_width_for_height = hdy_carousel_box_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = hdy_carousel_box_get_preferred_height_for_width;
  widget_class->map = hdy_carousel_box_map;
  widget_class->realize = hdy_carousel_box_realize;
  widget_class->unrealize = hdy_carousel_box_unrealize;
  widget_class->size_allocate = hdy_carousel_box_size_allocate;
  container_class->add = hdy_carousel_box_add;
  container_class->remove = hdy_carousel_box_remove;
  container_class->forall = hdy_carousel_box_forall;

  /**
   * HdyCarouselBox:n-pages:
   *
   * The number of pages in a #HdyCarouselBox
   *
   * Since: 1.0
   */
  props[PROP_N_PAGES] =
    g_param_spec_uint ("n-pages",
                       _("Number of pages"),
                       _("Number of pages"),
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarouselBox:position:
   *
   * Current scrolling position, unitless. 1 matches 1 page.
   *
   * Since: 1.0
   */
  props[PROP_POSITION] =
    g_param_spec_double ("position",
                         _("Position"),
                         _("Current scrolling position"),
                         0,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarouselBox:spacing:
   *
   * Spacing between pages in pixels.
   *
   * Since: 1.0
   */
  props[PROP_SPACING] =
    g_param_spec_uint ("spacing",
                       _("Spacing"),
                       _("Spacing between pages"),
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * HdyCarouselBox::animation-stopped:
   * @self: The #HdyCarouselBox instance
   *
   * This signal is emitted after an animation has been stopped. If animations
   * are disabled, the signal is emitted as well.
   *
   * Since: 1.0
   */
  signals[SIGNAL_ANIMATION_STOPPED] =
    g_signal_new ("animation-stopped",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  /**
   * HdyCarouselBox::position-shifted:
   * @self: The #HdyCarouselBox instance
   * @delta: The amount to shift the position by
   *
   * This signal is emitted when position has been programmatically shifted.
   *
   * Since: 1.0
   */
  signals[SIGNAL_POSITION_SHIFTED] =
    g_signal_new ("position-shifted",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_DOUBLE);
}

static void
hdy_carousel_box_init (HdyCarouselBox *self)
{
  GtkWidget *widget = GTK_WIDGET (self);

  self->orientation = GTK_ORIENTATION_HORIZONTAL;

  gtk_widget_set_has_window (widget, FALSE);
}

/**
 * hdy_carousel_box_new:
 *
 * Create a new #HdyCarouselBox widget.
 *
 * Returns: The newly created #HdyCarouselBox widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_carousel_box_new (void)
{
  return g_object_new (HDY_TYPE_CAROUSEL_BOX, NULL);
}

/**
 * hdy_carousel_box_insert:
 * @self: a #HdyCarouselBox
 * @widget: a widget to add
 * @position: the position to insert @widget in.
 *
 * Inserts @widget into @self at position @position.
 *
 * If position is -1, or larger than the number of pages, @widget will be
 * appended to the end.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_insert (HdyCarouselBox *self,
                         GtkWidget      *widget,
                         gint            position)
{
  HdyCarouselBoxChildInfo *info;
  gdouble closest_point;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  info = g_new0 (HdyCarouselBoxChildInfo, 1);
  info->widget = widget;

  if (gtk_widget_get_realized (GTK_WIDGET (self)))
    register_window (info, self);

  closest_point = hdy_carousel_box_get_closest_snap_point (self);

  self->children = g_list_insert (self->children, info, position);
  if (closest_point >= position && position >= 0)
    shift_position (self, 1);

  gtk_widget_set_parent (widget, GTK_WIDGET (self));

  invalidate_drawing_cache (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

/**
 * hdy_carousel_box_reorder:
 * @self: a #HdyCarouselBox
 * @widget: a widget to add
 * @position: the position to move @widget to.
 *
 * Moves @widget into position @position.
 *
 * If position is -1, or larger than the number of pages, @widget will be moved
 * to the end.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_reorder (HdyCarouselBox *self,
                          GtkWidget      *widget,
                          gint            position)
{
  HdyCarouselBoxChildInfo *info;
  GList *link;
  gint old_position;
  gdouble closest_point;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  closest_point = hdy_carousel_box_get_closest_snap_point (self);

  info = find_child_info (self, widget);
  link = g_list_find (self->children, info);
  old_position = g_list_position (self->children, link);
  self->children = g_list_delete_link (self->children, link);
  if (position < 0 || position >= hdy_carousel_box_get_n_pages (self))
    link = NULL;
  else {
    if (position > old_position)
      position--;
    link = g_list_nth (self->children, position);
  }

  self->children = g_list_insert_before (self->children, link, info);

  if (closest_point == old_position)
    shift_position (self, position - old_position);
  else if (old_position > closest_point && closest_point >= position)
    shift_position (self, 1);
  else if (position >= closest_point && closest_point > old_position)
    shift_position (self, -1);
}

/**
 * hdy_carousel_box_is_animating:
 * @self: a #HdyCarouselBox
 *
 * Get whether @self is animating position.
 *
 * Returns: %TRUE if an animation is running
 *
 * Since: 1.0
 */
gboolean
hdy_carousel_box_is_animating (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), FALSE);

  return (self->animation_data.tick_cb_id != 0);
}

/**
 * hdy_carousel_box_stop_animation:
 * @self: a #HdyCarouselBox
 *
 * Stops a running animation. If there's no animation running, does nothing.
 *
 * It does not reset position to a non-transient value automatically.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_stop_animation (HdyCarouselBox *self)
{
  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  if (self->animation_data.tick_cb_id == 0)
    return;

  gtk_widget_remove_tick_callback (GTK_WIDGET (self),
                                   self->animation_data.tick_cb_id);
  self->animation_data.tick_cb_id = 0;
}

/**
 * hdy_carousel_box_scroll_to:
 * @self: a #HdyCarouselBox
 * @widget: a child of @self
 * @duration: animation duration in milliseconds
 *
 * Scrolls to @widget position over the next @duration milliseconds using
 * easeOutCubic interpolator.
 *
 * If an animation was already running, it will be cancelled automatically.
 *
 * @duration can be 0, in that case the position will be
 * changed immediately.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_scroll_to (HdyCarouselBox *self,
                            GtkWidget      *widget,
                            gint64          duration)
{
  GdkFrameClock *frame_clock;
  gint64 frame_time;
  gdouble position;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (duration >= 0);

  position = find_child_index (self, widget);

  hdy_carousel_box_stop_animation (self);

  if (duration <= 0 || !hdy_get_enable_animations (GTK_WIDGET (self))) {
    hdy_carousel_box_set_position (self, position);
    g_signal_emit (self, signals[SIGNAL_ANIMATION_STOPPED], 0);
    return;
  }

  frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));
  if (!frame_clock) {
    hdy_carousel_box_set_position (self, position);
    g_signal_emit (self, signals[SIGNAL_ANIMATION_STOPPED], 0);
    return;
  }

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);

  self->animation_data.start_position = self->position;
  self->animation_data.end_position = position;

  self->animation_data.start_time = frame_time / 1000;
  self->animation_data.end_time = self->animation_data.start_time + duration;
  self->animation_data.tick_cb_id =
    gtk_widget_add_tick_callback (GTK_WIDGET (self), animation_cb, self, NULL);
}

/**
 * hdy_carousel_box_get_n_pages:
 * @self: a #HdyCarouselBox
 *
 * Gets the number of pages in @self.
 *
 * Returns: The number of pages in @self
 *
 * Since: 1.0
 */
guint
hdy_carousel_box_get_n_pages (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return g_list_length (self->children);
}

/**
 * hdy_carousel_box_get_distance:
 * @self: a #HdyCarouselBox
 *
 * Gets swiping distance between two adjacent children in pixels.
 *
 * Returns: The swiping distance in pixels
 *
 * Since: 1.0
 */
gdouble
hdy_carousel_box_get_distance (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return self->distance;
}

/**
 * hdy_carousel_box_get_position:
 * @self: a #HdyCarouselBox
 *
 * Gets current scroll position in @self. It's unitless, 1 matches 1 page.
 *
 * Returns: The scroll position
 *
 * Since: 1.0
 */
gdouble
hdy_carousel_box_get_position (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return self->position;
}

/**
 * hdy_carousel_box_set_position:
 * @self: a #HdyCarouselBox
 * @position: the new position value
 *
 * Sets current scroll position in @self, unitless, 1 matches 1 page.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_set_position (HdyCarouselBox *self,
                               gdouble         position)
{
  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  position = CLAMP (position, 0, hdy_carousel_box_get_n_pages (self) - 1);

  self->position = position;
  update_windows (self);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POSITION]);
}

/**
 * hdy_carousel_box_get_spacing:
 * @self: a #HdyCarouselBox
 *
 * Gets spacing between pages in pixels.
 *
 * Returns: Spacing between pages
 *
 * Since: 1.0
 */
guint
hdy_carousel_box_get_spacing (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return self->spacing;
}

/**
 * hdy_carousel_box_set_spacing:
 * @self: a #HdyCarouselBox
 * @spacing: the new spacing value
 *
 * Sets spacing between pages in pixels.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_set_spacing (HdyCarouselBox *self,
                              guint           spacing)
{
  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  if (self->spacing == spacing)
    return;

  self->spacing = spacing;
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPACING]);
}

/**
 * hdy_carousel_box_get_nth_child:
 * @self: a #HdyCarouselBox
 * @n: the child index
 *
 * Retrieves @n-th child widget of @self.
 *
 * Returns: The @n-th child widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_carousel_box_get_nth_child (HdyCarouselBox *self,
                                guint           n)
{
  HdyCarouselBoxChildInfo *info;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), NULL);
  g_return_val_if_fail (n < g_list_length (self->children), NULL);

  info = g_list_nth_data (self->children, n);

  return info->widget;
}

/**
 * hdy_carousel_box_get_snap_points:
 * @self: a #HdyCarouselBox
 * @n_snap_points: (out)
 *
 * Gets the snap points of @self, representing the points between each page,
 * before the first page and after the last page.
 *
 * Returns: (array length=n_snap_points) (transfer full): the snap points of @self
 *
 * Since: 1.0
 */
gdouble *
hdy_carousel_box_get_snap_points (HdyCarouselBox *self,
                                  gint           *n_snap_points)
{
  guint i, n_pages;
  gdouble *points;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), NULL);

  n_pages = hdy_carousel_box_get_n_pages (self);

  points = g_new (gdouble, n_pages);
  for (i = 0; i < n_pages; i++)
    points[i] = i;

  if (n_snap_points)
    *n_snap_points = n_pages;

  return points;
}

/**
 * hdy_carousel_box_get_range:
 * @self: a #HdyCarouselBox
 * @lower: (out) (allow-none): location to store the lowest possible position, or %NULL
 * @upper: (out) (allow-none): location to store the maximum possible position, or %NULL
 *
 * Gets the range of possible positions.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_get_range (HdyCarouselBox *self,
                            gdouble        *lower,
                            gdouble        *upper)
{
  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  if (lower)
    *lower = 0;

  if (upper)
    *upper = hdy_carousel_box_get_n_pages (self) - 1;
}

/**
 * hdy_carousel_box_get_closest_snap_point:
 * @self: a #HdyCarouselBox
 *
 * Gets the snap point closest to the current position.
 *
 * Returns: the closest snap point.
 *
 * Since: 1.0
 */
gdouble
hdy_carousel_box_get_closest_snap_point (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return CLAMP (round (self->position), 0,
                hdy_carousel_box_get_n_pages (self) - 1);
}

/**
 * hdy_carousel_box_get_page_at_position:
 * @self: a #HdyCarouselBox
 * @position: a scroll position
 *
 * Gets the page closest to @position. For example, if @position matches
 * the current position, the returned widget will match the currently
 * displayed page.
 *
 * Returns: the closest page.
 *
 * Since: 1.0
 */
GtkWidget *
hdy_carousel_box_get_page_at_position (HdyCarouselBox *self,
                                       gdouble         position)
{
  gdouble lower, upper;
  gint n;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), NULL);

  hdy_carousel_box_get_range (self, &lower, &upper);

  position = CLAMP (position, lower, upper);

  n = round (position);

  return hdy_carousel_box_get_nth_child (self, n);
}

/**
 * hdy_carousel_box_get_current_page_index:
 * @self: a #HdyCarouselBox
 *
 * Gets the index of the currently displayed page.
 *
 * Returns: the index of the current page.
 *
 * Since: 1.0
 */
gint
hdy_carousel_box_get_current_page_index (HdyCarouselBox *self)
{
  GtkWidget *child;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  child = hdy_carousel_box_get_page_at_position (self, self->position);

  return find_child_index (self, child);
}
