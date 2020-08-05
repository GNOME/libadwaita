/*
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * Author: Alexander Larsson <alexl@redhat.com>
 * Author: Adrien Plazas <adrien.plazas@puri.sm>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

/*
 * Forked from the GTK+ 3.24.2 GtkStack widget initially written by Alexander
 * Larsson, and heavily modified for libhandy by Adrien Plazas on behalf of
 * Purism SPC 2019.
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-squeezer.h"

#include "gtkprogresstrackerprivate.h"
#include "hdy-animation-private.h"
#include "hdy-cairo-private.h"
#include "hdy-css-private.h"

/**
 * SECTION:hdy-squeezer
 * @short_description: A best fit container.
 * @Title: HdySqueezer
 *
 * The HdySqueezer widget is a container which only shows the first of its
 * children that fits in the available size. It is convenient to offer different
 * widgets to represent the same data with different levels of detail, making
 * the widget seem to squeeze itself to fit in the available space.
 *
 * Transitions between children can be animated as fades. This can be controlled
 * with hdy_squeezer_set_transition_type().
 *
 * # CSS nodes
 *
 * #HdySqueezer has a single CSS node with name squeezer.
 */

/**
 * HdySqueezerTransitionType:
 * @HDY_SQUEEZER_TRANSITION_TYPE_NONE: No transition
 * @HDY_SQUEEZER_TRANSITION_TYPE_CROSSFADE: A cross-fade
 *
 * These enumeration values describe the possible transitions between children
 * in a #HdySqueezer widget.
 */

enum  {
  PROP_0,
  PROP_HOMOGENEOUS,
  PROP_VISIBLE_CHILD,
  PROP_TRANSITION_DURATION,
  PROP_TRANSITION_TYPE,
  PROP_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,
  PROP_XALIGN,
  PROP_YALIGN,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_YALIGN + 1,
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_ENABLED,

  LAST_CHILD_PROP,
};

typedef struct {
  GtkWidget *widget;
  gboolean enabled;
  GtkWidget *last_focus;
} HdySqueezerChildInfo;

struct _HdySqueezer
{
  GtkContainer parent_instance;

  GList *children;

  GdkWindow* bin_window;
  GdkWindow* view_window;

  HdySqueezerChildInfo *visible_child;

  gboolean homogeneous;

  HdySqueezerTransitionType transition_type;
  guint transition_duration;

  HdySqueezerChildInfo *last_visible_child;
  cairo_surface_t *last_visible_surface;
  GtkAllocation last_visible_surface_allocation;
  guint tick_id;
  GtkProgressTracker tracker;
  gboolean first_frame_skipped;

  gint last_visible_widget_width;
  gint last_visible_widget_height;

  HdySqueezerTransitionType active_transition_type;

  gboolean interpolate_size;

  gfloat xalign;
  gfloat yalign;

  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];
static GParamSpec *child_props[LAST_CHILD_PROP];

G_DEFINE_TYPE_WITH_CODE (HdySqueezer, hdy_squeezer, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static GtkOrientation
get_orientation (HdySqueezer *self)
{
  return self->orientation;
}

static void
set_orientation (HdySqueezer    *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify (G_OBJECT (self), "orientation");
}

static HdySqueezerChildInfo *
find_child_info_for_widget (HdySqueezer *self,
                            GtkWidget   *child)
{
  HdySqueezerChildInfo *info;
  GList *l;

  for (l = self->children; l != NULL; l = l->next) {
    info = l->data;
    if (info->widget == child)
      return info;
  }

  return NULL;
}

static void
hdy_squeezer_progress_updated (HdySqueezer *self)
{
  gtk_widget_queue_draw (GTK_WIDGET (self));

  if (!self->homogeneous)
    gtk_widget_queue_resize (GTK_WIDGET (self));

  if (gtk_progress_tracker_get_state (&self->tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (self->last_visible_surface != NULL) {
      cairo_surface_destroy (self->last_visible_surface);
      self->last_visible_surface = NULL;
    }

    if (self->last_visible_child != NULL) {
      gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
      self->last_visible_child = NULL;
    }
  }
}

static gboolean
hdy_squeezer_transition_cb (GtkWidget     *widget,
                            GdkFrameClock *frame_clock,
                            gpointer       user_data)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);

  if (self->first_frame_skipped) {
    gtk_progress_tracker_advance_frame (&self->tracker,
                                        gdk_frame_clock_get_frame_time (frame_clock));
  } else {
    self->first_frame_skipped = TRUE;
  }

  /* Finish the animation early if the widget isn't mapped anymore. */
  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&self->tracker);

  hdy_squeezer_progress_updated (HDY_SQUEEZER (widget));

  if (gtk_progress_tracker_get_state (&self->tracker) == GTK_PROGRESS_STATE_AFTER) {
    self->tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);

    return FALSE;
  }

  return TRUE;
}

static void
hdy_squeezer_schedule_ticks (HdySqueezer *self)
{
  if (self->tick_id == 0) {
    self->tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self), hdy_squeezer_transition_cb, self, NULL);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
  }
}

static void
hdy_squeezer_unschedule_ticks (HdySqueezer *self)
{
  if (self->tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tick_id);
    self->tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
  }
}

static void
hdy_squeezer_start_transition (HdySqueezer               *self,
                               HdySqueezerTransitionType  transition_type,
                               guint                      transition_duration)
{
  GtkWidget *widget = GTK_WIDGET (self);

  if (gtk_widget_get_mapped (widget) &&
      hdy_get_enable_animations (widget) &&
      transition_type != HDY_SQUEEZER_TRANSITION_TYPE_NONE &&
      transition_duration != 0 &&
      self->last_visible_child != NULL) {
    self->active_transition_type = transition_type;
    self->first_frame_skipped = FALSE;
    hdy_squeezer_schedule_ticks (self);
    gtk_progress_tracker_start (&self->tracker,
                                self->transition_duration * 1000,
                                0,
                                1.0);
  } else {
    hdy_squeezer_unschedule_ticks (self);
    self->active_transition_type = HDY_SQUEEZER_TRANSITION_TYPE_NONE;
    gtk_progress_tracker_finish (&self->tracker);
  }

  hdy_squeezer_progress_updated (HDY_SQUEEZER (widget));
}

static void
set_visible_child (HdySqueezer               *self,
                   HdySqueezerChildInfo      *child_info,
                   HdySqueezerTransitionType  transition_type,
                   guint                      transition_duration)
{
  HdySqueezerChildInfo *info;
  GtkWidget *widget = GTK_WIDGET (self);
  GList *l;
  GtkWidget *toplevel;
  GtkWidget *focus;
  gboolean contains_focus = FALSE;

  /* If we are being destroyed, do not bother with transitions and
   * notifications.
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick the first visible. */
  if (child_info == NULL) {
    for (l = self->children; l != NULL; l = l->next) {
      info = l->data;
      if (gtk_widget_get_visible (info->widget)) {
        child_info = info;
        break;
      }
    }
  }

  if (child_info == self->visible_child)
    return;

  toplevel = gtk_widget_get_toplevel (widget);
  if (GTK_IS_WINDOW (toplevel)) {
    focus = gtk_window_get_focus (GTK_WINDOW (toplevel));
    if (focus &&
        self->visible_child &&
        self->visible_child->widget &&
        gtk_widget_is_ancestor (focus, self->visible_child->widget)) {
      contains_focus = TRUE;

      if (self->visible_child->last_focus)
        g_object_remove_weak_pointer (G_OBJECT (self->visible_child->last_focus),
                                      (gpointer *)&self->visible_child->last_focus);
      self->visible_child->last_focus = focus;
      g_object_add_weak_pointer (G_OBJECT (self->visible_child->last_focus),
                                 (gpointer *)&self->visible_child->last_focus);
    }
  }

  if (self->last_visible_child != NULL)
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
  self->last_visible_child = NULL;

  if (self->last_visible_surface != NULL)
    cairo_surface_destroy (self->last_visible_surface);
  self->last_visible_surface = NULL;

  if (self->visible_child && self->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      GtkAllocation allocation;

      self->last_visible_child = self->visible_child;
      gtk_widget_get_allocated_size (self->last_visible_child->widget, &allocation, NULL);
      self->last_visible_widget_width = allocation.width;
      self->last_visible_widget_height = allocation.height;
    } else {
      gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
    }
  }

  self->visible_child = child_info;

  if (child_info) {
    gtk_widget_set_child_visible (child_info->widget, TRUE);

    if (contains_focus) {
      if (child_info->last_focus)
        gtk_widget_grab_focus (child_info->last_focus);
      else
        gtk_widget_child_focus (child_info->widget, GTK_DIR_TAB_FORWARD);
    }
  }

  if (self->homogeneous)
    gtk_widget_queue_allocate (widget);
  else
    gtk_widget_queue_resize (widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);

  hdy_squeezer_start_transition (self, transition_type, transition_duration);
}

static void
stack_child_visibility_notify_cb (GObject    *obj,
                                  GParamSpec *pspec,
                                  gpointer    user_data)
{
  HdySqueezer *self = HDY_SQUEEZER (user_data);
  GtkWidget *child = GTK_WIDGET (obj);
  HdySqueezerChildInfo *child_info;

  child_info = find_child_info_for_widget (self, child);

  if (self->visible_child == NULL &&
      gtk_widget_get_visible (child))
    set_visible_child (self, child_info, self->transition_type, self->transition_duration);
  else if (self->visible_child == child_info &&
           !gtk_widget_get_visible (child))
    set_visible_child (self, NULL, self->transition_type, self->transition_duration);

  if (child_info == self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }
}

static void
hdy_squeezer_add (GtkContainer *container,
                  GtkWidget    *child)
{
  HdySqueezer *self = HDY_SQUEEZER (container);
  HdySqueezerChildInfo *child_info;

  g_return_if_fail (child != NULL);

  child_info = g_slice_new (HdySqueezerChildInfo);
  child_info->widget = child;
  child_info->enabled = TRUE;
  child_info->last_focus = NULL;

  self->children = g_list_append (self->children, child_info);

  gtk_widget_set_child_visible (child, FALSE);
  gtk_widget_set_parent_window (child, self->bin_window);
  gtk_widget_set_parent (child, GTK_WIDGET (self));

  if (self->bin_window != NULL) {
    gdk_window_set_events (self->bin_window,
                           gdk_window_get_events (self->bin_window) |
                           gtk_widget_get_events (child));
  }

  g_signal_connect (child, "notify::visible",
                    G_CALLBACK (stack_child_visibility_notify_cb), self);

  if (self->visible_child == NULL &&
      gtk_widget_get_visible (child))
    set_visible_child (self, child_info, self->transition_type, self->transition_duration);

  if (self->visible_child == child_info)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
hdy_squeezer_remove (GtkContainer *container,
                     GtkWidget    *child)
{
  HdySqueezer *self = HDY_SQUEEZER (container);
  HdySqueezerChildInfo *child_info;
  gboolean was_visible;

  child_info = find_child_info_for_widget (self, child);
  if (child_info == NULL)
    return;

  self->children = g_list_remove (self->children, child_info);

  g_signal_handlers_disconnect_by_func (child,
                                        stack_child_visibility_notify_cb,
                                        self);

  was_visible = gtk_widget_get_visible (child);

  child_info->widget = NULL;

  if (self->visible_child == child_info)
    set_visible_child (self, NULL, self->transition_type, self->transition_duration);

  if (self->last_visible_child == child_info)
    self->last_visible_child = NULL;

  gtk_widget_unparent (child);

  if (child_info->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (child_info->last_focus),
                                  (gpointer *)&child_info->last_focus);

  g_slice_free (HdySqueezerChildInfo, child_info);

  if (self->homogeneous && was_visible)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
hdy_squeezer_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  HdySqueezer *self = HDY_SQUEEZER (object);

  switch (property_id) {
  case PROP_HOMOGENEOUS:
    g_value_set_boolean (value, hdy_squeezer_get_homogeneous (self));
    break;
  case PROP_VISIBLE_CHILD:
    g_value_set_object (value, hdy_squeezer_get_visible_child (self));
    break;
  case PROP_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_squeezer_get_transition_duration (self));
    break;
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, hdy_squeezer_get_transition_type (self));
    break;
  case PROP_TRANSITION_RUNNING:
    g_value_set_boolean (value, hdy_squeezer_get_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, hdy_squeezer_get_interpolate_size (self));
    break;
  case PROP_XALIGN:
    g_value_set_float (value, hdy_squeezer_get_xalign (self));
    break;
  case PROP_YALIGN:
    g_value_set_float (value, hdy_squeezer_get_yalign (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_squeezer_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  HdySqueezer *self = HDY_SQUEEZER (object);

  switch (property_id) {
  case PROP_HOMOGENEOUS:
    hdy_squeezer_set_homogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_TRANSITION_DURATION:
    hdy_squeezer_set_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_TRANSITION_TYPE:
    hdy_squeezer_set_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    hdy_squeezer_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  case PROP_XALIGN:
    hdy_squeezer_set_xalign (self, g_value_get_float (value));
    break;
  case PROP_YALIGN:
    hdy_squeezer_set_yalign (self, g_value_get_float (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_squeezer_realize (GtkWidget *widget)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  GtkAllocation allocation;
  GdkWindowAttr attributes = { 0 };
  GdkWindowAttributesType attributes_mask;
  HdySqueezerChildInfo *info;
  GList *l;

  gtk_widget_set_realized (widget, TRUE);
  gtk_widget_set_window (widget, g_object_ref (gtk_widget_get_parent_window (widget)));

  gtk_widget_get_allocation (widget, &allocation);

  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.event_mask =
    gtk_widget_get_events (widget);
  attributes_mask = (GDK_WA_X | GDK_WA_Y) | GDK_WA_VISUAL;

  self->view_window =
    gdk_window_new (gtk_widget_get_window (GTK_WIDGET (self)),
                    &attributes, attributes_mask);
  gtk_widget_register_window (widget, self->view_window);

  attributes.x = 0;
  attributes.y = 0;
  attributes.width = allocation.width;
  attributes.height = allocation.height;

  for (l = self->children; l != NULL; l = l->next) {
    info = l->data;
    attributes.event_mask |= gtk_widget_get_events (info->widget);
  }

  self->bin_window =
    gdk_window_new (self->view_window, &attributes, attributes_mask);
  gtk_widget_register_window (widget, self->bin_window);

  for (l = self->children; l != NULL; l = l->next) {
    info = l->data;

    gtk_widget_set_parent_window (info->widget, self->bin_window);
  }

  gdk_window_show (self->bin_window);
}

static void
hdy_squeezer_unrealize (GtkWidget *widget)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);

  gtk_widget_unregister_window (widget, self->bin_window);
  gdk_window_destroy (self->bin_window);
  self->bin_window = NULL;
  gtk_widget_unregister_window (widget, self->view_window);
  gdk_window_destroy (self->view_window);
  self->view_window = NULL;

  GTK_WIDGET_CLASS (hdy_squeezer_parent_class)->unrealize (widget);
}

static void
hdy_squeezer_map (GtkWidget *widget)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);

  GTK_WIDGET_CLASS (hdy_squeezer_parent_class)->map (widget);

  gdk_window_show (self->view_window);
}

static void
hdy_squeezer_unmap (GtkWidget *widget)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);

  gdk_window_hide (self->view_window);

  GTK_WIDGET_CLASS (hdy_squeezer_parent_class)->unmap (widget);
}

static void
hdy_squeezer_forall (GtkContainer *container,
                     gboolean      include_internals,
                     GtkCallback   callback,
                     gpointer      callback_data)
{
  HdySqueezer *self = HDY_SQUEEZER (container);
  HdySqueezerChildInfo *child_info;
  GList *l;

  l = self->children;
  while (l) {
    child_info = l->data;
    l = l->next;

    (* callback) (child_info->widget, callback_data);
  }
}

static void
hdy_squeezer_compute_expand (GtkWidget *widget,
                             gboolean  *hexpand_p,
                             gboolean  *vexpand_p)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  gboolean hexpand, vexpand;
  HdySqueezerChildInfo *child_info;
  GtkWidget *child;
  GList *l;

  hexpand = FALSE;
  vexpand = FALSE;
  for (l = self->children; l != NULL; l = l->next) {
    child_info = l->data;
    child = child_info->widget;

    if (!hexpand &&
        gtk_widget_compute_expand (child, GTK_ORIENTATION_HORIZONTAL))
      hexpand = TRUE;

    if (!vexpand &&
        gtk_widget_compute_expand (child, GTK_ORIENTATION_VERTICAL))
      vexpand = TRUE;

    if (hexpand && vexpand)
      break;
  }

  *hexpand_p = hexpand;
  *vexpand_p = vexpand;
}

static void
hdy_squeezer_draw_crossfade (GtkWidget *widget,
                             cairo_t   *cr)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  gdouble progress = gtk_progress_tracker_get_progress (&self->tracker, FALSE);

  cairo_push_group (cr);
  gtk_container_propagate_draw (GTK_CONTAINER (self),
                                self->visible_child->widget,
                                cr);
  cairo_save (cr);

  /* Multiply alpha by progress. */
  cairo_set_source_rgba (cr, 1, 1, 1, progress);
  cairo_set_operator (cr, CAIRO_OPERATOR_DEST_IN);
  cairo_paint (cr);

  if (self->last_visible_surface != NULL) {
    gint width_diff = gtk_widget_get_allocated_width (widget) - self->last_visible_surface_allocation.width;
    gint height_diff = gtk_widget_get_allocated_height (widget) - self->last_visible_surface_allocation.height;

    cairo_set_source_surface (cr, self->last_visible_surface,
                              width_diff * self->xalign,
                              height_diff * self->yalign);
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    cairo_paint_with_alpha (cr, MAX (1.0 - progress, 0));
  }

  cairo_restore (cr);

  cairo_pop_group_to_source (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_paint (cr);
}

static gboolean
hdy_squeezer_draw (GtkWidget *widget,
                   cairo_t   *cr)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);

  if (gtk_cairo_should_draw_window (cr, self->view_window)) {
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (widget);
    gtk_render_background (context,
                           cr,
                           0, 0,
                           gtk_widget_get_allocated_width (widget),
                           gtk_widget_get_allocated_height (widget));
  }

  if (self->visible_child) {
    if (gtk_progress_tracker_get_state (&self->tracker) != GTK_PROGRESS_STATE_AFTER) {
      if (self->last_visible_surface == NULL &&
          self->last_visible_child != NULL) {
        g_autoptr (cairo_t) pattern_cr = NULL;

        gtk_widget_get_allocation (self->last_visible_child->widget,
                                   &self->last_visible_surface_allocation);
        self->last_visible_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             self->last_visible_surface_allocation.width,
                                             self->last_visible_surface_allocation.height);
        pattern_cr = cairo_create (self->last_visible_surface);
        /* We don't use propagate_draw here, because we don't want to apply the
         * bin_window offset.
         */
        gtk_widget_draw (self->last_visible_child->widget, pattern_cr);
      }

      cairo_rectangle (cr,
                       0, 0,
                       gtk_widget_get_allocated_width (widget),
                       gtk_widget_get_allocated_height (widget));
      cairo_clip (cr);

      switch (self->active_transition_type) {
      case HDY_SQUEEZER_TRANSITION_TYPE_CROSSFADE:
        if (gtk_cairo_should_draw_window (cr, self->bin_window))
          hdy_squeezer_draw_crossfade (widget, cr);
        break;
      case HDY_SQUEEZER_TRANSITION_TYPE_NONE:
      default:
        g_assert_not_reached ();
      }

    } else if (gtk_cairo_should_draw_window (cr, self->bin_window))
      gtk_container_propagate_draw (GTK_CONTAINER (self),
                                    self->visible_child->widget,
                                    cr);
  }

  return FALSE;
}

static void
hdy_squeezer_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  HdySqueezerChildInfo *child_info = NULL;
  GtkWidget *child = NULL;
  gint child_min;
  GList *l;
  GtkAllocation child_allocation;

  hdy_css_size_allocate (widget, allocation);

  gtk_widget_set_allocation (widget, allocation);

  for (l = self->children; l != NULL; l = l->next) {
    child_info = l->data;
    child = child_info->widget;

    if (!gtk_widget_get_visible (child))
      continue;

    if (!child_info->enabled)
      continue;

    if (self->orientation == GTK_ORIENTATION_VERTICAL) {
      if (gtk_widget_get_request_mode (child) != GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH)
        gtk_widget_get_preferred_height (child, &child_min, NULL);
      else
        gtk_widget_get_preferred_height_for_width (child, allocation->width, &child_min, NULL);

      if (child_min <= allocation->height)
        break;
    } else {
      if (gtk_widget_get_request_mode (child) != GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT)
        gtk_widget_get_preferred_width (child, &child_min, NULL);
      else
        gtk_widget_get_preferred_width_for_height (child, allocation->height, &child_min, NULL);

      if (child_min <= allocation->width)
        break;
    }
  }

  set_visible_child (self, child_info,
                     self->transition_type,
                     self->transition_duration);

  child_allocation.x = 0;
  child_allocation.y = 0;

  if (gtk_widget_get_realized (widget)) {
    gdk_window_move_resize (self->view_window,
                            allocation->x, allocation->y,
                            allocation->width, allocation->height);
    gdk_window_move_resize (self->bin_window,
                            0, 0,
                            allocation->width, allocation->height);
  }

  if (self->last_visible_child != NULL) {
    int min, nat;
    gtk_widget_get_preferred_width (self->last_visible_child->widget, &min, &nat);
    child_allocation.width = MAX (min, allocation->width);
    gtk_widget_get_preferred_height_for_width (self->last_visible_child->widget,
                                               child_allocation.width,
                                               &min, &nat);
    child_allocation.height = MAX (min, allocation->height);

    gtk_widget_size_allocate (self->last_visible_child->widget, &child_allocation);
  }

  child_allocation.width = allocation->width;
  child_allocation.height = allocation->height;

  if (self->visible_child) {
    int min, nat;
    GtkAlign valign;

    gtk_widget_get_preferred_height_for_width (self->visible_child->widget,
                                               allocation->width,
                                               &min, &nat);
    if (self->interpolate_size) {
      valign = gtk_widget_get_valign (self->visible_child->widget);
      child_allocation.height = MAX (nat, allocation->height);
      if (valign == GTK_ALIGN_END &&
          child_allocation.height > allocation->height)
        child_allocation.y -= nat - allocation->height;
      else if (valign == GTK_ALIGN_CENTER &&
               child_allocation.height > allocation->height)
        child_allocation.y -= (nat - allocation->height) / 2;
    }

    gtk_widget_size_allocate (self->visible_child->widget, &child_allocation);
  }
}

/* This private method is prefixed by the class name because it will be a
 * virtual method in GTK 4.
 */
static void
hdy_squeezer_measure (GtkWidget      *widget,
                      GtkOrientation  orientation,
                      int             for_size,
                      int            *minimum,
                      int            *natural,
                      int            *minimum_baseline,
                      int            *natural_baseline)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  HdySqueezerChildInfo *child_info;
  GtkWidget *child;
  gint child_min, child_nat;
  GList *l;

  *minimum = 0;
  *natural = 0;

  for (l = self->children; l != NULL; l = l->next) {
    child_info = l->data;
    child = child_info->widget;

    if (self->orientation != orientation && !self->homogeneous &&
        self->visible_child != child_info)
      continue;

    if (!gtk_widget_get_visible (child))
      continue;

    /* Disabled children are taken into account when measuring the widget, to
     * keep its size request and allocation consistent. This avoids the
     * appearant size and position of a child to changes suddenly when a larger
     * child gets enabled/disabled.
     */

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

    if (self->orientation == orientation)
      *minimum = *minimum == 0 ? child_min : MIN (*minimum, child_min);
    else
      *minimum = MAX (*minimum, child_min);
    *natural = MAX (*natural, child_nat);
  }

  if (self->orientation != orientation && !self->homogeneous &&
      self->interpolate_size &&
      self->last_visible_child != NULL) {
    gdouble t = gtk_progress_tracker_get_ease_out_cubic (&self->tracker, FALSE);
    if (orientation == GTK_ORIENTATION_VERTICAL) {
      *minimum = hdy_lerp (self->last_visible_widget_height, *minimum, t);
      *natural = hdy_lerp (self->last_visible_widget_height, *natural, t);
    } else {
      *minimum = hdy_lerp (self->last_visible_widget_width, *minimum, t);
      *natural = hdy_lerp (self->last_visible_widget_width, *natural, t);
    }
  }

  hdy_css_measure (widget, orientation, minimum, natural);
}

static void
hdy_squeezer_get_preferred_width (GtkWidget *widget,
                                  gint      *minimum,
                                  gint      *natural)
{
  hdy_squeezer_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                        minimum, natural, NULL, NULL);
}

static void
hdy_squeezer_get_preferred_width_for_height (GtkWidget *widget,
                                             gint       height,
                                             gint      *minimum,
                                             gint      *natural)
{
  hdy_squeezer_measure (widget, GTK_ORIENTATION_HORIZONTAL, height,
                        minimum, natural, NULL, NULL);
}

static void
hdy_squeezer_get_preferred_height (GtkWidget *widget,
                                   gint      *minimum,
                                   gint      *natural)
{
  hdy_squeezer_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                        minimum, natural, NULL, NULL);
}

static void
hdy_squeezer_get_preferred_height_for_width (GtkWidget *widget,
                                             gint       width,
                                             gint      *minimum,
                                             gint      *natural)
{
  hdy_squeezer_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                        minimum, natural, NULL, NULL);
}

static void
hdy_squeezer_get_child_property (GtkContainer *container,
                                 GtkWidget    *widget,
                                 guint         property_id,
                                 GValue       *value,
                                 GParamSpec   *pspec)
{
  HdySqueezer *self = HDY_SQUEEZER (container);
  HdySqueezerChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);
  if (child_info == NULL) {
    g_param_value_set_default (pspec, value);

    return;
  }

  switch (property_id) {
  case CHILD_PROP_ENABLED:
    g_value_set_boolean (value, hdy_squeezer_get_child_enabled (self, widget));
    break;
  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_squeezer_set_child_property (GtkContainer *container,
                                 GtkWidget    *widget,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  HdySqueezer *self = HDY_SQUEEZER (container);
  HdySqueezerChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);
  if (child_info == NULL)
    return;

  switch (property_id) {
  case CHILD_PROP_ENABLED:
    hdy_squeezer_set_child_enabled (self, widget, g_value_get_boolean (value));
    break;
  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_squeezer_dispose (GObject *object)
{
  HdySqueezer *self = HDY_SQUEEZER (object);

  self->visible_child = NULL;

  G_OBJECT_CLASS (hdy_squeezer_parent_class)->dispose (object);
}

static void
hdy_squeezer_finalize (GObject *object)
{
  HdySqueezer *self = HDY_SQUEEZER (object);

  hdy_squeezer_unschedule_ticks (self);

  if (self->last_visible_surface != NULL)
    cairo_surface_destroy (self->last_visible_surface);

  G_OBJECT_CLASS (hdy_squeezer_parent_class)->finalize (object);
}

static void
hdy_squeezer_class_init (HdySqueezerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->get_property = hdy_squeezer_get_property;
  object_class->set_property = hdy_squeezer_set_property;
  object_class->dispose = hdy_squeezer_dispose;
  object_class->finalize = hdy_squeezer_finalize;

  widget_class->size_allocate = hdy_squeezer_size_allocate;
  widget_class->draw = hdy_squeezer_draw;
  widget_class->realize = hdy_squeezer_realize;
  widget_class->unrealize = hdy_squeezer_unrealize;
  widget_class->map = hdy_squeezer_map;
  widget_class->unmap = hdy_squeezer_unmap;
  widget_class->get_preferred_height = hdy_squeezer_get_preferred_height;
  widget_class->get_preferred_height_for_width = hdy_squeezer_get_preferred_height_for_width;
  widget_class->get_preferred_width = hdy_squeezer_get_preferred_width;
  widget_class->get_preferred_width_for_height = hdy_squeezer_get_preferred_width_for_height;
  widget_class->compute_expand = hdy_squeezer_compute_expand;

  container_class->add = hdy_squeezer_add;
  container_class->remove = hdy_squeezer_remove;
  container_class->forall = hdy_squeezer_forall;
  container_class->set_child_property = hdy_squeezer_set_child_property;
  container_class->get_child_property = hdy_squeezer_get_child_property;
  gtk_container_class_handle_border_width (container_class);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  props[PROP_HOMOGENEOUS] =
    g_param_spec_boolean ("homogeneous",
                          _("Homogeneous"),
                          _("Homogeneous sizing"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_VISIBLE_CHILD] =
    g_param_spec_object ("visible-child",
                         _("Visible child"),
                         _("The widget currently visible in the squeezer"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration",
                       _("Transition duration"),
                       _("The animation duration, in milliseconds"),
                       0, G_MAXUINT, 200,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type",
                       _("Transition type"),
                       _("The type of animation used to transition"),
                       HDY_TYPE_SQUEEZER_TRANSITION_TYPE,
                       HDY_SQUEEZER_TRANSITION_TYPE_NONE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_TRANSITION_RUNNING] =
    g_param_spec_boolean ("transition-running",
                          _("Transition running"),
                          _("Whether or not the transition is currently running"),
                          FALSE,
                          G_PARAM_READABLE);

  props[PROP_INTERPOLATE_SIZE] =
    g_param_spec_boolean ("interpolate-size",
                          _("Interpolate size"),
                          _("Whether or not the size should smoothly change when changing between differently sized children"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdySqueezer:xalign:
   *
   * The xalign property determines the horizontal aligment of the children
   * inside the squeezer's size allocation.
   * Compare this to #GtkWidget:halign, which determines how the squeezer's size
   * allocation is positioned in the space available for the squeezer.
   * The range goes from 0 (start) to 1 (end).
   *
   * This will affect the position of children too wide to fit in the squeezer
   * as they are fading out.
   *
   * Since: 1.0
   */
  props[PROP_XALIGN] =
    g_param_spec_float ("xalign",
                        _("X align"),
                        _("The horizontal alignment, from 0 (start) to 1 (end)"),
                        0.0, 1.0,
                        0.5,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdySqueezer:yalign:
   *
   * The yalign property determines the vertical aligment of the children inside
   * the squeezer's size allocation.
   * Compare this to #GtkWidget:valign, which determines how the squeezer's size
   * allocation is positioned in the space available for the squeezer.
   * The range goes from 0 (top) to 1 (bottom).
   *
   * This will affect the position of children too tall to fit in the squeezer
   * as they are fading out.
   *
   * Since: 1.0
   */
  props[PROP_YALIGN] =
    g_param_spec_float ("yalign",
                        _("Y align"),
                        _("The vertical alignment, from 0 (top) to 1 (bottom)"),
                        0.0, 1.0,
                        0.5,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  child_props[CHILD_PROP_ENABLED] =
    g_param_spec_boolean ("enabled",
                          _("Enabled"),
                          _("Whether the child can be picked or should be ignored when looking for the child fitting the available size best"),
                          TRUE,
                          G_PARAM_READWRITE);

  gtk_container_class_install_child_properties (container_class, LAST_CHILD_PROP, child_props);

  gtk_widget_class_set_css_name (widget_class, "squeezer");
}

static void
hdy_squeezer_init (HdySqueezer *self)
{

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  self->homogeneous = TRUE;
  self->transition_duration = 200;
  self->transition_type = HDY_SQUEEZER_TRANSITION_TYPE_NONE;
  self->xalign = 0.5;
  self->yalign = 0.5;
}

/**
 * hdy_squeezer_new:
 *
 * Creates a new #HdySqueezer container.
 *
 * Returns: a new #HdySqueezer
 */
GtkWidget *
hdy_squeezer_new (void)
{
  return g_object_new (HDY_TYPE_SQUEEZER, NULL);
}

/**
 * hdy_squeezer_get_homogeneous:
 * @self: a #HdySqueezer
 *
 * Gets whether @self is homogeneous.
 *
 * See hdy_squeezer_set_homogeneous().
 *
 * Returns: %TRUE if @self is homogeneous, %FALSE is not
 *
 * Since: 0.0.10
 */
gboolean
hdy_squeezer_get_homogeneous (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), FALSE);

  return self->homogeneous;
}

/**
 * hdy_squeezer_set_homogeneous:
 * @self: a #HdySqueezer
 * @homogeneous: %TRUE to make @self homogeneous
 *
 * Sets @self to be homogeneous or not. If it is homogeneous, @self will request
 * the same size for all its children for its opposite orientation, e.g. if
 * @self is oriented horizontally and is homogeneous, it will request the same
 * height for all its children. If it isn't, @self may change size when a
 * different child becomes visible.
 *
 * Since: 0.0.10
 */
void
hdy_squeezer_set_homogeneous (HdySqueezer *self,
                              gboolean     homogeneous)
{
  g_return_if_fail (HDY_IS_SQUEEZER (self));

  homogeneous = !!homogeneous;

  if (self->homogeneous == homogeneous)
    return;

  self->homogeneous = homogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET(self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HOMOGENEOUS]);
}

/**
 * hdy_squeezer_get_transition_duration:
 * @self: a #HdySqueezer
 *
 * Gets the amount of time (in milliseconds) that transitions between children
 * in @self will take.
 *
 * Returns: the transition duration
 */
guint
hdy_squeezer_get_transition_duration (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), 0);

  return self->transition_duration;
}

/**
 * hdy_squeezer_set_transition_duration:
 * @self: a #HdySqueezer
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between children in @self will take.
 */
void
hdy_squeezer_set_transition_duration (HdySqueezer *self,
                                      guint        duration)
{
  g_return_if_fail (HDY_IS_SQUEEZER (self));

  if (self->transition_duration == duration)
    return;

  self->transition_duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_DURATION]);
}

/**
 * hdy_squeezer_get_transition_type:
 * @self: a #HdySqueezer
 *
 * Gets the type of animation that will be used for transitions between children
 * in @self.
 *
 * Returns: the current transition type of @self
 */
HdySqueezerTransitionType
hdy_squeezer_get_transition_type (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), HDY_SQUEEZER_TRANSITION_TYPE_NONE);

  return self->transition_type;
}

/**
 * hdy_squeezer_set_transition_type:
 * @self: a #HdySqueezer
 * @transition: the new transition type
 *
 * Sets the type of animation that will be used for transitions between children
 * in @self. Available types include various kinds of fades and slides.
 *
 * The transition type can be changed without problems at runtime, so it is
 * possible to change the animation based on the child that is about to become
 * current.
 */
void
hdy_squeezer_set_transition_type (HdySqueezer               *self,
                                  HdySqueezerTransitionType  transition)
{
  g_return_if_fail (HDY_IS_SQUEEZER (self));

  if (self->transition_type == transition)
    return;

  self->transition_type = transition;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_TYPE]);
}

/**
 * hdy_squeezer_get_transition_running:
 * @self: a #HdySqueezer
 *
 * Gets whether @self is currently in a transition from one child to another.
 *
 * Returns: %TRUE if the transition is currently running, %FALSE otherwise.
 */
gboolean
hdy_squeezer_get_transition_running (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), FALSE);

  return (self->tick_id != 0);
}

/**
 * hdy_squeezer_get_interpolate_size:
 * @self: A #HdySqueezer
 *
 * Gets whether @self should interpolate its size on visible child change.
 *
 * See hdy_squeezer_set_interpolate_size().
 *
 * Returns: %TRUE if @self interpolates its size on visible child change, %FALSE if not
 *
 * Since: 0.0.10
 */
gboolean
hdy_squeezer_get_interpolate_size (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), FALSE);

  return self->interpolate_size;
}

/**
 * hdy_squeezer_set_interpolate_size:
 * @self: A #HdySqueezer
 * @interpolate_size: %TRUE to interpolate the size
 *
 * Sets whether or not @self will interpolate the size of its opposing
 * orientation when changing the visible child. If %TRUE, @self will interpolate
 * its size between the one of the previous visible child and the one of the new
 * visible child, according to the set transition duration and the orientation,
 * e.g. if @self is horizontal, it will interpolate the its height.
 *
 * Since: 0.0.10
 */
void
hdy_squeezer_set_interpolate_size (HdySqueezer *self,
                                   gboolean     interpolate_size)
{
  g_return_if_fail (HDY_IS_SQUEEZER (self));

  interpolate_size = !!interpolate_size;

  if (self->interpolate_size == interpolate_size)
    return;

  self->interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATE_SIZE]);
}

/**
 * hdy_squeezer_get_visible_child:
 * @self: a #HdySqueezer
 *
 * Gets the currently visible child of @self, or %NULL if there are no visible
 * children.
 *
 * Returns: (transfer none) (nullable): the visible child of the #HdySqueezer
 */
GtkWidget *
hdy_squeezer_get_visible_child (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), NULL);

  return self->visible_child ? self->visible_child->widget : NULL;
}

/**
 * hdy_squeezer_get_child_enabled:
 * @self: a #HdySqueezer
 * @child: a child of @self
 *
 * Gets whether @child is enabled.
 *
 * See hdy_squeezer_set_child_enabled().
 *
 * Returns: %TRUE if @child is enabled, %FALSE otherwise.
 */
gboolean
hdy_squeezer_get_child_enabled (HdySqueezer *self,
                                GtkWidget   *child)
{
  HdySqueezerChildInfo *child_info;

  g_return_val_if_fail (HDY_IS_SQUEEZER (self), FALSE);
  g_return_val_if_fail (GTK_IS_WIDGET (child), FALSE);

  child_info = find_child_info_for_widget (self, child);

  g_return_val_if_fail (child_info != NULL, FALSE);

  return child_info->enabled;
}

/**
 * hdy_squeezer_set_child_enabled:
 * @self: a #HdySqueezer
 * @child: a child of @self
 * @enabled: %TRUE to enable the child, %FALSE to disable it
 *
 * Make @self enable or disable @child. If a child is disabled, it will be
 * ignored when looking for the child fitting the available size best. This
 * allows to programmatically and prematurely hide a child of @self even if it
 * fits in the available space.
 *
 * This can be used e.g. to ensure a certain child is hidden below a certain
 * window width, or any other constraint you find suitable.
 */
void
hdy_squeezer_set_child_enabled (HdySqueezer *self,
                                GtkWidget   *child,
                                gboolean     enabled)
{
  HdySqueezerChildInfo *child_info;

  g_return_if_fail (HDY_IS_SQUEEZER (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  child_info = find_child_info_for_widget (self, child);

  g_return_if_fail (child_info != NULL);

  enabled = !!enabled;

  if (child_info->enabled == enabled)
    return;

  child_info->enabled = enabled;
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * hdy_squeezer_get_xalign:
 * @self: a #HdySqueezer
 *
 * Gets the #HdySqueezer:xalign property for @self.
 *
 * Returns: the xalign property
 *
 * Since: 1.0
 */
gfloat
hdy_squeezer_get_xalign (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), 0.5);

  return self->xalign;
}

/**
 * hdy_squeezer_set_xalign:
 * @self: a #HdySqueezer
 * @xalign: the new xalign value, between 0 and 1
 *
 * Sets the #HdySqueezer:xalign property for @self.
 *
 * Since: 1.0
 */
void
hdy_squeezer_set_xalign (HdySqueezer *self,
                         gfloat       xalign)
{
  g_return_if_fail (HDY_IS_SQUEEZER (self));

  xalign = CLAMP (xalign, 0.0, 1.0);

  if (self->xalign == xalign)
    return;

  self->xalign = xalign;
  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_XALIGN]);
}

/**
 * hdy_squeezer_get_yalign:
 * @self: a #HdySqueezer
 *
 * Gets the #HdySqueezer:yalign property for @self.
 *
 * Returns: the yalign property
 *
 * Since: 1.0
 */
gfloat
hdy_squeezer_get_yalign (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), 0.5);

  return self->yalign;
}

/**
 * hdy_squeezer_set_yalign:
 * @self: a #HdySqueezer
 * @yalign: the new yalign value, between 0 and 1
 *
 * Sets the #HdySqueezer:yalign property for @self.
 *
 * Since: 1.0
 */
void
hdy_squeezer_set_yalign (HdySqueezer *self,
                         gfloat       yalign)
{
  g_return_if_fail (HDY_IS_SQUEEZER (self));

  yalign = CLAMP (yalign, 0.0, 1.0);

  if (self->yalign == yalign)
    return;

  self->yalign = yalign;
  gtk_widget_queue_draw (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_YALIGN]);
}
