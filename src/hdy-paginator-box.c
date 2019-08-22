/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-paginator-box-private.h"
#include "hdy-animation-private.h"

#include <math.h>

/**
 * PRIVATE:hdy-paginator-box
 * @short_description: Scrolling box used in #HdyPaginator
 * @title: HdyPaginatorBox
 * @See_also: #HdyPaginator
 * @stability: Private
 *
 * The #HdyPaginatorBox object is meant to be used exclusively as part of
 * the #HdyPaginator implementation.
 *
 * Since: 0.0.11
 */

struct _HdyPaginatorBox
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

  gdouble distance;
  gdouble position;
  guint spacing;
  GtkOrientation orientation;
};

G_DEFINE_TYPE_WITH_CODE (HdyPaginatorBox, hdy_paginator_box, GTK_TYPE_CONTAINER,
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

static gboolean
animation_cb (GtkWidget     *widget,
              GdkFrameClock *frame_clock,
              gpointer       user_data)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (widget);
  gint64 frame_time, duration;
  gdouble position;
  gdouble t;

  g_assert (hdy_paginator_box_is_animating (self));

  frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;
  frame_time = MIN (frame_time, self->animation_data.end_time);

  duration = self->animation_data.end_time - self->animation_data.start_time;
  position = (gdouble) (frame_time - self->animation_data.start_time) / duration;

  t = hdy_ease_out_cubic (position);
  hdy_paginator_box_set_position (self,
                                  hdy_lerp (self->animation_data.start_position,
                                            self->animation_data.end_position, 1 - t));

  if (frame_time == self->animation_data.end_time) {
    self->animation_data.tick_cb_id = 0;
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
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
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (widget);
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
    GtkWidget *child = children->data;
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
hdy_paginator_box_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum_width,
                                       gint      *natural_width)
{
  measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
           minimum_width, natural_width, NULL, NULL);
}

static void
hdy_paginator_box_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum_height,
                                        gint      *natural_height)
{
  measure (widget, GTK_ORIENTATION_VERTICAL, -1,
           minimum_height, natural_height, NULL, NULL);
}

static void
hdy_paginator_box_get_preferred_width_for_height (GtkWidget *widget,
                                                  gint       for_height,
                                                  gint      *minimum_width,
                                                  gint      *natural_width)
{
  measure (widget, GTK_ORIENTATION_HORIZONTAL, for_height,
           minimum_width, natural_width, NULL, NULL);
}

static void
hdy_paginator_box_get_preferred_height_for_width (GtkWidget *widget,
                                                  gint       for_width,
                                                  gint      *minimum_height,
                                                  gint      *natural_height)
{
  measure (widget, GTK_ORIENTATION_VERTICAL, for_width,
           minimum_height, natural_height, NULL, NULL);
}

static void
hdy_paginator_box_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (widget);
  gint x, y, size;
  GList *children;
  gboolean is_rtl;

  gtk_widget_set_allocation (widget, allocation);

  x = allocation->x;
  y = allocation->y;

  size = 0;
  for (children = self->children; children; children = children->next) {
    GtkWidget *child = children->data;
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

  is_rtl = (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL);

  if (self->orientation == GTK_ORIENTATION_VERTICAL)
    y -= (self->distance * self->position) - (allocation->height - size) / 2.0;
  else if (is_rtl)
    x += (self->distance * self->position) + (allocation->width - size) / 2.0;
  else
    x -= (self->distance * self->position) - (allocation->width - size) / 2.0;

  for (children = self->children; children; children = children->next) {
    GtkWidget *child = children->data;
    gint width, height;
    GtkAllocation alloc;

    if (!gtk_widget_get_visible (child))
      continue;

    if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
      width = size;
      height = allocation->height;
    } else {
      width = allocation->width;
      height = size;
    }

    alloc.x = x;
    alloc.y = y;
    alloc.width = width;
    alloc.height = height;
    gtk_widget_size_allocate (child, &alloc);

    gtk_widget_set_child_visible (child,
                                  (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
                                   x < allocation->width && x + width > 0) ||
                                  (self->orientation == GTK_ORIENTATION_VERTICAL &&
                                   y < allocation->height && y + height > 0));

    if (self->orientation == GTK_ORIENTATION_VERTICAL)
      y += self->distance;
    else if (is_rtl)
      x -= self->distance;
    else
      x += self->distance;
  }

  gtk_widget_set_clip (widget, allocation);
}

static void
hdy_paginator_box_add (GtkContainer *container,
                       GtkWidget    *widget)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (container);

  gtk_widget_set_parent (widget, GTK_WIDGET (container));
  self->children = g_list_append (self->children, widget);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static void
hdy_paginator_box_remove (GtkContainer *container,
                          GtkWidget    *widget)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (container);

  gtk_widget_unparent (widget);
  self->children = g_list_remove (self->children, widget);

  if (self->position >= g_list_length (self->children))
    hdy_paginator_box_set_position (self, self->position - 1);
  else
    gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static void
hdy_paginator_box_forall (GtkContainer *container,
                          gboolean      include_internals,
                          GtkCallback   callback,
                          gpointer      callback_data)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (container);
  GList *list;
  GtkWidget *child;

  list = self->children;
  while (list) {
    child = list->data;
    list = list->next;

    (* callback) (child, callback_data);
  }
}

static void
hdy_paginator_box_finalize (GObject *object)
{
  HdyPaginatorBox *self = (HdyPaginatorBox *)object;

  hdy_paginator_box_stop_animation (self);

  g_list_free (self->children);

  G_OBJECT_CLASS (hdy_paginator_box_parent_class)->finalize (object);
}

static void
hdy_paginator_box_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (object);

  switch (prop_id) {
  case PROP_N_PAGES:
    g_value_set_uint (value, hdy_paginator_box_get_n_pages (self));
    break;

  case PROP_POSITION:
    g_value_set_double (value, hdy_paginator_box_get_position (self));
    break;

  case PROP_SPACING:
    g_value_set_uint (value, hdy_paginator_box_get_spacing (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_paginator_box_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdyPaginatorBox *self = HDY_PAGINATOR_BOX (object);

  switch (prop_id) {
  case PROP_POSITION:
    hdy_paginator_box_set_position (self, g_value_get_double (value));
    break;

  case PROP_SPACING:
    hdy_paginator_box_set_spacing (self, g_value_get_uint (value));
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
hdy_paginator_box_class_init (HdyPaginatorBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = hdy_paginator_box_finalize;
  object_class->get_property = hdy_paginator_box_get_property;
  object_class->set_property = hdy_paginator_box_set_property;
  widget_class->get_preferred_width = hdy_paginator_box_get_preferred_width;
  widget_class->get_preferred_height = hdy_paginator_box_get_preferred_height;
  widget_class->get_preferred_width_for_height = hdy_paginator_box_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = hdy_paginator_box_get_preferred_height_for_width;
  widget_class->size_allocate = hdy_paginator_box_size_allocate;
  container_class->add = hdy_paginator_box_add;
  container_class->remove = hdy_paginator_box_remove;
  container_class->forall = hdy_paginator_box_forall;

  /**
   * HdyPaginatorBox:n-pages:
   *
   * The number of pages in a #HdyPaginatorBox
   *
   * Since: 0.0.11
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
   * HdyPaginatorBox:position:
   *
   * Current scrolling position, unitless. 1 matches 1 page.
   *
   * Since: 0.0.11
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
   * HdyPaginatorBox:spacing:
   *
   * Spacing between pages in pixels.
   *
   * Since: 0.0.11
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
}

static void
hdy_paginator_box_init (HdyPaginatorBox *self)
{
  GtkWidget *widget = GTK_WIDGET (self);

  self->orientation = GTK_ORIENTATION_HORIZONTAL;

  gtk_widget_set_has_window (widget, FALSE);
  gtk_widget_set_redraw_on_allocate (widget, FALSE);
}

/**
 * hdy_paginator_box_new:
 *
 * Create a new #HdyPaginatorBox widget.
 *
 * Returns: The newly created #HdyPaginatorBox widget
 *
 * Since: 0.0.11
 */
HdyPaginatorBox *
hdy_paginator_box_new (void)
{
  return g_object_new (HDY_TYPE_PAGINATOR_BOX, NULL);
}

/**
 * hdy_paginator_box_insert:
 * @self: a #HdyPaginatorBox
 * @child: a widget to add
 * @position: the position to insert @child in.
 *
 * Inserts @child into @self at position @position.
 *
 * If position is -1, or larger than the number of pages, @child will be
 * appended to the end.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_insert (HdyPaginatorBox *self,
                          GtkWidget       *child,
                          gint             position)
{
  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  gtk_container_add (GTK_CONTAINER (self), child);
  hdy_paginator_box_reorder (self, child, position);
}

/**
 * hdy_paginator_box_reorder:
 * @self: a #HdyPaginatorBox
 * @child: a widget to add
 * @position: the position to move @child to.
 *
 * Moves @child into position @position.
 *
 * If position is -1, or larger than the number of pages, @child will be moved
 * to the end.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_reorder (HdyPaginatorBox *self,
                           GtkWidget       *child,
                           gint             position)
{
  GList *link;
  gint old_position, current_page;

  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  link = g_list_find (self->children, child);
  old_position = g_list_position (self->children, link);
  self->children = g_list_delete_link (self->children, link);
  if (position < 0 || position >= hdy_paginator_box_get_n_pages (self))
    link = NULL;
  else
    link = g_list_nth (self->children, position);

  self->children = g_list_insert_before (self->children, link, child);

  current_page = round (self->position);
  if (current_page == old_position)
    hdy_paginator_box_set_position (self, position);
  else if (old_position > current_page && position <= current_page)
    hdy_paginator_box_set_position (self, self->position + 1);
  else if (old_position <= current_page && position > current_page)
    hdy_paginator_box_set_position (self, self->position - 1);
}

/**
 * hdy_paginator_box_animate:
 * @self: a #HdyPaginatorBox
 * @position: A value to animate to
 * @duration: Animation duration in milliseconds
 *
 * Animates the widget's position to @position over the next @duration
 * milliseconds using easeOutCubic interpolator.
 *
 * If an animation was already running, it will be cancelled automatically.
 *
 * @duration can be 0, in that case the position will be
 * changed immediately.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_animate (HdyPaginatorBox *self,
                           gdouble          position,
                           gint64           duration)
{
  GdkFrameClock *frame_clock;
  gint64 frame_time;

  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));

  hdy_paginator_box_stop_animation (self);

  if (duration <= 0) {
    hdy_paginator_box_set_position (self, position);
    return;
  }

  frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));
  if (!frame_clock) {
    hdy_paginator_box_set_position (self, position);
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
 * hdy_paginator_box_is_animating:
 * @self: a #HdyPaginatorBox
 *
 * Get whether @self is animating position.
 *
 * Returns: %TRUE if an animation is running
 *
 * Since: 0.0.11
 */
gboolean
hdy_paginator_box_is_animating (HdyPaginatorBox *self)
{
  g_return_val_if_fail (HDY_IS_PAGINATOR_BOX (self), FALSE);

  return (self->animation_data.tick_cb_id != 0);
}

/**
 * hdy_paginator_box_stop_animation:
 * @self: a #HdyPaginatorBox
 *
 * Stops a running animation. If there's no animation running, does nothing.
 *
 * It does not reset position to a non-transient value automatically.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_stop_animation (HdyPaginatorBox *self)
{
  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));

  if (self->animation_data.tick_cb_id == 0)
    return;

  gtk_widget_remove_tick_callback (GTK_WIDGET (self),
                                   self->animation_data.tick_cb_id);
  self->animation_data.tick_cb_id = 0;
}

/**
 * hdy_paginator_box_scroll_to:
 * @self: a #HdyPaginatorBox
 * @widget: a child of @self
 * @duration: animation duration in milliseconds
 *
 * Scrolls to @widget position with an animation. If @duration is 0, changes
 * the position immediately.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_scroll_to (HdyPaginatorBox *self,
                             GtkWidget       *widget,
                             gint64           duration)
{
  gint index;

  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (duration >= 0);

  index = g_list_index (self->children, widget);

  hdy_paginator_box_animate (self, index, duration);
}

/**
 * hdy_paginator_box_get_n_pages:
 * @self: a #HdyPaginatorBox
 *
 * Gets the number of pages in @self.
 *
 * Returns: The number of pages in @self
 *
 * Since: 0.0.11
 */
guint
hdy_paginator_box_get_n_pages (HdyPaginatorBox *self)
{
  g_return_val_if_fail (HDY_IS_PAGINATOR_BOX (self), 0);

  return g_list_length (self->children);
}

/**
 * hdy_paginator_box_get_distance:
 * @self: a #HdyPaginatorBox
 *
 * Gets swiping distance between two adjacent children in pixels.
 *
 * Returns: The swiping distance in pixels
 *
 * Since: 0.0.11
 */
gdouble
hdy_paginator_box_get_distance (HdyPaginatorBox *self)
{
  g_return_val_if_fail (HDY_IS_PAGINATOR_BOX (self), 0);

  return self->distance;
}

/**
 * hdy_paginator_box_get_position:
 * @self: a #HdyPaginatorBox
 *
 * Gets current scroll position in @self. It's unitless, 1 matches 1 page.
 *
 * Returns: The scroll position
 *
 * Since: 0.0.11
 */
gdouble
hdy_paginator_box_get_position (HdyPaginatorBox *self)
{
  g_return_val_if_fail (HDY_IS_PAGINATOR_BOX (self), 0);

  return self->position;
}

/**
 * hdy_paginator_box_set_position:
 * @self: a #HdyPaginatorBox
 * @position: the new position value
 *
 * Sets current scroll position in @self, unitless, 1 matches 1 page.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_set_position (HdyPaginatorBox *self,
                                gdouble          position)
{
  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));

  position = CLAMP (position, 0, hdy_paginator_box_get_n_pages (self) - 1);

  self->position = position;
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POSITION]);
}

/**
 * hdy_paginator_box_get_spacing:
 * @self: a #HdyPaginatorBox
 *
 * Gets spacing between pages in pixels.
 *
 * Returns: Spacing between pages
 *
 * Since: 0.0.11
 */
guint
hdy_paginator_box_get_spacing (HdyPaginatorBox *self)
{
  g_return_val_if_fail (HDY_IS_PAGINATOR_BOX (self), 0);

  return self->spacing;
}

/**
 * hdy_paginator_box_set_spacing:
 * @self: a #HdyPaginatorBox
 * @spacing: the new spacing value
 *
 * Sets spacing between pages in pixels.
 *
 * Since: 0.0.11
 */
void
hdy_paginator_box_set_spacing (HdyPaginatorBox *self,
                               guint            spacing)
{
  g_return_if_fail (HDY_IS_PAGINATOR_BOX (self));

  if (self->spacing == spacing)
    return;

  self->spacing = spacing;
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPACING]);
}
