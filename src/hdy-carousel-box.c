/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-animation-private.h"
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

typedef struct _HdyCarouselBoxAnimation HdyCarouselBoxAnimation;

struct _HdyCarouselBoxAnimation
{
  gint64 start_time;
  gint64 end_time;
  gdouble start_value;
  gdouble end_value;
};

typedef struct _HdyCarouselBoxChildInfo HdyCarouselBoxChildInfo;

struct _HdyCarouselBoxChildInfo
{
  GtkWidget *widget;
  gint position;
  gboolean visible;
  gdouble size;
  gdouble snap_point;
  gboolean adding;
  gboolean removing;

  gboolean shift_position;
  HdyCarouselBoxAnimation resize_animation;
};

struct _HdyCarouselBox
{
  GtkWidget parent_instance;

  HdyCarouselBoxAnimation animation;
  HdyCarouselBoxChildInfo *destination_child;
  GList *children;

  gdouble distance;
  gdouble position;
  guint spacing;
  GtkOrientation orientation;
  guint reveal_duration;

  guint tick_cb_id;
};

G_DEFINE_TYPE_WITH_CODE (HdyCarouselBox, hdy_carousel_box, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL));

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_POSITION,
  PROP_SPACING,
  PROP_REVEAL_DURATION,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_REVEAL_DURATION + 1,
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
                  GtkWidget      *widget,
                  gboolean        count_removing)
{
  GList *l;
  gint i;

  i = 0;
  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    if (info->removing && !count_removing)
      continue;

    if (widget == info->widget)
      return i;

    i++;
  }

  return -1;
}

static GList *
get_nth_link (HdyCarouselBox *self,
              gint            n)
{

  GList *l;
  gint i;

  i = n;
  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    if (info->removing)
      continue;

    if (i-- == 0)
      return l;
  }

  return NULL;
}

static HdyCarouselBoxChildInfo *
get_closest_child_at (HdyCarouselBox *self,
                      gdouble         position,
                      gboolean        count_adding,
                      gboolean        count_removing)
{
  GList *l;
  HdyCarouselBoxChildInfo *closest_child = NULL;

  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *child = l->data;

    if (child->adding && !count_adding)
      continue;

    if (child->removing && !count_removing)
      continue;

    if (!closest_child ||
        ABS (closest_child->snap_point - position) >
        ABS (child->snap_point - position))
      closest_child = child;
  }

  return closest_child;
}

static gdouble
get_animation_value (HdyCarouselBoxAnimation *animation,
                     GdkFrameClock           *frame_clock)
{
  gint64 frame_time, duration;
  gdouble t;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;
  frame_time = MIN (frame_time, animation->end_time);

  duration = animation->end_time - animation->start_time;
  t = (gdouble) (frame_time - animation->start_time) / duration;
  t = hdy_ease_out_cubic (t);

  return hdy_lerp (animation->start_value, animation->end_value, t);
}

static gboolean
animate_position (HdyCarouselBox *self,
                  GdkFrameClock  *frame_clock)
{
  gint64 frame_time;
  gdouble value;

  if (!hdy_carousel_box_is_animating (self))
    return G_SOURCE_REMOVE;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;

  self->animation.end_value = self->destination_child->snap_point;
  value = get_animation_value (&self->animation, frame_clock);
  hdy_carousel_box_set_position (self, value);

  if (frame_time >= self->animation.end_time) {
    self->animation.start_time = 0;
    self->animation.end_time = 0;
    g_signal_emit (self, signals[SIGNAL_ANIMATION_STOPPED], 0);
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
complete_child_animation (HdyCarouselBox          *self,
                          HdyCarouselBoxChildInfo *child)
{
  gtk_widget_queue_allocate (GTK_WIDGET (self));

  if (child->adding)
    child->adding = FALSE;

  if (child->removing) {
    self->children = g_list_remove (self->children, child);

    g_free (child);
  }
}

static gboolean
animate_child_size (HdyCarouselBox          *self,
                    HdyCarouselBoxChildInfo *child,
                    GdkFrameClock           *frame_clock,
                    gdouble                 *delta)
{
  gint64 frame_time;
  gdouble d, new_value;

  if (child->resize_animation.start_time == 0)
    return G_SOURCE_REMOVE;

  new_value = get_animation_value (&child->resize_animation, frame_clock);
  d = new_value - child->size;

  child->size += d;

  frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;

  if (delta)
    *delta = d;

  if (frame_time >= child->resize_animation.end_time) {
    child->resize_animation.start_time = 0;
    child->resize_animation.end_time = 0;
    complete_child_animation (self, child);
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
set_position (HdyCarouselBox *self,
              gdouble         position)
{
  gdouble lower, upper;

  hdy_carousel_box_get_range (self, &lower, &upper);

  position = CLAMP (position, lower, upper);

  self->position = position;
  gtk_widget_queue_allocate (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POSITION]);
}

static gboolean
animation_cb (GtkWidget     *widget,
              GdkFrameClock *frame_clock,
              gpointer       user_data)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);
  g_autoptr (GList) children = NULL;
  GList *l;
  gboolean should_continue;
  gdouble position_shift;

  should_continue = G_SOURCE_REMOVE;

  position_shift = 0;

  children = g_list_copy (self->children);
  for (l = children; l; l = l->next) {
    HdyCarouselBoxChildInfo *child = l->data;
    gdouble delta;
    gboolean shift;

    delta = 0;
    shift = child->shift_position;

    should_continue |= animate_child_size (self, child, frame_clock, &delta);

    if (shift)
      position_shift += delta;
  }

  // FIXME there was update windows here, some stuff may depend on it

  if (position_shift != 0) {
    set_position (self, self->position + position_shift);
    g_signal_emit (self, signals[SIGNAL_POSITION_SHIFTED], 0, position_shift);
  }

  should_continue |= animate_position (self, frame_clock);

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  if (!should_continue)
    self->tick_cb_id = 0;

  return should_continue;
}

static void
update_shift_position_flag (HdyCarouselBox          *self,
                            HdyCarouselBoxChildInfo *child)
{
  HdyCarouselBoxChildInfo *closest_child;
  gint animating_index, closest_index;

  /* We want to still shift position when the active child is being removed */
  closest_child = get_closest_child_at (self, self->position, FALSE, TRUE);

  if (!closest_child)
    return;

  animating_index = g_list_index (self->children, child);
  closest_index = g_list_index (self->children, closest_child);

  child->shift_position = (closest_index >= animating_index);
}

static void
animate_child (HdyCarouselBox          *self,
               HdyCarouselBoxChildInfo *child,
               gdouble                  value,
               gint64                   duration)
{
  GdkFrameClock *frame_clock;
  gint64 frame_time;

  if (child->resize_animation.start_time > 0) {
    child->resize_animation.start_time = 0;
    child->resize_animation.end_time = 0;
  }

  update_shift_position_flag (self, child);

  if (!gtk_widget_get_realized (GTK_WIDGET (self)) ||
      duration <= 0 ||
      !hdy_get_enable_animations (GTK_WIDGET (self))) {
    gdouble delta = value - child->size;

    child->size = value;

    if (child->shift_position) {
      set_position (self, self->position + delta);
      g_signal_emit (self, signals[SIGNAL_POSITION_SHIFTED], 0, delta);
    }

    complete_child_animation (self, child);
    return;
  }

  frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));
  if (!frame_clock) {
    gdouble delta = value - child->size;

    child->size = value;

    if (child->shift_position) {
      set_position (self, self->position + delta);
      g_signal_emit (self, signals[SIGNAL_POSITION_SHIFTED], 0, delta);
    }

    complete_child_animation (self, child);
    return;
  }

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);

  child->resize_animation.start_value = child->size;
  child->resize_animation.end_value = value;

  child->resize_animation.start_time = frame_time / 1000;
  child->resize_animation.end_time = child->resize_animation.start_time + duration;
  if (self->tick_cb_id == 0)
    self->tick_cb_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self), animation_cb, self, NULL);
}

static void
hdy_carousel_box_measure (GtkWidget      *widget,
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

    if (child_info->removing)
      continue;

    if (!gtk_widget_get_visible (child))
      continue;

    gtk_widget_measure (child, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);

    if (minimum)
      *minimum = MAX (*minimum, child_min);
    if (natural)
      *natural = MAX (*natural, child_nat);
  }
}

static void
hdy_carousel_box_size_allocate (GtkWidget *widget,
                                gint       width,
                                gint       height,
                                gint       baseline)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (widget);
  gint size, child_width, child_height;
  GList *children;
  gdouble x, y, offset;
  gboolean is_rtl;
  gdouble snap_point;

  size = 0;
  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;
    GtkWidget *child = child_info->widget;
    gint min, nat;
    gint child_size;

    if (child_info->removing)
      continue;

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      gtk_widget_measure (child, self->orientation,
                          height, &min, &nat, NULL, NULL);
      if (gtk_widget_get_hexpand (child))
        child_size = MAX (min, width);
      else
        child_size = MAX (min, nat);
    } else {
      gtk_widget_measure (child, self->orientation,
                          width, &min, &nat, NULL, NULL);
      if (gtk_widget_get_vexpand (child))
        child_size = MAX (min, height);
      else
        child_size = MAX (min, nat);
    }

    size = MAX (size, child_size);
  }

  self->distance = size + self->spacing;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    child_width = size;
    child_height = height;
  } else {
    child_width = width;
    child_height = size;
  }

  snap_point = 0;

  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;

    child_info->snap_point = snap_point + child_info->size - 1;

    snap_point += child_info->size;
  }

  if (!gtk_widget_get_realized (GTK_WIDGET (self)))
    return;

  x = 0;
  y = 0;

  is_rtl = (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL);

  if (self->orientation == GTK_ORIENTATION_VERTICAL)
    offset = (self->distance * self->position) - (height - child_height) / 2.0;
  else if (is_rtl)
    offset = -(self->distance * self->position) + (width - child_width) / 2.0;
  else
    offset = (self->distance * self->position) - (width - child_width) / 2.0;

  if (self->orientation == GTK_ORIENTATION_VERTICAL)
    y -= offset;
  else
    x -= offset;

  for (children = self->children; children; children = children->next) {
    HdyCarouselBoxChildInfo *child_info = children->data;

    GskTransform *transform = gsk_transform_new ();

    if (!child_info->removing) {
      if (!gtk_widget_get_visible (child_info->widget))
        continue;

      if (self->orientation == GTK_ORIENTATION_VERTICAL) {
        child_info->position = y;
        child_info->visible = child_info->position < height &&
                              child_info->position + child_height > 0;

        transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (0, child_info->position));
      } else {
        child_info->position = x;
        child_info->visible = child_info->position < width &&
                              child_info->position + child_width > 0;

        transform = gsk_transform_translate (transform, &GRAPHENE_POINT_INIT (child_info->position, 0));
      }

      gtk_widget_allocate (child_info->widget, child_width, child_height, baseline, transform);
    }

    if (self->orientation == GTK_ORIENTATION_VERTICAL)
      y += self->distance * child_info->size;
    else if (is_rtl)
      x -= self->distance * child_info->size;
    else
      x += self->distance * child_info->size;
  }
}

static void
shift_position (HdyCarouselBox *self,
                gdouble         delta)
{
  hdy_carousel_box_set_position (self, self->position + delta);
  g_signal_emit (self, signals[SIGNAL_POSITION_SHIFTED], 0, delta);
}

static void
hdy_carousel_box_finalize (GObject *object)
{
  HdyCarouselBox *self = HDY_CAROUSEL_BOX (object);

  if (self->tick_cb_id > 0)
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tick_cb_id);

  g_list_free_full (self->children, (GDestroyNotify) g_free);

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

  case PROP_REVEAL_DURATION:
    g_value_set_uint (value, hdy_carousel_box_get_reveal_duration (self));
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

  case PROP_REVEAL_DURATION:
    hdy_carousel_box_set_reveal_duration (self, g_value_get_uint (value));
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

  object_class->finalize = hdy_carousel_box_finalize;
  object_class->get_property = hdy_carousel_box_get_property;
  object_class->set_property = hdy_carousel_box_set_property;
  widget_class->measure = hdy_carousel_box_measure;
  widget_class->size_allocate = hdy_carousel_box_size_allocate;

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

  /**
   * HdyCarouselBox:reveal-duration:
   *
   * Duration of the animation used when adding or removing pages, in
   * milliseconds.
   *
   * Since: 1.0
   */
  props[PROP_REVEAL_DURATION] =
    g_param_spec_uint ("reveal-duration",
                       _("Reveal duration"),
                       _("Page reveal duration"),
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
  self->orientation = GTK_ORIENTATION_HORIZONTAL;
  self->reveal_duration = 0;
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
  GList *prev_link;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  info = g_new0 (HdyCarouselBoxChildInfo, 1);
  info->widget = widget;
  info->size = 0;
  info->adding = TRUE;

  if (position >= 0)
    prev_link = get_nth_link (self, position);
  else
    prev_link = NULL;

  self->children = g_list_insert_before (self->children, prev_link, info);

  gtk_widget_set_parent (widget, GTK_WIDGET (self));

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  animate_child (self, info, 1, self->reveal_duration);

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
  HdyCarouselBoxChildInfo *info, *prev_info;
  GList *link, *prev_link;
  gint old_position;
  gdouble closest_point, old_point, new_point;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  closest_point = hdy_carousel_box_get_closest_snap_point (self);

  info = find_child_info (self, widget);
  link = g_list_find (self->children, info);
  old_position = g_list_position (self->children, link);

  if (position == old_position)
    return;

  old_point = ((HdyCarouselBoxChildInfo *) link->data)->snap_point;

  if (position < 0 || position >= hdy_carousel_box_get_n_pages (self))
    prev_link = g_list_last (self->children);
  else
    prev_link = get_nth_link (self, position);

  prev_info = prev_link->data;
  new_point = prev_info->snap_point;
  if (new_point > old_point)
    new_point -= prev_info->size;

  self->children = g_list_remove_link (self->children, link);
  self->children = g_list_insert_before (self->children, prev_link, link->data);

  if (closest_point == old_point)
    shift_position (self, new_point - old_point);
  else if (old_point > closest_point && closest_point >= new_point)
    shift_position (self, info->size);
  else if (new_point >= closest_point && closest_point > old_point)
    shift_position (self, -info->size);
}

void
hdy_carousel_box_remove (HdyCarouselBox *self,
                         GtkWidget      *widget)
{
  HdyCarouselBoxChildInfo *info;

  info = find_child_info (self, widget);
  if (!info)
    return;

  info->removing = TRUE;

  gtk_widget_unparent (widget);

  info->widget = NULL;

  if (!gtk_widget_in_destruction (GTK_WIDGET (self)))
    animate_child (self, info, 0, self->reveal_duration);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
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

  return (self->animation.start_time != 0);
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

  if (self->animation.start_time == 0)
    return;

  self->animation.start_time = 0;
  self->animation.end_time = 0;
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
  HdyCarouselBoxChildInfo *child;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (duration >= 0);

  child = find_child_info (self, widget);
  position = child->snap_point;

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

  self->destination_child = child;

  self->animation.start_value = self->position;
  self->animation.end_value = position;

  self->animation.start_time = frame_time / 1000;
  self->animation.end_time = self->animation.start_time + duration;
  if (self->tick_cb_id == 0)
    self->tick_cb_id =
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
  GList *l;
  guint n_pages;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  n_pages = 0;
  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *child = l->data;

    if (!child->removing)
      n_pages++;
  }

  return n_pages;
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
  GList *l;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  set_position (self, position);

  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *child = l->data;

    if (child->adding || child->removing)
      update_shift_position_flag (self, child);
  }
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
 * hdy_carousel_box_get_reveal_duration:
 * @self: a #HdyCarouselBox
 *
 * Gets duration of the animation used when adding or removing pages in
 * milliseconds.
 *
 * Returns: Page reveal duration
 *
 * Since: 1.0
 */
guint
hdy_carousel_box_get_reveal_duration (HdyCarouselBox *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return self->reveal_duration;
}

/**
 * hdy_carousel_box_set_reveal_duration:
 * @self: a #HdyCarouselBox
 * @reveal_duration: the new reveal duration value
 *
 * Sets duration of the animation used when adding or removing pages in
 * milliseconds.
 *
 * Since: 1.0
 */
void
hdy_carousel_box_set_reveal_duration (HdyCarouselBox *self,
                                      guint           reveal_duration)
{
  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  if (self->reveal_duration == reveal_duration)
    return;

  self->reveal_duration = reveal_duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_DURATION]);
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
  g_return_val_if_fail (n < hdy_carousel_box_get_n_pages (self), NULL);

  info = get_nth_link (self, n)->data;

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
  GList *l;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), NULL);

  n_pages = MAX (g_list_length (self->children), 1);

  points = g_new0 (gdouble, n_pages);

  i = 0;
  for (l = self->children; l; l = l->next) {
    HdyCarouselBoxChildInfo *info = l->data;

    points[i++] = info->snap_point;
  }

  if (n_snap_points)
    *n_snap_points = n_pages;

  return points;
}

/**
 * hdy_carousel_box_get_range:
 * @self: a #HdyCarouselBox
 * @lower: (out) (optional): location to store the lowest possible position, or %NULL
 * @upper: (out) (optional): location to store the maximum possible position, or %NULL
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
  GList *l;
  HdyCarouselBoxChildInfo *child;

  g_return_if_fail (HDY_IS_CAROUSEL_BOX (self));

  l = g_list_last (self->children);
  child = l ? l->data : NULL;

  if (lower)
    *lower = 0;

  if (upper)
    *upper = child ? child->snap_point : 0;
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
  HdyCarouselBoxChildInfo *closest_child;

  closest_child = get_closest_child_at (self, self->position, TRUE, TRUE);

  if (!closest_child)
    return 0;

  return closest_child->snap_point;
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
 * Returns: (nullable): the closest page.
 *
 * Since: 1.0
 */
GtkWidget *
hdy_carousel_box_get_page_at_position (HdyCarouselBox *self,
                                       gdouble         position)
{
  gdouble lower, upper;
  HdyCarouselBoxChildInfo *child;

  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), NULL);

  hdy_carousel_box_get_range (self, &lower, &upper);

  position = CLAMP (position, lower, upper);

  child = get_closest_child_at (self, position, TRUE, FALSE);

  if (!child)
    return NULL;

  return child->widget;
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

  return find_child_index (self, child, FALSE);
}

gint
hdy_carousel_box_get_page_index (HdyCarouselBox *self,
                                 GtkWidget      *child)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_BOX (self), 0);

  return find_child_index (self, child, FALSE);
}
