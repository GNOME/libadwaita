/*
 * Copyright (C) 2018 Purism SPC
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "gtkprogresstrackerprivate.h"
#include "hdy-animation-private.h"
#include "hdy-stackable-box-private.h"
#include "hdy-shadow-helper-private.h"
#include "hdy-swipeable-private.h"
#include "hdy-swipe-tracker-private.h"

/**
 * PRIVATE:hdy-stackable-box
 * @short_description: An adaptive container acting like a box or a stack.
 * @Title: HdyStackableBox
 * @stability: Private
 * @See_also: #HdyDeck, #HdyLeaflet
 *
 * The #HdyStackableBox object can arrange the widgets it manages like #GtkBox
 * does or like a #GtkStack does, adapting to size changes by switching between
 * the two modes. These modes are named respectively “unfoled” and “folded”.
 *
 * When there is enough space the children are displayed side by side, otherwise
 * only one is displayed. The threshold is dictated by the preferred minimum
 * sizes of the children.
 *
 * #HdyStackableBox is used as an internal implementation of #HdyDeck and
 * #HdyLeaflet.
 *
 * Since: 1.0
 */

/**
 * HdyStackableBoxTransitionType:
 * @HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE: No transition
 * @HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE: Slide from left, right, up or down according to the orientation, text direction and the children order
 * @HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER: Cover the old page or uncover the new page, sliding from or towards the end according to orientation, text direction and children order
 * @HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER: Uncover the new page or cover the old page, sliding from or towards the start according to orientation, text direction and children order
 *
 * This enumeration value describes the possible transitions between modes and
 * children in a #HdyStackableBox widget.
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_FOLDED,
  PROP_HHOMOGENEOUS_FOLDED,
  PROP_VHOMOGENEOUS_FOLDED,
  PROP_HHOMOGENEOUS_UNFOLDED,
  PROP_VHOMOGENEOUS_UNFOLDED,
  PROP_VISIBLE_CHILD,
  PROP_VISIBLE_CHILD_NAME,
  PROP_TRANSITION_TYPE,
  PROP_MODE_TRANSITION_DURATION,
  PROP_CHILD_TRANSITION_DURATION,
  PROP_CHILD_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,
  PROP_CAN_SWIPE_BACK,
  PROP_CAN_SWIPE_FORWARD,
  PROP_ORIENTATION,
  LAST_PROP,
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_NAME,
  CHILD_PROP_ALLOW_VISIBLE,
  LAST_CHILD_PROP,
};

#define HDY_FOLD_UNFOLDED FALSE
#define HDY_FOLD_FOLDED TRUE
#define HDY_FOLD_MAX 2
#define GTK_ORIENTATION_MAX 2

typedef struct _HdyStackableBoxChildInfo HdyStackableBoxChildInfo;

struct _HdyStackableBoxChildInfo
{
  GtkWidget *widget;
  gchar *name;
  gboolean allow_visible;

  /* Convenience storage for per-child temporary frequently computed values. */
  GtkAllocation alloc;
  GtkRequisition min;
  GtkRequisition nat;
  gboolean visible;
};

struct _HdyStackableBox
{
  GObject parent;

  GtkContainer *container;
  GtkContainerClass *klass;
  gboolean can_unfold;

  GList *children;
  /* It is probably cheaper to store and maintain a reversed copy of the
   * children list that to reverse the list every time we need to allocate or
   * draw children for RTL languages on a horizontal widget.
   */
  GList *children_reversed;
  HdyStackableBoxChildInfo *visible_child;
  HdyStackableBoxChildInfo *last_visible_child;

  GdkWindow* bin_window;
  GdkWindow* view_window;

  gboolean folded;

  gboolean homogeneous[HDY_FOLD_MAX][GTK_ORIENTATION_MAX];

  GtkOrientation orientation;

  gboolean move_bin_window_request;

  HdyStackableBoxTransitionType transition_type;

  HdySwipeTracker *tracker;

  struct {
    guint duration;

    gdouble current_pos;
    gdouble source_pos;
    gdouble target_pos;

    cairo_surface_t *start_surface;
    GtkAllocation start_surface_allocation;
    gdouble start_distance;
    gdouble start_progress;
    cairo_surface_t *end_surface;
    GtkAllocation end_surface_allocation;
    GtkAllocation end_surface_clip;
    gdouble end_distance;
    gdouble end_progress;
    guint tick_id;
    GtkProgressTracker tracker;
  } mode_transition;

  /* Child transition variables. */
  struct {
    guint duration;

    gdouble progress;
    gdouble start_progress;
    gdouble end_progress;

    gboolean is_gesture_active;
    gboolean is_cancelled;

    cairo_surface_t *last_visible_surface;
    GtkAllocation last_visible_surface_allocation;
    guint tick_id;
    GtkProgressTracker tracker;
    gboolean first_frame_skipped;

    gint last_visible_widget_width;
    gint last_visible_widget_height;

    gboolean interpolate_size;
    gboolean can_swipe_back;
    gboolean can_swipe_forward;

    HdyStackableBoxTransitionType active_type;
    GtkPanDirection active_direction;
  } child_transition;

  HdyShadowHelper *shadow_helper;
};

static GParamSpec *props[LAST_PROP];
static GParamSpec *child_props[LAST_CHILD_PROP];

static gint HOMOGENEOUS_PROP[HDY_FOLD_MAX][GTK_ORIENTATION_MAX] = {
  { PROP_HHOMOGENEOUS_UNFOLDED, PROP_VHOMOGENEOUS_UNFOLDED},
  { PROP_HHOMOGENEOUS_FOLDED, PROP_VHOMOGENEOUS_FOLDED},
};

G_DEFINE_TYPE (HdyStackableBox, hdy_stackable_box, G_TYPE_OBJECT);

static void
free_child_info (HdyStackableBoxChildInfo *child_info)
{
  g_free (child_info->name);
  g_free (child_info);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (HdyStackableBoxChildInfo, free_child_info)

static HdyStackableBoxChildInfo *
find_child_info_for_widget (HdyStackableBox *self,
                            GtkWidget       *widget)
{
  GList *children;
  HdyStackableBoxChildInfo *child_info;

  for (children = self->children; children; children = children->next) {
    child_info = children->data;

    if (child_info->widget == widget)
      return child_info;
  }

  return NULL;
}

static HdyStackableBoxChildInfo *
find_child_info_for_name (HdyStackableBox *self,
                          const gchar     *name)
{
  GList *children;
  HdyStackableBoxChildInfo *child_info;

  for (children = self->children; children; children = children->next) {
    child_info = children->data;

    if (g_strcmp0 (child_info->name, name) == 0)
      return child_info;
  }

  return NULL;
}

static GList *
get_directed_children (HdyStackableBox *self)
{
  return self->orientation == GTK_ORIENTATION_HORIZONTAL &&
         gtk_widget_get_direction (GTK_WIDGET (self->container)) == GTK_TEXT_DIR_RTL ?
         self->children_reversed : self->children;
}

/* Transitions that cause the bin window to move */
static inline gboolean
is_window_moving_child_transition (HdyStackableBox *self)
{
  GtkPanDirection direction;
  gboolean is_rtl;
  GtkPanDirection left_or_right, right_or_left;

  direction = self->child_transition.active_direction;
  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self->container)) == GTK_TEXT_DIR_RTL;
  left_or_right = is_rtl ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  right_or_left = is_rtl ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;

  switch (self->child_transition.active_type) {
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE:
    return FALSE;
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE:
    return TRUE;
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER:
    return direction == GTK_PAN_DIRECTION_UP || direction == left_or_right;
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER:
    return direction == GTK_PAN_DIRECTION_DOWN || direction == right_or_left;
  default:
    g_assert_not_reached ();
  }
}

/* Transitions that change direction depending on the relative order of the
old and new child */
static inline gboolean
is_direction_dependent_child_transition (HdyStackableBoxTransitionType transition_type)
{
  return (transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE ||
          transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER ||
          transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER);
}

static GtkPanDirection
get_pan_direction (HdyStackableBox *self,
                   gboolean         new_child_first)
{
  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (gtk_widget_get_direction (GTK_WIDGET (self->container)) == GTK_TEXT_DIR_RTL)
      return new_child_first ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;
    else
      return new_child_first ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  }
  else
    return new_child_first ? GTK_PAN_DIRECTION_DOWN : GTK_PAN_DIRECTION_UP;
}

static gint
get_bin_window_x (HdyStackableBox     *self,
                  const GtkAllocation *allocation)
{
  int x = 0;

  if (self->child_transition.is_gesture_active ||
      gtk_progress_tracker_get_state (&self->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
    if (self->child_transition.active_direction == GTK_PAN_DIRECTION_LEFT)
      x = allocation->width * (1 - self->child_transition.progress);
    if (self->child_transition.active_direction == GTK_PAN_DIRECTION_RIGHT)
      x = -allocation->width * (1 - self->child_transition.progress);
  }

  return x;
}

static gint
get_bin_window_y (HdyStackableBox     *self,
                  const GtkAllocation *allocation)
{
  int y = 0;

  if (self->child_transition.is_gesture_active ||
      gtk_progress_tracker_get_state (&self->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
    if (self->child_transition.active_direction == GTK_PAN_DIRECTION_UP)
      y = allocation->height * (1 - self->child_transition.progress);
    if (self->child_transition.active_direction == GTK_PAN_DIRECTION_DOWN)
      y = -allocation->height * (1 - self->child_transition.progress);
  }

  return y;
}

static void
move_resize_bin_window (HdyStackableBox *self,
                        GtkAllocation   *allocation,
                        gboolean         resize)
{
  GtkAllocation alloc;
  gboolean move;

  if (self->bin_window == NULL)
    return;

  if (allocation == NULL) {
    gtk_widget_get_allocation (GTK_WIDGET (self->container), &alloc);
    allocation = &alloc;
  }

  move = self->move_bin_window_request || is_window_moving_child_transition (self);

  if (move && resize)
    gdk_window_move_resize (self->bin_window,
                            get_bin_window_x (self, allocation), get_bin_window_y (self, allocation),
                            allocation->width, allocation->height);
  else if (move)
    gdk_window_move (self->bin_window,
                     get_bin_window_x (self, allocation), get_bin_window_y (self, allocation));
  else if (resize)
    gdk_window_resize (self->bin_window,
                       allocation->width, allocation->height);

  self->move_bin_window_request = FALSE;
}

static void
hdy_stackable_box_child_progress_updated (HdyStackableBox *self)
{
  gtk_widget_queue_draw (GTK_WIDGET (self->container));

  if (!self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] ||
      !self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL])
    gtk_widget_queue_resize (GTK_WIDGET (self->container));

  move_resize_bin_window (self, NULL, FALSE);

  if (!self->child_transition.is_gesture_active &&
      gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (self->child_transition.last_visible_surface != NULL) {
      cairo_surface_destroy (self->child_transition.last_visible_surface);
      self->child_transition.last_visible_surface = NULL;
    }

    if (self->child_transition.is_cancelled) {
      if (self->last_visible_child != NULL) {
        if (self->folded) {
          gtk_widget_set_child_visible (self->last_visible_child->widget, TRUE);
          gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
        }
        self->visible_child = self->last_visible_child;
        self->last_visible_child = NULL;
      }

      g_object_freeze_notify (G_OBJECT (self));
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD_NAME]);
      g_object_thaw_notify (G_OBJECT (self));
    } else {
      if (self->last_visible_child != NULL) {
        if (self->folded)
          gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
        self->last_visible_child = NULL;
      }
    }

    gtk_widget_queue_allocate (GTK_WIDGET (self->container));
    hdy_shadow_helper_clear_cache (self->shadow_helper);
  }
}

static gboolean
hdy_stackable_box_child_transition_cb (GtkWidget     *widget,
                                       GdkFrameClock *frame_clock,
                                       gpointer       user_data)
{
  HdyStackableBox *self = HDY_STACKABLE_BOX (user_data);
  gdouble progress;

  if (self->child_transition.first_frame_skipped) {
    gtk_progress_tracker_advance_frame (&self->child_transition.tracker,
                                        gdk_frame_clock_get_frame_time (frame_clock));
    progress = gtk_progress_tracker_get_ease_out_cubic (&self->child_transition.tracker, FALSE);
    self->child_transition.progress =
      hdy_lerp (self->child_transition.end_progress,
                self->child_transition.start_progress, progress);
  } else
    self->child_transition.first_frame_skipped = TRUE;

  /* Finish animation early if not mapped anymore */
  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&self->child_transition.tracker);

  hdy_stackable_box_child_progress_updated (self);

  if (gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    self->child_transition.tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);

    return FALSE;
  }

  return TRUE;
}

static void
hdy_stackable_box_schedule_child_ticks (HdyStackableBox *self)
{
  if (self->child_transition.tick_id == 0) {
    self->child_transition.tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self->container),
                                    hdy_stackable_box_child_transition_cb,
                                    self, NULL);
    if (!self->child_transition.is_gesture_active)
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
  }
}

static void
hdy_stackable_box_unschedule_child_ticks (HdyStackableBox *self)
{
  if (self->child_transition.tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self->container), self->child_transition.tick_id);
    self->child_transition.tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
  }
}

static void
hdy_stackable_box_stop_child_transition (HdyStackableBox *self)
{
  hdy_stackable_box_unschedule_child_ticks (self);
  self->child_transition.active_type = HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE;
  gtk_progress_tracker_finish (&self->child_transition.tracker);
  if (self->child_transition.last_visible_surface != NULL) {
    cairo_surface_destroy (self->child_transition.last_visible_surface);
    self->child_transition.last_visible_surface = NULL;
  }
  if (self->last_visible_child != NULL) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }

  hdy_shadow_helper_clear_cache (self->shadow_helper);

  /* Move the bin window back in place as a child transition might have moved it. */
  self->move_bin_window_request = TRUE;
}

static void
hdy_stackable_box_start_child_transition (HdyStackableBox               *self,
                                          HdyStackableBoxTransitionType  transition_type,
                                          guint                          transition_duration,
                                          GtkPanDirection                transition_direction)
{
  GtkWidget *widget = GTK_WIDGET (self->container);

  if (gtk_widget_get_mapped (widget) &&
      (hdy_get_enable_animations (widget) || self->child_transition.is_gesture_active) &&
      transition_type != HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE &&
      transition_duration != 0 &&
      self->last_visible_child != NULL &&
      /* Don't animate child transition when a mode transition is ongoing. */
      self->mode_transition.tick_id == 0) {
    self->child_transition.active_type = transition_type;
    self->child_transition.active_direction = transition_direction;
    self->child_transition.first_frame_skipped = FALSE;
    self->child_transition.start_progress = 0;
    self->child_transition.end_progress = 1;
    self->child_transition.progress = 0;
    self->child_transition.is_cancelled = FALSE;

    if (!self->child_transition.is_gesture_active) {
      hdy_stackable_box_schedule_child_ticks (self);
      gtk_progress_tracker_start (&self->child_transition.tracker,
                                  transition_duration * 1000,
                                  0,
                                  1.0);
    }
  }
  else {
    hdy_stackable_box_unschedule_child_ticks (self);
    self->child_transition.active_type = HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE;
    gtk_progress_tracker_finish (&self->child_transition.tracker);
  }

  hdy_stackable_box_child_progress_updated (self);
}

static void
set_visible_child_info (HdyStackableBox               *self,
                        HdyStackableBoxChildInfo      *new_visible_child,
                        HdyStackableBoxTransitionType  transition_type,
                        guint                          transition_duration,
                        gboolean                       emit_switch_child)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GList *children;
  HdyStackableBoxChildInfo *child_info;
  GtkPanDirection transition_direction = GTK_PAN_DIRECTION_LEFT;

  /* If we are being destroyed, do not bother with transitions and   *
   * notifications.
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick first visible. */
  if (new_visible_child == NULL) {
    for (children = self->children; children; children = children->next) {
      child_info = children->data;

      if (gtk_widget_get_visible (child_info->widget)) {
        new_visible_child = child_info;

        break;
      }
    }
  }

  if (new_visible_child == self->visible_child)
    return;

  /* FIXME Probably copied from Gtk Stack, should check whether it's needed. */
  /* toplevel = gtk_widget_get_toplevel (widget); */
  /* if (GTK_IS_WINDOW (toplevel)) { */
  /*   focus = gtk_window_get_focus (GTK_WINDOW (toplevel)); */
  /*   if (focus && */
  /*       self->visible_child && */
  /*       self->visible_child->widget && */
  /*       gtk_widget_is_ancestor (focus, self->visible_child->widget)) { */
  /*     contains_focus = TRUE; */

  /*     if (self->visible_child->last_focus) */
  /*       g_object_remove_weak_pointer (G_OBJECT (self->visible_child->last_focus), */
  /*                                     (gpointer *)&self->visible_child->last_focus); */
  /*     self->visible_child->last_focus = focus; */
  /*     g_object_add_weak_pointer (G_OBJECT (self->visible_child->last_focus), */
  /*                                (gpointer *)&self->visible_child->last_focus); */
  /*   } */
  /* } */

  if (self->last_visible_child)
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
  self->last_visible_child = NULL;

  if (self->child_transition.last_visible_surface != NULL)
    cairo_surface_destroy (self->child_transition.last_visible_surface);
  self->child_transition.last_visible_surface = NULL;

  hdy_shadow_helper_clear_cache (self->shadow_helper);

  if (self->visible_child && self->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      GtkAllocation allocation;

      self->last_visible_child = self->visible_child;
      gtk_widget_get_allocated_size (self->last_visible_child->widget, &allocation, NULL);
      self->child_transition.last_visible_widget_width = allocation.width;
      self->child_transition.last_visible_widget_height = allocation.height;
    }
    else
      gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
  }

  /* FIXME This comes from GtkStack and should be adapted. */
  /* hdy_stackable_box_accessible_update_visible_child (stack, */
  /*                                              self->visible_child ? self->visible_child->widget : NULL, */
  /*                                              new_visible_child ? new_visible_child->widget : NULL); */

  self->visible_child = new_visible_child;

  if (new_visible_child) {
    gtk_widget_set_child_visible (new_visible_child->widget, TRUE);

    /* FIXME This comes from GtkStack and should be adapted. */
    /* if (contains_focus) { */
    /*   if (new_visible_child->last_focus) */
    /*     gtk_widget_grab_focus (new_visible_child->last_focus); */
    /*   else */
    /*     gtk_widget_child_focus (new_visible_child->widget, GTK_DIR_TAB_FORWARD); */
    /* } */
  }

  if ((new_visible_child == NULL || self->last_visible_child == NULL) &&
      is_direction_dependent_child_transition (transition_type))
    transition_type = HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE;
  else if (is_direction_dependent_child_transition (transition_type)) {
    gboolean new_first = FALSE;
    for (children = self->children; children; children = children->next) {
      if (new_visible_child == children->data) {
        new_first = TRUE;

        break;
      }
      if (self->last_visible_child == children->data)
        break;
    }

    transition_direction = get_pan_direction (self, new_first);
  }

  if (self->folded) {
    if (self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] &&
        self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL])
      gtk_widget_queue_allocate (widget);
    else
      gtk_widget_queue_resize (widget);

    hdy_stackable_box_start_child_transition (self, transition_type, transition_duration, transition_direction);
  }

  if (emit_switch_child) {
    gint index = 0;

    for (children = self->children; children; children = children->next) {
      child_info = children->data;

      if (!child_info->allow_visible)
        continue;

      if (child_info == new_visible_child)
        break;

      index++;
    }

    hdy_swipeable_emit_switch_child (HDY_SWIPEABLE (self->container), index,
                                     transition_duration);
  }

  g_object_freeze_notify (G_OBJECT (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD_NAME]);
  g_object_thaw_notify (G_OBJECT (self));
}

static void
get_padding (GtkWidget *widget,
             GtkBorder *padding)
{
  GtkStyleContext *context;
  GtkStateFlags state;

  context = gtk_widget_get_style_context (widget);
  state = gtk_style_context_get_state (context);

  gtk_style_context_get_padding (context, state, padding);
}

static void
hdy_stackable_box_set_position (HdyStackableBox *self,
                                gdouble          pos)
{
  gboolean new_visible;
  GtkWidget *child;

  self->mode_transition.current_pos = pos;

  /* We check mode_transition.target_pos here too, because we want to ensure we set
   * child_visible immediately when starting a reveal operation
   * otherwise the child widgets will not be properly realized
   * after the reveal returns.
   */
  new_visible = self->mode_transition.current_pos != 0.0 || self->mode_transition.target_pos != 0.0;

  child = hdy_stackable_box_get_visible_child (self);
  if (child != NULL &&
      new_visible != gtk_widget_get_child_visible (child))
    gtk_widget_set_child_visible (child, new_visible);

  /* FIXME Copied from GtkRevealer IIRC, check whether it's useful. */
  /* transition = effective_transition (self); */
  /* if (transition == GTK_REVEALER_TRANSITION_TYPE_CROSSFADE) { */
  /*   gtk_widget_set_opacity (GTK_WIDGET (self->container), self->mode_transition.current_pos); */
  /*   gtk_widget_queue_draw (GTK_WIDGET (self->container)); */
  /* } */
  /* else */
    gtk_widget_queue_resize (GTK_WIDGET (self->container));
  /* } */

  /* if (self->mode_transition.current_pos == self->mode_transition.target_pos) */
  /*   g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_REVEALED]); */
}

static void
hdy_stackable_box_mode_progress_updated (HdyStackableBox *self)
{
  if (gtk_progress_tracker_get_state (&self->mode_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (self->mode_transition.start_surface != NULL) {
      cairo_surface_destroy (self->mode_transition.start_surface);
      self->mode_transition.start_surface = NULL;
    }

    if (self->mode_transition.end_surface != NULL) {
      cairo_surface_destroy (self->mode_transition.end_surface);
      self->mode_transition.end_surface = NULL;
    }

    hdy_shadow_helper_clear_cache (self->shadow_helper);
  }
}

static gboolean
hdy_stackable_box_mode_transition_cb (GtkWidget     *widget,
                                      GdkFrameClock *frame_clock,
                                      gpointer       user_data)
{
  HdyStackableBox *self = HDY_STACKABLE_BOX (user_data);
  gdouble ease;

  gtk_progress_tracker_advance_frame (&self->mode_transition.tracker,
                                      gdk_frame_clock_get_frame_time (frame_clock));
  ease = gtk_progress_tracker_get_ease_out_cubic (&self->mode_transition.tracker, FALSE);
  hdy_stackable_box_set_position (self,
                            self->mode_transition.source_pos + (ease * (self->mode_transition.target_pos - self->mode_transition.source_pos)));

  hdy_stackable_box_mode_progress_updated (self);

  if (gtk_progress_tracker_get_state (&self->mode_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    self->mode_transition.tick_id = 0;
    return FALSE;
  }

  return TRUE;
}

static void
hdy_stackable_box_start_mode_transition (HdyStackableBox *self,
                                         gdouble          target)
{
  GtkWidget *widget = GTK_WIDGET (self->container);

  if (self->mode_transition.target_pos == target)
    return;

  self->mode_transition.target_pos = target;
  /* FIXME PROP_REVEAL_CHILD needs to be implemented. */
  /* g_object_notify_by_pspec (G_OBJECT (revealer), props[PROP_REVEAL_CHILD]); */

  hdy_stackable_box_stop_child_transition (self);

  if (gtk_widget_get_mapped (widget) &&
      self->mode_transition.duration != 0 &&
      self->transition_type != HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE &&
      hdy_get_enable_animations (widget)) {
    self->mode_transition.source_pos = self->mode_transition.current_pos;
    if (self->mode_transition.tick_id == 0)
      self->mode_transition.tick_id = gtk_widget_add_tick_callback (widget, hdy_stackable_box_mode_transition_cb, self, NULL);
    gtk_progress_tracker_start (&self->mode_transition.tracker,
                                self->mode_transition.duration * 1000,
                                0,
                                1.0);
  }
  else
    hdy_stackable_box_set_position (self, target);
}

/* FIXME Use this to stop the mode transition animation when it makes sense (see *
 * GtkRevealer for exmples).
 */
/* static void */
/* hdy_stackable_box_stop_mode_animation (HdyStackableBox *self) */
/* { */
/*   if (self->mode_transition.current_pos != self->mode_transition.target_pos) { */
/*     self->mode_transition.current_pos = self->mode_transition.target_pos; */
    /* g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_REVEALED]); */
/*   } */
/*   if (self->mode_transition.tick_id != 0) { */
/*     gtk_widget_remove_tick_callback (GTK_WIDGET (self->container), self->mode_transition.tick_id); */
/*     self->mode_transition.tick_id = 0; */
/*   } */
/* } */

/**
 * hdy_stackable_box_get_folded:
 * @self: a #HdyStackableBox
 *
 * Gets whether @self is folded.
 *
 * Returns: whether @self is folded.
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_get_folded (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  return self->folded;
}

static void
hdy_stackable_box_set_folded (HdyStackableBox *self,
                              gboolean         folded)
{
  GtkStyleContext *context;

  if (self->folded == folded)
    return;

  self->folded = folded;

  hdy_stackable_box_start_mode_transition (self, folded ? 0.0 : 1.0);

  if (self->can_unfold) {
    context = gtk_widget_get_style_context (GTK_WIDGET (self->container));
    if (folded) {
      gtk_style_context_add_class (context, "folded");
      gtk_style_context_remove_class (context, "unfolded");
    } else {
      gtk_style_context_remove_class (context, "folded");
      gtk_style_context_add_class (context, "unfolded");
    }
  }

  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_FOLDED]);
}

/**
 * hdy_stackable_box_set_homogeneous:
 * @self: a #HdyStackableBox
 * @folded: the fold
 * @orientation: the orientation
 * @homogeneous: %TRUE to make @self homogeneous
 *
 * Sets the #HdyStackableBox to be homogeneous or not for the given fold and orientation.
 * If it is homogeneous, the #HdyStackableBox will request the same
 * width or height for all its children depending on the orientation.
 * If it isn't and it is folded, the widget may change width or height
 * when a different child becomes visible.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_homogeneous (HdyStackableBox *self,
                                   gboolean         folded,
                                   GtkOrientation   orientation,
                                   gboolean         homogeneous)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  folded = !!folded;
  homogeneous = !!homogeneous;

  if (self->homogeneous[folded][orientation] == homogeneous)
    return;

  self->homogeneous[folded][orientation] = homogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self->container)))
    gtk_widget_queue_resize (GTK_WIDGET (self->container));

  g_object_notify_by_pspec (G_OBJECT (self), props[HOMOGENEOUS_PROP[folded][orientation]]);
}

/**
 * hdy_stackable_box_get_homogeneous:
 * @self: a #HdyStackableBox
 * @folded: the fold
 * @orientation: the orientation
 *
 * Gets whether @self is homogeneous for the given fold and orientation.
 * See hdy_stackable_box_set_homogeneous().
 *
 * Returns: whether @self is homogeneous for the given fold and orientation.
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_get_homogeneous (HdyStackableBox *self,
                                   gboolean         folded,
                                   GtkOrientation   orientation)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  folded = !!folded;

  return self->homogeneous[folded][orientation];
}

/**
 * hdy_stackable_box_get_transition_type:
 * @self: a #HdyStackableBox
 *
 * Gets the type of animation that will be used
 * for transitions between modes and children in @self.
 *
 * Returns: the current transition type of @self
 *
 * Since: 1.0
 */
HdyStackableBoxTransitionType
hdy_stackable_box_get_transition_type (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE);

  return self->transition_type;
}

/**
 * hdy_stackable_box_set_transition_type:
 * @self: a #HdyStackableBox
 * @transition: the new transition type
 *
 * Sets the type of animation that will be used for transitions between modes
 * and children in @self.
 *
 * The transition type can be changed without problems at runtime, so it is
 * possible to change the animation based on the mode or child that is about to
 * become current.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_transition_type (HdyStackableBox               *self,
                                       HdyStackableBoxTransitionType  transition)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  if (self->transition_type == transition)
    return;

  self->transition_type = transition;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_TRANSITION_TYPE]);
}

/**
 * hdy_stackable_box_get_mode_transition_duration:
 * @self: a #HdyStackableBox
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between modes in @self will take.
 *
 * Returns: the mode transition duration
 *
 * Since: 1.0
 */
guint
hdy_stackable_box_get_mode_transition_duration (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), 0);

  return self->mode_transition.duration;
}

/**
 * hdy_stackable_box_set_mode_transition_duration:
 * @self: a #HdyStackableBox
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between modes in @self
 * will take.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_mode_transition_duration (HdyStackableBox *self,
                                                guint            duration)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  if (self->mode_transition.duration == duration)
    return;

  self->mode_transition.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_MODE_TRANSITION_DURATION]);
}

/**
 * hdy_stackable_box_get_child_transition_duration:
 * @self: a #HdyStackableBox
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between children in @self will take.
 *
 * Returns: the child transition duration
 *
 * Since: 1.0
 */
guint
hdy_stackable_box_get_child_transition_duration (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), 0);

  return self->child_transition.duration;
}

/**
 * hdy_stackable_box_set_child_transition_duration:
 * @self: a #HdyStackableBox
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between children in @self
 * will take.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_child_transition_duration (HdyStackableBox *self,
                                                 guint            duration)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  if (self->child_transition.duration == duration)
    return;

  self->child_transition.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_CHILD_TRANSITION_DURATION]);
}

/**
 * hdy_stackable_box_get_visible_child:
 * @self: a #HdyStackableBox
 *
 * Gets the visible child widget.
 *
 * Returns: (transfer none): the visible child widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_stackable_box_get_visible_child (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), NULL);

  if (self->visible_child == NULL)
    return NULL;

  return self->visible_child->widget;
}

/**
 * hdy_stackable_box_set_visible_child:
 * @self: a #HdyStackableBox
 * @visible_child: the new child
 *
 * Makes @visible_child visible using a transition determined by
 * HdyStackableBox:transition-type and HdyStackableBox:child-transition-duration.
 * The transition can be cancelled by the user, in which case visible child will
 * change back to the previously visible child.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_visible_child (HdyStackableBox *self,
                                     GtkWidget       *visible_child)
{
  HdyStackableBoxChildInfo *child_info;
  gboolean contains_child;

  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (visible_child));

  child_info = find_child_info_for_widget (self, visible_child);
  contains_child = child_info != NULL;

  g_return_if_fail (contains_child);

  set_visible_child_info (self, child_info, self->transition_type, self->child_transition.duration, TRUE);
}

/**
 * hdy_stackable_box_get_visible_child_name:
 * @self: a #HdyStackableBox
 *
 * Gets the name of the currently visible child widget.
 *
 * Returns: (transfer none): the name of the visible child
 *
 * Since: 1.0
 */
const gchar *
hdy_stackable_box_get_visible_child_name (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), NULL);

  if (self->visible_child == NULL)
    return NULL;

  return self->visible_child->name;
}

/**
 * hdy_stackable_box_set_visible_child_name:
 * @self: a #HdyStackableBox
 * @name: the name of a child
 *
 * Makes the child with the name @name visible.
 *
 * See hdy_stackable_box_set_visible_child() for more details.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_visible_child_name (HdyStackableBox *self,
                                          const gchar     *name)
{
  HdyStackableBoxChildInfo *child_info;
  gboolean contains_child;

  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));
  g_return_if_fail (name != NULL);

  child_info = find_child_info_for_name (self, name);
  contains_child = child_info != NULL;

  g_return_if_fail (contains_child);

  set_visible_child_info (self, child_info, self->transition_type, self->child_transition.duration, TRUE);
}

/**
 * hdy_stackable_box_get_child_transition_running:
 * @self: a #HdyStackableBox
 *
 * Returns whether @self is currently in a transition from one page to
 * another.
 *
 * Returns: %TRUE if the transition is currently running, %FALSE otherwise.
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_get_child_transition_running (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  return (self->child_transition.tick_id != 0 ||
          self->child_transition.is_gesture_active);
}

/**
 * hdy_stackable_box_set_interpolate_size:
 * @self: a #HdyStackableBox
 * @interpolate_size: the new value
 *
 * Sets whether or not @self will interpolate its size when
 * changing the visible child. If the #HdyStackableBox:interpolate-size
 * property is set to %TRUE, @self will interpolate its size between
 * the current one and the one it'll take after changing the
 * visible child, according to the set transition duration.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_interpolate_size (HdyStackableBox *self,
                                        gboolean         interpolate_size)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  interpolate_size = !!interpolate_size;

  if (self->child_transition.interpolate_size == interpolate_size)
    return;

  self->child_transition.interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATE_SIZE]);
}

/**
 * hdy_stackable_box_get_interpolate_size:
 * @self: a #HdyStackableBox
 *
 * Returns wether the #HdyStackableBox is set up to interpolate between
 * the sizes of children on page switch.
 *
 * Returns: %TRUE if child sizes are interpolated
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_get_interpolate_size (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  return self->child_transition.interpolate_size;
}

/**
 * hdy_stackable_box_set_can_swipe_back:
 * @self: a #HdyStackableBox
 * @can_swipe_back: the new value
 *
 * Sets whether or not @self allows switching to the previous child that has
 * 'allow-visible' child property set to %TRUE via a swipe gesture
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_can_swipe_back (HdyStackableBox *self,
                                      gboolean         can_swipe_back)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  can_swipe_back = !!can_swipe_back;

  if (self->child_transition.can_swipe_back == can_swipe_back)
    return;

  self->child_transition.can_swipe_back = can_swipe_back;
  hdy_swipe_tracker_set_enabled (self->tracker, can_swipe_back || self->child_transition.can_swipe_forward);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SWIPE_BACK]);
}

/**
 * hdy_stackable_box_get_can_swipe_back
 * @self: a #HdyStackableBox
 *
 * Returns whether the #HdyStackableBox allows swiping to the previous child.
 *
 * Returns: %TRUE if back swipe is enabled.
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_get_can_swipe_back (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  return self->child_transition.can_swipe_back;
}

/**
 * hdy_stackable_box_set_can_swipe_forward:
 * @self: a #HdyStackableBox
 * @can_swipe_forward: the new value
 *
 * Sets whether or not @self allows switching to the next child that has
 * 'allow-visible' child property set to %TRUE via a swipe gesture.
 *
 * Since: 1.0
 */
void
hdy_stackable_box_set_can_swipe_forward (HdyStackableBox *self,
                                         gboolean         can_swipe_forward)
{
  g_return_if_fail (HDY_IS_STACKABLE_BOX (self));

  can_swipe_forward = !!can_swipe_forward;

  if (self->child_transition.can_swipe_forward == can_swipe_forward)
    return;

  self->child_transition.can_swipe_forward = can_swipe_forward;
  hdy_swipe_tracker_set_enabled (self->tracker, self->child_transition.can_swipe_back || can_swipe_forward);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SWIPE_FORWARD]);
}

/**
 * hdy_stackable_box_get_can_swipe_forward
 * @self: a #HdyStackableBox
 *
 * Returns whether the #HdyStackableBox allows swiping to the next child.
 *
 * Returns: %TRUE if forward swipe is enabled.
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_get_can_swipe_forward (HdyStackableBox *self)
{
  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  return self->child_transition.can_swipe_forward;
}

static HdyStackableBoxChildInfo *
find_swipeable_child (HdyStackableBox        *self,
                      HdyNavigationDirection  direction)
{
  GList *children;
  HdyStackableBoxChildInfo *child = NULL;

  children = g_list_find (self->children, self->visible_child);
  do {
    children = (direction == HDY_NAVIGATION_DIRECTION_BACK) ? children->prev : children->next;

    if (children == NULL)
      break;

    child = children->data;
  } while (child && !child->allow_visible);

  return child;
}

/**
 * hdy_stackable_box_navigate
 * @self: a #HdyStackableBox
 * @direction: the direction
 *
 * Switches to the previous or next child that doesn't have 'allow-visible'
 * child property set to %FALSE, similar to performing a swipe gesture to go
 * in @direction.
 *
 * Returns: %TRUE if visible child was changed, %FALSE otherwise.
 *
 * Since: 1.0
 */
gboolean
hdy_stackable_box_navigate (HdyStackableBox        *self,
                            HdyNavigationDirection  direction)
{
  HdyStackableBoxChildInfo *child;

  g_return_val_if_fail (HDY_IS_STACKABLE_BOX (self), FALSE);

  child = find_swipeable_child (self, direction);

  if (!child)
    return FALSE;

  set_visible_child_info (self, child, self->transition_type, self->child_transition.duration, TRUE);

  return TRUE;
}

static void
get_preferred_size (gint     *min,
                    gint     *nat,
                    gboolean  same_orientation,
                    gboolean  homogeneous_folded,
                    gboolean  homogeneous_unfolded,
                    gint      visible_children,
                    gdouble   visible_child_progress,
                    gint      sum_nat,
                    gint      max_min,
                    gint      max_nat,
                    gint      visible_min,
                    gint      last_visible_min)
{
  if (same_orientation) {
    *min = homogeneous_folded ?
             max_min :
             hdy_lerp (visible_min, last_visible_min, visible_child_progress);
    *nat = homogeneous_unfolded ?
             max_nat * visible_children :
             sum_nat;
  }
  else {
    *min = homogeneous_folded ?
             max_min :
             hdy_lerp (visible_min, last_visible_min, visible_child_progress);
    *nat = max_nat;
  }
}

void
hdy_stackable_box_measure (HdyStackableBox *self,
                           GtkOrientation   orientation,
                           int              for_size,
                           int             *minimum,
                           int             *natural,
                           int             *minimum_baseline,
                           int             *natural_baseline)
{
  GList *children;
  HdyStackableBoxChildInfo *child_info;
  gint visible_children;
  gdouble visible_child_progress;
  gint child_min, max_min, visible_min, last_visible_min;
  gint child_nat, max_nat, sum_nat;
  void (*get_preferred_size_static) (GtkWidget *widget,
                                     gint      *minimum_width,
                                     gint      *natural_width);
  void (*get_preferred_size_for_size) (GtkWidget *widget,
                                       gint       height,
                                       gint      *minimum_width,
                                       gint      *natural_width);

  get_preferred_size_static = orientation == GTK_ORIENTATION_HORIZONTAL ?
    gtk_widget_get_preferred_width :
    gtk_widget_get_preferred_height;
  get_preferred_size_for_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
    gtk_widget_get_preferred_width_for_height :
    gtk_widget_get_preferred_height_for_width;

  visible_children = 0;
  child_min = max_min = visible_min = last_visible_min = 0;
  child_nat = max_nat = sum_nat = 0;
  for (children = self->children; children; children = children->next) {
    child_info = children->data;

    if (child_info->widget == NULL || !gtk_widget_get_visible (child_info->widget))
      continue;

    visible_children++;
    if (for_size < 0)
      get_preferred_size_static (child_info->widget,
                                 &child_min, &child_nat);
    else
      get_preferred_size_for_size (child_info->widget, for_size,
                                   &child_min, &child_nat);

    max_min = MAX (max_min, child_min);
    max_nat = MAX (max_nat, child_nat);
    sum_nat += child_nat;
  }

  if (self->visible_child != NULL) {
    if (for_size < 0)
      get_preferred_size_static (self->visible_child->widget,
                                 &visible_min, NULL);
    else
      get_preferred_size_for_size (self->visible_child->widget, for_size,
                                   &visible_min, NULL);
  }

  if (self->last_visible_child != NULL) {
    if (for_size < 0)
      get_preferred_size_static (self->last_visible_child->widget,
                                 &last_visible_min, NULL);
    else
      get_preferred_size_for_size (self->last_visible_child->widget, for_size,
                                   &last_visible_min, NULL);
  }

  visible_child_progress = self->child_transition.interpolate_size ? self->child_transition.progress : 1.0;

  get_preferred_size (minimum, natural,
                      gtk_orientable_get_orientation (GTK_ORIENTABLE (self->container)) == orientation,
                      self->homogeneous[HDY_FOLD_FOLDED][orientation],
                      self->homogeneous[HDY_FOLD_UNFOLDED][orientation],
                      visible_children, visible_child_progress,
                      sum_nat, max_min, max_nat, visible_min, last_visible_min);
}

static void
hdy_stackable_box_size_allocate_folded (HdyStackableBox *self,
                                        GtkAllocation   *allocation)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GList *directed_children, *children;
  HdyStackableBoxChildInfo *child_info, *visible_child;
  gint start_size, end_size, visible_size;
  gint remaining_start_size, remaining_end_size, remaining_size;
  gint current_pad;
  gint max_child_size = 0;
  gboolean box_homogeneous;
  HdyStackableBoxTransitionType mode_transition_type;
  GtkTextDirection direction;
  gboolean under;

  directed_children = get_directed_children (self);
  visible_child = self->visible_child;

  if (!visible_child)
    return;

  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    if (!child_info->widget)
      continue;

    if (child_info->widget == visible_child->widget)
      continue;

    if (self->last_visible_child &&
        child_info->widget == self->last_visible_child->widget)
      continue;

    gtk_widget_set_child_visible (child_info->widget, FALSE);
  }

  if (visible_child->widget == NULL)
    return;

  /* FIXME is this needed? */
  if (!gtk_widget_get_visible (visible_child->widget)) {
    gtk_widget_set_child_visible (visible_child->widget, FALSE);

    return;
  }

  gtk_widget_set_child_visible (visible_child->widget, TRUE);

  mode_transition_type = self->transition_type;

  /* Avoid useless computations and allow visible child transitions. */
  if (self->mode_transition.current_pos <= 0.0)
    mode_transition_type = HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE;

  switch (mode_transition_type) {
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE:
    /* Child transitions should be applied only when folded and when no mode
     * transition is ongoing.
     */
    for (children = directed_children; children; children = children->next) {
      child_info = children->data;

      if (child_info != visible_child &&
          child_info != self->last_visible_child) {
        child_info->visible = FALSE;

        continue;
      }

      child_info->alloc.x = 0;
      child_info->alloc.y = 0;
      child_info->alloc.width = allocation->width;
      child_info->alloc.height = allocation->height;
      child_info->visible = TRUE;
    }

    break;
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE:
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER:
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER:
    /* Compute visible child size. */

    visible_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
      MIN (allocation->width, MAX (visible_child->nat.width, (gint) (allocation->width * (1.0 - self->mode_transition.current_pos)))) :
      MIN (allocation->height, MAX (visible_child->nat.height, (gint) (allocation->height * (1.0 - self->mode_transition.current_pos))));

    /* Compute homogeneous box child size. */
    box_homogeneous = (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] && orientation == GTK_ORIENTATION_HORIZONTAL) ||
                      (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] && orientation == GTK_ORIENTATION_VERTICAL);
    if (box_homogeneous) {
      for (children = directed_children; children; children = children->next) {
        child_info = children->data;

        max_child_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
          MAX (max_child_size, child_info->nat.width) :
          MAX (max_child_size, child_info->nat.height);
      }
    }

    /* Compute the start size. */
    start_size = 0;
    for (children = directed_children; children; children = children->next) {
      child_info = children->data;

      if (child_info == visible_child)
        break;

      start_size += orientation == GTK_ORIENTATION_HORIZONTAL ?
        (box_homogeneous ? max_child_size : child_info->nat.width) :
        (box_homogeneous ? max_child_size : child_info->nat.height);
    }

    /* Compute the end size. */
    end_size = 0;
    for (children = g_list_last (directed_children); children; children = children->prev) {
      child_info = children->data;

      if (child_info == visible_child)
        break;

      end_size += orientation == GTK_ORIENTATION_HORIZONTAL ?
        (box_homogeneous ? max_child_size : child_info->nat.width) :
        (box_homogeneous ? max_child_size : child_info->nat.height);
    }

    /* Compute pads. */
    remaining_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
      allocation->width - visible_size :
      allocation->height - visible_size;
    remaining_start_size = (gint) (remaining_size * ((gdouble) start_size / (gdouble) (start_size + end_size)));
    remaining_end_size = remaining_size - remaining_start_size;

    /* Store start and end allocations. */
    switch (orientation) {
    case GTK_ORIENTATION_HORIZONTAL:
      direction = gtk_widget_get_direction (GTK_WIDGET (self->container));
      under = (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_LTR) ||
              (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_RTL);
      self->mode_transition.start_surface_allocation.width = under ? remaining_size : start_size;
      self->mode_transition.start_surface_allocation.height = allocation->height;
      self->mode_transition.start_surface_allocation.x = under ? 0 : remaining_start_size - start_size;
      self->mode_transition.start_surface_allocation.y = 0;
      self->mode_transition.start_progress = under ? (gdouble) remaining_size / start_size : 1;
      under = (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_LTR) ||
              (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_RTL);
      self->mode_transition.end_surface_allocation.width = end_size;
      self->mode_transition.end_surface_allocation.height = allocation->height;
      self->mode_transition.end_surface_allocation.x = under ? allocation->width - end_size : remaining_start_size + visible_size;
      self->mode_transition.end_surface_allocation.y = 0;
      self->mode_transition.end_surface_clip.width = end_size;
      self->mode_transition.end_surface_clip.height = self->mode_transition.end_surface_allocation.height;
      self->mode_transition.end_surface_clip.x = remaining_start_size + visible_size;
      self->mode_transition.end_surface_clip.y = self->mode_transition.end_surface_allocation.y;
      self->mode_transition.end_progress = under ? (gdouble) remaining_end_size / end_size : 1;
      break;
    case GTK_ORIENTATION_VERTICAL:
      under = mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER;
      self->mode_transition.start_surface_allocation.width = allocation->width;
      self->mode_transition.start_surface_allocation.height = under ? remaining_size : start_size;
      self->mode_transition.start_surface_allocation.x = 0;
      self->mode_transition.start_surface_allocation.y = under ? 0 : remaining_start_size - start_size;
      self->mode_transition.start_progress = under ? (gdouble) remaining_size / start_size : 1;
      under = mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER;
      self->mode_transition.end_surface_allocation.width = allocation->width;
      self->mode_transition.end_surface_allocation.height = end_size;
      self->mode_transition.end_surface_allocation.x = 0;
      self->mode_transition.end_surface_allocation.y = remaining_start_size + visible_size;
      self->mode_transition.end_surface_clip.width = self->mode_transition.end_surface_allocation.width;
      self->mode_transition.end_surface_clip.height = end_size;
      self->mode_transition.end_surface_clip.x = self->mode_transition.end_surface_allocation.x;
      self->mode_transition.end_surface_clip.y = remaining_start_size + visible_size;
      self->mode_transition.end_progress = under ? (gdouble) remaining_end_size / end_size : 1;
      break;
    default:
      g_assert_not_reached ();
    }

    self->mode_transition.start_distance = start_size;
    self->mode_transition.end_distance = end_size;

    /* Allocate visible child. */
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      visible_child->alloc.width = visible_size;
      visible_child->alloc.height = allocation->height;
      visible_child->alloc.x = remaining_start_size;
      visible_child->alloc.y = 0;
      visible_child->visible = TRUE;
    }
    else {
      visible_child->alloc.width = allocation->width;
      visible_child->alloc.height = visible_size;
      visible_child->alloc.x = 0;
      visible_child->alloc.y = remaining_start_size;
      visible_child->visible = TRUE;
    }

    /* Allocate starting children. */
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      current_pad = -self->mode_transition.start_surface_allocation.x;
    else
      current_pad = -self->mode_transition.start_surface_allocation.y;

    for (children = directed_children; children; children = children->next) {
      child_info = children->data;

      if (child_info == visible_child)
        break;

      if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        child_info->alloc.width = box_homogeneous ?
          max_child_size :
          child_info->nat.width;
        child_info->alloc.height = allocation->height;
        child_info->alloc.x = -current_pad;
        child_info->alloc.y = 0;
        child_info->visible = child_info->alloc.x + child_info->alloc.width > 0;

        current_pad -= child_info->alloc.width;
      }
      else {
        child_info->alloc.width = allocation->width;
        child_info->alloc.height = box_homogeneous ?
          max_child_size :
          child_info->nat.height;
        child_info->alloc.x = 0;
        child_info->alloc.y = -current_pad;
        child_info->visible = child_info->alloc.y + child_info->alloc.height > 0;

        current_pad -= child_info->alloc.height;
      }
    }

    /* Allocate ending children. */
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      current_pad = self->mode_transition.end_surface_allocation.x;
    else
      current_pad = self->mode_transition.end_surface_allocation.y;

    for (children = g_list_last (directed_children); children; children = children->prev) {
      child_info = children->data;

      if (child_info == visible_child)
        break;

      if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        current_pad -= child_info->alloc.width;

        child_info->alloc.width = box_homogeneous ?
          max_child_size :
          child_info->nat.width;
        child_info->alloc.height = allocation->height;
        child_info->alloc.x = current_pad;
        child_info->alloc.y = 0;
        child_info->visible = child_info->alloc.x < allocation->width;
      }
      else {
        current_pad -= child_info->alloc.height;

        child_info->alloc.width = allocation->width;
        child_info->alloc.height = box_homogeneous ?
          max_child_size :
          child_info->nat.height;
        child_info->alloc.x = 0;
        child_info->alloc.y = current_pad;
        child_info->visible = child_info->alloc.y < allocation->height;
      }
    }

    break;
  default:
    g_assert_not_reached ();
  }
}

static void
hdy_stackable_box_size_allocate_unfolded (HdyStackableBox *self,
                                          GtkAllocation   *allocation)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GtkAllocation remaining_alloc;
  GList *directed_children, *children;
  HdyStackableBoxChildInfo *child_info, *visible_child;
  gint homogeneous_size = 0, min_size, extra_size;
  gint per_child_extra, n_extra_widgets;
  gint n_visible_children, n_expand_children;
  gint start_pad = 0, end_pad = 0;
  gboolean box_homogeneous;
  HdyStackableBoxTransitionType mode_transition_type;
  GtkTextDirection direction;
  gboolean under;

  directed_children = get_directed_children (self);
  visible_child = self->visible_child;

  box_homogeneous = (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] && orientation == GTK_ORIENTATION_HORIZONTAL) ||
                    (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] && orientation == GTK_ORIENTATION_VERTICAL);

  n_visible_children = n_expand_children = 0;
  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    child_info->visible = child_info->widget != NULL && gtk_widget_get_visible (child_info->widget);

    if (child_info->visible) {
      n_visible_children++;
      if (gtk_widget_compute_expand (child_info->widget, orientation))
        n_expand_children++;
    }
    else {
      child_info->min.width = child_info->min.height = 0;
      child_info->nat.width = child_info->nat.height = 0;
    }
  }

  /* Compute repartition of extra space. */

  if (box_homogeneous) {
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      homogeneous_size = n_visible_children > 0 ? allocation->width / n_visible_children : 0;
      n_expand_children = n_visible_children > 0 ? allocation->width % n_visible_children : 0;
      min_size = allocation->width - n_expand_children;
    }
    else {
      homogeneous_size = n_visible_children > 0 ? allocation->height / n_visible_children : 0;
      n_expand_children = n_visible_children > 0 ? allocation->height % n_visible_children : 0;
      min_size = allocation->height - n_expand_children;
    }
  }
  else {
    min_size = 0;
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      for (children = directed_children; children; children = children->next) {
        child_info = children->data;

        min_size += child_info->nat.width;
      }
    }
    else {
      for (children = directed_children; children; children = children->next) {
        child_info = children->data;

        min_size += child_info->nat.height;
      }
    }
  }

  remaining_alloc.x = 0;
  remaining_alloc.y = 0;
  remaining_alloc.width = allocation->width;
  remaining_alloc.height = allocation->height;

  extra_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
    remaining_alloc.width - min_size :
    remaining_alloc.height - min_size;

  per_child_extra = 0, n_extra_widgets = 0;
  if (n_expand_children > 0) {
    per_child_extra = extra_size / n_expand_children;
    n_extra_widgets = extra_size % n_expand_children;
  }

  /* Compute children allocation */
  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    if (!child_info->visible)
      continue;

    child_info->alloc.x = remaining_alloc.x;
    child_info->alloc.y = remaining_alloc.y;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      if (box_homogeneous) {
        child_info->alloc.width = homogeneous_size;
        if (n_extra_widgets > 0) {
          child_info->alloc.width++;
          n_extra_widgets--;
        }
      }
      else {
        child_info->alloc.width = child_info->nat.width;
        if (gtk_widget_compute_expand (child_info->widget, orientation)) {
          child_info->alloc.width += per_child_extra;
          if (n_extra_widgets > 0) {
            child_info->alloc.width++;
            n_extra_widgets--;
          }
        }
      }
      child_info->alloc.height = remaining_alloc.height;

      remaining_alloc.x += child_info->alloc.width;
      remaining_alloc.width -= child_info->alloc.width;
    }
    else {
      if (box_homogeneous) {
        child_info->alloc.height = homogeneous_size;
        if (n_extra_widgets > 0) {
          child_info->alloc.height++;
          n_extra_widgets--;
        }
      }
      else {
        child_info->alloc.height = child_info->nat.height;
        if (gtk_widget_compute_expand (child_info->widget, orientation)) {
          child_info->alloc.height += per_child_extra;
          if (n_extra_widgets > 0) {
            child_info->alloc.height++;
            n_extra_widgets--;
          }
        }
      }
      child_info->alloc.width = remaining_alloc.width;

      remaining_alloc.y += child_info->alloc.height;
      remaining_alloc.height -= child_info->alloc.height;
    }
  }

  /* Apply animations. */

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    start_pad = (gint) ((visible_child->alloc.x) * (1.0 - self->mode_transition.current_pos));
    end_pad = (gint) ((allocation->width - (visible_child->alloc.x + visible_child->alloc.width)) * (1.0 - self->mode_transition.current_pos));

    self->mode_transition.start_distance = visible_child->alloc.x;
    self->mode_transition.end_distance = allocation->width - (visible_child->alloc.x + visible_child->alloc.width);
  }
  else {
    start_pad = (gint) ((visible_child->alloc.y) * (1.0 - self->mode_transition.current_pos));
    end_pad = (gint) ((allocation->height - (visible_child->alloc.y + visible_child->alloc.height)) * (1.0 - self->mode_transition.current_pos));

    self->mode_transition.start_distance = visible_child->alloc.y;
    self->mode_transition.end_distance = allocation->height - (visible_child->alloc.y + visible_child->alloc.height);
  }

  mode_transition_type = self->transition_type;
  direction = gtk_widget_get_direction (GTK_WIDGET (self->container));

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    under = (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_LTR) ||
            (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_RTL);
  else
    under = mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER;
  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    if (child_info == visible_child)
      break;

    if (!child_info->visible)
      continue;

    if (under)
      continue;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      child_info->alloc.x -= start_pad;
    else
      child_info->alloc.y -= start_pad;
  }

  self->mode_transition.start_progress = under ? self->mode_transition.current_pos : 1;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    under = (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_LTR) ||
            (mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_RTL);
  else
    under = mode_transition_type == HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER;
  for (children = g_list_last (directed_children); children; children = children->prev) {
    child_info = children->data;

    if (child_info == visible_child)
      break;

    if (!child_info->visible)
      continue;

    if (under)
      continue;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      child_info->alloc.x += end_pad;
    else
      child_info->alloc.y += end_pad;
  }

  self->mode_transition.end_progress = under ? self->mode_transition.current_pos : 1;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    visible_child->alloc.x -= start_pad;
    visible_child->alloc.width += start_pad + end_pad;
  }
  else {
    visible_child->alloc.y -= start_pad;
    visible_child->alloc.height += start_pad + end_pad;
  }
}

void
hdy_stackable_box_size_allocate (HdyStackableBox *self,
                                 GtkAllocation   *allocation)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GList *directed_children, *children;
  HdyStackableBoxChildInfo *child_info;
  gint nat_box_size, nat_max_size, visible_children;
  gboolean folded;

  directed_children = get_directed_children (self);

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget)) {
    gdk_window_move_resize (self->view_window,
                            allocation->x, allocation->y,
                            allocation->width, allocation->height);
    move_resize_bin_window (self, allocation, TRUE);
  }

  /* Prepare children information. */
  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    gtk_widget_get_preferred_size (child_info->widget, &child_info->min, &child_info->nat);
    child_info->alloc.x = child_info->alloc.y = child_info->alloc.width = child_info->alloc.height = 0;
    child_info->visible = FALSE;
  }

  /* Check whether the children should be stacked or not. */
  nat_box_size = 0;
  nat_max_size = 0;
  visible_children = 0;
  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    for (children = directed_children; children; children = children->next) {
      child_info = children->data;

      /* FIXME Check the child is visible. */
      if (!child_info->widget)
        continue;

      nat_box_size += child_info->nat.width;
      nat_max_size = MAX (nat_max_size, child_info->nat.width);
      visible_children++;
    }
    if (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL])
      nat_box_size = nat_max_size * visible_children;
    folded = allocation->width < nat_box_size;
  }
  else {
    for (children = directed_children; children; children = children->next) {
      child_info = children->data;

      /* FIXME Check the child is visible. */
      if (!child_info->widget)
        continue;

      nat_box_size += child_info->nat.height;
      nat_max_size = MAX (nat_max_size, child_info->nat.height);
      visible_children++;
    }
    if (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL])
      nat_box_size = nat_max_size * visible_children;
    folded = allocation->height < nat_box_size;
  }

  folded |= !self->can_unfold;

  hdy_stackable_box_set_folded (self, folded);

  /* Allocate size to the children. */
  if (folded)
    hdy_stackable_box_size_allocate_folded (self, allocation);
  else
    hdy_stackable_box_size_allocate_unfolded (self, allocation);

  /* Apply visibility and allocation. */
  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    gtk_widget_set_child_visible (child_info->widget, child_info->visible);
    if (!child_info->visible)
      continue;

    gtk_widget_size_allocate (child_info->widget, &child_info->alloc);
    if (gtk_widget_get_realized (widget))
      gtk_widget_show (child_info->widget);
  }
}

static void
hdy_stackable_box_draw_under (HdyStackableBox *self,
                              cairo_t         *cr)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GtkAllocation allocation;
  int x, y;

  gtk_widget_get_allocation (widget, &allocation);

  x = get_bin_window_x (self, &allocation);
  y = get_bin_window_y (self, &allocation);

  if (gtk_cairo_should_draw_window (cr, self->bin_window)) {
    gint clip_x, clip_y, clip_w, clip_h;
    gdouble progress;

    clip_x = 0;
    clip_y = 0;
    clip_w = allocation.width;
    clip_h = allocation.height;

    switch (self->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_LEFT:
      clip_x = x;
      clip_w -= x;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      clip_w += x;
      break;
    case GTK_PAN_DIRECTION_UP:
      clip_y = y;
      clip_h -= y;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      clip_h += y;
      break;
    default:
      g_assert_not_reached ();
      break;
    }

    progress = self->child_transition.progress;

    cairo_save (cr);
    cairo_rectangle (cr, clip_x, clip_y, clip_w, clip_h);
    cairo_clip (cr);
    gtk_container_propagate_draw (self->container,
                                  self->visible_child->widget,
                                  cr);
    cairo_translate (cr, x, y);
    hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, allocation.width,
                                   allocation.height, progress,
                                   self->child_transition.active_direction);
    cairo_restore (cr);
  }

  if (self->child_transition.last_visible_surface &&
      gtk_cairo_should_draw_window (cr, self->view_window)) {
    switch (self->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_LEFT:
      x -= allocation.width;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      x += allocation.width;
      break;
    case GTK_PAN_DIRECTION_UP:
      y -= allocation.height;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      y += allocation.height;
      break;
    default:
      g_assert_not_reached ();
      break;
    }

    x += self->child_transition.last_visible_surface_allocation.x;
    y += self->child_transition.last_visible_surface_allocation.y;

    if (gtk_widget_get_valign (self->last_visible_child->widget) == GTK_ALIGN_END &&
        self->child_transition.last_visible_widget_height > allocation.height)
      y -= self->child_transition.last_visible_widget_height - allocation.height;
    else if (gtk_widget_get_valign (self->last_visible_child->widget) == GTK_ALIGN_CENTER)
      y -= (self->child_transition.last_visible_widget_height - allocation.height) / 2;

    cairo_save (cr);
    cairo_set_source_surface (cr, self->child_transition.last_visible_surface, x, y);
    cairo_paint (cr);
    cairo_restore (cr);
  }
}

static void
hdy_stackable_box_draw_over (HdyStackableBox *self,
                             cairo_t         *cr)
{
  GtkWidget *widget = GTK_WIDGET (self->container);

  if (self->child_transition.last_visible_surface &&
      gtk_cairo_should_draw_window (cr, self->view_window)) {
    GtkAllocation allocation;
    gint x, y, clip_x, clip_y, clip_w, clip_h, shadow_x, shadow_y;
    gdouble progress;
    GtkPanDirection direction;

    gtk_widget_get_allocation (widget, &allocation);

    x = get_bin_window_x (self, &allocation);
    y = get_bin_window_y (self, &allocation);

    clip_x = 0;
    clip_y = 0;
    clip_w = allocation.width;
    clip_h = allocation.height;
    shadow_x = 0;
    shadow_y = 0;

    switch (self->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_LEFT:
      shadow_x = x - allocation.width;
      clip_w = x;
      x = 0;
      direction = GTK_PAN_DIRECTION_RIGHT;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      clip_x = shadow_x = x + allocation.width;
      clip_w = -x;
      x = 0;
      direction = GTK_PAN_DIRECTION_LEFT;
      break;
    case GTK_PAN_DIRECTION_UP:
      shadow_y = y - allocation.height;
      clip_h = y;
      y = 0;
      direction = GTK_PAN_DIRECTION_DOWN;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      clip_y = shadow_y = y + allocation.height;
      clip_h = -y;
      y = 0;
      direction = GTK_PAN_DIRECTION_UP;
      break;
    default:
      g_assert_not_reached ();
      break;
    }

    x += self->child_transition.last_visible_surface_allocation.x;
    y += self->child_transition.last_visible_surface_allocation.y;

    if (gtk_widget_get_valign (self->last_visible_child->widget) == GTK_ALIGN_END &&
        self->child_transition.last_visible_widget_height > allocation.height)
      y -= self->child_transition.last_visible_widget_height - allocation.height;
    else if (gtk_widget_get_valign (self->last_visible_child->widget) == GTK_ALIGN_CENTER)
      y -= (self->child_transition.last_visible_widget_height - allocation.height) / 2;

    progress = 1 - self->child_transition.progress;

    cairo_save (cr);
    cairo_rectangle (cr, clip_x, clip_y, clip_w, clip_h);
    cairo_clip (cr);
    cairo_set_source_surface (cr, self->child_transition.last_visible_surface, x, y);
    cairo_paint (cr);
    cairo_translate (cr, shadow_x, shadow_y);
    hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, allocation.width,
                                   allocation.height, progress, direction);
    cairo_restore (cr);
  }

  if (gtk_cairo_should_draw_window (cr, self->bin_window))
    gtk_container_propagate_draw (self->container,
                                  self->visible_child->widget,
                                  cr);
}

static void
hdy_stackable_box_draw_slide (HdyStackableBox *self,
                              cairo_t         *cr)
{
  GtkWidget *widget = GTK_WIDGET (self->container);

  if (self->child_transition.last_visible_surface &&
      gtk_cairo_should_draw_window (cr, self->view_window)) {
    GtkAllocation allocation;
    int x, y;

    gtk_widget_get_allocation (widget, &allocation);

    x = get_bin_window_x (self, &allocation);
    y = get_bin_window_y (self, &allocation);

    switch (self->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_LEFT:
      x -= allocation.width;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      x += allocation.width;
      break;
    case GTK_PAN_DIRECTION_UP:
      y -= allocation.height;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      y += allocation.height;
      break;
    default:
      g_assert_not_reached ();
      break;
    }

    x += self->child_transition.last_visible_surface_allocation.x;
    y += self->child_transition.last_visible_surface_allocation.y;

    if (gtk_widget_get_valign (self->last_visible_child->widget) == GTK_ALIGN_END &&
        self->child_transition.last_visible_widget_height > allocation.height)
      y -= self->child_transition.last_visible_widget_height - allocation.height;
    else if (gtk_widget_get_valign (self->last_visible_child->widget) == GTK_ALIGN_CENTER)
      y -= (self->child_transition.last_visible_widget_height - allocation.height) / 2;

    cairo_save (cr);
    cairo_set_source_surface (cr, self->child_transition.last_visible_surface, x, y);
    cairo_paint (cr);
    cairo_restore (cr);
  }

  if (gtk_cairo_should_draw_window (cr, self->bin_window))
    gtk_container_propagate_draw (self->container,
                                  self->visible_child->widget,
                                  cr);
}

static void
hdy_stackable_box_draw_over_or_under (HdyStackableBox *self,
                                      cairo_t         *cr)
{
  gboolean is_rtl;
  GtkPanDirection direction, left_or_right, right_or_left;

  direction = self->child_transition.active_direction;

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self->container)) == GTK_TEXT_DIR_RTL;
  left_or_right = is_rtl ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  right_or_left = is_rtl ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;

  switch (self->child_transition.active_type) {
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER:
    if (direction == GTK_PAN_DIRECTION_UP || direction == left_or_right)
      hdy_stackable_box_draw_over (self, cr);
    else if (direction == GTK_PAN_DIRECTION_DOWN || direction == right_or_left)
      hdy_stackable_box_draw_under (self, cr);
    else
      g_assert_not_reached ();
    break;
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER:
    if (direction == GTK_PAN_DIRECTION_UP || direction == left_or_right)
      hdy_stackable_box_draw_under (self, cr);
    else if (direction == GTK_PAN_DIRECTION_DOWN || direction == right_or_left)
      hdy_stackable_box_draw_over (self, cr);
    else
      g_assert_not_reached ();
    break;
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE:
  case HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE:
  default:
    g_assert_not_reached ();
  }
}

static gboolean
hdy_stackable_box_draw_unfolded (HdyStackableBox *self,
                                 cairo_t         *cr)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  gboolean is_horizontal = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget)) == GTK_ORIENTATION_HORIZONTAL;
  GList *directed_children, *children;
  HdyStackableBoxChildInfo *child_info;
  GtkAllocation allocation, child_allocation;

  directed_children = get_directed_children (self);

  gtk_widget_get_allocation (widget, &allocation);

  gtk_widget_get_allocation (self->visible_child->widget, &child_allocation);
  cairo_save (cr);
  cairo_rectangle (cr,
                   0,
                   0,
                   is_horizontal ? child_allocation.x : allocation.width,
                   is_horizontal ? allocation.height : child_allocation.y);
  cairo_clip (cr);
  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    if (child_info == self->visible_child)
      break;

    gtk_container_propagate_draw (self->container,
                                  child_info->widget,
                                  cr);
  }

  if (self->mode_transition.start_progress < 1) {
    gint w, h;
    w = is_horizontal ? child_allocation.x : allocation.width;
    h = is_horizontal ? allocation.height : child_allocation.y;
    if (is_horizontal)
      w = self->mode_transition.start_distance;
    else
      h = self->mode_transition.start_distance;

    cairo_translate (cr,
                     is_horizontal ? child_allocation.x - w : 0,
                     is_horizontal ? 0 : child_allocation.y - h);
    hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, w, h,
                                   self->mode_transition.start_progress,
                                   is_horizontal ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_DOWN);
  }

  cairo_restore (cr);

  gtk_container_propagate_draw (self->container,
                                self->visible_child->widget,
                                cr);

  gtk_widget_get_allocation (self->visible_child->widget, &child_allocation);
  cairo_save (cr);
  cairo_rectangle (cr,
                   is_horizontal ? child_allocation.x + child_allocation.width : 0,
                   is_horizontal ? 0 : child_allocation.y + child_allocation.height,
                   is_horizontal ? allocation.width - child_allocation.x - child_allocation.width : allocation.width,
                   is_horizontal ? allocation.height : allocation.height - child_allocation.y - child_allocation.height);
  cairo_clip (cr);
  for (children = g_list_last (directed_children); children; children = children->prev) {
    child_info = children->data;

    if (child_info == self->visible_child)
      break;

    gtk_container_propagate_draw (self->container,
                                  child_info->widget,
                                  cr);
  }

  if (self->mode_transition.start_progress < 1) {
    gint w, h;
    w = is_horizontal ? child_allocation.x : allocation.width;
    h = is_horizontal ? allocation.height : child_allocation.y;
    if (is_horizontal)
      w = self->mode_transition.start_distance;
    else
      h = self->mode_transition.start_distance;

    cairo_translate (cr,
                     is_horizontal ? child_allocation.x - w : 0,
                     is_horizontal ? 0 : child_allocation.y - h);
    hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, w, h,
                                   self->mode_transition.start_progress,
                                   is_horizontal ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_DOWN);
  }

  if (self->mode_transition.end_progress < 1) {
    gint w, h;
    w = allocation.width - (is_horizontal ? (child_allocation.x + child_allocation.width) : 0);
    h = allocation.height - (is_horizontal ? 0 : (child_allocation.y + child_allocation.height));
    if (is_horizontal)
      w = self->mode_transition.end_distance;
    else
      h = self->mode_transition.end_distance;

    cairo_translate (cr,
                     is_horizontal ? child_allocation.x + child_allocation.width : 0,
                     is_horizontal ? 0 : child_allocation.y + child_allocation.height);
    hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, w, h,
                                   self->mode_transition.end_progress,
                                   is_horizontal ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_UP);
  }

  cairo_restore (cr);

  return FALSE;
}

gboolean
hdy_stackable_box_draw (HdyStackableBox *self,
                        cairo_t         *cr)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GList *directed_children, *children;
  HdyStackableBoxChildInfo *child_info;
  GtkAllocation allocation;
  cairo_surface_t *subsurface;
  cairo_t *pattern_cr;

  if (!self->folded)
    return hdy_stackable_box_draw_unfolded (self, cr);

  directed_children = get_directed_children (self);

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
    if (gtk_progress_tracker_get_state (&self->mode_transition.tracker) != GTK_PROGRESS_STATE_AFTER &&
        self->folded) {
      gboolean is_horizontal = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget)) == GTK_ORIENTATION_HORIZONTAL;

      if (self->mode_transition.start_surface == NULL &&
          self->mode_transition.start_surface_allocation.width != 0 &&
          self->mode_transition.start_surface_allocation.height != 0) {
        self->mode_transition.start_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             self->mode_transition.start_surface_allocation.width,
                                             self->mode_transition.start_surface_allocation.height);

        for (children = directed_children; children; children = children->next) {
          child_info = children->data;

          if (child_info == self->visible_child)
            break;

          if (!gtk_widget_get_child_visible (child_info->widget))
            continue;

          gtk_widget_get_allocation (child_info->widget, &allocation);
          subsurface = cairo_surface_create_for_rectangle (self->mode_transition.start_surface,
                                                           allocation.x - self->mode_transition.start_surface_allocation.x,
                                                           allocation.y - self->mode_transition.start_surface_allocation.y,
                                                           allocation.width,
                                                           allocation.height);
          pattern_cr = cairo_create (subsurface);
          gtk_widget_draw (child_info->widget, pattern_cr);
          cairo_destroy (pattern_cr);
          cairo_surface_destroy (subsurface);
        }
      }

      if (self->mode_transition.end_surface == NULL &&
          self->mode_transition.end_surface_allocation.width != 0 &&
          self->mode_transition.end_surface_allocation.height != 0) {
        self->mode_transition.end_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             self->mode_transition.end_surface_allocation.width,
                                             self->mode_transition.end_surface_allocation.height);

        for (children = g_list_last (directed_children); children; children = children->prev) {
          child_info = children->data;

          if (child_info == self->visible_child)
            break;

          if (!gtk_widget_get_child_visible (child_info->widget))
            continue;

          gtk_widget_get_allocation (child_info->widget, &allocation);
          subsurface = cairo_surface_create_for_rectangle (self->mode_transition.end_surface,
                                                           allocation.x - self->mode_transition.end_surface_allocation.x,
                                                           allocation.y - self->mode_transition.end_surface_allocation.y,
                                                           allocation.width,
                                                           allocation.height);
          pattern_cr = cairo_create (subsurface);
          gtk_widget_draw (child_info->widget, pattern_cr);
          cairo_destroy (pattern_cr);
          cairo_surface_destroy (subsurface);
        }
      }

      cairo_rectangle (cr,
                       0, 0,
                       gtk_widget_get_allocated_width (widget),
                       gtk_widget_get_allocated_height (widget));
      cairo_clip (cr);

      cairo_save (cr);
      if (self->mode_transition.start_surface != NULL) {
        cairo_rectangle (cr,
                         self->mode_transition.start_surface_allocation.x,
                         self->mode_transition.start_surface_allocation.y,
                         self->mode_transition.start_surface_allocation.width,
                         self->mode_transition.start_surface_allocation.height);
        cairo_clip (cr);
        cairo_set_source_surface (cr, self->mode_transition.start_surface,
                                  self->mode_transition.start_surface_allocation.x,
                                  self->mode_transition.start_surface_allocation.y);
        cairo_paint (cr);

        if (self->mode_transition.start_progress < 1) {
          gint w, h;
          w = self->mode_transition.start_surface_allocation.width;
          h = self->mode_transition.start_surface_allocation.height;
          if (is_horizontal)
            w = self->mode_transition.start_distance;
          else
            h = self->mode_transition.start_distance;

          cairo_translate (cr,
                           self->mode_transition.start_surface_allocation.width - w,
                           self->mode_transition.start_surface_allocation.height - h);
          hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, w, h,
                                         self->mode_transition.start_progress,
                                         is_horizontal ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_DOWN);
        }
      }
      cairo_restore (cr);

      cairo_save (cr);
      if (self->mode_transition.end_surface != NULL) {
        cairo_rectangle (cr,
                         self->mode_transition.end_surface_clip.x,
                         self->mode_transition.end_surface_clip.y,
                         self->mode_transition.end_surface_clip.width,
                         self->mode_transition.end_surface_clip.height);
        cairo_clip (cr);
        cairo_set_source_surface (cr, self->mode_transition.end_surface,
                                  self->mode_transition.end_surface_allocation.x,
                                  self->mode_transition.end_surface_allocation.y);
        cairo_paint (cr);

        if (self->mode_transition.end_progress < 1) {
          gint w, h;
          w = self->mode_transition.end_surface_allocation.width;
          h = self->mode_transition.end_surface_allocation.height;
          if (is_horizontal)
            w = self->mode_transition.end_distance;
          else
            h = self->mode_transition.end_distance;

          cairo_translate (cr, self->mode_transition.end_surface_clip.x,
                           self->mode_transition.end_surface_clip.y);
          hdy_shadow_helper_draw_shadow (self->shadow_helper, cr, w, h,
                                         self->mode_transition.end_progress,
                                         is_horizontal ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_UP);
        }
      }
      cairo_restore (cr);

      if (gtk_cairo_should_draw_window (cr, self->bin_window))
        gtk_container_propagate_draw (self->container,
                                      self->visible_child->widget,
                                      cr);
    }
    else if ((self->child_transition.is_gesture_active &&
              self->transition_type != HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE) ||
             gtk_progress_tracker_get_state (&self->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
      if (self->child_transition.last_visible_surface == NULL &&
          self->last_visible_child != NULL) {
        gtk_widget_get_allocation (self->last_visible_child->widget,
                                   &self->child_transition.last_visible_surface_allocation);
        self->child_transition.last_visible_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             self->child_transition.last_visible_surface_allocation.width,
                                             self->child_transition.last_visible_surface_allocation.height);
        pattern_cr = cairo_create (self->child_transition.last_visible_surface);
        /* We don't use propagate_draw here, because we don't want to apply
         * the bin_window offset
         */
        gtk_widget_draw (self->last_visible_child->widget, pattern_cr);
        cairo_destroy (pattern_cr);
      }

      cairo_rectangle (cr,
                       0, 0,
                       gtk_widget_get_allocated_width (widget),
                       gtk_widget_get_allocated_height (widget));
      cairo_clip (cr);

      switch (self->child_transition.active_type) {
      case HDY_STACKABLE_BOX_TRANSITION_TYPE_SLIDE:
        hdy_stackable_box_draw_slide (self, cr);
        break;
      case HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER:
      case HDY_STACKABLE_BOX_TRANSITION_TYPE_UNDER:
        hdy_stackable_box_draw_over_or_under (self, cr);
        break;
      case HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE:
      default:
        g_assert_not_reached ();
      }
    }
    else if (gtk_cairo_should_draw_window (cr, self->bin_window))
      gtk_container_propagate_draw (self->container,
                                    self->visible_child->widget,
                                    cr);
  }

  return FALSE;
}

static void
update_tracker_orientation (HdyStackableBox *self)
{
  gboolean reverse;

  reverse = (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
             gtk_widget_get_direction (GTK_WIDGET (self->container)) == GTK_TEXT_DIR_RTL);

  g_object_set (self->tracker,
                "orientation", self->orientation,
                "reversed", reverse,
                NULL);
}

void
hdy_stackable_box_direction_changed (HdyStackableBox  *self,
                                     GtkTextDirection  previous_direction)
{
  update_tracker_orientation (self);
}

static void
hdy_stackable_box_child_visibility_notify_cb (GObject    *obj,
                                              GParamSpec *pspec,
                                              gpointer    user_data)
{
  HdyStackableBox *self = HDY_STACKABLE_BOX (user_data);
  GtkWidget *widget = GTK_WIDGET (obj);
  HdyStackableBoxChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);

  if (self->visible_child == NULL && gtk_widget_get_visible (widget))
    set_visible_child_info (self, child_info, self->transition_type, self->child_transition.duration, TRUE);
  else if (self->visible_child == child_info && !gtk_widget_get_visible (widget))
    set_visible_child_info (self, NULL, self->transition_type, self->child_transition.duration, TRUE);
}

void
hdy_stackable_box_add (HdyStackableBox *self,
                       GtkWidget       *widget)
{
  HdyStackableBoxChildInfo *child_info;

  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  gtk_widget_set_child_visible (widget, FALSE);
  gtk_widget_set_parent_window (widget, self->bin_window);
  gtk_widget_set_parent (widget, GTK_WIDGET (self->container));

  child_info = g_new0 (HdyStackableBoxChildInfo, 1);
  child_info->widget = widget;
  child_info->allow_visible = TRUE;

  self->children = g_list_append (self->children, child_info);
  self->children_reversed = g_list_prepend (self->children_reversed, child_info);

  if (self->bin_window)
    gdk_window_set_events (self->bin_window,
                           gdk_window_get_events (self->bin_window) |
                           gtk_widget_get_events (widget));

  g_signal_connect (widget, "notify::visible",
                    G_CALLBACK (hdy_stackable_box_child_visibility_notify_cb), self);

  if (hdy_stackable_box_get_visible_child (self) == NULL &&
      gtk_widget_get_visible (widget)) {
    set_visible_child_info (self, child_info, self->transition_type, self->child_transition.duration, FALSE);
  }

  if (!self->folded ||
      (self->folded && (self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] ||
                        self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] ||
                        self->visible_child == child_info)))
    gtk_widget_queue_resize (GTK_WIDGET (self->container));
}

void
hdy_stackable_box_remove (HdyStackableBox *self,
                          GtkWidget       *widget)
{
  g_autoptr (HdyStackableBoxChildInfo) child_info = find_child_info_for_widget (self, widget);
  gboolean contains_child = child_info != NULL;

  g_return_if_fail (contains_child);

  self->children = g_list_remove (self->children, child_info);
  self->children_reversed = g_list_remove (self->children_reversed, child_info);

  if (hdy_stackable_box_get_visible_child (self) == widget)
    set_visible_child_info (self, NULL, self->transition_type, self->child_transition.duration, TRUE);

  if (gtk_widget_get_visible (widget))
    gtk_widget_queue_resize (GTK_WIDGET (self->container));

  gtk_widget_unparent (widget);
}

void
hdy_stackable_box_forall (HdyStackableBox *self,
                          gboolean         include_internals,
                          GtkCallback      callback,
                          gpointer         callback_data)
{
  /* This shallow copy is needed when the callback changes the list while we are
   * looping through it, for example by calling hdy_stackable_box_remove() on all
   * children when destroying the HdyStackableBox_private_offset.
   */
  g_autoptr (GList) children_copy = g_list_copy (self->children);
  GList *children;
  HdyStackableBoxChildInfo *child_info;

  for (children = children_copy; children; children = children->next) {
    child_info = children->data;

    (* callback) (child_info->widget, callback_data);
  }

  g_list_free (self->children_reversed);
  self->children_reversed = g_list_copy (self->children);
  self->children_reversed = g_list_reverse (self->children_reversed);
}

static void
hdy_stackable_box_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdyStackableBox *self = HDY_STACKABLE_BOX (object);

  switch (prop_id) {
  case PROP_FOLDED:
    g_value_set_boolean (value, hdy_stackable_box_get_folded (self));
    break;
  case PROP_HHOMOGENEOUS_FOLDED:
    g_value_set_boolean (value, hdy_stackable_box_get_homogeneous (self, TRUE, GTK_ORIENTATION_HORIZONTAL));
    break;
  case PROP_VHOMOGENEOUS_FOLDED:
    g_value_set_boolean (value, hdy_stackable_box_get_homogeneous (self, TRUE, GTK_ORIENTATION_VERTICAL));
    break;
  case PROP_HHOMOGENEOUS_UNFOLDED:
    g_value_set_boolean (value, hdy_stackable_box_get_homogeneous (self, FALSE, GTK_ORIENTATION_HORIZONTAL));
    break;
  case PROP_VHOMOGENEOUS_UNFOLDED:
    g_value_set_boolean (value, hdy_stackable_box_get_homogeneous (self, FALSE, GTK_ORIENTATION_VERTICAL));
    break;
  case PROP_VISIBLE_CHILD:
    g_value_set_object (value, hdy_stackable_box_get_visible_child (self));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    g_value_set_string (value, hdy_stackable_box_get_visible_child_name (self));
    break;
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, hdy_stackable_box_get_transition_type (self));
    break;
  case PROP_MODE_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_stackable_box_get_mode_transition_duration (self));
    break;
  case PROP_CHILD_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_stackable_box_get_child_transition_duration (self));
    break;
  case PROP_CHILD_TRANSITION_RUNNING:
    g_value_set_boolean (value, hdy_stackable_box_get_child_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, hdy_stackable_box_get_interpolate_size (self));
    break;
  case PROP_CAN_SWIPE_BACK:
    g_value_set_boolean (value, hdy_stackable_box_get_can_swipe_back (self));
    break;
  case PROP_CAN_SWIPE_FORWARD:
    g_value_set_boolean (value, hdy_stackable_box_get_can_swipe_forward (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, hdy_stackable_box_get_orientation (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_stackable_box_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdyStackableBox *self = HDY_STACKABLE_BOX (object);

  switch (prop_id) {
  case PROP_HHOMOGENEOUS_FOLDED:
    hdy_stackable_box_set_homogeneous (self, TRUE, GTK_ORIENTATION_HORIZONTAL, g_value_get_boolean (value));
    break;
  case PROP_VHOMOGENEOUS_FOLDED:
    hdy_stackable_box_set_homogeneous (self, TRUE, GTK_ORIENTATION_VERTICAL, g_value_get_boolean (value));
    break;
  case PROP_HHOMOGENEOUS_UNFOLDED:
    hdy_stackable_box_set_homogeneous (self, FALSE, GTK_ORIENTATION_HORIZONTAL, g_value_get_boolean (value));
    break;
  case PROP_VHOMOGENEOUS_UNFOLDED:
    hdy_stackable_box_set_homogeneous (self, FALSE, GTK_ORIENTATION_VERTICAL, g_value_get_boolean (value));
    break;
  case PROP_VISIBLE_CHILD:
    hdy_stackable_box_set_visible_child (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    hdy_stackable_box_set_visible_child_name (self, g_value_get_string (value));
    break;
  case PROP_TRANSITION_TYPE:
    hdy_stackable_box_set_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_MODE_TRANSITION_DURATION:
    hdy_stackable_box_set_mode_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_CHILD_TRANSITION_DURATION:
    hdy_stackable_box_set_child_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    hdy_stackable_box_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_BACK:
    hdy_stackable_box_set_can_swipe_back (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_FORWARD:
    hdy_stackable_box_set_can_swipe_forward (self, g_value_get_boolean (value));
    break;
  case PROP_ORIENTATION:
    hdy_stackable_box_set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_stackable_box_finalize (GObject *object)
{
  HdyStackableBox *self = HDY_STACKABLE_BOX (object);

  self->visible_child = NULL;

  if (self->shadow_helper)
    g_clear_object (&self->shadow_helper);

  hdy_stackable_box_unschedule_child_ticks (self);

  if (self->child_transition.last_visible_surface != NULL)
    cairo_surface_destroy (self->child_transition.last_visible_surface);

  G_OBJECT_CLASS (hdy_stackable_box_parent_class)->finalize (object);
}

void
hdy_stackable_box_realize (HdyStackableBox *self)
{
  GtkWidget *widget = GTK_WIDGET (self->container);
  GtkAllocation allocation;
  GdkWindowAttr attributes = { 0 };
  GdkWindowAttributesType attributes_mask;
  GList *children;
  HdyStackableBoxChildInfo *child_info;
  GtkBorder padding;

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
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes_mask = (GDK_WA_X | GDK_WA_Y) | GDK_WA_VISUAL;

  self->view_window = gdk_window_new (gtk_widget_get_window (widget),
                                      &attributes, attributes_mask);
  gtk_widget_register_window (widget, self->view_window);

  get_padding (widget, &padding);
  attributes.x = padding.left;
  attributes.y = padding.top;
  attributes.width = allocation.width;
  attributes.height = allocation.height;

  for (children = self->children; children != NULL; children = children->next) {
    child_info = children->data;
    attributes.event_mask |= gtk_widget_get_events (child_info->widget);
  }

  self->bin_window = gdk_window_new (self->view_window, &attributes, attributes_mask);
  gtk_widget_register_window (widget, self->bin_window);

  for (children = self->children; children != NULL; children = children->next) {
    child_info = children->data;

    gtk_widget_set_parent_window (child_info->widget, self->bin_window);
  }

  gdk_window_show (self->bin_window);
}

void
hdy_stackable_box_unrealize (HdyStackableBox *self)
{
  GtkWidget *widget = GTK_WIDGET (self->container);

  gtk_widget_unregister_window (widget, self->bin_window);
  gdk_window_destroy (self->bin_window);
  self->bin_window = NULL;
  gtk_widget_unregister_window (widget, self->view_window);
  gdk_window_destroy (self->view_window);
  self->view_window = NULL;

  GTK_WIDGET_CLASS (self->klass)->unrealize (widget);
}

void
hdy_stackable_box_map (HdyStackableBox *self)
{
  GTK_WIDGET_CLASS (self->klass)->map (GTK_WIDGET (self->container));

  gdk_window_show (self->view_window);
}

void
hdy_stackable_box_unmap (HdyStackableBox *self)
{
  gdk_window_hide (self->view_window);

  GTK_WIDGET_CLASS (self->klass)->unmap (GTK_WIDGET (self->container));
}

gboolean
hdy_stackable_box_captured_event (HdyStackableBox *self,
                                  GdkEvent        *event)
{
  return hdy_swipe_tracker_captured_event (self->tracker, event);
}

void
hdy_stackable_box_switch_child (HdyStackableBox *self,
                                guint            index,
                                gint64           duration)
{
  HdyStackableBoxChildInfo *child_info = NULL;
  GList *children;
  guint i = 0;

  for (children = self->children; children; children = children->next) {
    child_info = children->data;

    if (!child_info->allow_visible)
      continue;

    if (i == index)
      break;

    i++;
  }

  if (child_info == NULL) {
    g_critical ("Couldn't find eligible child with index %u", index);
    return;
  }

  set_visible_child_info (self, child_info, self->transition_type,
                          duration, FALSE);
}

static double
get_current_progress (HdyStackableBox *self)
{
  gboolean new_first = FALSE;
  GList *children;

  if (!self->child_transition.is_gesture_active &&
      gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER)
    return 0;

  for (children = self->children; children; children = children->next) {
    if (self->last_visible_child == children->data) {
      new_first = TRUE;

      break;
    }
    if (self->visible_child == children->data)
      break;
  }

  return self->child_transition.progress * (new_first ? 1 : -1);
}

static gboolean
can_swipe_in_direction (HdyStackableBox        *self,
                        HdyNavigationDirection  direction)
{
  switch (direction) {
  case HDY_NAVIGATION_DIRECTION_BACK:
    return self->child_transition.can_swipe_back;
  case HDY_NAVIGATION_DIRECTION_FORWARD:
    return self->child_transition.can_swipe_forward;
  default:
    g_assert_not_reached ();
  }
}

void
hdy_stackable_box_begin_swipe (HdyStackableBox        *self,
                               HdyNavigationDirection  direction,
                               gboolean                direct)
{
  gint n;
  gdouble *points, distance, progress;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    distance = gtk_widget_get_allocated_width (GTK_WIDGET (self->container));
  else
    distance = gtk_widget_get_allocated_height (GTK_WIDGET (self->container));

  if (self->child_transition.tick_id > 0) {
    gboolean is_rtl =
      (gtk_widget_get_direction (GTK_WIDGET (self->container)) == GTK_TEXT_DIR_RTL);

    n = 2;
    points = g_new0 (gdouble, n);

    switch (self->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_UP:
      points[1] = 1;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      points[0] = -1;
      break;
    case GTK_PAN_DIRECTION_LEFT:
      if (is_rtl)
        points[0] = -1;
      else
        points[1] = 1;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      if (is_rtl)
        points[1] = 1;
      else
        points[0] = -1;
      break;
    default:
      g_assert_not_reached ();
    }

    progress = get_current_progress (self);

    gtk_widget_remove_tick_callback (GTK_WIDGET (self->container), self->child_transition.tick_id);
    self->child_transition.tick_id = 0;
    self->child_transition.is_gesture_active = TRUE;
    self->child_transition.is_cancelled = FALSE;
  } else {
    HdyStackableBoxChildInfo *child;

    if ((can_swipe_in_direction (self, direction) || !direct) && self->folded)
      child = find_swipeable_child (self, direction);
    else
      child = NULL;

    if (child) {
      self->child_transition.is_gesture_active = TRUE;
      set_visible_child_info (self, child, self->transition_type,
                              self->child_transition.duration, FALSE);

      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
    }

    progress = 0;

    n = child ? 2 : 1;
    points = g_new0 (gdouble, n);
    if (child)
      switch (direction) {
      case HDY_NAVIGATION_DIRECTION_BACK:
        points[0] = -1;
        break;
      case HDY_NAVIGATION_DIRECTION_FORWARD:
        points[1] = 1;
        break;
      default:
        g_assert_not_reached ();
      }
  }

  hdy_swipe_tracker_confirm_swipe (self->tracker, distance, points, n, progress, 0);
}

void
hdy_stackable_box_update_swipe (HdyStackableBox *self,
                                gdouble          value)
{
  self->child_transition.progress = ABS (value);
  hdy_stackable_box_child_progress_updated (self);
}

void
hdy_stackable_box_end_swipe (HdyStackableBox *self,
                             gint64           duration,
                             gdouble          to)
{
 if (!self->child_transition.is_gesture_active)
    return;

  self->child_transition.start_progress = self->child_transition.progress;
  self->child_transition.end_progress = ABS (to);
  self->child_transition.is_cancelled = (to == 0);
  self->child_transition.first_frame_skipped = TRUE;

  hdy_stackable_box_schedule_child_ticks (self);
  if (hdy_get_enable_animations (GTK_WIDGET (self->container)) &&
      duration != 0 &&
      self->transition_type != HDY_STACKABLE_BOX_TRANSITION_TYPE_NONE) {
    gtk_progress_tracker_start (&self->child_transition.tracker,
                                duration * 1000,
                                0,
                                1.0);
  } else {
    self->child_transition.progress = self->child_transition.end_progress;
    gtk_progress_tracker_finish (&self->child_transition.tracker);
  }

  self->child_transition.is_gesture_active = FALSE;
  hdy_stackable_box_child_progress_updated (self);

  gtk_widget_queue_draw (GTK_WIDGET (self->container));
}

GtkOrientation
hdy_stackable_box_get_orientation (HdyStackableBox *self)
{
  return self->orientation;
}

void
hdy_stackable_box_set_orientation (HdyStackableBox *self,
                                   GtkOrientation   orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  update_tracker_orientation (self);
  gtk_widget_queue_resize (GTK_WIDGET (self->container));
  g_object_notify (G_OBJECT (self), "orientation");
}

const gchar *
hdy_stackable_box_get_child_name (HdyStackableBox *self,
                                  GtkWidget       *widget)
{
  HdyStackableBoxChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);

  g_return_val_if_fail (child_info != NULL, NULL);

  return child_info->name;
}

void
hdy_stackable_box_set_child_name (HdyStackableBox *self,
                                  GtkWidget       *widget,
                                  const gchar     *name)
{
  HdyStackableBoxChildInfo *child_info;
  HdyStackableBoxChildInfo *child_info2;
  GList *children;

  child_info = find_child_info_for_widget (self, widget);

  g_return_if_fail (child_info != NULL);

  for (children = self->children; children; children = children->next) {
    child_info2 = children->data;

    if (child_info == child_info2)
      continue;
    if (g_strcmp0 (child_info2->name, name) == 0) {
      g_warning ("Duplicate child name in HdyStackableBox: %s", name);

      break;
    }
  }

  g_free (child_info->name);
  child_info->name = g_strdup (name);

  if (self->visible_child == child_info)
    g_object_notify_by_pspec (G_OBJECT (self),
                              props[PROP_VISIBLE_CHILD_NAME]);
}

gboolean
hdy_stackable_box_get_child_allow_visible (HdyStackableBox *self,
                                           GtkWidget       *widget)
{
  HdyStackableBoxChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);

  g_return_val_if_fail (child_info != NULL, FALSE);

  return child_info->allow_visible;
}

void
hdy_stackable_box_set_child_allow_visible (HdyStackableBox *self,
                                           GtkWidget       *widget,
                                           gboolean         allow_visible)
{
  HdyStackableBoxChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);

  g_return_if_fail (child_info != NULL);

  child_info->allow_visible = allow_visible;

  if (!child_info->allow_visible &&
      hdy_stackable_box_get_visible_child (self) == widget)
    set_visible_child_info (self, NULL, self->transition_type, self->child_transition.duration, TRUE);
}

static void
hdy_stackable_box_class_init (HdyStackableBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = hdy_stackable_box_get_property;
  object_class->set_property = hdy_stackable_box_set_property;
  object_class->finalize = hdy_stackable_box_finalize;

  /**
   * HdyStackableBox:folded:
   *
   * %TRUE if the widget is folded.
   *
   * The #HdyStackableBox will be folded if the size allocated to it is smaller
   * than the sum of the natural size of its children, it will be unfolded
   * otherwise.
   */
  props[PROP_FOLDED] =
    g_param_spec_boolean ("folded",
                          _("Folded"),
                          _("Whether the widget is folded"),
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:hhomogeneous_folded:
   *
   * %TRUE if the widget allocates the same width for all children when folded.
   */
  props[PROP_HHOMOGENEOUS_FOLDED] =
    g_param_spec_boolean ("hhomogeneous-folded",
                          _("Horizontally homogeneous folded"),
                          _("Horizontally homogeneous sizing when the widget is folded"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:vhomogeneous_folded:
   *
   * %TRUE if the widget allocates the same height for all children when folded.
   */
  props[PROP_VHOMOGENEOUS_FOLDED] =
    g_param_spec_boolean ("vhomogeneous-folded",
                          _("Vertically homogeneous folded"),
                          _("Vertically homogeneous sizing when the widget is folded"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:hhomogeneous_unfolded:
   *
   * %TRUE if the widget allocates the same width for all children when unfolded.
   */
  props[PROP_HHOMOGENEOUS_UNFOLDED] =
    g_param_spec_boolean ("hhomogeneous-unfolded",
                          _("Box horizontally homogeneous"),
                          _("Horizontally homogeneous sizing when the widget is unfolded"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:vhomogeneous_unfolded:
   *
   * %TRUE if the widget allocates the same height for all children when unfolded.
   */
  props[PROP_VHOMOGENEOUS_UNFOLDED] =
    g_param_spec_boolean ("vhomogeneous-unfolded",
                          _("Box vertically homogeneous"),
                          _("Vertically homogeneous sizing when the widget is unfolded"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_VISIBLE_CHILD] =
    g_param_spec_object ("visible-child",
                         _("Visible child"),
                         _("The widget currently visible when the widget is folded"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_VISIBLE_CHILD_NAME] =
    g_param_spec_string ("visible-child-name",
                         _("Name of visible child"),
                         _("The name of the widget currently visible when the children are stacked"),
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:transition-type:
   *
   * The type of animation that will be used for transitions between modes and
   * children.
   *
   * The transition type can be changed without problems at runtime, so it is
   * possible to change the animation based on the mode or child that is about
   * to become current.
   *
   * Since: 1.0
   */
  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type",
                       _("Transition type"),
                       _("The type of animation used to transition between modes and children"),
                       HDY_TYPE_STACKABLE_BOX_TRANSITION_TYPE, HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_MODE_TRANSITION_DURATION] =
    g_param_spec_uint ("mode-transition-duration",
                       _("Mode transition duration"),
                       _("The mode transition animation duration, in milliseconds"),
                       0, G_MAXUINT, 250,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CHILD_TRANSITION_DURATION] =
    g_param_spec_uint ("child-transition-duration",
                       _("Child transition duration"),
                       _("The child transition animation duration, in milliseconds"),
                       0, G_MAXUINT, 200,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CHILD_TRANSITION_RUNNING] =
      g_param_spec_boolean ("child-transition-running",
                            _("Child transition running"),
                            _("Whether or not the child transition is currently running"),
                            FALSE,
                            G_PARAM_READABLE);

  props[PROP_INTERPOLATE_SIZE] =
      g_param_spec_boolean ("interpolate-size",
                            _("Interpolate size"),
                            _("Whether or not the size should smoothly change when changing between differently sized children"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:can-swipe-back:
   *
   * Whether or not @self allows switching to the previous child that has
   * 'allow-visible' child property set to %TRUE via a swipe gesture.
   *
   * Since: 1.0
   */
  props[PROP_CAN_SWIPE_BACK] =
      g_param_spec_boolean ("can-swipe-back",
                            _("Can swipe back"),
                            _("Whether or not swipe gesture can be used to switch to the previous child"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyStackableBox:can-swipe-forward:
   *
   * Whether or not @self allows switching to the next child that has
   * 'allow-visible' child property set to %TRUE via a swipe gesture.
   *
   * Since: 1.0
   */
  props[PROP_CAN_SWIPE_FORWARD] =
      g_param_spec_boolean ("can-swipe-forward",
                            _("Can swipe forward"),
                            _("Whether or not swipe gesture can be used to switch to the next child"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ORIENTATION] =
      g_param_spec_enum ("orientation",
                         _("Orientation"),
                         _("Orientation"),
                         GTK_TYPE_ORIENTATION,
                         GTK_ORIENTATION_HORIZONTAL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  child_props[CHILD_PROP_NAME] =
    g_param_spec_string ("name",
                         _("Name"),
                         _("The name of the child page"),
                         NULL,
                         G_PARAM_READWRITE);
}

HdyStackableBox *
hdy_stackable_box_new (GtkContainer      *container,
                       GtkContainerClass *klass,
                       gboolean           can_unfold)
{
  GtkWidget *widget;
  HdyStackableBox *self;

  g_return_val_if_fail (GTK_IS_CONTAINER (container), NULL);
  g_return_val_if_fail (GTK_IS_ORIENTABLE (container), NULL);
  g_return_val_if_fail (GTK_IS_CONTAINER_CLASS (klass), NULL);

  widget = GTK_WIDGET (container);
  self = g_object_new (HDY_TYPE_STACKABLE_BOX, NULL);

  self->container = container;
  self->klass = klass;
  self->can_unfold = can_unfold;

  self->children = NULL;
  self->children_reversed = NULL;
  self->visible_child = NULL;
  self->folded = FALSE;
  self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] = FALSE;
  self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] = FALSE;
  self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] = TRUE;
  self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] = TRUE;
  self->transition_type = HDY_STACKABLE_BOX_TRANSITION_TYPE_OVER;
  self->mode_transition.duration = 250;
  self->child_transition.duration = 200;
  self->mode_transition.current_pos = 1.0;
  self->mode_transition.target_pos = 1.0;

  self->tracker = hdy_swipe_tracker_new (HDY_SWIPEABLE (self->container));

  g_object_set (self->tracker, "orientation", self->orientation, "enabled", FALSE, NULL);

  self->shadow_helper = hdy_shadow_helper_new (widget);

  gtk_widget_set_has_window (widget, FALSE);
  gtk_widget_set_can_focus (widget, FALSE);
  gtk_widget_set_redraw_on_allocate (widget, FALSE);

  if (can_unfold) {
    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    gtk_style_context_add_class (context, "unfolded");
  }

  return self;
}

static void
hdy_stackable_box_init (HdyStackableBox *self)
{
}
