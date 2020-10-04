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
#include "hdy-enums-private.h"
#include "hdy-leaflet.h"
#include "hdy-shadow-helper-private.h"
#include "hdy-swipeable.h"
#include "hdy-swipe-tracker-private.h"

/**
 * SECTION:hdy-leaflet
 * @short_description: An adaptive container acting like a box or a stack.
 * @Title: HdyLeaflet
 *
 * The #HdyLeaflet widget can display its children like a #GtkBox does or
 * like a #GtkStack does, adapting to size changes by switching between
 * the two modes.
 *
 * When there is enough space the children are displayed side by side, otherwise
 * only one is displayed and the leaflet is said to be “folded”.
 * The threshold is dictated by the preferred minimum sizes of the children.
 * When a leaflet is folded, the children can be navigated using swipe gestures.
 *
 * The “over” and “under” stack the children one on top of the other, while the
 * “slide” transition puts the children side by side. While navigating to a
 * child on the side or below can be performed by swiping the current child
 * away, navigating to an upper child requires dragging it from the edge where
 * it resides. This doesn't affect non-dragging swipes.
 *
 * The “over” and “under” transitions can draw their shadow on top of the
 * window's transparent areas, like the rounded corners. This is a side-effect
 * of allowing shadows to be drawn on top of OpenGL areas. It can be mitigated
 * by using #HdyWindow or #HdyApplicationWindow as they will crop anything drawn
 * beyond the rounded corners.
 *
 * # CSS nodes
 *
 * #HdyLeaflet has a single CSS node with name leaflet. The node will get the
 * style classes .folded when it is folded, .unfolded when it's not, or none if
 * it didn't compute its fold yet.
 */

/**
 * HdyLeafletTransitionType:
 * @HDY_LEAFLET_TRANSITION_TYPE_OVER: Cover the old page or uncover the new page, sliding from or towards the end according to orientation, text direction and children order
 * @HDY_LEAFLET_TRANSITION_TYPE_UNDER: Uncover the new page or cover the old page, sliding from or towards the start according to orientation, text direction and children order
 * @HDY_LEAFLET_TRANSITION_TYPE_SLIDE: Slide from left, right, up or down according to the orientation, text direction and the children order
 *
 * This enumeration value describes the possible transitions between modes and
 * children in a #HdyLeaflet widget.
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 0.0.12
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
  PROP_CAN_UNFOLD,
  PROP_PAGES,

  /* orientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ORIENTATION,
};

#define HDY_FOLD_UNFOLDED FALSE
#define HDY_FOLD_FOLDED TRUE
#define HDY_FOLD_MAX 2
#define GTK_ORIENTATION_MAX 2
#define HDY_SWIPE_BORDER 32

struct _HdyLeafletPage {
  GObject parent_instance;

  GtkWidget *widget;
  gchar *name;
  gboolean navigatable;

  /* Convenience storage for per-child temporary frequently computed values. */
  GtkAllocation alloc;
  GtkRequisition min;
  GtkRequisition nat;
  gboolean visible;
  GtkWidget *last_focus;
};

G_DEFINE_TYPE (HdyLeafletPage, hdy_leaflet_page, G_TYPE_OBJECT)

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_NAME,
  PAGE_PROP_NAVIGATABLE,
  LAST_PAGE_PROP
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _HdyLeaflet {
  GtkWidget parent_instance;

  GList *children;
  /* It is probably cheaper to store and maintain a reversed copy of the
   * children list that to reverse the list every time we need to allocate or
   * draw children for RTL languages on a horizontal widget.
   */
  GList *children_reversed;
  HdyLeafletPage *visible_child;
  HdyLeafletPage *last_visible_child;

  gboolean folded;

  gboolean homogeneous[HDY_FOLD_MAX][GTK_ORIENTATION_MAX];

  GtkOrientation orientation;

  HdyLeafletTransitionType transition_type;

  HdySwipeTracker *tracker;

  struct {
    guint duration;

    gdouble current_pos;
    gdouble source_pos;
    gdouble target_pos;

    gdouble start_progress;
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

    guint tick_id;
    GtkProgressTracker tracker;
    gboolean first_frame_skipped;

    gint last_visible_widget_width;
    gint last_visible_widget_height;

    gboolean interpolate_size;
    gboolean can_swipe_back;
    gboolean can_swipe_forward;

    GtkPanDirection active_direction;
    gboolean is_direct_swipe;
    gint swipe_direction;
  } child_transition;

  HdyShadowHelper *shadow_helper;
  gboolean can_unfold;

  GtkSelectionModel *pages;
};

static GParamSpec *props[LAST_PROP];

static gint HOMOGENEOUS_PROP[HDY_FOLD_MAX][GTK_ORIENTATION_MAX] = {
  { PROP_HHOMOGENEOUS_UNFOLDED, PROP_VHOMOGENEOUS_UNFOLDED},
  { PROP_HHOMOGENEOUS_FOLDED, PROP_VHOMOGENEOUS_FOLDED},
};

static void hdy_leaflet_buildable_init (GtkBuildableIface *iface);
static void hdy_leaflet_swipeable_init (HdySwipeableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyLeaflet, hdy_leaflet, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, hdy_leaflet_buildable_init)
                         G_IMPLEMENT_INTERFACE (HDY_TYPE_SWIPEABLE, hdy_leaflet_swipeable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
hdy_leaflet_page_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  HdyLeafletPage *self = HDY_LEAFLET_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, hdy_leaflet_page_get_child (self));
    break;
  case PAGE_PROP_NAME:
    g_value_set_string (value, hdy_leaflet_page_get_name (self));
    break;
  case PAGE_PROP_NAVIGATABLE:
    g_value_set_boolean (value, hdy_leaflet_page_get_navigatable (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_leaflet_page_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HdyLeafletPage *self = HDY_LEAFLET_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_set_object (&self->widget, g_value_get_object (value));
    break;
  case PAGE_PROP_NAME:
    hdy_leaflet_page_set_name (self, g_value_get_string (value));
    break;
  case PAGE_PROP_NAVIGATABLE:
    hdy_leaflet_page_set_navigatable (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_leaflet_page_finalize (GObject *object)
{
  HdyLeafletPage *self = HDY_LEAFLET_PAGE (object);

  g_clear_object (&self->widget);
  g_clear_pointer (&self->name, g_free);

  if (self->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                  (gpointer *) &self->last_focus);

  G_OBJECT_CLASS (hdy_leaflet_page_parent_class)->finalize (object);
}

static void
hdy_leaflet_page_class_init (HdyLeafletPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = hdy_leaflet_page_get_property;
  object_class->set_property = hdy_leaflet_page_set_property;
  object_class->finalize = hdy_leaflet_page_finalize;

  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child",
                         _("Child"),
                         _("The child of the page"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  page_props[PAGE_PROP_NAME] =
    g_param_spec_string ("name",
                         _("Name"),
                         _("The name of the child page"),
                         NULL,
                         G_PARAM_READWRITE);

  /**
   * HdyLeafletPage:navigatable:
   *
   * Whether the child can be navigated to when folded.
   * If %FALSE, the child will be ignored by hdy_leaflet_get_adjacent_child(),
   * hdy_leaflet_navigate(), and swipe gestures.
   *
   * This can be used used to prevent switching to widgets like separators.
   *
   * Since: 1.0
   */
  page_props[PAGE_PROP_NAVIGATABLE] =
    g_param_spec_boolean ("navigatable",
                          _("Navigatable"),
                          _("Whether the child can be navigated to"),
                          TRUE,
                          G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);
}

static void
hdy_leaflet_page_init (HdyLeafletPage *self)
{
  self->navigatable = TRUE;
}

#define HDY_TYPE_LEAFLET_PAGES (hdy_leaflet_pages_get_type ())

G_DECLARE_FINAL_TYPE (HdyLeafletPages, hdy_leaflet_pages, HDY, LEAFLET_PAGES, GObject)

struct _HdyLeafletPages
{
  GObject parent_instance;
  HdyLeaflet *leaflet;
};

static GType
hdy_leaflet_pages_get_item_type (GListModel *model)
{
  return HDY_TYPE_LEAFLET_PAGE;
}

static guint
hdy_leaflet_pages_get_n_items (GListModel *model)
{
  HdyLeafletPages *self = HDY_LEAFLET_PAGES (model);

  return g_list_length (self->leaflet->children);
}

static gpointer
hdy_leaflet_pages_get_item (GListModel *model,
                             guint       position)
{
  HdyLeafletPages *self = HDY_LEAFLET_PAGES (model);
  HdyLeafletPage *page;

  page = g_list_nth_data (self->leaflet->children, position);

  return g_object_ref (page);
}

static void
hdy_leaflet_pages_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = hdy_leaflet_pages_get_item_type;
  iface->get_n_items = hdy_leaflet_pages_get_n_items;
  iface->get_item = hdy_leaflet_pages_get_item;
}

static gboolean
hdy_leaflet_pages_is_selected (GtkSelectionModel *model,
                               guint              position)
{
  HdyLeafletPages *self = HDY_LEAFLET_PAGES (model);
  HdyLeafletPage *page;

  page = g_list_nth_data (self->leaflet->children, position);

  return page == self->leaflet->visible_child;
}

static void
hdy_leaflet_pages_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = hdy_leaflet_pages_is_selected;
}

G_DEFINE_TYPE_WITH_CODE (HdyLeafletPages, hdy_leaflet_pages, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, hdy_leaflet_pages_list_model_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, hdy_leaflet_pages_selection_model_init))

static void
hdy_leaflet_pages_init (HdyLeafletPages *pages)
{
}

static void
hdy_leaflet_pages_class_init (HdyLeafletPagesClass *klass)
{
}

static HdyLeafletPages *
hdy_leaflet_pages_new (HdyLeaflet *leaflet)
{
  HdyLeafletPages *pages;

  pages = g_object_new (HDY_TYPE_LEAFLET_PAGES, NULL);
  pages->leaflet = leaflet;

  return pages;
}

static HdyLeafletPage *
find_page_for_widget (HdyLeaflet *self,
                      GtkWidget  *widget)
{
  HdyLeafletPage *page;
  GList *l;

  for (l = self->children; l; l = l->next) {
    page = l->data;

    if (page->widget == widget)
      return page;
  }

  return NULL;
}

static HdyLeafletPage *
find_page_for_name (HdyLeaflet  *self,
                    const gchar *name)
{
  HdyLeafletPage *page;
  GList *l;

  for (l = self->children; l; l = l->next) {
    page = l->data;

    if (g_strcmp0 (page->name, name) == 0)
      return page;
  }

  return NULL;
}

static HdyLeafletPage *
find_swipeable_page (HdyLeaflet             *self,
                     HdyNavigationDirection  direction)
{
  HdyLeafletPage *page = NULL;
  GList *l;

  l = g_list_find (self->children, self->visible_child);
  do {
    l = (direction == HDY_NAVIGATION_DIRECTION_BACK) ? l->prev : l->next;

    if (!l)
      break;

    page = l->data;
  } while (page && !page->navigatable);

  return page;
}

static GList *
get_directed_children (HdyLeaflet *self)
{
  return self->orientation == GTK_ORIENTATION_HORIZONTAL &&
         gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL ?
         self->children_reversed : self->children;
}

static GtkPanDirection
get_pan_direction (HdyLeaflet *self,
                   gboolean    new_child_first)
{
  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      return new_child_first ? GTK_PAN_DIRECTION_LEFT : GTK_PAN_DIRECTION_RIGHT;
    else
      return new_child_first ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;
  }
  else
    return new_child_first ? GTK_PAN_DIRECTION_DOWN : GTK_PAN_DIRECTION_UP;
}

static gint
get_child_window_x (HdyLeaflet     *self,
                    HdyLeafletPage *page,
                    gint            width)
{
  gboolean is_rtl;
  gint rtl_multiplier;

  if (!self->child_transition.is_gesture_active &&
      gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER)
    return 0;

  if (self->child_transition.active_direction != GTK_PAN_DIRECTION_LEFT &&
      self->child_transition.active_direction != GTK_PAN_DIRECTION_RIGHT)
    return 0;

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  rtl_multiplier = is_rtl ? -1 : 1;

  if ((self->child_transition.active_direction == GTK_PAN_DIRECTION_RIGHT) == is_rtl) {
    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->visible_child)
      return width * (1 - self->child_transition.progress) * rtl_multiplier;

    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->last_visible_child)
      return -width * self->child_transition.progress * rtl_multiplier;
  } else {
    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->visible_child)
      return -width * (1 - self->child_transition.progress) * rtl_multiplier;

    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->last_visible_child)
      return width * self->child_transition.progress * rtl_multiplier;
  }

  return 0;
}

static gint
get_child_window_y (HdyLeaflet     *self,
                    HdyLeafletPage *page,
                    gint            height)
{
  if (!self->child_transition.is_gesture_active &&
      gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER)
    return 0;

  if (self->child_transition.active_direction != GTK_PAN_DIRECTION_UP &&
      self->child_transition.active_direction != GTK_PAN_DIRECTION_DOWN)
    return 0;

  if (self->child_transition.active_direction == GTK_PAN_DIRECTION_UP) {
    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->visible_child)
      return height * (1 - self->child_transition.progress);

    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->last_visible_child)
      return -height * self->child_transition.progress;
  } else {
    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->visible_child)
      return -height * (1 - self->child_transition.progress);

    if ((self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER ||
         self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE) &&
        page == self->last_visible_child)
      return height * self->child_transition.progress;
  }

  return 0;
}

static void
child_progress_updated (HdyLeaflet *self)
{
  gtk_widget_queue_draw (GTK_WIDGET (self));

  if (!self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] ||
      !self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL])
    gtk_widget_queue_resize (GTK_WIDGET (self));
  else
    gtk_widget_queue_allocate (GTK_WIDGET (self));

  if (!self->child_transition.is_gesture_active &&
      gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (self->child_transition.is_cancelled) {
      if (self->last_visible_child != NULL) {
        if (self->folded) {
          gtk_widget_set_child_visible (self->last_visible_child->widget, TRUE);
          gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
        }
        self->visible_child = self->last_visible_child;
        self->last_visible_child = NULL;
      }

      self->child_transition.is_cancelled = FALSE;

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

    gtk_widget_queue_allocate (GTK_WIDGET (self));
    self->child_transition.swipe_direction = 0;
//    hdy_shadow_helper_clear_cache (self->shadow_helper);
  }
}

static gboolean
child_transition_cb (GtkWidget     *widget,
                     GdkFrameClock *frame_clock,
                     gpointer       user_data)
{
  HdyLeaflet *self = HDY_LEAFLET (user_data);
  gdouble progress;

  if (self->child_transition.first_frame_skipped) {
    gtk_progress_tracker_advance_frame (&self->child_transition.tracker,
                                        gdk_frame_clock_get_frame_time (frame_clock));
    progress = gtk_progress_tracker_get_ease_out_cubic (&self->child_transition.tracker, FALSE);
    self->child_transition.progress =
      hdy_lerp (self->child_transition.start_progress,
                self->child_transition.end_progress, progress);
  } else
    self->child_transition.first_frame_skipped = TRUE;

  /* Finish animation early if not mapped anymore */
  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&self->child_transition.tracker);

  child_progress_updated (self);

  if (gtk_progress_tracker_get_state (&self->child_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    self->child_transition.tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);

    return FALSE;
  }

  return TRUE;
}

static void
schedule_child_ticks (HdyLeaflet *self)
{
  if (self->child_transition.tick_id == 0) {
    self->child_transition.tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                    child_transition_cb,
                                    self, NULL);
    if (!self->child_transition.is_gesture_active)
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
  }
}

static void
unschedule_child_ticks (HdyLeaflet *self)
{
  if (self->child_transition.tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->child_transition.tick_id);
    self->child_transition.tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
  }
}

static void
stop_child_transition (HdyLeaflet *self)
{
  unschedule_child_ticks (self);
  gtk_progress_tracker_finish (&self->child_transition.tracker);
  if (self->last_visible_child != NULL) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }

  self->child_transition.swipe_direction = 0;
//  hdy_shadow_helper_clear_cache (self->shadow_helper);
}

static void
start_child_transition (HdyLeaflet      *self,
                        guint            transition_duration,
                        GtkPanDirection  transition_direction)
{
  GtkWidget *widget = GTK_WIDGET (self);

  if (gtk_widget_get_mapped (widget) &&
      ((hdy_get_enable_animations (widget) &&
        transition_duration != 0) ||
       self->child_transition.is_gesture_active) &&
      self->last_visible_child != NULL &&
      /* Don't animate child transition when a mode transition is ongoing. */
      self->mode_transition.tick_id == 0) {
    self->child_transition.active_direction = transition_direction;
    self->child_transition.first_frame_skipped = FALSE;
    self->child_transition.start_progress = 0;
    self->child_transition.end_progress = 1;
    self->child_transition.progress = 0;
    self->child_transition.is_cancelled = FALSE;

    if (!self->child_transition.is_gesture_active) {
      schedule_child_ticks (self);
      gtk_progress_tracker_start (&self->child_transition.tracker,
                                  transition_duration * 1000,
                                  0,
                                  1.0);
    }
  }
  else {
    unschedule_child_ticks (self);
    gtk_progress_tracker_finish (&self->child_transition.tracker);
  }

  child_progress_updated (self);
}

static void
set_visible_child (HdyLeaflet               *self,
                   HdyLeafletPage           *page,
                   HdyLeafletTransitionType  transition_type,
                   guint                     transition_duration,
                   gboolean                  emit_child_switched)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GtkRoot *root;
  GtkWidget *focus;
  gboolean contains_focus = FALSE;
  GtkPanDirection transition_direction = GTK_PAN_DIRECTION_LEFT;
  guint old_pos = GTK_INVALID_LIST_POSITION;
  guint new_pos = GTK_INVALID_LIST_POSITION;

  /* If we are being destroyed, do not bother with transitions and
   * notifications.
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick first visible. */
  if (!page) {
    GList *l;

    for (l = self->children; l; l = l->next) {
      HdyLeafletPage *p = l->data;

      if (gtk_widget_get_visible (p->widget)) {
        page = p;

        break;
      }
    }
  }

  if (page == self->visible_child)
    return;

  if (self->pages) {
    guint position;
    GList *l;

    for (l = self->children, position = 0; l; l = l->next, position++) {
      HdyLeafletPage *p = l->data;
      if (p == self->visible_child)
        old_pos = position;
      else if (p == page)
        new_pos = position;
    }
  }

  root = gtk_widget_get_root (widget);
  if (root)
    focus = gtk_root_get_focus (root);
  else
    focus = NULL;

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

  if (self->last_visible_child)
    gtk_widget_set_child_visible (self->last_visible_child->widget, !self->folded);
  self->last_visible_child = NULL;

  if (self->visible_child && self->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      self->last_visible_child = self->visible_child;
      self->child_transition.last_visible_widget_width = gtk_widget_get_width (self->last_visible_child->widget);
      self->child_transition.last_visible_widget_height = gtk_widget_get_height (self->last_visible_child->widget);
    } else {
      gtk_widget_set_child_visible (self->visible_child->widget, !self->folded);
    }
  }

  self->visible_child = page;

  if (page) {
    gtk_widget_set_child_visible (page->widget, TRUE);

    if (contains_focus) {
      if (page->last_focus)
        gtk_widget_grab_focus (page->last_focus);
      else
        gtk_widget_child_focus (page->widget, GTK_DIR_TAB_FORWARD);
    }
  }

  if (page == NULL || self->last_visible_child == NULL)
    transition_duration = 0;
  else {
    gboolean new_first = FALSE;
    GList *l;

    for (l = self->children; l; l = l->next) {
      if (page == l->data) {
        new_first = TRUE;

        break;
      }
      if (self->last_visible_child == l->data)
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

    start_child_transition (self, transition_duration, transition_direction);
  }

  if (emit_child_switched) {
    gint index = 0;
    GList *l;

    for (l = self->children; l; l = l->next) {
      HdyLeafletPage *p = l->data;

      if (!p->navigatable)
        continue;

      if (p == page)
        break;

      index++;
    }

    hdy_swipeable_emit_child_switched (HDY_SWIPEABLE (self), index,
                                       transition_duration);
  }

  if (self->pages) {
    if (old_pos == GTK_INVALID_LIST_POSITION && new_pos == GTK_INVALID_LIST_POSITION)
      ; /* nothing to do */
    else if (old_pos == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, new_pos, 1);
    else if (new_pos == GTK_INVALID_LIST_POSITION)
      gtk_selection_model_selection_changed (self->pages, old_pos, 1);
    else
      gtk_selection_model_selection_changed (self->pages,
                                             MIN (old_pos, new_pos),
                                             MAX (old_pos, new_pos) - MIN (old_pos, new_pos) + 1);
  }

  g_object_freeze_notify (G_OBJECT (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD_NAME]);
  g_object_thaw_notify (G_OBJECT (self));
}

static void
set_mode_transition_progress (HdyLeaflet *self,
                              gdouble     pos)
{
  self->mode_transition.current_pos = pos;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
mode_progress_updated (HdyLeaflet *self)
{
//  if (gtk_progress_tracker_get_state (&self->mode_transition.tracker) == GTK_PROGRESS_STATE_AFTER)
//    hdy_shadow_helper_clear_cache (self->shadow_helper);
}

static gboolean
mode_transition_cb (GtkWidget     *widget,
                    GdkFrameClock *frame_clock,
                    gpointer       user_data)
{
  HdyLeaflet *self = HDY_LEAFLET (user_data);
  gdouble ease;

  gtk_progress_tracker_advance_frame (&self->mode_transition.tracker,
                                      gdk_frame_clock_get_frame_time (frame_clock));
  ease = gtk_progress_tracker_get_ease_out_cubic (&self->mode_transition.tracker, FALSE);
  set_mode_transition_progress (self,
                                self->mode_transition.source_pos + (ease * (self->mode_transition.target_pos - self->mode_transition.source_pos)));

  mode_progress_updated (self);

  if (gtk_progress_tracker_get_state (&self->mode_transition.tracker) == GTK_PROGRESS_STATE_AFTER) {
    self->mode_transition.tick_id = 0;
    return FALSE;
  }

  return TRUE;
}

static void
start_mode_transition (HdyLeaflet *self,
                       gdouble     target)
{
  GtkWidget *widget = GTK_WIDGET (self);

  if (self->mode_transition.target_pos == target)
    return;

  self->mode_transition.target_pos = target;
  /* FIXME PROP_REVEAL_CHILD needs to be implemented. */
  /* g_object_notify_by_pspec (G_OBJECT (revealer), props[PROP_REVEAL_CHILD]); */

  stop_child_transition (self);

  if (gtk_widget_get_mapped (widget) &&
      self->mode_transition.duration != 0 &&
      hdy_get_enable_animations (widget) &&
      self->can_unfold) {
    self->mode_transition.source_pos = self->mode_transition.current_pos;
    if (self->mode_transition.tick_id == 0)
      self->mode_transition.tick_id = gtk_widget_add_tick_callback (widget, mode_transition_cb, self, NULL);
    gtk_progress_tracker_start (&self->mode_transition.tracker,
                                self->mode_transition.duration * 1000,
                                0,
                                1.0);
  }
  else
    set_mode_transition_progress (self, target);
}


/* FIXME Use this to stop the mode transition animation when it makes sense (see *
 * GtkRevealer for exmples).
 */
/* static void */
/* stop_mode_animation (HdyLeaflet *self) */
/* { */
/*   if (self->mode_transition.current_pos != self->mode_transition.target_pos) { */
/*     self->mode_transition.current_pos = self->mode_transition.target_pos; */
    /* g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_REVEALED]); */
/*   } */
/*   if (self->mode_transition.tick_id != 0) { */
/*     gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->mode_transition.tick_id); */
/*     self->mode_transition.tick_id = 0; */
/*   } */
/* } */

static void
set_folded (HdyLeaflet *self,
            gboolean    folded)
{
  if (self->folded == folded)
    return;

  self->folded = folded;

  start_mode_transition (self, folded ? 0.0 : 1.0);

  if (folded) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "folded");
    gtk_widget_remove_css_class (GTK_WIDGET (self), "unfolded");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "folded");
    gtk_widget_add_css_class (GTK_WIDGET (self), "unfolded");
  }

  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_FOLDED]);
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
             hdy_lerp (last_visible_min, visible_min, visible_child_progress);
    *nat = homogeneous_unfolded ?
             max_nat * visible_children :
             sum_nat;
  }
  else {
    *min = homogeneous_folded ?
             max_min :
             hdy_lerp (last_visible_min, visible_min, visible_child_progress);
    *nat = max_nat;
  }
}

static void
hdy_leaflet_size_allocate_folded (HdyLeaflet *self,
                                  gint        width,
                                  gint        height)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GList *directed_children, *children;
  HdyLeafletPage *page, *visible_child;
  gint start_size, end_size, visible_size;
  gint remaining_start_size, remaining_end_size, remaining_size;
  gint current_pad;
  gint max_child_size = 0;
  gint start_position, end_position;
  gboolean box_homogeneous;
  HdyLeafletTransitionType mode_transition_type;
  GtkTextDirection direction;
  gboolean under;

  directed_children = get_directed_children (self);
  visible_child = self->visible_child;

  if (!visible_child)
    return;

  for (children = directed_children; children; children = children->next) {
    page = children->data;

    if (!page->widget)
      continue;

    if (page->widget == visible_child->widget)
      continue;

    if (self->last_visible_child &&
        page->widget == self->last_visible_child->widget)
      continue;

    page->visible = FALSE;
  }

  if (visible_child->widget == NULL)
    return;

  /* FIXME is this needed? */
  if (!gtk_widget_get_visible (visible_child->widget)) {
    visible_child->visible = FALSE;

    return;
  }

  visible_child->visible = TRUE;

  mode_transition_type = self->transition_type;

  /* Avoid useless computations and allow visible child transitions. */
  if (self->mode_transition.current_pos <= 0.0) {
    /* Child transitions should be applied only when folded and when no mode
     * transition is ongoing.
     */
    for (children = directed_children; children; children = children->next) {
      page = children->data;

      if (page != visible_child &&
          page != self->last_visible_child) {
        page->visible = FALSE;

        continue;
      }

      page->alloc.x = get_child_window_x (self, page, width);
      page->alloc.y = get_child_window_y (self, page, height);
      page->alloc.width = width;
      page->alloc.height = height;
      page->visible = TRUE;
    }

    return;
  }

  /* Compute visible child size. */
  visible_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
    MIN (width, MAX (visible_child->nat.width, (gint) (width * (1.0 - self->mode_transition.current_pos)))) :
    MIN (height, MAX (visible_child->nat.height, (gint) (height * (1.0 - self->mode_transition.current_pos))));

  /* Compute homogeneous box child size. */
  box_homogeneous = (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] && orientation == GTK_ORIENTATION_HORIZONTAL) ||
                    (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] && orientation == GTK_ORIENTATION_VERTICAL);
  if (box_homogeneous) {
    for (children = directed_children; children; children = children->next) {
      page = children->data;

      max_child_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
        MAX (max_child_size, page->nat.width) :
        MAX (max_child_size, page->nat.height);
    }
  }

  /* Compute the start size. */
  start_size = 0;
  for (children = directed_children; children; children = children->next) {
    page = children->data;

    if (page == visible_child)
      break;

    start_size += orientation == GTK_ORIENTATION_HORIZONTAL ?
      (box_homogeneous ? max_child_size : page->nat.width) :
      (box_homogeneous ? max_child_size : page->nat.height);
  }

  /* Compute the end size. */
  end_size = 0;
  for (children = g_list_last (directed_children); children; children = children->prev) {
    page = children->data;

    if (page == visible_child)
      break;

    end_size += orientation == GTK_ORIENTATION_HORIZONTAL ?
      (box_homogeneous ? max_child_size : page->nat.width) :
      (box_homogeneous ? max_child_size : page->nat.height);
  }

  /* Compute pads. */
  remaining_size = orientation == GTK_ORIENTATION_HORIZONTAL ?
    width - visible_size :
    height - visible_size;
  remaining_start_size = (gint) (remaining_size * ((gdouble) start_size / (gdouble) (start_size + end_size)));
  remaining_end_size = remaining_size - remaining_start_size;

  /* Store start and end allocations. */
  switch (orientation) {
  case GTK_ORIENTATION_HORIZONTAL:
    direction = gtk_widget_get_direction (GTK_WIDGET (self));
    under = (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_LTR) ||
            (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_RTL);
    start_position = under ? 0 : remaining_start_size - start_size;
    self->mode_transition.start_progress = under ? (gdouble) remaining_size / start_size : 1;
    under = (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_LTR) ||
            (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_RTL);
    end_position = under ? width - end_size : remaining_start_size + visible_size;
    self->mode_transition.end_progress = under ? (gdouble) remaining_end_size / end_size : 1;
    break;
  case GTK_ORIENTATION_VERTICAL:
    under = mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER;
    start_position = under ? 0 : remaining_start_size - start_size;
    self->mode_transition.start_progress = under ? (gdouble) remaining_size / start_size : 1;
    under = mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER;
    end_position = remaining_start_size + visible_size;
    self->mode_transition.end_progress = under ? (gdouble) remaining_end_size / end_size : 1;
    break;
  default:
    g_assert_not_reached ();
  }

  /* Allocate visible child. */
  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    visible_child->alloc.width = visible_size;
    visible_child->alloc.height = height;
    visible_child->alloc.x = remaining_start_size;
    visible_child->alloc.y = 0;
    visible_child->visible = TRUE;
  }
  else {
    visible_child->alloc.width = width;
    visible_child->alloc.height = visible_size;
    visible_child->alloc.x = 0;
    visible_child->alloc.y = remaining_start_size;
    visible_child->visible = TRUE;
  }

  /* Allocate starting children. */
  current_pad = start_position;

  for (children = directed_children; children; children = children->next) {
    page = children->data;

    if (page == visible_child)
      break;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      page->alloc.width = box_homogeneous ?
        max_child_size :
        page->nat.width;
      page->alloc.height = height;
      page->alloc.x = current_pad;
      page->alloc.y = 0;
      page->visible = page->alloc.x + page->alloc.width > 0;

      current_pad += page->alloc.width;
    }
    else {
      page->alloc.width = width;
      page->alloc.height = box_homogeneous ?
        max_child_size :
        page->nat.height;
      page->alloc.x = 0;
      page->alloc.y = current_pad;
      page->visible = page->alloc.y + page->alloc.height > 0;

      current_pad += page->alloc.height;
    }
  }

  /* Allocate ending children. */
  current_pad = end_position;

  if (!children || !children->next)
    return;

  for (children = children->next; children; children = children->next) {
    page = children->data;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      page->alloc.width = box_homogeneous ?
        max_child_size :
        page->nat.width;
      page->alloc.height = height;
      page->alloc.x = current_pad;
      page->alloc.y = 0;
      page->visible = page->alloc.x < width;

      current_pad += page->alloc.width;
    }
    else {
      page->alloc.width = width;
      page->alloc.height = box_homogeneous ?
        max_child_size :
        page->nat.height;
      page->alloc.x = 0;
      page->alloc.y = current_pad;
      page->visible = page->alloc.y < height;

      current_pad += page->alloc.height;
    }
  }
}

static void
hdy_leaflet_size_allocate_unfolded (HdyLeaflet *self,
                                    gint        width,
                                    gint        height)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GtkAllocation remaining_alloc;
  GList *directed_children, *children;
  HdyLeafletPage *page, *visible_child;
  gint homogeneous_size = 0, min_size, extra_size;
  gint per_child_extra, n_extra_widgets;
  gint n_visible_children, n_expand_children;
  gint start_pad = 0, end_pad = 0;
  gboolean box_homogeneous;
  HdyLeafletTransitionType mode_transition_type;
  GtkTextDirection direction;
  gboolean under;

  directed_children = get_directed_children (self);
  visible_child = self->visible_child;

  box_homogeneous = (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] && orientation == GTK_ORIENTATION_HORIZONTAL) ||
                    (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] && orientation == GTK_ORIENTATION_VERTICAL);

  n_visible_children = n_expand_children = 0;
  for (children = directed_children; children; children = children->next) {
    page = children->data;

    page->visible = page->widget != NULL && gtk_widget_get_visible (page->widget);

    if (page->visible) {
      n_visible_children++;
      if (gtk_widget_compute_expand (page->widget, orientation))
        n_expand_children++;
    }
    else {
      page->min.width = page->min.height = 0;
      page->nat.width = page->nat.height = 0;
    }
  }

  /* Compute repartition of extra space. */

  if (box_homogeneous) {
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      homogeneous_size = n_visible_children > 0 ? width / n_visible_children : 0;
      n_expand_children = n_visible_children > 0 ? width % n_visible_children : 0;
      min_size = width - n_expand_children;
    }
    else {
      homogeneous_size = n_visible_children > 0 ? height / n_visible_children : 0;
      n_expand_children = n_visible_children > 0 ? height % n_visible_children : 0;
      min_size = height - n_expand_children;
    }
  }
  else {
    min_size = 0;
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      for (children = directed_children; children; children = children->next) {
        page = children->data;

        min_size += page->nat.width;
      }
    }
    else {
      for (children = directed_children; children; children = children->next) {
        page = children->data;

        min_size += page->nat.height;
      }
    }
  }

  remaining_alloc.x = 0;
  remaining_alloc.y = 0;
  remaining_alloc.width = width;
  remaining_alloc.height = height;

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
    page = children->data;

    if (!page->visible)
      continue;

    page->alloc.x = remaining_alloc.x;
    page->alloc.y = remaining_alloc.y;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
      if (box_homogeneous) {
        page->alloc.width = homogeneous_size;
        if (n_extra_widgets > 0) {
          page->alloc.width++;
          n_extra_widgets--;
        }
      }
      else {
        page->alloc.width = page->nat.width;
        if (gtk_widget_compute_expand (page->widget, orientation)) {
          page->alloc.width += per_child_extra;
          if (n_extra_widgets > 0) {
            page->alloc.width++;
            n_extra_widgets--;
          }
        }
      }
      page->alloc.height = remaining_alloc.height;

      remaining_alloc.x += page->alloc.width;
      remaining_alloc.width -= page->alloc.width;
    }
    else {
      if (box_homogeneous) {
        page->alloc.height = homogeneous_size;
        if (n_extra_widgets > 0) {
          page->alloc.height++;
          n_extra_widgets--;
        }
      }
      else {
        page->alloc.height = page->nat.height;
        if (gtk_widget_compute_expand (page->widget, orientation)) {
          page->alloc.height += per_child_extra;
          if (n_extra_widgets > 0) {
            page->alloc.height++;
            n_extra_widgets--;
          }
        }
      }
      page->alloc.width = remaining_alloc.width;

      remaining_alloc.y += page->alloc.height;
      remaining_alloc.height -= page->alloc.height;
    }
  }

  /* Apply animations. */

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    start_pad = (gint) ((visible_child->alloc.x) * (1.0 - self->mode_transition.current_pos));
    end_pad = (gint) ((width - (visible_child->alloc.x + visible_child->alloc.width)) * (1.0 - self->mode_transition.current_pos));
  }
  else {
    start_pad = (gint) ((visible_child->alloc.y) * (1.0 - self->mode_transition.current_pos));
    end_pad = (gint) ((height - (visible_child->alloc.y + visible_child->alloc.height)) * (1.0 - self->mode_transition.current_pos));
  }

  mode_transition_type = self->transition_type;
  direction = gtk_widget_get_direction (GTK_WIDGET (self));

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    under = (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_LTR) ||
            (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_RTL);
  else
    under = mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER;
  for (children = directed_children; children; children = children->next) {
    page = children->data;

    if (page == visible_child)
      break;

    if (!page->visible)
      continue;

    if (under)
      continue;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      page->alloc.x -= start_pad;
    else
      page->alloc.y -= start_pad;
  }

  self->mode_transition.start_progress = under ? self->mode_transition.current_pos : 1;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    under = (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER && direction == GTK_TEXT_DIR_LTR) ||
            (mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER && direction == GTK_TEXT_DIR_RTL);
  else
    under = mode_transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER;
  for (children = g_list_last (directed_children); children; children = children->prev) {
    page = children->data;

    if (page == visible_child)
      break;

    if (!page->visible)
      continue;

    if (under)
      continue;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      page->alloc.x += end_pad;
    else
      page->alloc.y += end_pad;
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

static HdyLeafletPage *
get_top_overlap_child (HdyLeaflet *self)
{
  gboolean is_rtl, start;

  if (!self->last_visible_child)
    return self->visible_child;

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  start = (self->child_transition.active_direction == GTK_PAN_DIRECTION_LEFT && !is_rtl) ||
          (self->child_transition.active_direction == GTK_PAN_DIRECTION_RIGHT && is_rtl) ||
           self->child_transition.active_direction == GTK_PAN_DIRECTION_UP;

  switch (self->transition_type) {
  case HDY_LEAFLET_TRANSITION_TYPE_SLIDE:
    // Nothing overlaps in this case
    return NULL;
  case HDY_LEAFLET_TRANSITION_TYPE_OVER:
    return start ? self->visible_child : self->last_visible_child;
  case HDY_LEAFLET_TRANSITION_TYPE_UNDER:
    return start ? self->last_visible_child : self->visible_child;
  default:
    g_assert_not_reached ();
  }
}

static void
update_tracker_orientation (HdyLeaflet *self)
{
  gboolean reverse;

  reverse = (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
             gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL);

  g_object_set (self->tracker,
                "orientation", self->orientation,
                "reversed", reverse,
                NULL);
}

static void
update_child_visible (HdyLeaflet     *self,
                      HdyLeafletPage *page)
{
  gboolean enabled;

  enabled = gtk_widget_get_visible (page->widget);

  if (self->visible_child == NULL && enabled)
    set_visible_child (self, page, self->transition_type, self->child_transition.duration, TRUE);
  else if (self->visible_child == page && !enabled)
    set_visible_child (self, NULL, self->transition_type, self->child_transition.duration, TRUE);

  if (page == self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }
}

static void
leaflet_child_visibility_notify_cb (GObject    *obj,
                                    GParamSpec *pspec,
                                    gpointer    user_data)
{
  HdyLeaflet *self = HDY_LEAFLET (user_data);
  GtkWidget *child = GTK_WIDGET (obj);
  HdyLeafletPage *page;

  page = find_page_for_widget (self, child);
  g_return_if_fail (page != NULL);

  update_child_visible (self, page);
}

static gboolean
can_swipe_in_direction (HdyLeaflet             *self,
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

static void
set_orientation (HdyLeaflet     *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  self->orientation = orientation;
  update_tracker_orientation (self);
  gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify (G_OBJECT (self), "orientation");
}

static void
begin_swipe_cb (HdySwipeTracker        *tracker,
                HdyNavigationDirection  direction,
                gboolean                direct,
                HdyLeaflet             *self)
{
  self->child_transition.is_direct_swipe = direct;
  self->child_transition.swipe_direction = direction;

  if (self->child_transition.tick_id > 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self),
                                     self->child_transition.tick_id);
    self->child_transition.tick_id = 0;
    self->child_transition.is_gesture_active = TRUE;
    self->child_transition.is_cancelled = FALSE;
  } else {
    HdyLeafletPage *page = NULL;

    if ((can_swipe_in_direction (self, direction) || !direct) && self->folded)
      page = find_swipeable_page (self, direction);

    if (page) {
      self->child_transition.is_gesture_active = TRUE;
      set_visible_child (self, page, self->transition_type,
                         self->child_transition.duration, FALSE);

      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_TRANSITION_RUNNING]);
    }
  }
}

static void
update_swipe_cb (HdySwipeTracker *tracker,
                 gdouble          progress,
                 HdyLeaflet      *self)
{
  self->child_transition.progress = ABS (progress);
  child_progress_updated (self);
}

static void
end_swipe_cb (HdySwipeTracker *tracker,
              gint64           duration,
              gdouble          to,
              HdyLeaflet      *self)
{
 if (!self->child_transition.is_gesture_active)
    return;

  self->child_transition.start_progress = self->child_transition.progress;
  self->child_transition.end_progress = ABS (to);
  self->child_transition.is_cancelled = (to == 0);
  self->child_transition.first_frame_skipped = TRUE;

  schedule_child_ticks (self);
  if (hdy_get_enable_animations (GTK_WIDGET (self)) && duration != 0) {
    gtk_progress_tracker_start (&self->child_transition.tracker,
                                duration * 1000,
                                0,
                                1.0);
  } else {
    self->child_transition.progress = self->child_transition.end_progress;
    gtk_progress_tracker_finish (&self->child_transition.tracker);
  }

  self->child_transition.is_gesture_active = FALSE;
  child_progress_updated (self);

  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
add_page (HdyLeaflet     *self,
          HdyLeafletPage *page,
          HdyLeafletPage *sibling_page)
{
  gint visible_child_pos_before_insert = -1;
  gint visible_child_pos_after_insert = -1;

  g_return_if_fail (page->widget != NULL);

  if (self->visible_child)
    visible_child_pos_before_insert = g_list_index (self->children, self->visible_child);

  g_object_ref (page);

  if (!sibling_page) {
    self->children = g_list_prepend (self->children, page);
    self->children_reversed = g_list_append (self->children_reversed, page);
  } else {
    gint sibling_pos = g_list_index (self->children, sibling_page);

    self->children =
      g_list_insert (self->children, page, sibling_pos + 1);
    self->children_reversed =
      g_list_insert (self->children_reversed, page,
                     g_list_length (self->children) - sibling_pos - 1);
  }

  if (self->visible_child)
    visible_child_pos_after_insert = g_list_index (self->children, self->visible_child);

  gtk_widget_set_child_visible (page->widget, FALSE);

  if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER)
    gtk_widget_insert_before (page->widget, GTK_WIDGET (self),
                              sibling_page ? sibling_page->widget : NULL);
  else
    gtk_widget_insert_after (page->widget, GTK_WIDGET (self),
                              sibling_page ? sibling_page->widget : NULL);

  if (self->pages) {
    gint position = g_list_index (self->children, page);

    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 0, 1);
  }

  g_signal_connect (page->widget, "notify::visible",
                    G_CALLBACK (leaflet_child_visibility_notify_cb), self);

  if (self->visible_child == NULL &&
      gtk_widget_get_visible (page->widget))
    set_visible_child (self, page, self->transition_type,
                       self->child_transition.duration, FALSE);
  else if (visible_child_pos_before_insert != visible_child_pos_after_insert)
    hdy_swipeable_emit_child_switched (HDY_SWIPEABLE (self),
                                       visible_child_pos_after_insert,
                                       0);

  if (!self->folded ||
      (self->folded && (self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] ||
                        self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] ||
                        self->visible_child == page)))
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
leaflet_remove (HdyLeaflet *self,
                GtkWidget  *child,
                gboolean    in_dispose)
{
  HdyLeafletPage *page;
  gboolean was_visible;

  page = find_page_for_widget (self, child);
  if (!page)
    return;

  self->children = g_list_remove (self->children, page);
  self->children_reversed = g_list_remove (self->children_reversed, page);

  g_signal_handlers_disconnect_by_func (child,
                                        leaflet_child_visibility_notify_cb,
                                        self);

  was_visible = gtk_widget_get_visible (child);

  g_clear_object (&page->widget);

  if (self->visible_child == page)
    {
      if (in_dispose)
        self->visible_child = NULL;
      else
        set_visible_child (self, NULL, self->transition_type, self->child_transition.duration, TRUE);
    }

  if (self->last_visible_child == page)
    self->last_visible_child = NULL;

  gtk_widget_unparent (child);

  g_object_unref (page);

  if (was_visible)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
hdy_leaflet_measure (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     gint            for_size,
                     gint           *minimum,
                     gint           *natural,
                     gint           *minimum_baseline,
                     gint           *natural_baseline)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  GList *l;
  gint visible_children;
  gdouble visible_child_progress;
  gint child_min, max_min, visible_min, last_visible_min;
  gint child_nat, max_nat, sum_nat;

  visible_children = 0;
  child_min = max_min = visible_min = last_visible_min = 0;
  child_nat = max_nat = sum_nat = 0;
  for (l = self->children; l; l = l->next) {
    HdyLeafletPage *page = l->data;

    if (page->widget == NULL || !gtk_widget_get_visible (page->widget))
      continue;

    visible_children++;

    gtk_widget_measure (page->widget, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);

    max_min = MAX (max_min, child_min);
    max_nat = MAX (max_nat, child_nat);
    sum_nat += child_nat;
  }

  if (self->visible_child != NULL)
    gtk_widget_measure (self->visible_child->widget, orientation, for_size,
                        &visible_min, NULL, NULL, NULL);

  if (self->last_visible_child != NULL) {
    gtk_widget_measure (self->last_visible_child->widget, orientation, for_size,
                        &last_visible_min, NULL, NULL, NULL);
  } else {
    last_visible_min = visible_min;
  }

  visible_child_progress = self->child_transition.interpolate_size ? self->child_transition.progress : 1.0;

  get_preferred_size (minimum, natural,
                      gtk_orientable_get_orientation (GTK_ORIENTABLE (self)) == orientation,
                      self->homogeneous[HDY_FOLD_FOLDED][orientation],
                      self->homogeneous[HDY_FOLD_UNFOLDED][orientation],
                      visible_children, visible_child_progress,
                      sum_nat, max_min, max_nat, visible_min, last_visible_min);
}

static void
allocate_shadow (HdyLeaflet *self,
                 gint        width,
                 gint        height,
                 gint        baseline)
{
  HdyLeafletPage *overlap_child;
  gboolean is_transition;
  gboolean is_vertical;
  gboolean is_rtl;
  gboolean is_over;
  GtkAllocation shadow_rect;
  gdouble shadow_progress, mode_progress;
  GtkPanDirection shadow_direction;

  is_transition = self->child_transition.is_gesture_active ||
                  gtk_progress_tracker_get_state (&self->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER ||
                  gtk_progress_tracker_get_state (&self->mode_transition.tracker) != GTK_PROGRESS_STATE_AFTER;

  overlap_child = get_top_overlap_child (self);

  if (!is_transition ||
      self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE ||
      !overlap_child) {
    hdy_shadow_helper_clear (self->shadow_helper);

    return;
  }

  is_vertical = gtk_orientable_get_orientation (GTK_ORIENTABLE (self)) == GTK_ORIENTATION_VERTICAL;
  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  is_over = self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER;

  shadow_rect.x = 0;
  shadow_rect.y = 0;
  shadow_rect.width = gtk_widget_get_width (GTK_WIDGET (self));
  shadow_rect.height = gtk_widget_get_height (GTK_WIDGET (self));

  if (is_vertical) {
    if (!is_over) {
      shadow_rect.y = overlap_child->alloc.y + overlap_child->alloc.height;
      shadow_rect.height -= shadow_rect.y;
      shadow_direction = GTK_PAN_DIRECTION_UP;
      mode_progress = self->mode_transition.end_progress;
    } else {
      shadow_rect.height = overlap_child->alloc.y;
      shadow_direction = GTK_PAN_DIRECTION_DOWN;
      mode_progress = self->mode_transition.start_progress;
    }
  } else {
    if (is_over == is_rtl) {
      shadow_rect.x = overlap_child->alloc.x + overlap_child->alloc.width;
      shadow_rect.width -= shadow_rect.x;
      shadow_direction = GTK_PAN_DIRECTION_LEFT;
      mode_progress = self->mode_transition.end_progress;
    } else {
      shadow_rect.width = overlap_child->alloc.x;
      shadow_direction = GTK_PAN_DIRECTION_RIGHT;
      mode_progress = self->mode_transition.start_progress;
    }
  }

  if (gtk_progress_tracker_get_state (&self->mode_transition.tracker) != GTK_PROGRESS_STATE_AFTER) {
    shadow_progress = mode_progress;
  } else {
    GtkPanDirection direction = self->child_transition.active_direction;
    GtkPanDirection left_or_right = is_rtl ? GTK_PAN_DIRECTION_RIGHT : GTK_PAN_DIRECTION_LEFT;

    if (direction == GTK_PAN_DIRECTION_UP || direction == left_or_right)
      shadow_progress = self->child_transition.progress;
    else
      shadow_progress = 1 - self->child_transition.progress;

    if (is_over)
      shadow_progress = 1 - shadow_progress;

    /* Normalize the shadow rect size so that we can cache the shadow */
    if (shadow_direction == GTK_PAN_DIRECTION_RIGHT)
      shadow_rect.x -= (width - shadow_rect.width);
    else if (shadow_direction == GTK_PAN_DIRECTION_DOWN)
      shadow_rect.y -= (height - shadow_rect.height);

    shadow_rect.width = width;
    shadow_rect.height = height;
  }

  if (shadow_progress > 0)
    hdy_shadow_helper_size_allocate (self->shadow_helper, shadow_rect.width, shadow_rect.height,
                                     baseline, shadow_rect.x, shadow_rect.y,
                                     shadow_progress, shadow_direction);
}

static void
hdy_leaflet_size_allocate (GtkWidget *widget,
                           gint       width,
                           gint       height,
                           gint       baseline)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget));
  GList *directed_children, *children;
  gboolean folded;

  directed_children = get_directed_children (self);

  /* Prepare children information. */
  for (children = directed_children; children; children = children->next) {
    HdyLeafletPage *page = children->data;

    gtk_widget_get_preferred_size (page->widget, &page->min, &page->nat);
    page->alloc.x = page->alloc.y = page->alloc.width = page->alloc.height = 0;
    page->visible = FALSE;
  }

  /* Check whether the children should be stacked or not. */
  if (self->can_unfold) {
    gint nat_box_size = 0, nat_max_size = 0, visible_children = 0;

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {

      for (children = directed_children; children; children = children->next) {
        HdyLeafletPage *page = children->data;

        /* FIXME Check the child is visible. */
        if (!page->widget)
          continue;

        if (page->nat.width <= 0)
          continue;

        nat_box_size += page->nat.width;
        nat_max_size = MAX (nat_max_size, page->nat.width);
        visible_children++;
      }
      if (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL])
        nat_box_size = nat_max_size * visible_children;
      folded = visible_children > 1 && width < nat_box_size;
    }
    else {
      for (children = directed_children; children; children = children->next) {
        HdyLeafletPage *page = children->data;

        /* FIXME Check the child is visible. */
        if (!page->widget)
          continue;

        if (page->nat.height <= 0)
          continue;

        nat_box_size += page->nat.height;
        nat_max_size = MAX (nat_max_size, page->nat.height);
        visible_children++;
      }
      if (self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL])
        nat_box_size = nat_max_size * visible_children;
      folded = visible_children > 1 && height < nat_box_size;
    }
  } else {
    folded = TRUE;
  }

  set_folded (self, folded);

  /* Allocate size to the children. */
  if (folded)
    hdy_leaflet_size_allocate_folded (self, width, height);
  else
    hdy_leaflet_size_allocate_unfolded (self, width, height);

  /* Apply visibility and allocation. */
  for (children = directed_children; children; children = children->next) {
    HdyLeafletPage *page = children->data;

    gtk_widget_set_child_visible (page->widget, page->visible);

    if (!page->visible)
      continue;

    gtk_widget_size_allocate (page->widget, &page->alloc, baseline);

    if (gtk_widget_get_realized (widget))
      gtk_widget_show (page->widget);
  }

  allocate_shadow (self, width, height, baseline);
}

static void
hdy_leaflet_snapshot (GtkWidget   *widget,
                      GtkSnapshot *snapshot)
{
  HdyLeaflet *self = HDY_LEAFLET (widget);
  GList *stacked_children, *l;
  HdyLeafletPage *overlap_child;
  gboolean is_transition;
  gboolean is_vertical;
  gboolean is_rtl;
  gboolean is_over;
  GdkRectangle shadow_rect;

  overlap_child = get_top_overlap_child (self);

  is_transition = self->child_transition.is_gesture_active ||
                  gtk_progress_tracker_get_state (&self->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER ||
                  gtk_progress_tracker_get_state (&self->mode_transition.tracker) != GTK_PROGRESS_STATE_AFTER;

  if (!is_transition ||
      self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE ||
      !overlap_child) {
    GTK_WIDGET_CLASS (hdy_leaflet_parent_class)->snapshot (widget, snapshot);

    return;
  }

  stacked_children = self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER ?
                     self->children_reversed : self->children;

  is_vertical = gtk_orientable_get_orientation (GTK_ORIENTABLE (widget)) == GTK_ORIENTATION_VERTICAL;
  is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
  is_over = self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER;

  shadow_rect.x = 0;
  shadow_rect.y = 0;
  shadow_rect.width = gtk_widget_get_width (widget);
  shadow_rect.height = gtk_widget_get_height (widget);

  if (is_vertical) {
    if (!is_over) {
      shadow_rect.y = overlap_child->alloc.y + overlap_child->alloc.height;
      shadow_rect.height -= shadow_rect.y;
    } else {
      shadow_rect.height = overlap_child->alloc.y;
    }
  } else {
    if (is_over == is_rtl) {
      shadow_rect.x = overlap_child->alloc.x + overlap_child->alloc.width;
      shadow_rect.width -= shadow_rect.x;
    } else {
      shadow_rect.width = overlap_child->alloc.x;
    }
  }

  gtk_snapshot_push_clip (snapshot,
                          &GRAPHENE_RECT_INIT (shadow_rect.x,
                                               shadow_rect.y,
                                               shadow_rect.width,
                                               shadow_rect.height));

  for (l = stacked_children; l; l = l->next) {
    HdyLeafletPage *page = l->data;

    if (page == overlap_child)
      gtk_snapshot_pop (snapshot);

    gtk_widget_snapshot_child (widget, page->widget, snapshot);
  }

  hdy_shadow_helper_snapshot (self->shadow_helper, snapshot);
}

static void
hdy_leaflet_direction_changed (GtkWidget        *widget,
                               GtkTextDirection  previous_direction)
{
  update_tracker_orientation (HDY_LEAFLET (widget));
}

static void
hdy_leaflet_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  HdyLeaflet *self = HDY_LEAFLET (object);

  switch (prop_id) {
  case PROP_FOLDED:
    g_value_set_boolean (value, hdy_leaflet_get_folded (self));
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
  case PROP_TRANSITION_TYPE:
    g_value_set_enum (value, hdy_leaflet_get_transition_type (self));
    break;
  case PROP_MODE_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_leaflet_get_mode_transition_duration (self));
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
  case PROP_CAN_SWIPE_BACK:
    g_value_set_boolean (value, hdy_leaflet_get_can_swipe_back (self));
    break;
  case PROP_CAN_SWIPE_FORWARD:
    g_value_set_boolean (value, hdy_leaflet_get_can_swipe_forward (self));
    break;
  case PROP_CAN_UNFOLD:
    g_value_set_boolean (value, hdy_leaflet_get_can_unfold (self));
    break;
  case PROP_PAGES:
    g_value_set_object (value, hdy_leaflet_get_pages (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
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
  case PROP_TRANSITION_TYPE:
    hdy_leaflet_set_transition_type (self, g_value_get_enum (value));
    break;
  case PROP_MODE_TRANSITION_DURATION:
    hdy_leaflet_set_mode_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_CHILD_TRANSITION_DURATION:
    hdy_leaflet_set_child_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    hdy_leaflet_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_BACK:
    hdy_leaflet_set_can_swipe_back (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SWIPE_FORWARD:
    hdy_leaflet_set_can_swipe_forward (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_UNFOLD:
    hdy_leaflet_set_can_unfold (self, g_value_get_boolean (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_leaflet_finalize (GObject *object)
{
  HdyLeaflet *self = HDY_LEAFLET (object);

  self->visible_child = NULL;

//  g_clear_object (&self->shadow_helper);

  if (self->pages)
    g_object_remove_weak_pointer (G_OBJECT (self->pages),
                                  (gpointer *) &self->pages);

  unschedule_child_ticks (self);

  G_OBJECT_CLASS (hdy_leaflet_parent_class)->finalize (object);
}

static void
hdy_leaflet_class_init (HdyLeafletClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_leaflet_get_property;
  object_class->set_property = hdy_leaflet_set_property;
  object_class->finalize = hdy_leaflet_finalize;

  widget_class->measure = hdy_leaflet_measure;
  widget_class->size_allocate = hdy_leaflet_size_allocate;
  widget_class->snapshot = hdy_leaflet_snapshot;
  widget_class->direction_changed = hdy_leaflet_direction_changed;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * HdyLeaflet:folded:
   *
   * %TRUE if the leaflet is folded.
   *
   * The leaflet will be folded if the size allocated to it is smaller than the
   * sum of the natural size of its children, it will be unfolded otherwise.
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

  /**
   * HdyLeaflet:transition-type:
   *
   * The type of animation that will be used for transitions between modes and
   * children.
   *
   * The transition type can be changed without problems at runtime, so it is
   * possible to change the animation based on the mode or child that is about
   * to become current.
   *
   * Since: 0.0.12
   */
  props[PROP_TRANSITION_TYPE] =
    g_param_spec_enum ("transition-type",
                       _("Transition type"),
                       _("The type of animation used to transition between modes and children"),
                       HDY_TYPE_LEAFLET_TRANSITION_TYPE, HDY_LEAFLET_TRANSITION_TYPE_OVER,
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
   * HdyLeaflet:can-swipe-back:
   *
   * Whether or not the leaflet allows switching to the previous child that has
   * 'navigatable' child property set to %TRUE via a swipe gesture.
   *
   * Since: 0.0.12
   */
  props[PROP_CAN_SWIPE_BACK] =
      g_param_spec_boolean ("can-swipe-back",
                            _("Can swipe back"),
                            _("Whether or not swipe gesture can be used to switch to the previous child"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyLeaflet:can-swipe-forward:
   *
   * Whether or not the leaflet allows switching to the next child that has
   * 'navigatable' child property set to %TRUE via a swipe gesture.
   *
   * Since: 0.0.12
   */
  props[PROP_CAN_SWIPE_FORWARD] =
      g_param_spec_boolean ("can-swipe-forward",
                            _("Can swipe forward"),
                            _("Whether or not swipe gesture can be used to switch to the next child"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CAN_UNFOLD] =
      g_param_spec_boolean ("can-unfold",
                            _("Can unfold"),
                            _("Whether or not the leaflet can unfold"),
                            TRUE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

 props[PROP_PAGES] =
    g_param_spec_object ("pages",
                         _("Pages"),
                         _("A selection model with the leaflet's pages"),
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "leaflet");
}


static void
hdy_leaflet_init (HdyLeaflet *self)
{
  GtkWidget *widget = GTK_WIDGET (self);

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  self->children = NULL;
  self->children_reversed = NULL;
  self->visible_child = NULL;
  self->folded = FALSE;
  self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_HORIZONTAL] = FALSE;
  self->homogeneous[HDY_FOLD_UNFOLDED][GTK_ORIENTATION_VERTICAL] = FALSE;
  self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_HORIZONTAL] = TRUE;
  self->homogeneous[HDY_FOLD_FOLDED][GTK_ORIENTATION_VERTICAL] = TRUE;
  self->transition_type = HDY_LEAFLET_TRANSITION_TYPE_OVER;
  self->mode_transition.duration = 250;
  self->child_transition.duration = 200;
  self->mode_transition.current_pos = 1.0;
  self->mode_transition.target_pos = 1.0;
  self->can_unfold = TRUE;

  self->tracker = hdy_swipe_tracker_new (HDY_SWIPEABLE (self));

  g_object_set (self->tracker, "orientation", self->orientation, "enabled", FALSE, NULL);

  g_signal_connect_object (self->tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self, 0);

  self->shadow_helper = hdy_shadow_helper_new (widget);

  gtk_widget_add_css_class (widget, "unfolded");
}

static void
hdy_leaflet_buildable_add_child (GtkBuildable *buildable,
                                 GtkBuilder   *builder,
                                 GObject      *child,
                                 const char   *type)
{
  HdyLeaflet *self = HDY_LEAFLET (buildable);

  if (HDY_IS_LEAFLET_PAGE (child))
    add_page (self, HDY_LEAFLET_PAGE (child),
              self->children ? g_list_last (self->children)->data : NULL);
  else if (GTK_IS_WIDGET (child))
    hdy_leaflet_append (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_leaflet_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = hdy_leaflet_buildable_add_child;
}

static void
hdy_leaflet_switch_child (HdySwipeable *swipeable,
                          guint         index,
                          gint64        duration)
{
  HdyLeaflet *self = HDY_LEAFLET (swipeable);
  HdyLeafletPage *page = NULL;
  GList *l;
  guint i = 0;

  for (l = self->children; l; l = l->next) {
    page = l->data;

    if (!page->navigatable)
      continue;

    if (i == index)
      break;

    i++;
  }

  if (page == NULL) {
    g_critical ("Couldn't find eligible child with index %u", index);
    return;
  }

  set_visible_child (self, page, self->transition_type, duration, FALSE);
}

static HdySwipeTracker *
hdy_leaflet_get_swipe_tracker (HdySwipeable *swipeable)
{
  HdyLeaflet *self = HDY_LEAFLET (swipeable);

  return self->tracker;
}

static gdouble
hdy_leaflet_get_distance (HdySwipeable *swipeable)
{
  HdyLeaflet *self = HDY_LEAFLET (swipeable);

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    return gtk_widget_get_allocated_width (GTK_WIDGET (self));
  else
    return gtk_widget_get_allocated_height (GTK_WIDGET (self));
}

static gdouble *
hdy_leaflet_get_snap_points (HdySwipeable *swipeable,
                             gint         *n_snap_points)
{
  HdyLeaflet *self = HDY_LEAFLET (swipeable);
  gint n;
  gdouble *points, lower, upper;

  if (self->child_transition.tick_id > 0 ||
      self->child_transition.is_gesture_active) {
    gint current_direction;
    gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

    switch (self->child_transition.active_direction) {
    case GTK_PAN_DIRECTION_UP:
      current_direction = 1;
      break;
    case GTK_PAN_DIRECTION_DOWN:
      current_direction = -1;
      break;
    case GTK_PAN_DIRECTION_LEFT:
      current_direction = is_rtl ? -1 : 1;
      break;
    case GTK_PAN_DIRECTION_RIGHT:
      current_direction = is_rtl ? 1 : -1;
      break;
    default:
      g_assert_not_reached ();
    }

    lower = MIN (0, current_direction);
    upper = MAX (0, current_direction);
  } else {
    HdyLeafletPage *page = NULL;

    if ((can_swipe_in_direction (self, self->child_transition.swipe_direction) ||
         !self->child_transition.is_direct_swipe) && self->folded)
      page = find_swipeable_page (self, self->child_transition.swipe_direction);

    lower = MIN (0, page ? self->child_transition.swipe_direction : 0);
    upper = MAX (0, page ? self->child_transition.swipe_direction : 0);
  }

  n = (lower != upper) ? 2 : 1;

  points = g_new0 (gdouble, n);
  points[0] = lower;
  points[n - 1] = upper;

  if (n_snap_points)
    *n_snap_points = n;

  return points;
}

static gdouble
hdy_leaflet_get_progress (HdySwipeable *swipeable)
{
  HdyLeaflet *self = HDY_LEAFLET (swipeable);
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

static gdouble
hdy_leaflet_get_cancel_progress (HdySwipeable *swipeable)
{
  return 0;
}

static void
hdy_leaflet_get_swipe_area (HdySwipeable           *swipeable,
                            HdyNavigationDirection  navigation_direction,
                            gboolean                is_drag,
                            GdkRectangle           *rect)
{
  HdyLeaflet *self = HDY_LEAFLET (swipeable);
  gint width = gtk_widget_get_allocated_width (GTK_WIDGET (self));
  gint height = gtk_widget_get_allocated_height (GTK_WIDGET (self));
  gdouble progress = 0;

  rect->x = 0;
  rect->y = 0;
  rect->width = width;
  rect->height = height;

  if (!is_drag)
    return;

  if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_SLIDE)
    return;

  if (self->child_transition.is_gesture_active ||
      gtk_progress_tracker_get_state (&self->child_transition.tracker) != GTK_PROGRESS_STATE_AFTER)
    progress = self->child_transition.progress;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL) {
    gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

    if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER &&
         navigation_direction == HDY_NAVIGATION_DIRECTION_FORWARD) {
      rect->width = MAX (progress * width, HDY_SWIPE_BORDER);
      rect->x = is_rtl ? 0 : width - rect->width;
    } else if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER &&
               navigation_direction == HDY_NAVIGATION_DIRECTION_BACK) {
      rect->width = MAX (progress * width, HDY_SWIPE_BORDER);
      rect->x = is_rtl ? width - rect->width : 0;
    }
  } else {
    if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER &&
        navigation_direction == HDY_NAVIGATION_DIRECTION_FORWARD) {
      rect->height = MAX (progress * height, HDY_SWIPE_BORDER);
      rect->y = height - rect->height;
    } else if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_UNDER &&
               navigation_direction == HDY_NAVIGATION_DIRECTION_BACK) {
      rect->height = MAX (progress * height, HDY_SWIPE_BORDER);
      rect->y = 0;
    }
  }
}

static void
hdy_leaflet_swipeable_init (HdySwipeableInterface *iface)
{
  iface->switch_child = hdy_leaflet_switch_child;
  iface->get_swipe_tracker = hdy_leaflet_get_swipe_tracker;
  iface->get_distance = hdy_leaflet_get_distance;
  iface->get_snap_points = hdy_leaflet_get_snap_points;
  iface->get_progress = hdy_leaflet_get_progress;
  iface->get_cancel_progress = hdy_leaflet_get_cancel_progress;
  iface->get_swipe_area = hdy_leaflet_get_swipe_area;
}

/**
 * hdy_leaflet_page_get_child:
 * @self: a #HdyLeafletPage
 *
 * Returns the leaflet child to which @self belongs.
 *
 * Returns: (transfer none): the child to which @self belongs
 */
GtkWidget *
hdy_leaflet_page_get_child (HdyLeafletPage *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET_PAGE (self), NULL);

  return self->widget;
}

/**
 * hdy_leaflet_page_get_name:
 * @self: a #HdyLeafletPage
 *
 * Returns the current value of the #HdyLeafletPage:name property.
 *
 * Returns: (nullable): The value of the #HdyLeafletPage:name property.
 *   See hdy_leaflet_page_set_name() for details on how to set a new value.
 */
const char *
hdy_leaflet_page_get_name (HdyLeafletPage *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET_PAGE (self), NULL);

  return self->name;
}

/**
 * hdy_leaflet_page_set_name:
 * @self: a #HdyLeafletPage
 * @name: (transfer none): the new value to set
 *
 * Sets the new value of the #HdyLeafletPage:name property.
 * See also hdy_leaflet_page_get_name()
 */
void
hdy_leaflet_page_set_name (HdyLeafletPage *self,
                           const char     *name)
{
  HdyLeaflet *leaflet = NULL;

  g_return_if_fail (HDY_IS_LEAFLET_PAGE (self));

  if (self->widget &&
    gtk_widget_get_parent (self->widget) &&
    HDY_IS_LEAFLET (gtk_widget_get_parent (self->widget))) {
    GList *l;

    leaflet = HDY_LEAFLET (gtk_widget_get_parent (self->widget));

    for (l = leaflet->children; l; l = l->next) {
      HdyLeafletPage *page = l->data;

      if (self == page)
        continue;

      if (g_strcmp0 (page->name, name) == 0)
        {
          g_warning ("Duplicate child name in HdyLeaflet: %s", name);
          break;
        }
    }
  }

  if (name == self->name)
    return;

  g_free (self->name);
  self->name = g_strdup (name);
  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NAME]);

  if (leaflet && leaflet->visible_child == self)
    g_object_notify_by_pspec (G_OBJECT (leaflet), props[PROP_VISIBLE_CHILD_NAME]);
}

/**
 * hdy_leaflet_page_get_navigatable:
 * @self: a #HdyLeafletPage
 *
 * Gets whether the child can be navigated to when folded.
 *
 * See hdy_leaflet_page_set_navigatable() and #HdyLeafletPage:navigatable.
 *
 * Returns: %TRUE if @self is enabled, %FALSE otherwise
 */
gboolean
hdy_leaflet_page_get_navigatable (HdyLeafletPage *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET_PAGE (self), FALSE);

  return self->navigatable;
}

/**
 * hdy_leaflet_page_set_navigatable:
 * @self: a #HdyLeafletPage
 * @navigatable: %TRUE if the child can be navigated to when folded
 *
 * Sets whether the child can be navigated to when folded.
 * If %FALSE, the child will be ignored by hdy_leaflet_get_adjacent_child(),
 * hdy_leaflet_navigate(), and swipe gestures.
 *
 * This can be used used to prevent switching to widgets like separators.
 *
 * Sets the new value of the #HdyLeafletPage:navigatable property to
 * @navigatable.
 */
void
hdy_leaflet_page_set_navigatable (HdyLeafletPage *self,
                                  gboolean        navigatable)
{
  g_return_if_fail (HDY_IS_LEAFLET_PAGE (self));

  navigatable = !!navigatable;

  if (navigatable == self->navigatable)
    return;

  self->navigatable = navigatable;

  if (self->widget && gtk_widget_get_parent (self->widget)) {
    HdyLeaflet *leaflet = HDY_LEAFLET (gtk_widget_get_parent (self->widget));

    if (self == leaflet->visible_child)
      set_visible_child (leaflet, NULL, leaflet->transition_type,
                         leaflet->child_transition.duration, TRUE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_NAVIGATABLE]);
}

GtkWidget *
hdy_leaflet_new (void)
{
  return g_object_new (HDY_TYPE_LEAFLET, NULL);
}

/**
 * hdy_leaflet_append:
 * @self: a #HdyLeaflet
 * @child: the widget to add
 *
 * Adds a child to @self.
 *
 * Returns: (transfer none): the #HdyLeafletPage for @child
 */
HdyLeafletPage *
hdy_leaflet_append (HdyLeaflet *self,
                    GtkWidget  *child)
{
  GtkWidget *sibling;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  if (self->children)
    sibling = hdy_leaflet_page_get_child (g_list_last (self->children)->data);
  else
    sibling = NULL;

  return hdy_leaflet_insert_child_after (self, child, sibling);
}

/**
 * hdy_leaflet_prepend:
 * @self: a #HdyLeaflet
 * @child: the #GtkWidget to prepend
 *
 * Inserts @child at the first position in @self.
 *
 * Returns: (transfer none): the #HdyLeafletPage for @child
 *
 * Since: 1.1
 */
HdyLeafletPage *
hdy_leaflet_prepend (HdyLeaflet *self,
                     GtkWidget  *child)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);

  return hdy_leaflet_insert_child_after (self, child, NULL);
}

/**
 * hdy_leaflet_insert_child_after:
 * @self: a #HdyLeaflet
 * @child: the #GtkWidget to insert
 * @sibling: (nullable): the sibling after which to insert @child
 *
 * Inserts @child in the position after @sibling in the list of children.
 * If @sibling is %NULL, insert @child at the first position.
 *
 * Returns: (transfer none): the #HdyLeafletPage for @child
 *
 * Since: 1.1
 */
HdyLeafletPage *
hdy_leaflet_insert_child_after (HdyLeaflet *self,
                                GtkWidget  *child,
                                GtkWidget  *sibling)
{
  HdyLeafletPage *page;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);
  g_return_val_if_fail (sibling == NULL || GTK_IS_WIDGET (sibling), NULL);

  g_return_val_if_fail (gtk_widget_get_parent (child) == NULL, NULL);
  g_return_val_if_fail (sibling == NULL || gtk_widget_get_parent (sibling) == GTK_WIDGET (self), NULL);

  page = g_object_new (HDY_TYPE_LEAFLET_PAGE, NULL);
  page->widget = g_object_ref (child);

  add_page (self, page, find_page_for_widget (self, sibling));

  g_object_unref (page);

  return page;

}

/**
 * hdy_leaflet_reorder_child_after:
 * @self: a #HdyLeaflet
 * @child: the #GtkWidget to move, must be a child of @self
 * @sibling: (nullable): the sibling to move @child after, or %NULL
 *
 * Moves @child to the position after @sibling in the list of children.
 * If @sibling is %NULL, move @child to the first position.
 *
 * Since: 1.1
 */
void
hdy_leaflet_reorder_child_after (HdyLeaflet *self,
                                 GtkWidget  *child,
                                 GtkWidget  *sibling)
{
  HdyLeafletPage *child_page;
  HdyLeafletPage *sibling_page;
  gint sibling_page_pos;
  gint visible_child_pos_before_reorder;
  gint visible_child_pos_after_reorder;
  gint previous_position;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (sibling == NULL || GTK_IS_WIDGET (sibling));

  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));
  g_return_if_fail (sibling == NULL || gtk_widget_get_parent (sibling) == GTK_WIDGET (self));

  if (child == sibling)
    return;

  visible_child_pos_before_reorder = g_list_index (self->children, self->visible_child);
  previous_position = g_list_index (self->children, child) - 1;

  /* Cancel a gesture if there's one in progress */
  hdy_swipe_tracker_emit_end_swipe (self->tracker, 0, 0.0);

  child_page = find_page_for_widget (self, child);
  self->children = g_list_remove (self->children, child_page);
  self->children_reversed = g_list_remove (self->children_reversed, child_page);

  sibling_page = find_page_for_widget (self, sibling);
  sibling_page_pos = g_list_index (self->children, sibling_page);

  self->children =
    g_list_insert (self->children, child_page,
                   sibling_page_pos + 1);
  self->children_reversed =
    g_list_insert (self->children_reversed, child_page,
                   g_list_length (self->children) - sibling_page_pos - 1);

  if (self->pages) {
    /* Copied from gtk_list_list_model_item_moved() */
    guint position = g_list_index (self->children, child_page);
    guint min, max;

    if (previous_position < 0)
      previous_position = 0;
    else if (position > previous_position)
      previous_position++;

    if (position == previous_position)
      return;

    min = MIN (position, previous_position);
    max = MAX (position, previous_position) + 1;
    g_list_model_items_changed (G_LIST_MODEL (self->pages), min, max - min, max - min);
  }

  visible_child_pos_after_reorder = g_list_index (self->children, self->visible_child);

  if (visible_child_pos_before_reorder != visible_child_pos_after_reorder)
    hdy_swipeable_emit_child_switched (HDY_SWIPEABLE (self), visible_child_pos_after_reorder, 0);
}

/**
 * hdy_leaflet_remove:
 * @self: a #HdyLeaflet
 * @child: the child to remove
 *
 * Removes a child widget from @self.
 */
void
hdy_leaflet_remove (HdyLeaflet *self,
                    GtkWidget  *child)
{
  GList *l;
  guint position;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));

  for (l = self->children, position = 0; l; l = l->next, position++) {
    HdyLeafletPage *page = l->data;

    if (page->widget == child)
      break;
  }

  leaflet_remove (self, child, FALSE);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 1, 0);
}

/**
 * hdy_leaflet_get_page:
 * @self: a #HdyLeaflet
 * @child: a child of @self
 *
 * Returns the #HdyLeafletPage object for @child.
 *
 * Returns: (transfer none): the #HdyLeafletPage for @child
 */
HdyLeafletPage *
hdy_leaflet_get_page (HdyLeaflet *self,
                      GtkWidget  *child)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return find_page_for_widget (self, child);
}

/**
 * hdy_leaflet_get_folded:
 * @self: a #HdyLeaflet
 *
 * Gets whether @self is folded.
 *
 * Returns: whether @self is folded.
 */
gboolean
hdy_leaflet_get_folded (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  return self->folded;
}

/**
 * hdy_leaflet_set_homogeneous:
 * @self: a #HdyLeaflet
 * @folded: the fold
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
                             gboolean        folded,
                             GtkOrientation  orientation,
                             gboolean        homogeneous)
{
  g_return_if_fail (HDY_IS_LEAFLET (self));

  folded = !!folded;
  homogeneous = !!homogeneous;

  if (self->homogeneous[folded][orientation] == homogeneous)
    return;

  self->homogeneous[folded][orientation] = homogeneous;

  if (gtk_widget_get_visible (GTK_WIDGET (self)))
    gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[HOMOGENEOUS_PROP[folded][orientation]]);
}

/**
 * hdy_leaflet_get_homogeneous:
 * @self: a #HdyLeaflet
 * @folded: the fold
 * @orientation: the orientation
 *
 * Gets whether @self is homogeneous for the given fold and orientation.
 * See hdy_leaflet_set_homogeneous().
 *
 * Returns: whether @self is homogeneous for the given fold and orientation.
 */
gboolean
hdy_leaflet_get_homogeneous (HdyLeaflet     *self,
                             gboolean        folded,
                             GtkOrientation  orientation)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  folded = !!folded;

  return self->homogeneous[folded][orientation];
}

/**
 * hdy_leaflet_get_transition_type:
 * @self: a #HdyLeaflet
 *
 * Gets the type of animation that will be used
 * for transitions between modes and children in @self.
 *
 * Returns: the current transition type of @self
 *
 * Since: 0.0.12
 */
HdyLeafletTransitionType
hdy_leaflet_get_transition_type (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), HDY_LEAFLET_TRANSITION_TYPE_OVER);

  return self->transition_type;
}

/**
 * hdy_leaflet_set_transition_type:
 * @self: a #HdyLeaflet
 * @transition: the new transition type
 *
 * Sets the type of animation that will be used for transitions between modes
 * and children in @self.
 *
 * The transition type can be changed without problems at runtime, so it is
 * possible to change the animation based on the mode or child that is about to
 * become current.
 *
 * Since: 0.0.12
 */
void
hdy_leaflet_set_transition_type (HdyLeaflet               *self,
                                 HdyLeafletTransitionType  transition)
{
  GList *l;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (transition <= HDY_LEAFLET_TRANSITION_TYPE_SLIDE);

  if (self->transition_type == transition)
    return;

  self->transition_type = transition;

  for (l = self->children; l; l = l->next) {
    HdyLeafletPage *page = l->data;

    if (self->transition_type == HDY_LEAFLET_TRANSITION_TYPE_OVER)
      gtk_widget_insert_before (page->widget, GTK_WIDGET (self), NULL);
    else
      gtk_widget_insert_after (page->widget, GTK_WIDGET (self), NULL);
  }

  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_TRANSITION_TYPE]);
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
  g_return_val_if_fail (HDY_IS_LEAFLET (self), 0);

  return self->mode_transition.duration;
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
  g_return_if_fail (HDY_IS_LEAFLET (self));

  if (self->mode_transition.duration == duration)
    return;

  self->mode_transition.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_MODE_TRANSITION_DURATION]);
}

/**
 * hdy_leaflet_get_child_transition_duration:
 * @self: a #HdyLeaflet
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between children in @self will take.
 *
 * Returns: the child transition duration
 */
guint
hdy_leaflet_get_child_transition_duration (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), 0);

  return self->child_transition.duration;
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
  g_return_if_fail (HDY_IS_LEAFLET (self));

  if (self->child_transition.duration == duration)
    return;

  self->child_transition.duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self),
                            props[PROP_CHILD_TRANSITION_DURATION]);
}

/**
 * hdy_leaflet_get_visible_child:
 * @self: a #HdyLeaflet
 *
 * Gets the visible child widget.
 *
 * Returns: (transfer none): the visible child widget
 */
GtkWidget *
hdy_leaflet_get_visible_child (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);

  if (self->visible_child == NULL)
    return NULL;

  return self->visible_child->widget;
}

/**
 * hdy_leaflet_set_visible_child:
 * @self: a #HdyLeaflet
 * @visible_child: the new child
 *
 * Makes @visible_child visible using a transition determined by
 * HdyLeaflet:transition-type and HdyLeaflet:child-transition-duration. The
 * transition can be cancelled by the user, in which case visible child will
 * change back to the previously visible child.
 */
void
hdy_leaflet_set_visible_child (HdyLeaflet *self,
                               GtkWidget  *visible_child)
{
  HdyLeafletPage *page;
  gboolean contains_child;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (GTK_IS_WIDGET (visible_child));

  page = find_page_for_widget (self, visible_child);

  contains_child = page != NULL;

  g_return_if_fail (contains_child);

  set_visible_child (self, page, self->transition_type, self->child_transition.duration, TRUE);
}

/**
 * hdy_leaflet_get_visible_child_name:
 * @self: a #HdyLeaflet
 *
 * Gets the name of the currently visible child widget.
 *
 * Returns: (transfer none): the name of the visible child
 */
const gchar *
hdy_leaflet_get_visible_child_name (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);

  if (self->visible_child == NULL)
    return NULL;

  return self->visible_child->name;
}

/**
 * hdy_leaflet_set_visible_child_name:
 * @self: a #HdyLeaflet
 * @name: the name of a child
 *
 * Makes the child with the name @name visible.
 *
 * See hdy_leaflet_set_visible_child() for more details.
 */
void
hdy_leaflet_set_visible_child_name (HdyLeaflet  *self,
                                    const gchar *name)
{
  HdyLeafletPage *page;
  gboolean contains_child;

  g_return_if_fail (HDY_IS_LEAFLET (self));
  g_return_if_fail (name != NULL);

  page = find_page_for_name (self, name);
  contains_child = page != NULL;

  g_return_if_fail (contains_child);

  set_visible_child (self, page, self->transition_type, self->child_transition.duration, TRUE);
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
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  return (self->child_transition.tick_id != 0 ||
          self->child_transition.is_gesture_active);
}

/**
 * hdy_leaflet_set_interpolate_size:
 * @self: a #HdyLeaflet
 * @interpolate_size: the new value
 *
 * Sets whether or not @self will interpolate its size when
 * changing the visible child. If the #HdyLeaflet:interpolate-size
 * property is set to %TRUE, @self will interpolate its size between
 * the current one and the one it'll take after changing the
 * visible child, according to the set transition duration.
 */
void
hdy_leaflet_set_interpolate_size (HdyLeaflet *self,
                                  gboolean    interpolate_size)
{
  g_return_if_fail (HDY_IS_LEAFLET (self));

  interpolate_size = !!interpolate_size;

  if (self->child_transition.interpolate_size == interpolate_size)
    return;

  self->child_transition.interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATE_SIZE]);
}

/**
 * hdy_leaflet_get_interpolate_size:
 * @self: a #HdyLeaflet
 *
 * Returns whether the #HdyLeaflet is set up to interpolate between
 * the sizes of children on page switch.
 *
 * Returns: %TRUE if child sizes are interpolated
 */
gboolean
hdy_leaflet_get_interpolate_size (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  return self->child_transition.interpolate_size;
}

/**
 * hdy_leaflet_set_can_swipe_back:
 * @self: a #HdyLeaflet
 * @can_swipe_back: the new value
 *
 * Sets whether or not @self allows switching to the previous child that has
 * 'navigatable' child property set to %TRUE via a swipe gesture
 *
 * Since: 0.0.12
 */
void
hdy_leaflet_set_can_swipe_back (HdyLeaflet *self,
                                gboolean    can_swipe_back)
{
  g_return_if_fail (HDY_IS_LEAFLET (self));

  can_swipe_back = !!can_swipe_back;

  if (self->child_transition.can_swipe_back == can_swipe_back)
    return;

  self->child_transition.can_swipe_back = can_swipe_back;
  hdy_swipe_tracker_set_enabled (self->tracker, can_swipe_back || self->child_transition.can_swipe_forward);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SWIPE_BACK]);
}

/**
 * hdy_leaflet_get_can_swipe_back
 * @self: a #HdyLeaflet
 *
 * Returns whether the #HdyLeaflet allows swiping to the previous child.
 *
 * Returns: %TRUE if back swipe is enabled.
 *
 * Since: 0.0.12
 */
gboolean
hdy_leaflet_get_can_swipe_back (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  return self->child_transition.can_swipe_back;
}

/**
 * hdy_leaflet_set_can_swipe_forward:
 * @self: a #HdyLeaflet
 * @can_swipe_forward: the new value
 *
 * Sets whether or not @self allows switching to the next child that has
 * 'navigatable' child property set to %TRUE via a swipe gesture.
 *
 * Since: 0.0.12
 */
void
hdy_leaflet_set_can_swipe_forward (HdyLeaflet *self,
                                   gboolean    can_swipe_forward)
{
  g_return_if_fail (HDY_IS_LEAFLET (self));

  can_swipe_forward = !!can_swipe_forward;

  if (self->child_transition.can_swipe_forward == can_swipe_forward)
    return;

  self->child_transition.can_swipe_forward = can_swipe_forward;
  hdy_swipe_tracker_set_enabled (self->tracker, self->child_transition.can_swipe_back || can_swipe_forward);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SWIPE_FORWARD]);
}

/**
 * hdy_leaflet_get_can_swipe_forward
 * @self: a #HdyLeaflet
 *
 * Returns whether the #HdyLeaflet allows swiping to the next child.
 *
 * Returns: %TRUE if forward swipe is enabled.
 *
 * Since: 0.0.12
 */
gboolean
hdy_leaflet_get_can_swipe_forward (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  return self->child_transition.can_swipe_forward;
}

/**
 * hdy_leaflet_get_adjacent_child
 * @self: a #HdyLeaflet
 * @direction: the direction
 *
 * Gets the previous or next child that doesn't have 'navigatable' child
 * property set to %FALSE, or %NULL if it doesn't exist. This will be the same
 * widget hdy_leaflet_navigate() will navigate to.
 *
 * Returns: (nullable) (transfer none): the previous or next child, or
 *   %NULL if it doesn't exist.
 *
 * Since: 1.0
 */
GtkWidget *
hdy_leaflet_get_adjacent_child (HdyLeaflet             *self,
                                HdyNavigationDirection  direction)
{
  HdyLeafletPage *page;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);

  page = find_swipeable_page (self, direction);

  return page ? page->widget : NULL;
}

/**
 * hdy_leaflet_navigate
 * @self: a #HdyLeaflet
 * @direction: the direction
 *
 * Switches to the previous or next child that doesn't have 'navigatable' child
 * property set to %FALSE, similar to performing a swipe gesture to go in
 * @direction.
 *
 * Returns: %TRUE if visible child was changed, %FALSE otherwise.
 *
 * Since: 1.0
 */
gboolean
hdy_leaflet_navigate (HdyLeaflet             *self,
                      HdyNavigationDirection  direction)
{
  HdyLeafletPage *page;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  page = find_swipeable_page (self, direction);

  if (!page)
    return FALSE;

  set_visible_child (self, page, self->transition_type, self->child_transition.duration, TRUE);

  return TRUE;
}

/**
 * hdy_leaflet_get_child_by_name:
 * @self: a #HdyLeaflet
 * @name: the name of the child to find
 *
 * Finds the child of @self with the name given as the argument. Returns %NULL
 * if there is no child with this name.
 *
 * Returns: (transfer none) (nullable): the requested child of @self
 *
 * Since: 1.0
 */
GtkWidget *
hdy_leaflet_get_child_by_name (HdyLeaflet  *self,
                               const gchar *name)
{
  HdyLeafletPage *page;

  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  page = find_page_for_name (self, name);

  return page ? page->widget : NULL;
}

void
hdy_leaflet_set_can_unfold (HdyLeaflet *self,
                            gboolean    can_unfold)
{
  g_return_if_fail (HDY_IS_LEAFLET (self));

  can_unfold = !!can_unfold;

  if (self->can_unfold == can_unfold)
    return;

  self->can_unfold = can_unfold;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_UNFOLD]);
}

gboolean
hdy_leaflet_get_can_unfold (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), FALSE);

  return self->can_unfold;
}

/**
 * hdy_leaflet_get_pages:
 * @self: a #HdyLeaflet
 *
 * Returns a #GListModel that contains the pages of the leaflet, and can be
 * used to keep an up-to-date view. The model also implements #GtkSelectionModel
 * and can be used to track the visible page.
 *
 * Returns: (transfer full): a #GtkSelectionModel for the leaflet's children
 */
GtkSelectionModel *
hdy_leaflet_get_pages (HdyLeaflet *self)
{
  g_return_val_if_fail (HDY_IS_LEAFLET (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  self->pages = GTK_SELECTION_MODEL (hdy_leaflet_pages_new (self));
  g_object_add_weak_pointer (G_OBJECT (self->pages), (gpointer *) &self->pages);

  return self->pages;
}
