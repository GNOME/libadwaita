/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "gtkprogresstrackerprivate.h"
#include "hdy-leaflet.h"

/* TODO:
 * - Ensure folding and unfolding animations behave similarly.
 * - Unify mode and child transition types?
 * - Support RTL languages.
 */

/**
 * SECTION:hdy-leaflet
 * @short_description: An adaptive container acting like a box or a stack.
 * @Title: HdyLeaflet
 *
 * The #HdyLeaflet widget can display its children like a #GtkBox does or
 * like a #HdyLeaflet does, adapting to size changes by switching between
 * the two modes.
 *
 * When there is enough space the children are displayed side by side, otherwise
 * only one is displayed. The threshold is dictated by the preferred minimum
 * sizes of the children.
 */

/**
 * HdyLeafletModeTransitionType:
 * @HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE: No transition
 * @HDY_LEAFLET_MODE_TRANSITION_TYPE_SLIDE: Slide from left, right, up or down according to the orientation, text direction and the children order
 *
 * These enumeration values describe the possible transitions between pages in a
 * #HdyLeaflet widget.
 *
 * New values may be added to this enumeration over time.
 */

/**
 * HdyLeafletChildTransitionType:
 * @HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE: No transition
 * @HDY_LEAFLET_CHILD_TRANSITION_TYPE_CROSSFADE: A cross-fade
 * @HDY_LEAFLET_CHILD_TRANSITION_TYPE_SLIDE: Slide from left, right, up or down according to the orientation, text direction and the children order
 * @HDY_LEAFLET_CHILD_TRANSITION_TYPE_OVER: Cover the old page or uncover the new page, sliding from or towards the end according to orientation, text direction and children order
 * @HDY_LEAFLET_CHILD_TRANSITION_TYPE_UNDER: Uncover the new page or cover the old page, sliding from or towards the start according to orientation, text direction and children order
 *
 * These enumeration values describe the possible transitions between pages in a
 * #HdyLeaflet widget.
 *
 * New values may be added to this enumeration over time.
 */

enum {
  PROP_0,
  PROP_FOLD,
  PROP_FOLDED,
  PROP_HHOMOGENEOUS_FOLDED,
  PROP_VHOMOGENEOUS_FOLDED,
  PROP_HHOMOGENEOUS_UNFOLDED,
  PROP_VHOMOGENEOUS_UNFOLDED,
  PROP_VISIBLE_CHILD,
  PROP_VISIBLE_CHILD_NAME,
  PROP_MODE_TRANSITION_TYPE,
  PROP_MODE_TRANSITION_DURATION,
  PROP_CHILD_TRANSITION_TYPE,
  PROP_CHILD_TRANSITION_DURATION,
  PROP_CHILD_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,

  /* orientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ORIENTATION,
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_NAME,
  LAST_CHILD_PROP,
};

#define HDY_FOLD_MAX 2
#define GTK_ORIENTATION_MAX 2

typedef struct _HdyLeafletChildInfo HdyLeafletChildInfo;

struct _HdyLeafletChildInfo
{
  GtkWidget *widget;
  gchar *name;

  /* Convenience storage for per-child temporary frequently computed values. */
  GtkAllocation alloc;
  GtkRequisition min;
  GtkRequisition nat;
  gboolean visible;
};

typedef struct
{
  GList *children;
  /* It is probably cheaper to store and maintain a reversed copy of the
   * children list that to reverse the list every time we need to allocate or
   * draw children for RTL languages on a horizontal leaflet.
   */
  GList *children_reversed;
  HdyLeafletChildInfo *visible_child;
  HdyLeafletChildInfo *last_visible_child;

  GdkWindow* bin_window;
  GdkWindow* view_window;

  HdyFold fold;

  gboolean homogeneous[HDY_FOLD_MAX][GTK_ORIENTATION_MAX];

  GtkOrientation orientation;

  gboolean move_bin_window_request;

  struct {
    HdyLeafletModeTransitionType type;
    guint duration;

    gdouble current_pos;
    gdouble source_pos;
    gdouble target_pos;

    cairo_surface_t *start_surface;
    GtkAllocation start_surface_allocation;
    cairo_surface_t *end_surface;
    GtkAllocation end_surface_allocation;
    guint tick_id;
    GtkProgressTracker tracker;
  } mode_transition;

  /* Child transition variables. */
  struct {
    HdyLeafletChildTransitionType type;
    guint duration;

    cairo_surface_t *last_visible_surface;
    GtkAllocation last_visible_surface_allocation;
    guint tick_id;
    GtkProgressTracker tracker;
    gboolean first_frame_skipped;

    gint last_visible_widget_width;
    gint last_visible_widget_height;

    gboolean interpolate_size;

    HdyLeafletChildTransitionType active_type;
    GtkPanDirection active_direction;
  } child_transition;

} HdyLeafletPrivate;

static GParamSpec *props[LAST_PROP];
static GParamSpec *child_props[LAST_CHILD_PROP];

static gint HOMOGENEOUS_PROP[HDY_FOLD_MAX][GTK_ORIENTATION_MAX] = {
  { PROP_HHOMOGENEOUS_UNFOLDED, PROP_VHOMOGENEOUS_UNFOLDED},
  { PROP_HHOMOGENEOUS_FOLDED, PROP_VHOMOGENEOUS_FOLDED},
};

static void hdy_leaflet_buildable_init (GtkBuildableIface  *iface);

G_DEFINE_TYPE_WITH_CODE (HdyLeaflet, hdy_leaflet, GTK_TYPE_CONTAINER,
                         G_ADD_PRIVATE (HdyLeaflet)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, hdy_leaflet_buildable_init))

static void
free_child_info (HdyLeafletChildInfo *child_info)
{
  g_free (child_info->name);
  g_free (child_info);
}

static HdyLeafletChildInfo *
find_child_info_for_widget (HdyLeaflet *self,
                            GtkWidget  *widget)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GList *children;
  HdyLeafletChildInfo *child_info;

  for (children = priv->children; children; children = children->next) {
    child_info = children->data;

    if (child_info->widget == widget)
      return child_info;
  }

  return NULL;
}

static HdyLeafletChildInfo *
find_child_info_for_name (HdyLeaflet  *self,
                          const gchar *name)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GList *children;
  HdyLeafletChildInfo *child_info;

  for (children = priv->children; children; children = children->next) {
    child_info = children->data;

    if (g_strcmp0 (child_info->name, name) == 0)
      return child_info;
  }

  return NULL;
}

static GList *
get_directed_children (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  return priv->orientation == GTK_ORIENTATION_HORIZONTAL &&
         gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL ?
         priv->children_reversed : priv->children;
}

static gboolean
get_enable_animations (void)
{
  gboolean enable_animations;

  g_object_get (gtk_settings_get_default (),
                "gtk-enable-animations", &enable_animations,
                NULL);

  return enable_animations;
}

/* Transitions that cause the bin window to move */
static inline gboolean
is_window_moving_child_transition (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkPanDirection direction;
  gboolean is_rtl;
  GtkPanDirection left_or_right, right_or_left;

  direction = priv->child_transition.active_direction;
  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  left_or_right = is_rtl ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  right_or_left = is_rtl ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;

  switch (priv->child_transition.active_type) {
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE:
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_CROSSFADE:
    return FALSE;
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_SLIDE:
    return TRUE;
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_OVER:
    return direction == GTK_PAN_DIRECTION_UP || direction == left_or_right;
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_UNDER:
    return direction == GTK_PAN_DIRECTION_DOWN || direction == right_or_left;
  default:
    g_assert_not_reached ();
  }
}

/* Transitions that change direction depending on the relative order of the
old and new child */
static inline gboolean
is_direction_dependent_child_transition (HdyLeafletChildTransitionType transition_type)
{
  return (transition_type == HDY_LEAFLET_CHILD_TRANSITION_TYPE_SLIDE ||
          transition_type == HDY_LEAFLET_CHILD_TRANSITION_TYPE_OVER ||
          transition_type == HDY_LEAFLET_CHILD_TRANSITION_TYPE_UNDER);
}

static GtkPanDirection
get_pan_direction (HdyLeaflet *self,
                   gboolean    new_child_first)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      return new_child_first ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;
    else
      return new_child_first ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  }
  else
    return new_child_first ? GTK_PAN_DIRECTION_DOWN : GTK_PAN_DIRECTION_UP;
}

static gint
get_bin_window_x (HdyLeaflet          *self,
                  const GtkAllocation *allocation)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  int x = 0;

  if (gtk_progress_tracker_get_state (&priv->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
    if (priv->child_transition.active_direction == GTK_PAN_DIRECTION_LEFT)
      x = allocation->width * (1 - gtk_progress_tracker_get_ease_out_cubic (&priv->child_transition.tracker, FALSE));
    if (priv->child_transition.active_direction == GTK_PAN_DIRECTION_RIGHT)
      x = -allocation->width * (1 - gtk_progress_tracker_get_ease_out_cubic (&priv->child_transition.tracker, FALSE));
  }

  return x;
}

static gint
get_bin_window_y (HdyLeaflet          *self,
                  const GtkAllocation *allocation)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  int y = 0;

  if (gtk_progress_tracker_get_state (&priv->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
    if (priv->child_transition.active_direction == GTK_PAN_DIRECTION_UP)
      y = allocation->height * (1 - gtk_progress_tracker_get_ease_out_cubic (&priv->child_transition.tracker, FALSE));
    if (priv->child_transition.active_direction == GTK_PAN_DIRECTION_DOWN)
      y = -allocation->height * (1 - gtk_progress_tracker_get_ease_out_cubic (&priv->child_transition.tracker, FALSE));
  }

  return y;
}

static void
move_resize_bin_window (HdyLeaflet    *self,
                        GtkAllocation *allocation,
                        gboolean       resize)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkAllocation alloc;
  gboolean move;

  if (priv->bin_window == NULL)
    return;

  if (allocation == NULL) {
    gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);
    allocation = &alloc;
  }

  move = priv->move_bin_window_request || is_window_moving_child_transition (self);

  if (move && resize)
    gdk_window_move_resize (priv->bin_window,
                            get_bin_window_x (self, allocation), get_bin_window_y (self, allocation),
                            allocation->width, allocation->height);
  else if (move)
    gdk_window_move (priv->bin_window,
                     get_bin_window_x (self, allocation), get_bin_window_y (self, allocation));
  else if (resize)
    gdk_window_resize (priv->bin_window,
                       allocation->width, allocation->height);

  priv->move_bin_window_request = FALSE;
}

static void
hdy_leaflet_child_progress_updated (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  gtk_widget_queue_draw (GTK_WIDGET (self));

  if (!priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] ||
      !priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL])
    gtk_widget_queue_resize (GTK_WIDGET (self));

  move_resize_bin_window (self, NULL, FALSE);

  if (gtk_progress_tracker_get_state (&priv->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (priv->child_transition.last_visible_surface != NULL) {
      cairo_surface_destroy (priv->child_transition.last_visible_surface);
      priv->child_transition.last_visible_surface = NULL;
    }

    if (priv->last_visible_child != NULL) {
      if (hdy_leaflet_get_fold (self) == HDY_FOLD_FOLDED)
        gtk_widget_set_child_visible (priv->last_visible_child->widget, FALSE);
      priv->last_visible_child = NULL;
    }
  }
}

static gboolean
hdy_leaflet_child_transition_cb (GtkWidget     *widget,
                                 GdkFrameClock *frame_clock,
                                 gpointer       user_data)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.first_frame_skipped)
    gtk_progress_tracker_advance_frame (&priv->child_transition.tracker,
                                        gdk_frame_clock_get_frame_time (frame_clock));
  else
    priv->child_transition.first_frame_skipped = TRUE;

  /* Finish animation early if not mapped anymore */
  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&priv->child_transition.tracker);

  hdy_leaflet_child_progress_updated (self);

  if (gtk_progress_tracker_get_state (&priv->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    priv->child_transition.tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);

    return FALSE;
  }

  return TRUE;
}

static void
hdy_leaflet_schedule_child_ticks (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.tick_id == 0) {
    priv->child_transition.tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self), hdy_leaflet_child_transition_cb, self, NULL);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
  }
}

static void
hdy_leaflet_unschedule_child_ticks (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), priv->child_transition.tick_id);
    priv->child_transition.tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
  }
}

static void
hdy_leaflet_stop_child_transition (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  hdy_leaflet_unschedule_child_ticks (self);
  priv->child_transition.active_type = HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE;
  gtk_progress_tracker_finish (&priv->child_transition.tracker);
  if (priv->child_transition.last_visible_surface != NULL) {
    cairo_surface_destroy (priv->child_transition.last_visible_surface);
    priv->child_transition.last_visible_surface = NULL;
  }
  if (priv->last_visible_child != NULL) {
    gtk_widget_set_child_visible (priv->last_visible_child->widget, FALSE);
    priv->last_visible_child = NULL;
  }

  /* Move the bin window back in place as a child transition might have moved it. */
  priv->move_bin_window_request = TRUE;
}

static void
hdy_leaflet_start_child_transition (HdyLeaflet                    *self,
                                    HdyLeafletChildTransitionType  transition_type,
                                    guint                          transition_duration,
                                    GtkPanDirection                transition_direction)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (self);

  if (gtk_widget_get_mapped (widget) &&
      get_enable_animations () &&
      transition_type != HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE &&
      transition_duration != 0 &&
      priv->last_visible_child != NULL &&
      /* Don't animate child transition when a mode transition is ongoing. */
      priv->mode_transition.tick_id == 0) {
    priv->child_transition.active_type = transition_type;
    priv->child_transition.active_direction = transition_direction;
    priv->child_transition.first_frame_skipped = FALSE;
    hdy_leaflet_schedule_child_ticks (self);
    gtk_progress_tracker_start (&priv->child_transition.tracker,
                                priv->child_transition.duration * 1000,
                                0,
                                1.0);
  }
  else {
    hdy_leaflet_unschedule_child_ticks (self);
    priv->child_transition.active_type = HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE;
    gtk_progress_tracker_finish (&priv->child_transition.tracker);
  }

  hdy_leaflet_child_progress_updated (self);
}

static void
set_visible_child_info (HdyLeaflet                    *self,
                        HdyLeafletChildInfo           *new_visible_child,
                        HdyLeafletChildTransitionType  transition_type,
                        guint                          transition_duration)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (self);
  GList *children;
  HdyLeafletChildInfo *child_info;
  GtkPanDirection transition_direction = GTK_PAN_DIRECTION_LEFT;

  /* If we are being destroyed, do not bother with transitions and   *
   * notifications.
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick first visible. */
  if (new_visible_child == NULL) {
    for (children = priv->children; children; children = children->next) {
      child_info = children->data;

      if (gtk_widget_get_visible (child_info->widget)) {
        new_visible_child = child_info;

        break;
      }
    }
  }

  if (new_visible_child == priv->visible_child)
    return;

  /* FIXME Probably copied from Gtk Stack, should check whether it's needed. */
  /* toplevel = gtk_widget_get_toplevel (widget); */
  /* if (GTK_IS_WINDOW (toplevel)) { */
  /*   focus = gtk_window_get_focus (GTK_WINDOW (toplevel)); */
  /*   if (focus && */
  /*       priv->visible_child && */
  /*       priv->visible_child->widget && */
  /*       gtk_widget_is_ancestor (focus, priv->visible_child->widget)) { */
  /*     contains_focus = TRUE; */

  /*     if (priv->visible_child->last_focus) */
  /*       g_object_remove_weak_pointer (G_OBJECT (priv->visible_child->last_focus), */
  /*                                     (gpointer *)&priv->visible_child->last_focus); */
  /*     priv->visible_child->last_focus = focus; */
  /*     g_object_add_weak_pointer (G_OBJECT (priv->visible_child->last_focus), */
  /*                                (gpointer *)&priv->visible_child->last_focus); */
  /*   } */
  /* } */

  if (priv->last_visible_child)
    gtk_widget_set_child_visible (priv->last_visible_child->widget, FALSE);
  priv->last_visible_child = NULL;

  if (priv->child_transition.last_visible_surface != NULL)
    cairo_surface_destroy (priv->child_transition.last_visible_surface);
  priv->child_transition.last_visible_surface = NULL;

  if (priv->visible_child && priv->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      GtkAllocation allocation;

      priv->last_visible_child = priv->visible_child;
      gtk_widget_get_allocated_size (priv->last_visible_child->widget, &allocation, NULL);
      priv->child_transition.last_visible_widget_width = allocation.width;
      priv->child_transition.last_visible_widget_height = allocation.height;
    }
    else
      gtk_widget_set_child_visible (priv->visible_child->widget, FALSE);
  }

  /* FIXME This comes from GtkStack and should be adapted. */
  /* hdy_leaflet_accessible_update_visible_child (stack, */
  /*                                              priv->visible_child ? priv->visible_child->widget : NULL, */
  /*                                              new_visible_child ? new_visible_child->widget : NULL); */

  priv->visible_child = new_visible_child;

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

  if ((new_visible_child == NULL || priv->last_visible_child == NULL) &&
      is_direction_dependent_child_transition (transition_type))
    transition_type = HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE;
  else if (is_direction_dependent_child_transition (transition_type)) {
    gboolean new_first = FALSE;
    for (children = priv->children; children; children = children->next) {
      if (new_visible_child == children->data) {
        new_first = TRUE;

        break;
      }
      if (priv->last_visible_child == children->data)
        break;
    }

    transition_direction = get_pan_direction (self, new_first);
  }

  if (priv->fold == HDY_FOLD_FOLDED) {
    if (priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] &&
        priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL])
      gtk_widget_queue_allocate (widget);
    else
      gtk_widget_queue_resize (widget);

    hdy_leaflet_start_child_transition (self, transition_type, transition_duration, transition_direction);
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
hdy_leaflet_set_position (HdyLeaflet *self,
                          gdouble     pos)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  gboolean new_visible;
  GtkWidget *child;
  /* HdyLeafletModeTransitionType transition; */

  priv->mode_transition.current_pos = pos;

  /* We check mode_transition.target_pos here too, because we want to ensure we set
   * child_visible immediately when starting a reveal operation
   * otherwise the child widgets will not be properly realized
   * after the reveal returns.
   */
  new_visible = priv->mode_transition.current_pos != 0.0 || priv->mode_transition.target_pos != 0.0;

  child = hdy_leaflet_get_visible_child (self);
  if (child != NULL &&
      new_visible != gtk_widget_get_child_visible (child))
    gtk_widget_set_child_visible (child, new_visible);

  /* FIXME Copied from GtkRevealer IIRC, check whether it's useful. */
  /* transition = effective_transition (self); */
  /* if (transition == GTK_REVEALER_TRANSITION_TYPE_CROSSFADE) { */
  /*   gtk_widget_set_opacity (GTK_WIDGET (self), priv->mode_transition.current_pos); */
  /*   gtk_widget_queue_draw (GTK_WIDGET (self)); */
  /* } */
  /* else */
    gtk_widget_queue_resize (GTK_WIDGET (self));
  /* } */

  /* if (priv->mode_transition.current_pos == priv->mode_transition.target_pos) */
  /*   g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_REVEALED]); */
}

static void
hdy_leaflet_mode_progress_updated (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (gtk_progress_tracker_get_state (&priv->mode_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (priv->mode_transition.start_surface != NULL) {
      cairo_surface_destroy (priv->mode_transition.start_surface);
      priv->mode_transition.start_surface = NULL;
    }

    if (priv->mode_transition.end_surface != NULL) {
      cairo_surface_destroy (priv->mode_transition.end_surface);
      priv->mode_transition.end_surface = NULL;
    }
  }
}

static gboolean
hdy_leaflet_mode_transition_cb (GtkWidget     *widget,
                                GdkFrameClock *frame_clock,
                                gpointer       user_data)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  gdouble ease;

  gtk_progress_tracker_advance_frame (&priv->mode_transition.tracker,
                                      gdk_frame_clock_get_frame_time (frame_clock));
  ease = gtk_progress_tracker_get_ease_out_cubic (&priv->mode_transition.tracker, FALSE);
  hdy_leaflet_set_position (self,
                            priv->mode_transition.source_pos + (ease * (priv->mode_transition.target_pos - priv->mode_transition.source_pos)));

  hdy_leaflet_mode_progress_updated (self);

  if (gtk_progress_tracker_get_state (&priv->mode_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    priv->mode_transition.tick_id = 0;
    return FALSE;
  }

  return TRUE;
}

static void
hdy_leaflet_start_mode_transition (HdyLeaflet *self,
                                   gdouble     target)
{
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (self);
  HdyLeafletModeTransitionType transition;

  if (priv->mode_transition.target_pos == target)
    return;

  priv->mode_transition.target_pos = target;
  /* FIXME PROP_REVEAL_CHILD needs to be implemented. */
  /* g_object_notify_by_pspec (G_OBJECT (revealer), props[PROP_REVEAL_CHILD]); */

  hdy_leaflet_stop_child_transition (self);

  transition = priv->mode_transition.type;
  if (gtk_widget_get_mapped (widget) &&
      priv->mode_transition.duration != 0 &&
      transition != HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE &&
      get_enable_animations ()) {
    priv->mode_transition.source_pos = priv->mode_transition.current_pos;
    if (priv->mode_transition.tick_id == 0)
      priv->mode_transition.tick_id = gtk_widget_add_tick_callback (widget, hdy_leaflet_mode_transition_cb, self, NULL);
    gtk_progress_tracker_start (&priv->mode_transition.tracker,
                                priv->mode_transition.duration * 1000,
                                0,
                                1.0);
  }
  else
    hdy_leaflet_set_position (self, target);
}

/* FIXME Use this to stop the mode transition animation when it makes sense (see *
 * GtkRevealer for exmples).
 */
/* static void */
/* hdy_leaflet_stop_mode_animation (HdyLeaflet *self) */
/* { */
/*   HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self); */

/*   if (priv->mode_transition.current_pos != priv->mode_transition.target_pos) { */
/*     priv->mode_transition.current_pos = priv->mode_transition.target_pos; */
    /* g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_REVEALED]); */
/*   } */
/*   if (priv->mode_transition.tick_id != 0) { */
/*     gtk_widget_remove_tick_callback (GTK_WIDGET (self), priv->mode_transition.tick_id); */
/*     priv->mode_transition.tick_id = 0; */
/*   } */
/* } */

/**
 * hdy_leaflet_get_fold:
 * @self: a #HdyLeaflet
 *
 * Gets the fold of @self.
 *
 * Returns: the fold of @self.
 */
HdyFold
hdy_leaflet_get_fold (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->fold;
}

static void
hdy_leaflet_set_fold (HdyLeaflet *self,
                      HdyFold     fold)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->fold == fold)
    return;

  priv->fold = fold;

  if (fold)
    hdy_leaflet_start_mode_transition (self, 0.0);
  else
    hdy_leaflet_start_mode_transition (self, 1.0);

  g_object_freeze_notify (G_OBJECT (self));
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_FOLD]);
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_FOLDED]);
  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * hdy_leaflet_set_homogeneous:
 * @self: a #HdyLeaflet
 * @fold: the fold
 * @orientation: the orientation
 * @homogeneous: %TRUE to make @self homogeneous
 *
 * Sets the #HdyLeaflet to be homogeneous or not for the given fold and orientation.
 * If it is homogeneous, the #HdyLeaflet will request the same
 * width or height for all its children depending on the orientation.
 * If it isn't and it is folded, the leaflet may change width or height
 * when a different child becomes visible.
 */
void
hdy_leaflet_set_homogeneous (HdyLeaflet     *self,
                             HdyFold         fold,
                             GtkOrientation  orientation,
                             gboolean        homogeneous)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  homogeneous = !!homogeneous;

  if (priv->homogeneous[fold][orientation] == homogeneous)
    return;

  priv->homogeneous[fold][orientation] = homogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[HOMOGENEOUS_PROP[fold][orientation]]);
}

/**
 * hdy_leaflet_get_homogeneous:
 * @self: a #HdyLeaflet
 * @fold: the fold
 * @orientation: the orientation
 *
 * Gets whether @self is homogeneous for the given fold and orientation.
 * See hdy_leaflet_set_homogeneous().
 *
 * Returns: whether @self is homogeneous for the given fold and orientation.
 */
gboolean
hdy_leaflet_get_homogeneous (HdyLeaflet     *self,
                             HdyFold         fold,
                             GtkOrientation  orientation)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->homogeneous[fold][orientation];
}

/**
 * hdy_leaflet_get_mode_transition_type:
 * @self: a #HdyLeaflet
 *
 * Gets the type of animation that will be used
 * for transitions between modes in @self.
 *
 * Returns: the current mode transition type of @self
 */
HdyLeafletModeTransitionType
hdy_leaflet_get_mode_transition_type (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->mode_transition.type;
}

/**
 * hdy_leaflet_set_mode_transition_type:
 * @self: a #HdyLeaflet
 * @transition: the new transition type
 *
 * Sets the type of animation that will be used for
 * transitions between modes in @self.
 *
 * The transition type can be changed without problems
 * at runtime, so it is possible to change the animation
 * based on the mode that is about to become current.
 */
void
hdy_leaflet_set_mode_transition_type (HdyLeaflet                   *self,
                                      HdyLeafletModeTransitionType  transition)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->mode_transition.type == transition)
    return;

  priv->mode_transition.type = transition;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_MODE_TRANSITION_TYPE]);
}

/**
 * hdy_leaflet_get_mode_transition_duration:
 * @self: a #HdyLeaflet
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between modes in @self will take.
 *
 * Returns: the mode transition duration
 */
guint
hdy_leaflet_get_mode_transition_duration (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), 0);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->mode_transition.duration;
}

/**
 * hdy_leaflet_set_mode_transition_duration:
 * @self: a #HdyLeaflet
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between modes in @self
 * will take.
 */
void
hdy_leaflet_set_mode_transition_duration (HdyLeaflet *self,
                                          guint       duration)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->mode_transition.duration == duration)
    return;

  priv->mode_transition.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_MODE_TRANSITION_DURATION]);
}

/**
 * hdy_leaflet_get_child_transition_type:
 * @self: a #HdyLeaflet
 *
 * Gets the type of animation that will be used
 * for transitions between modes in @self.
 *
 * Returns: the current mode transition type of @self
 */
HdyLeafletChildTransitionType
hdy_leaflet_get_child_transition_type (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->child_transition.type;
}

/**
 * hdy_leaflet_set_child_transition_type:
 * @self: a #HdyLeaflet
 * @transition: the new transition type
 *
 * Sets the type of animation that will be used for
 * transitions between children in @self.
 *
 * The transition type can be changed without problems
 * at runtime, so it is possible to change the animation
 * based on the mode that is about to become current.
 */
void
hdy_leaflet_set_child_transition_type (HdyLeaflet                    *self,
                                       HdyLeafletChildTransitionType  transition)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.type == transition)
    return;

  priv->child_transition.type = transition;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_CHILD_TRANSITION_TYPE]);
}

/**
 * hdy_leaflet_get_child_transition_duration:
 * @self: a #HdyLeaflet
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between children in @self will take.
 *
 * Returns: the mode transition duration
 */
guint
hdy_leaflet_get_child_transition_duration (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), 0);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->child_transition.duration;
}

/**
 * hdy_leaflet_set_child_transition_duration:
 * @self: a #HdyLeaflet
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between children in @self
 * will take.
 */
void
hdy_leaflet_set_child_transition_duration (HdyLeaflet *self,
                                           guint       duration)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.duration == duration)
    return;

  priv->child_transition.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_CHILD_TRANSITION_DURATION]);
}

/**
 * hdy_leaflet_get_visible_child:
 * @self: a #HdyLeaflet
 *
 * Get the visible child widget.
 *
 * Returns: (transfer none): the visible child widget
 */
GtkWidget *
hdy_leaflet_get_visible_child (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->visible_child == NULL)
    return NULL;

  return priv->visible_child->widget;
}

void
hdy_leaflet_set_visible_child (HdyLeaflet *self,
                               GtkWidget  *visible_child)
{
  HdyLeafletPrivate *priv;
  HdyLeafletChildInfo *child_info;
  gboolean contains_child;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (GTK_IS_WIDGET (visible_child));

  priv = hdy_leaflet_get_instance_private (self);

  child_info = find_child_info_for_widget (self, visible_child);
  contains_child = child_info != NULL;

  g_return_if_fail (contains_child);

  set_visible_child_info (self, child_info, priv->child_transition.type, priv->child_transition.duration);
}

const gchar *
hdy_leaflet_get_visible_child_name (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);

  priv = hdy_leaflet_get_instance_private (self);

  if (priv->visible_child == NULL)
    return NULL;

  return priv->visible_child->name;
}

void
hdy_leaflet_set_visible_child_name (HdyLeaflet  *self,
                                    const gchar *name)
{
  HdyLeafletPrivate *priv;
  HdyLeafletChildInfo *child_info;
  gboolean contains_child;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (name != NULL);

  priv = hdy_leaflet_get_instance_private (self);

  child_info = find_child_info_for_name (self, name);
  contains_child = child_info != NULL;

  g_return_if_fail (contains_child);

  set_visible_child_info (self, child_info, priv->child_transition.type, priv->child_transition.duration);
}

/**
 * hdy_leaflet_get_child_transition_running:
 * @self: a #HdyLeaflet
 *
 * Returns whether @self is currently in a transition from one page to
 * another.
 *
 * Returns: %TRUE if the transition is currently running, %FALSE otherwise.
 */
gboolean
hdy_leaflet_get_child_transition_running (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  priv = hdy_leaflet_get_instance_private (self);

  return (priv->child_transition.tick_id != 0);
}

/**
 * hdy_leaflet_set_interpolate_size:
 * @self: a #HdyLeaflet
 * @interpolate_size: the new value
 *
 * Sets whether or not @self will interpolate its size when
 * changing the visible child. If the #HdyLeaflet:interpolate-size
 * property is set to %TRUE, @stack will interpolate its size between
 * the current one and the one it'll take after changing the
 * visible child, according to the set transition duration.
 */
void
hdy_leaflet_set_interpolate_size (HdyLeaflet *self,
                                  gboolean    interpolate_size)
{
  HdyLeafletPrivate *priv;

  g_return_if_fail (HDY_IS_LEAFLET (self));

  priv = hdy_leaflet_get_instance_private (self);

  interpolate_size = !!interpolate_size;

  if (priv->child_transition.interpolate_size == interpolate_size)
    return;

  priv->child_transition.interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATE_SIZE]);
}

/**
 * hdy_leaflet_get_interpolate_size:
 * @self: a #HdyLeaflet
 *
 * Returns wether the #HdyLeaflet is set up to interpolate between
 * the sizes of children on page switch.
 *
 * Returns: %TRUE if child sizes are interpolated
 */
gboolean
hdy_leaflet_get_interpolate_size (HdyLeaflet *self)
{
  HdyLeafletPrivate *priv;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  priv = hdy_leaflet_get_instance_private (self);

  return priv->child_transition.interpolate_size;
}

#define LERP(a, b, t) ((a) + (((b) - (a)) * (1.0 - (t))))

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
             LERP (visible_min, last_visible_min, visible_child_progress);
    *nat = homogeneous_unfolded ?
             max_nat * visible_children :
             sum_nat;
  }
  else {
    *min = homogeneous_folded ?
             max_min :
             LERP (visible_min, last_visible_min, visible_child_progress);
    *nat = max_nat;
  }
}

/* This private method is prefixed by the call name because it will be a virtual
 * method in GTK 4.
 */
static void
hdy_leaflet_measure (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GList *children;
  HdyLeafletChildInfo *child_info;
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
  for (children = priv->children; children; children = children->next) {
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

  if (priv->visible_child != NULL) {
    if (for_size < 0)
      get_preferred_size_static (priv->visible_child->widget,
                                 &visible_min, NULL);
    else
      get_preferred_size_for_size (priv->visible_child->widget, for_size,
                                   &visible_min, NULL);
  }

  if (priv->last_visible_child != NULL) {
    if (for_size < 0)
      get_preferred_size_static (priv->last_visible_child->widget,
                                 &last_visible_min, NULL);
    else
      get_preferred_size_for_size (priv->last_visible_child->widget, for_size,
                                   &last_visible_min, NULL);
  }

  visible_child_progress = priv->child_transition.interpolate_size ? gtk_progress_tracker_get_ease_out_cubic (&priv->child_transition.tracker, FALSE) : 1.0;

  get_preferred_size (minimum, natural,
                      gtk_orientable_get_orientation (GTK_ORIENTABLE (widget)) == orientation,
                      priv->homogeneous[HDY_FOLD_FOLDED][orientation],
                      priv->homogeneous[HDY_FOLD_UNFOLDED][orientation],
                      visible_children, visible_child_progress,
                      sum_nat, max_min, max_nat, visible_min, last_visible_min);
}

static void
hdy_leaflet_get_preferred_width (GtkWidget *widget,
                                 gint      *minimum_width,
                                 gint      *natural_width)
{
  hdy_leaflet_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                       minimum_width, natural_width, NULL, NULL);
}

static void
hdy_leaflet_get_preferred_height (GtkWidget *widget,
                                  gint      *minimum_height,
                                  gint      *natural_height)
{
  hdy_leaflet_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                       minimum_height, natural_height, NULL, NULL);
}

static void
hdy_leaflet_get_preferred_width_for_height (GtkWidget *widget,
                                            gint       height,
                                            gint      *minimum_width,
                                            gint      *natural_width)
{
  hdy_leaflet_measure (widget, GTK_ORIENTATION_HORIZONTAL, height,
                       minimum_width, natural_width, NULL, NULL);
}

static void
hdy_leaflet_get_preferred_height_for_width (GtkWidget *widget,
                                            gint       width,
                                            gint      *minimum_height,
                                            gint      *natural_height)
{
  hdy_leaflet_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                       minimum_height, natural_height, NULL, NULL);
}

static void
hdy_leaflet_size_allocate_folded (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GList *directed_children, *children;
  HdyLeafletChildInfo *child_info, *visible_child;
  gint start_size, end_size, visible_size;
  gint remaining_start_size, remaining_end_size, remaining_size;
  gint current_pad;
  gint max_child_size = 0;
  gboolean box_homogeneous;
  HdyLeafletModeTransitionType mode_transition_type;

  directed_children = get_directed_children (self);
  visible_child = priv->visible_child;

  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    if (!child_info->widget)
      continue;

    if (child_info->widget != visible_child->widget)
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

  mode_transition_type = priv->mode_transition.type;

  /* Avoid useless computations and allow visible child transitions. */
  if (priv->mode_transition.current_pos <= 0.0)
    mode_transition_type = HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE;

  switch (mode_transition_type) {
  case HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE:
    /* Child transitions should be applied only when folded and when no mode
     * transition is ongoing.
     */
    for (children = directed_children; children; children = children->next) {
      child_info = children->data;

      if (child_info != visible_child) {
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
  case HDY_LEAFLET_MODE_TRANSITION_TYPE_SLIDE:
    /* Compute visible child size. */

    visible_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
      MIN (allocation->width, MAX (visible_child->nat.width, (gint) (allocation->width * (1.0 - priv->mode_transition.current_pos)))) :
      MIN (allocation->height, MAX (visible_child->nat.height, (gint) (allocation->height * (1.0 - priv->mode_transition.current_pos))));

    /* Compute homogeneous box child size. */
    box_homogeneous = (priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] && orientation == GTK_ORIENTATION_HORIZONTAL) ||
                      (priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] && orientation == GTK_ORIENTATION_VERTICAL);
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
      priv->mode_transition.start_surface_allocation.width = start_size;
      priv->mode_transition.start_surface_allocation.height = allocation->height;
      /* FIXME RTL */
      priv->mode_transition.start_surface_allocation.x = remaining_start_size - start_size;
      priv->mode_transition.start_surface_allocation.y = 0;
      priv->mode_transition.end_surface_allocation.width = end_size;
      priv->mode_transition.end_surface_allocation.height = allocation->height;
      /* FIXME RTL */
      priv->mode_transition.end_surface_allocation.x = remaining_start_size + visible_size;
      priv->mode_transition.end_surface_allocation.y = 0;
      break;
    case GTK_ORIENTATION_VERTICAL:
      priv->mode_transition.start_surface_allocation.width = allocation->width;
      priv->mode_transition.start_surface_allocation.height = start_size;
      priv->mode_transition.start_surface_allocation.x = 0;
      priv->mode_transition.start_surface_allocation.y = remaining_start_size - start_size;
      priv->mode_transition.end_surface_allocation.width = allocation->width;
      priv->mode_transition.end_surface_allocation.height = end_size;
      priv->mode_transition.end_surface_allocation.x = 0;
      priv->mode_transition.end_surface_allocation.y = remaining_start_size + visible_size;
      break;
    default:
      g_assert_not_reached ();
    }

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
    current_pad = start_size - remaining_start_size;
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
    current_pad = end_size - remaining_end_size;
    for (children = g_list_last (directed_children); children; children = children->prev) {
      child_info = children->data;

      if (child_info == visible_child)
        break;

      if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        child_info->alloc.width = box_homogeneous ?
          max_child_size :
          child_info->nat.width;
        child_info->alloc.height = allocation->height;
        child_info->alloc.x = allocation->width - child_info->alloc.width + current_pad;
        child_info->alloc.y = 0;
        child_info->visible = child_info->alloc.x < allocation->width;

        current_pad -= child_info->alloc.width;
      }
      else {
        child_info->alloc.width = allocation->width;
        child_info->alloc.height = box_homogeneous ?
          max_child_size :
          child_info->nat.height;
        child_info->alloc.x = 0;
        child_info->alloc.y = allocation->height - child_info->alloc.height + current_pad;
        child_info->visible = child_info->alloc.y < allocation->height;

        current_pad -= child_info->alloc.height;
      }
    }

    break;
  default:
    g_assert_not_reached ();
  }
}

static void
hdy_leaflet_size_allocate_unfolded (GtkWidget     *widget,
                                    GtkAllocation *allocation)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GtkAllocation remaining_alloc;
  GList *directed_children, *children;
  HdyLeafletChildInfo *child_info, *visible_child;
  gint homogeneous_size = 0, min_size, extra_size;
  gint per_child_extra, n_extra_widgets;
  gint n_visible_children, n_expand_children;
  gint start_pad = 0, end_pad = 0;
  gboolean box_homogeneous;

  directed_children = get_directed_children (self);
  visible_child = priv->visible_child;

  box_homogeneous = (priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] && orientation == GTK_ORIENTATION_HORIZONTAL) ||
                    (priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] && orientation == GTK_ORIENTATION_VERTICAL);

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
      homogeneous_size = allocation->width / n_visible_children;
      n_expand_children = allocation->width % n_visible_children;
      min_size = allocation->width - n_expand_children;
    }
    else {
      homogeneous_size = allocation->height / n_visible_children;
      n_expand_children = allocation->height % n_visible_children;
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
    start_pad = (gint) ((visible_child->alloc.x) * (1.0 - priv->mode_transition.current_pos));
    end_pad = (gint) ((allocation->width - (visible_child->alloc.x + visible_child->alloc.width)) * (1.0 - priv->mode_transition.current_pos));
  }
  else {
    start_pad = (gint) ((visible_child->alloc.y) * (1.0 - priv->mode_transition.current_pos));
    end_pad = (gint) ((allocation->height - (visible_child->alloc.y + visible_child->alloc.height)) * (1.0 - priv->mode_transition.current_pos));
  }

  for (children = directed_children; children; children = children->next) {
    child_info = children->data;

    if (child_info == visible_child)
      break;

    if (!child_info->visible)
      continue;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      child_info->alloc.x -= start_pad;
    else
      child_info->alloc.y -= start_pad;
  }

  for (children = g_list_last (directed_children); children; children = children->prev) {
    child_info = children->data;

    if (child_info == visible_child)
      break;

    if (!child_info->visible)
      continue;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      child_info->alloc.x += end_pad;
    else
      child_info->alloc.y += end_pad;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    visible_child->alloc.x -= start_pad;
    visible_child->alloc.width += start_pad + end_pad;
  }
  else {
    visible_child->alloc.y -= start_pad;
    visible_child->alloc.height += start_pad + end_pad;
  }
}

static void
hdy_leaflet_size_allocate (GtkWidget     *widget,
                           GtkAllocation *allocation)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GList *directed_children, *children;
  HdyLeafletChildInfo *child_info;
  gint nat_box_size, nat_max_size, visible_children;
  gboolean folded;

  directed_children = get_directed_children (self);

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget)) {
    gdk_window_move_resize (priv->view_window,
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
    if (priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL])
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
    if (priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL])
      nat_box_size = nat_max_size * visible_children;
    folded = allocation->height < nat_box_size;
  }

  hdy_leaflet_set_fold (self, folded ? HDY_FOLD_FOLDED : HDY_FOLD_UNFOLDED);

  /* Allocate size to the children. */
  if (folded)
    hdy_leaflet_size_allocate_folded (widget, allocation);
  else
    hdy_leaflet_size_allocate_unfolded (widget, allocation);

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
hdy_leaflet_draw_crossfade (GtkWidget *widget,
                            cairo_t   *cr)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  gdouble progress = gtk_progress_tracker_get_progress (&priv->child_transition.tracker, FALSE);

  cairo_push_group (cr);
  gtk_container_propagate_draw (GTK_CONTAINER (self),
                                priv->visible_child->widget,
                                cr);
  cairo_save (cr);

  /* Multiply alpha by progress */
  cairo_set_source_rgba (cr, 1, 1, 1, progress);
  cairo_set_operator (cr, CAIRO_OPERATOR_DEST_IN);
  cairo_paint (cr);

  if (priv->child_transition.last_visible_surface) {
    cairo_set_source_surface (cr, priv->child_transition.last_visible_surface,
                              priv->child_transition.last_visible_surface_allocation.x,
                              priv->child_transition.last_visible_surface_allocation.y);
    cairo_set_operator (cr, CAIRO_OPERATOR_ADD);
    cairo_paint_with_alpha (cr, MAX (1.0 - progress, 0));
  }

  cairo_restore (cr);

  cairo_pop_group_to_source (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_paint (cr);
}

static void
hdy_leaflet_draw_under (GtkWidget *widget,
                        cairo_t   *cr)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkAllocation allocation;
  int x, y;

  gtk_widget_get_allocation (widget, &allocation);

  x = get_bin_window_x (self, &allocation);
  y = get_bin_window_y (self, &allocation);

  if (gtk_cairo_should_draw_window (cr, priv->bin_window)) {
    cairo_save (cr);
    cairo_rectangle (cr, x, y, allocation.width, allocation.height);
    cairo_clip (cr);
    gtk_container_propagate_draw (GTK_CONTAINER (self),
                                  priv->visible_child->widget,
                                  cr);
    cairo_restore (cr);
  }

  if (priv->child_transition.last_visible_surface &&
      gtk_cairo_should_draw_window (cr, priv->view_window)) {
    switch (priv->child_transition.active_direction) {
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

    x += priv->child_transition.last_visible_surface_allocation.x;
    y += priv->child_transition.last_visible_surface_allocation.y;

    if (gtk_widget_get_valign (priv->last_visible_child->widget) == GTK_ALIGN_END &&
        priv->child_transition.last_visible_widget_height > allocation.height)
      y -= priv->child_transition.last_visible_widget_height - allocation.height;
    else if (gtk_widget_get_valign (priv->last_visible_child->widget) == GTK_ALIGN_CENTER)
      y -= (priv->child_transition.last_visible_widget_height - allocation.height) / 2;

    cairo_save (cr);
    cairo_set_source_surface (cr, priv->child_transition.last_visible_surface, x, y);
    cairo_paint (cr);
    cairo_restore (cr);
  }
}

static void
hdy_leaflet_draw_over (GtkWidget *widget,
                       cairo_t   *cr)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.last_visible_surface &&
      gtk_cairo_should_draw_window (cr, priv->view_window)) {
    GtkAllocation allocation;
    int x, y, clip_x, clip_y;

    gtk_widget_get_allocation (widget, &allocation);

    x = clip_x = get_bin_window_x (self, &allocation);
    y = clip_y = get_bin_window_y (self, &allocation);

    switch (priv->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_LEFT:
      x = 0;
      clip_x -= allocation.width;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      x = 0;
      clip_x += allocation.width;
      break;
    case GTK_PAN_DIRECTION_UP:
      y = 0;
      clip_y -= allocation.height;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      y = 0;
      clip_y += allocation.height;
      break;
    default:
      g_assert_not_reached ();
      break;
    }

    x += priv->child_transition.last_visible_surface_allocation.x;
    y += priv->child_transition.last_visible_surface_allocation.y;

    if (gtk_widget_get_valign (priv->last_visible_child->widget) == GTK_ALIGN_END &&
        priv->child_transition.last_visible_widget_height > allocation.height)
      y -= priv->child_transition.last_visible_widget_height - allocation.height;
    else if (gtk_widget_get_valign (priv->last_visible_child->widget) == GTK_ALIGN_CENTER)
      y -= (priv->child_transition.last_visible_widget_height - allocation.height) / 2;

    cairo_save (cr);
    cairo_rectangle (cr, clip_x, clip_y, allocation.width, allocation.height);
    cairo_clip (cr);
    cairo_set_source_surface (cr, priv->child_transition.last_visible_surface, x, y);
    cairo_paint (cr);
    cairo_restore (cr);
   }

  if (gtk_cairo_should_draw_window (cr, priv->bin_window))
    gtk_container_propagate_draw (GTK_CONTAINER (self),
                                  priv->visible_child->widget,
                                  cr);
}

static void
hdy_leaflet_draw_slide (GtkWidget *widget,
                        cairo_t   *cr)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  if (priv->child_transition.last_visible_surface &&
      gtk_cairo_should_draw_window (cr, priv->view_window)) {
    GtkAllocation allocation;
    int x, y;

    gtk_widget_get_allocation (widget, &allocation);

    x = get_bin_window_x (self, &allocation);
    y = get_bin_window_y (self, &allocation);

    switch (priv->child_transition.active_direction) {
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

    x += priv->child_transition.last_visible_surface_allocation.x;
    y += priv->child_transition.last_visible_surface_allocation.y;

    if (gtk_widget_get_valign (priv->last_visible_child->widget) == GTK_ALIGN_END &&
        priv->child_transition.last_visible_widget_height > allocation.height)
      y -= priv->child_transition.last_visible_widget_height - allocation.height;
    else if (gtk_widget_get_valign (priv->last_visible_child->widget) == GTK_ALIGN_CENTER)
      y -= (priv->child_transition.last_visible_widget_height - allocation.height) / 2;

    cairo_save (cr);
    cairo_set_source_surface (cr, priv->child_transition.last_visible_surface, x, y);
    cairo_paint (cr);
    cairo_restore (cr);
  }

  if (gtk_cairo_should_draw_window (cr, priv->bin_window))
    gtk_container_propagate_draw (GTK_CONTAINER (self),
                                  priv->visible_child->widget,
                                  cr);
}

static void
hdy_leaflet_draw_over_or_under (GtkWidget *widget,
                                cairo_t   *cr)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  gboolean is_rtl;
  GtkPanDirection direction, left_or_right, right_or_left;

  direction = priv->child_transition.active_direction;

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  left_or_right = is_rtl ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  right_or_left = is_rtl ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;

  switch (priv->child_transition.active_type) {
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_OVER:
    if (direction == GTK_PAN_DIRECTION_UP || direction == left_or_right)
      hdy_leaflet_draw_over (widget, cr);
    else if (direction == GTK_PAN_DIRECTION_DOWN || direction == right_or_left)
      hdy_leaflet_draw_under (widget, cr);
    else
      g_assert_not_reached ();
    break;
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_UNDER:
    if (direction == GTK_PAN_DIRECTION_UP || direction == left_or_right)
      hdy_leaflet_draw_under (widget, cr);
    else if (direction == GTK_PAN_DIRECTION_DOWN || direction == right_or_left)
      hdy_leaflet_draw_over (widget, cr);
    else
      g_assert_not_reached ();
    break;
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE:
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_CROSSFADE:
  case HDY_LEAFLET_CHILD_TRANSITION_TYPE_SLIDE:
  default:
    g_assert_not_reached ();
  }
}

static gboolean
hdy_leaflet_draw (GtkWidget *widget,
                  cairo_t   *cr)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GList *directed_children, *children;
  HdyLeafletChildInfo *child_info;
  GtkAllocation allocation;
  cairo_surface_t *subsurface;
  cairo_t *pattern_cr;

  if (priv->fold == HDY_FOLD_UNFOLDED)
    return GTK_WIDGET_CLASS (hdy_leaflet_parent_class)->draw (widget, cr);

  directed_children = get_directed_children (self);

  if (gtk_cairo_should_draw_window (cr, priv->view_window)) {
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (widget);
    gtk_render_background (context,
                           cr,
                           0, 0,
                           gtk_widget_get_allocated_width (widget),
                           gtk_widget_get_allocated_height (widget));
  }

  if (priv->visible_child) {
    if (gtk_progress_tracker_get_state (&priv->mode_transition.tracker) != GTK_PROGRESS_STATE_AFTER &&
        priv->fold == HDY_FOLD_FOLDED) {
      if (priv->mode_transition.start_surface == NULL &&
          priv->mode_transition.start_surface_allocation.width != 0 &&
          priv->mode_transition.start_surface_allocation.height != 0) {
        priv->mode_transition.start_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             priv->mode_transition.start_surface_allocation.width,
                                             priv->mode_transition.start_surface_allocation.height);

        for (children = directed_children; children; children = children->next) {
          child_info = children->data;

          if (child_info == priv->visible_child)
            break;

          if (!gtk_widget_get_child_visible (child_info->widget))
            continue;

          gtk_widget_get_allocation (child_info->widget, &allocation);
          subsurface = cairo_surface_create_for_rectangle (priv->mode_transition.start_surface,
                                                           allocation.x - priv->mode_transition.start_surface_allocation.x,
                                                           allocation.y - priv->mode_transition.start_surface_allocation.y,
                                                           allocation.width,
                                                           allocation.height);
          pattern_cr = cairo_create (subsurface);
          gtk_widget_draw (child_info->widget, pattern_cr);
          cairo_destroy (pattern_cr);
          cairo_surface_destroy (subsurface);
        }
      }

      if (priv->mode_transition.end_surface == NULL &&
          priv->mode_transition.end_surface_allocation.width != 0 &&
          priv->mode_transition.end_surface_allocation.height != 0) {
        priv->mode_transition.end_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             priv->mode_transition.end_surface_allocation.width,
                                             priv->mode_transition.end_surface_allocation.height);

        for (children = g_list_last (directed_children); children; children = children->prev) {
          child_info = children->data;

          if (child_info == priv->visible_child)
            break;

          if (!gtk_widget_get_child_visible (child_info->widget))
            continue;

          gtk_widget_get_allocation (child_info->widget, &allocation);
          subsurface = cairo_surface_create_for_rectangle (priv->mode_transition.end_surface,
                                                           allocation.x - priv->mode_transition.end_surface_allocation.x,
                                                           allocation.y - priv->mode_transition.end_surface_allocation.y,
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
      if (priv->mode_transition.start_surface != NULL) {
        cairo_set_source_surface (cr, priv->mode_transition.start_surface,
                                  priv->mode_transition.start_surface_allocation.x,
                                  priv->mode_transition.start_surface_allocation.y);
        cairo_paint (cr);
      }
      if (priv->mode_transition.end_surface != NULL) {
        cairo_set_source_surface (cr, priv->mode_transition.end_surface,
                                  priv->mode_transition.end_surface_allocation.x,
                                  priv->mode_transition.end_surface_allocation.y);
        cairo_paint (cr);
      }
      cairo_restore (cr);

      if (gtk_cairo_should_draw_window (cr, priv->bin_window))
        gtk_container_propagate_draw (GTK_CONTAINER (self),
                                      priv->visible_child->widget,
                                      cr);
    }
    else if (gtk_progress_tracker_get_state (&priv->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
      if (priv->child_transition.last_visible_surface == NULL &&
          priv->last_visible_child != NULL) {
        gtk_widget_get_allocation (priv->last_visible_child->widget,
                                   &priv->child_transition.last_visible_surface_allocation);
        priv->child_transition.last_visible_surface =
          gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                             CAIRO_CONTENT_COLOR_ALPHA,
                                             priv->child_transition.last_visible_surface_allocation.width,
                                             priv->child_transition.last_visible_surface_allocation.height);
        pattern_cr = cairo_create (priv->child_transition.last_visible_surface);
        /* We don't use propagate_draw here, because we don't want to apply
         * the bin_window offset
         */
        {
          /* FIXME Dirty workaround to get the last visible child to be drawn.
           * Please fix it properly.
           *  */
          gtk_widget_size_allocate (priv->last_visible_child->widget,
                                    &priv->child_transition.last_visible_surface_allocation);
          gtk_widget_set_child_visible (priv->last_visible_child->widget, TRUE);
        }
        gtk_widget_draw (priv->last_visible_child->widget, pattern_cr);
        cairo_destroy (pattern_cr);
      }

      cairo_rectangle (cr,
                       0, 0,
                       gtk_widget_get_allocated_width (widget),
                       gtk_widget_get_allocated_height (widget));
      cairo_clip (cr);

      switch (priv->child_transition.active_type) {
      case HDY_LEAFLET_CHILD_TRANSITION_TYPE_CROSSFADE:
        if (gtk_cairo_should_draw_window (cr, priv->bin_window))
          hdy_leaflet_draw_crossfade (widget, cr);
        break;
      case HDY_LEAFLET_CHILD_TRANSITION_TYPE_SLIDE:
        hdy_leaflet_draw_slide (widget, cr);
        break;
      case HDY_LEAFLET_CHILD_TRANSITION_TYPE_OVER:
      case HDY_LEAFLET_CHILD_TRANSITION_TYPE_UNDER:
        hdy_leaflet_draw_over_or_under (widget, cr);
        break;
      case HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE:
      default:
        g_assert_not_reached ();
      }
    }
    else if (gtk_cairo_should_draw_window (cr, priv->bin_window))
      gtk_container_propagate_draw (GTK_CONTAINER (self),
                                    priv->visible_child->widget,
                                    cr);
  }

  return FALSE;
}

static void
hdy_leaflet_child_visibility_notify_cb (GObject    *obj,
                                        GParamSpec *pspec,
                                        gpointer    user_data)
{
  HdyLeaflet *self = HDY_LEAFLET (user_data);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (obj);
  HdyLeafletChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);

  if (priv->visible_child == NULL && gtk_widget_get_visible (widget))
    set_visible_child_info (self, child_info, priv->child_transition.type, priv->child_transition.duration);
  else if (priv->visible_child == child_info && !gtk_widget_get_visible (widget))
    set_visible_child_info (self, NULL, priv->child_transition.type, priv->child_transition.duration);
}

static void
hdy_leaflet_add (GtkContainer *container,
                 GtkWidget    *widget)
{
  HdyLeaflet *self;
  HdyLeafletPrivate *priv;
  HdyLeafletChildInfo *child_info;

  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  self = HDY_LEAFLET (container);
  priv = hdy_leaflet_get_instance_private (self);

  gtk_widget_set_child_visible (widget, FALSE);
  gtk_widget_set_parent_window (widget, priv->bin_window);
  gtk_widget_set_parent (widget, GTK_WIDGET (self));

  child_info = g_new0 (HdyLeafletChildInfo, 1);
  child_info->widget = widget;

  priv->children = g_list_append (priv->children, child_info);
  priv->children_reversed = g_list_prepend (priv->children_reversed, child_info);

  if (priv->bin_window)
    gdk_window_set_events (priv->bin_window,
                           gdk_window_get_events (priv->bin_window) |
                           gtk_widget_get_events (widget));

  g_signal_connect (widget, "notify::visible",
                    G_CALLBACK (hdy_leaflet_child_visibility_notify_cb), self);

  if (hdy_leaflet_get_visible_child (self) == NULL &&
      gtk_widget_get_visible (widget))
    set_visible_child_info (self, child_info, priv->child_transition.type, priv->child_transition.duration);

  if (priv->fold == HDY_FOLD_UNFOLDED ||
      (priv->fold == HDY_FOLD_FOLDED && (priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] ||
                                         priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] ||
                                         priv->visible_child == child_info)))
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
hdy_leaflet_remove (GtkContainer *container,
                    GtkWidget    *widget)
{
  HdyLeaflet *self = HDY_LEAFLET (container);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  HdyLeafletChildInfo *child_info;
  gboolean contains_child;

  child_info = find_child_info_for_widget (self, widget);
  contains_child = child_info != NULL;

  g_return_if_fail (contains_child);

  priv->children = g_list_remove (priv->children, child_info);
  priv->children_reversed = g_list_remove (priv->children_reversed, child_info);
  free_child_info (child_info);

  if (hdy_leaflet_get_visible_child (self) == widget)
    set_visible_child_info (self, NULL, priv->child_transition.type, priv->child_transition.duration);

  if (gtk_widget_get_visible (widget))
    gtk_widget_queue_resize (GTK_WIDGET (container));

  gtk_widget_unparent (widget);
}

static void
hdy_leaflet_forall (GtkContainer *container,
                    gboolean      include_internals,
                    GtkCallback   callback,
                    gpointer      callback_data)
{
  HdyLeaflet *self = HDY_LEAFLET (container);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  /* This shallow copy is needed when the callback changes the list while we are
   * looping through it, for example by calling hdy_leaflet_remove() on all
   * children when destroying the HdyLeaflet_private_offset.
   */
  g_autoptr (GList) children_copy = g_list_copy (priv->children);
  GList *children;
  HdyLeafletChildInfo *child_info;

  for (children = children_copy; children; children = children->next) {
    child_info = children->data;

    (* callback) (child_info->widget, callback_data);
  }

  g_list_free (priv->children_reversed);
  priv->children_reversed = g_list_copy (priv->children);
  priv->children_reversed = g_list_reverse (priv->children_reversed);
}

static void
hdy_leaflet_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  HdyLeaflet *self = HDY_LEAFLET (object);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  switch (prop_id) {
  case PROP_FOLD:
    g_value_set_enum (value, hdy_leaflet_get_fold (self));
    break;
  case PROP_FOLDED:
    g_value_set_boolean (value, hdy_leaflet_get_fold (self) == HDY_FOLD_FOLDED);
    break;
  case PROP_HHOMOGENEOUS_FOLDED:
    g_value_set_boolean (value, hdy_leaflet_get_homogeneous (self, TRUE, GTK_ORIENTATION_HORIZONTAL));
    break;
  case PROP_VHOMOGENEOUS_FOLDED:
    g_value_set_boolean (value, hdy_leaflet_get_homogeneous (self, TRUE, GTK_ORIENTATION_VERTICAL));
    break;
  case PROP_HHOMOGENEOUS_UNFOLDED:
    g_value_set_boolean (value, hdy_leaflet_get_homogeneous (self, FALSE, GTK_ORIENTATION_HORIZONTAL));
    break;
  case PROP_VHOMOGENEOUS_UNFOLDED:
    g_value_set_boolean (value, hdy_leaflet_get_homogeneous (self, FALSE, GTK_ORIENTATION_VERTICAL));
    break;
  case PROP_VISIBLE_CHILD:
    g_value_set_object (value, hdy_leaflet_get_visible_child (self));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    g_value_set_string (value, hdy_leaflet_get_visible_child_name (self));
    break;
  case PROP_MODE_TRANSITION_TYPE:
    g_value_set_enum (value, hdy_leaflet_get_mode_transition_type (self));
    break;
  case PROP_MODE_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_leaflet_get_mode_transition_duration (self));
    break;
  case PROP_CHILD_TRANSITION_TYPE:
    g_value_set_enum (value, hdy_leaflet_get_child_transition_type (self));
    break;
  case PROP_CHILD_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_leaflet_get_child_transition_duration (self));
    break;
  case PROP_CHILD_TRANSITION_RUNNING:
    g_value_set_boolean (value, hdy_leaflet_get_child_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, hdy_leaflet_get_interpolate_size (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, priv->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_leaflet_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  HdyLeaflet *self = HDY_LEAFLET (object);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  switch (prop_id) {
  case PROP_HHOMOGENEOUS_FOLDED:
    hdy_leaflet_set_homogeneous (self, TRUE, GTK_ORIENTATION_HORIZONTAL, g_value_get_boolean (value));
    break;
  case PROP_VHOMOGENEOUS_FOLDED:
    hdy_leaflet_set_homogeneous (self, TRUE, GTK_ORIENTATION_VERTICAL, g_value_get_boolean (value));
    break;
  case PROP_HHOMOGENEOUS_UNFOLDED:
    hdy_leaflet_set_homogeneous (self, FALSE, GTK_ORIENTATION_HORIZONTAL, g_value_get_boolean (value));
    break;
  case PROP_VHOMOGENEOUS_UNFOLDED:
    hdy_leaflet_set_homogeneous (self, FALSE, GTK_ORIENTATION_VERTICAL, g_value_get_boolean (value));
    break;
  case PROP_VISIBLE_CHILD:
    hdy_leaflet_set_visible_child (self, g_value_get_object (value));
    break;
  case PROP_VISIBLE_CHILD_NAME:
    hdy_leaflet_set_visible_child_name (self, g_value_get_string (value));
    break;
  case PROP_MODE_TRANSITION_TYPE:
    hdy_leaflet_set_mode_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_MODE_TRANSITION_DURATION:
    hdy_leaflet_set_mode_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_CHILD_TRANSITION_TYPE:
    hdy_leaflet_set_child_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_CHILD_TRANSITION_DURATION:
    hdy_leaflet_set_child_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    hdy_leaflet_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  case PROP_ORIENTATION:
    {
      GtkOrientation orientation = g_value_get_enum (value);
      if (priv->orientation != orientation) {
        priv->orientation = orientation;
        gtk_widget_queue_resize (GTK_WIDGET (self));
        g_object_notify (object, "orientation");
      }
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_leaflet_dispose (GObject *object)
{
  HdyLeaflet *self = HDY_LEAFLET (object);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  priv->visible_child = NULL;

  G_OBJECT_CLASS (hdy_leaflet_parent_class)->dispose (object);
}

static void
hdy_leaflet_finalize (GObject *object)
{
  HdyLeaflet *self = HDY_LEAFLET (object);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  hdy_leaflet_unschedule_child_ticks (self);

  if (priv->child_transition.last_visible_surface != NULL)
    cairo_surface_destroy (priv->child_transition.last_visible_surface);

  G_OBJECT_CLASS (hdy_leaflet_parent_class)->finalize (object);
}

static void
hdy_leaflet_get_child_property (GtkContainer *container,
                                GtkWidget    *widget,
                                guint         property_id,
                                GValue       *value,
                                GParamSpec   *pspec)
{
  HdyLeaflet *self = HDY_LEAFLET (container);
  HdyLeafletChildInfo *child_info;

  child_info = find_child_info_for_widget (self, widget);
  if (child_info == NULL) {
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    return;
  }

  switch (property_id) {
  case CHILD_PROP_NAME:
    g_value_set_string (value, child_info->name);
    break;

  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_leaflet_set_child_property (GtkContainer *container,
                                GtkWidget    *widget,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdyLeaflet *self = HDY_LEAFLET (container);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  HdyLeafletChildInfo *child_info;
  HdyLeafletChildInfo *child_info2;
  gchar *name;
  GList *children;

  child_info = find_child_info_for_widget (self, widget);
  if (child_info == NULL) {
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    return;
  }

  switch (property_id) {
  case CHILD_PROP_NAME:
    name = g_value_dup_string (value);
    for (children = priv->children; children; children = children->next) {
      child_info2 = children->data;

      if (child_info == child_info2)
        continue;
      if (g_strcmp0 (child_info2->name, name) == 0) {
        g_warning ("Duplicate child name in HdyLeaflet: %s", name);

        break;
      }
    }

    g_free (child_info->name);
    child_info->name = name;

    gtk_container_child_notify_by_pspec (container, widget, pspec);

    if (priv->visible_child == child_info)
      g_object_notify_by_pspec (G_OBJECT (self),
                                props[PROP_VISIBLE_CHILD_NAME]);

    break;

  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_leaflet_realize (GtkWidget *widget)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);
  GtkAllocation allocation;
  GdkWindowAttr attributes = { 0 };
  GdkWindowAttributesType attributes_mask;
  GList *children;
  HdyLeafletChildInfo *child_info;
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

  priv->view_window = gdk_window_new (gtk_widget_get_window (widget),
                                      &attributes, attributes_mask);
  gtk_widget_register_window (widget, priv->view_window);

  get_padding (widget, &padding);
  attributes.x = padding.left;
  attributes.y = padding.top;
  attributes.width = allocation.width;
  attributes.height = allocation.height;

  for (children = priv->children; children != NULL; children = children->next) {
    child_info = children->data;
    attributes.event_mask |= gtk_widget_get_events (child_info->widget);
  }

  priv->bin_window = gdk_window_new (priv->view_window, &attributes, attributes_mask);
  gtk_widget_register_window (widget, priv->bin_window);

  for (children = priv->children; children != NULL; children = children->next) {
    child_info = children->data;

    gtk_widget_set_parent_window (child_info->widget, priv->bin_window);
  }

  gdk_window_show (priv->bin_window);
}

static void
hdy_leaflet_unrealize (GtkWidget *widget)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  gtk_widget_unregister_window (widget, priv->bin_window);
  gdk_window_destroy (priv->bin_window);
  priv->bin_window = NULL;
  gtk_widget_unregister_window (widget, priv->view_window);
  gdk_window_destroy (priv->view_window);
  priv->view_window = NULL;

  GTK_WIDGET_CLASS (hdy_leaflet_parent_class)->unrealize (widget);
}

static void
hdy_leaflet_map (GtkWidget *widget)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  GTK_WIDGET_CLASS (hdy_leaflet_parent_class)->map (widget);

  gdk_window_show (priv->view_window);
}

static void
hdy_leaflet_unmap (GtkWidget *widget)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  gdk_window_hide (priv->view_window);

  GTK_WIDGET_CLASS (hdy_leaflet_parent_class)->unmap (widget);
}

static void
hdy_leaflet_class_init (HdyLeafletClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;
  GtkContainerClass *container_class = (GtkContainerClass*) klass;

  object_class->get_property = hdy_leaflet_get_property;
  object_class->set_property = hdy_leaflet_set_property;
  object_class->dispose = hdy_leaflet_dispose;
  object_class->finalize = hdy_leaflet_finalize;

  widget_class->realize = hdy_leaflet_realize;
  widget_class->unrealize = hdy_leaflet_unrealize;
  widget_class->map = hdy_leaflet_map;
  widget_class->unmap = hdy_leaflet_unmap;
  widget_class->get_preferred_width = hdy_leaflet_get_preferred_width;
  widget_class->get_preferred_height = hdy_leaflet_get_preferred_height;
  widget_class->get_preferred_width_for_height = hdy_leaflet_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = hdy_leaflet_get_preferred_height_for_width;
  widget_class->size_allocate = hdy_leaflet_size_allocate;
  widget_class->draw = hdy_leaflet_draw;

  container_class->add = hdy_leaflet_add;
  container_class->remove = hdy_leaflet_remove;
  container_class->forall = hdy_leaflet_forall;
  container_class->set_child_property = hdy_leaflet_set_child_property;
  container_class->get_child_property = hdy_leaflet_get_child_property;
  gtk_container_class_handle_border_width (container_class);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * HdyLeaflet:fold:
   *
   * The fold of the leaflet.
   *
   * The leaflet will be folded if the size allocated to it is smaller than the
   * sum of the natural size of its children, it will be unfolded otherwise.
   *
   * See also: #HdyLeaflet:folded.
   */
  props[PROP_FOLD] =
    g_param_spec_enum ("fold",
                       _("Fold"),
                       _("Whether the widget is folded"),
                       HDY_TYPE_FOLD, HDY_FOLD_UNFOLDED,
                       G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyLeaflet:folded:
   *
   * %TRUE if the leaflet is folded.
   *
   * This is similar to the #HdyLeaflet:fold property but expressed as a
   * #gboolean rather than a #GEnum. This makes it convenient to bind the
   * #HdyLeaflet:fold of a leaflet to any other #gboolean property of other
   * #GObject's using #g_object_bind_property().
   */
  props[PROP_FOLDED] =
    g_param_spec_boolean ("folded",
                          _("Folded"),
                          _("Whether the widget is folded"),
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyLeaflet:hhomogeneous_folded:
   *
   * %TRUE if the leaflet allocates the same width for all children when folded.
   */
  props[PROP_HHOMOGENEOUS_FOLDED] =
    g_param_spec_boolean ("hhomogeneous-folded",
                          _("Horizontally homogeneous folded"),
                          _("Horizontally homogeneous sizing when the leaflet is folded"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyLeaflet:vhomogeneous_folded:
   *
   * %TRUE if the leaflet allocates the same height for all children when folded.
   */
  props[PROP_VHOMOGENEOUS_FOLDED] =
    g_param_spec_boolean ("vhomogeneous-folded",
                          _("Vertically homogeneous folded"),
                          _("Vertically homogeneous sizing when the leaflet is folded"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyLeaflet:hhomogeneous_unfolded:
   *
   * %TRUE if the leaflet allocates the same width for all children when unfolded.
   */
  props[PROP_HHOMOGENEOUS_UNFOLDED] =
    g_param_spec_boolean ("hhomogeneous-unfolded",
                          _("Box horizontally homogeneous"),
                          _("Horizontally homogeneous sizing when the leaflet is unfolded"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyLeaflet:vhomogeneous_unfolded:
   *
   * %TRUE if the leaflet allocates the same height for all children when unfolded.
   */
  props[PROP_VHOMOGENEOUS_UNFOLDED] =
    g_param_spec_boolean ("vhomogeneous-unfolded",
                          _("Box vertically homogeneous"),
                          _("Vertically homogeneous sizing when the leaflet is unfolded"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_VISIBLE_CHILD] =
    g_param_spec_object ("visible-child",
                         _("Visible child"),
                         _("The widget currently visible when the leaflet is folded"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_VISIBLE_CHILD_NAME] =
    g_param_spec_string ("visible-child-name",
                         _("Name of visible child"),
                         _("The name of the widget currently visible when the children are stacked"),
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_MODE_TRANSITION_TYPE] =
    g_param_spec_enum ("mode-transition-type",
                       _("Mode transition type"),
                       _("The type of animation used to transition between modes"),
                       HDY_TYPE_LEAFLET_MODE_TRANSITION_TYPE, HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_MODE_TRANSITION_DURATION] =
    g_param_spec_uint ("mode-transition-duration",
                       _("Mode transition duration"),
                       _("The mode transition animation duration, in milliseconds"),
                       0, G_MAXUINT, 250,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CHILD_TRANSITION_TYPE] =
    g_param_spec_enum ("child-transition-type",
                       _("Child transition type"),
                       _("The type of animation used to transition between children"),
                       HDY_TYPE_LEAFLET_CHILD_TRANSITION_TYPE, HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE,
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

  g_object_class_install_properties (object_class, LAST_PROP, props);

  child_props[CHILD_PROP_NAME] =
    g_param_spec_string ("name",
                         _("Name"),
                         _("The name of the child page"),
                         NULL,
                         G_PARAM_READWRITE);

  gtk_container_class_install_child_properties (container_class, LAST_CHILD_PROP, child_props);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_PANEL);
  gtk_widget_class_set_css_name (widget_class, "hdyleaflet");
}

GtkWidget *
hdy_leaflet_new (void)
{
  return g_object_new (HDY_TYPE_LEAFLET, NULL);
}

static void
hdy_leaflet_init (HdyLeaflet *self)
{
  GtkWidget *widget = GTK_WIDGET (self);
  HdyLeafletPrivate *priv = hdy_leaflet_get_instance_private (self);

  priv->children = NULL;
  priv->children_reversed = NULL;
  priv->visible_child = NULL;
  priv->fold = HDY_FOLD_UNFOLDED;
  priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] = FALSE;
  priv->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] = FALSE;
  priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] = TRUE;
  priv->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] = TRUE;
  priv->mode_transition.type = HDY_LEAFLET_MODE_TRANSITION_TYPE_NONE;
  priv->mode_transition.duration = 250;
  priv->child_transition.type = HDY_LEAFLET_CHILD_TRANSITION_TYPE_NONE;
  priv->child_transition.duration = 200;
  priv->mode_transition.current_pos = 1.0;
  priv->mode_transition.target_pos = 1.0;

  gtk_widget_set_has_window (widget, FALSE);
  gtk_widget_set_can_focus (widget, FALSE);
  gtk_widget_set_redraw_on_allocate (widget, FALSE);
}

static void
hdy_leaflet_buildable_init (GtkBuildableIface *iface)
{
}

