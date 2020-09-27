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

struct _HdySqueezerPage {
  GObject parent_instance;

  GtkWidget *widget;
  GtkWidget *last_focus;
  gboolean enabled;
};

G_DEFINE_TYPE (HdySqueezerPage, hdy_squeezer_page, G_TYPE_OBJECT)

enum {
  PAGE_PROP_0,
  PAGE_PROP_CHILD,
  PAGE_PROP_ENABLED,
  LAST_PAGE_PROP
};

static GParamSpec *page_props[LAST_PAGE_PROP];

struct _HdySqueezer
{
  GtkWidget parent_instance;

  GList *children;

  HdySqueezerPage *visible_child;

  gboolean homogeneous;

  HdySqueezerTransitionType transition_type;
  guint transition_duration;

  HdySqueezerPage *last_visible_child;
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

  GtkSelectionModel *pages;
};

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
  PROP_PAGES,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_PAGES + 1,
};

static GParamSpec *props[LAST_PROP];

static void hdy_squeezer_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdySqueezer, hdy_squeezer, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, hdy_squeezer_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static void
hdy_squeezer_page_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdySqueezerPage *self = HDY_SQUEEZER_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_value_set_object (value, hdy_squeezer_page_get_child (self));
    break;
  case PAGE_PROP_ENABLED:
    g_value_set_boolean (value, hdy_squeezer_page_get_enabled (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_squeezer_page_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdySqueezerPage *self = HDY_SQUEEZER_PAGE (object);

  switch (property_id) {
  case PAGE_PROP_CHILD:
    g_set_object (&self->widget, g_value_get_object (value));
    break;
  case PAGE_PROP_ENABLED:
    hdy_squeezer_page_set_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_squeezer_page_finalize (GObject *object)
{
  HdySqueezerPage *self = HDY_SQUEEZER_PAGE (object);

  g_clear_object (&self->widget);

  if (self->last_focus)
    g_object_remove_weak_pointer (G_OBJECT (self->last_focus),
                                  (gpointer *) &self->last_focus);

  G_OBJECT_CLASS (hdy_squeezer_page_parent_class)->finalize (object);
}

static void
hdy_squeezer_page_class_init (HdySqueezerPageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = hdy_squeezer_page_get_property;
  object_class->set_property = hdy_squeezer_page_set_property;
  object_class->finalize = hdy_squeezer_page_finalize;

  page_props[PAGE_PROP_CHILD] =
    g_param_spec_object ("child",
                         _("Child"),
                         _("The child of the page"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  page_props[PAGE_PROP_ENABLED] =
    g_param_spec_boolean ("enabled",
                          _("Enabled"),
                          _("Whether the child can be picked or should be ignored when looking for the child fitting the available size best"),
                          TRUE,
                          G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PAGE_PROP, page_props);
}

static void
hdy_squeezer_page_init (HdySqueezerPage *self)
{
  self->enabled = TRUE;
}

#define HDY_TYPE_SQUEEZER_PAGES (hdy_squeezer_pages_get_type ())

G_DECLARE_FINAL_TYPE (HdySqueezerPages, hdy_squeezer_pages, HDY, SQUEEZER_PAGES, GObject)

struct _HdySqueezerPages
{
  GObject parent_instance;
  HdySqueezer *squeezer;
};

static GType
hdy_squeezer_pages_get_item_type (GListModel *model)
{
  return HDY_TYPE_SQUEEZER_PAGE;
}

static guint
hdy_squeezer_pages_get_n_items (GListModel *model)
{
  HdySqueezerPages *self = HDY_SQUEEZER_PAGES (model);

  return g_list_length (self->squeezer->children);
}

static gpointer
hdy_squeezer_pages_get_item (GListModel *model,
                             guint       position)
{
  HdySqueezerPages *self = HDY_SQUEEZER_PAGES (model);
  HdySqueezerPage *page;

  page = g_list_nth_data (self->squeezer->children, position);

  return g_object_ref (page);
}

static void
hdy_squeezer_pages_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = hdy_squeezer_pages_get_item_type;
  iface->get_n_items = hdy_squeezer_pages_get_n_items;
  iface->get_item = hdy_squeezer_pages_get_item;
}

static gboolean
hdy_squeezer_pages_is_selected (GtkSelectionModel *model,
                                guint              position)
{
  HdySqueezerPages *self = HDY_SQUEEZER_PAGES (model);
  HdySqueezerPage *page;

  page = g_list_nth_data (self->squeezer->children, position);

  return page == self->squeezer->visible_child;
}

static void
hdy_squeezer_pages_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = hdy_squeezer_pages_is_selected;
}

G_DEFINE_TYPE_WITH_CODE (HdySqueezerPages, hdy_squeezer_pages, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, hdy_squeezer_pages_list_model_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, hdy_squeezer_pages_selection_model_init))

static void
hdy_squeezer_pages_init (HdySqueezerPages *pages)
{
}

static void
hdy_squeezer_pages_class_init (HdySqueezerPagesClass *klass)
{
}

static HdySqueezerPages *
hdy_squeezer_pages_new (HdySqueezer *squeezer)
{
  HdySqueezerPages *pages;

  pages = g_object_new (HDY_TYPE_SQUEEZER_PAGES, NULL);
  pages->squeezer = squeezer;

  return pages;
}

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

static HdySqueezerPage *
find_page_for_widget (HdySqueezer *self,
                      GtkWidget   *child)
{
  HdySqueezerPage *page;
  GList *l;

  for (l = self->children; l != NULL; l = l->next) {
    page = l->data;
    if (page->widget == child)
      return page;
  }

  return NULL;
}

static void
hdy_squeezer_progress_updated (HdySqueezer *self)
{
  if (!self->homogeneous)
    gtk_widget_queue_resize (GTK_WIDGET (self));
  else
    gtk_widget_queue_draw (GTK_WIDGET (self));

  if (gtk_progress_tracker_get_state (&self->tracker) == GTK_PROGRESS_STATE_AFTER &&
      self->last_visible_child != NULL) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
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
                   HdySqueezerPage           *page,
                   HdySqueezerTransitionType  transition_type,
                   guint                      transition_duration)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GtkRoot *root;
  GtkWidget *focus;
  gboolean contains_focus = FALSE;
  guint old_pos = GTK_INVALID_LIST_POSITION;
  guint new_pos = GTK_INVALID_LIST_POSITION;

  /* If we are being destroyed, do not bother with transitions and
   * notifications.
   */
  if (gtk_widget_in_destruction (widget))
    return;

  /* If none, pick the first visible. */
  if (!page) {
    GList *l;

    for (l = self->children; l; l = l->next) {
      HdySqueezerPage *p = l->data;
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
      HdySqueezerPage *p = l->data;
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

  if (self->last_visible_child != NULL)
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
  self->last_visible_child = NULL;

  if (self->visible_child && self->visible_child->widget) {
    if (gtk_widget_is_visible (widget)) {
      self->last_visible_child = self->visible_child;
      self->last_visible_widget_width = gtk_widget_get_width (self->last_visible_child->widget);
      self->last_visible_widget_height = gtk_widget_get_height (self->last_visible_child->widget);
    } else {
      gtk_widget_set_child_visible (self->visible_child->widget, FALSE);
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

  if (self->homogeneous)
    gtk_widget_queue_allocate (widget);
  else
    gtk_widget_queue_resize (widget);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISIBLE_CHILD]);

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

  hdy_squeezer_start_transition (self, transition_type, transition_duration);
}

static void
update_child_visible (HdySqueezer     *self,
                      HdySqueezerPage *page)
{
  gboolean enabled;

  enabled = page->enabled && gtk_widget_get_visible (page->widget);

  if (self->visible_child == NULL && enabled)
    set_visible_child (self, page, self->transition_type, self->transition_duration);
  else if (self->visible_child == page && !enabled)
    set_visible_child (self, NULL, self->transition_type, self->transition_duration);

  if (page == self->last_visible_child) {
    gtk_widget_set_child_visible (self->last_visible_child->widget, FALSE);
    self->last_visible_child = NULL;
  }
}

static void
stack_child_visibility_notify_cb (GObject    *obj,
                                  GParamSpec *pspec,
                                  gpointer    user_data)
{
  HdySqueezer *self = HDY_SQUEEZER (user_data);
  GtkWidget *child = GTK_WIDGET (obj);
  HdySqueezerPage *page;

  page = find_page_for_widget (self, child);
  g_return_if_fail (page != NULL);

  update_child_visible (self, page);
}

static void
add_page (HdySqueezer     *self,
          HdySqueezerPage *page)
{
  g_return_if_fail (page->widget != NULL);

  self->children = g_list_append (self->children, g_object_ref (page));

  gtk_widget_set_child_visible (page->widget, FALSE);
  gtk_widget_set_parent (page->widget, GTK_WIDGET (self));

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), g_list_length (self->children) - 1, 0, 1);

  g_signal_connect (page->widget, "notify::visible",
                    G_CALLBACK (stack_child_visibility_notify_cb), self);

  if (self->visible_child == NULL &&
      gtk_widget_get_visible (page->widget))
    set_visible_child (self, page, self->transition_type, self->transition_duration);

  if (self->homogeneous || self->visible_child == page)
    gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
squeezer_remove (HdySqueezer *self,
                 GtkWidget   *child,
                 gboolean     in_dispose)
{
  HdySqueezerPage *page;
  gboolean was_visible;

  page = find_page_for_widget (self, child);
  if (!page)
    return;

  self->children = g_list_remove (self->children, page);

  g_signal_handlers_disconnect_by_func (child,
                                        stack_child_visibility_notify_cb,
                                        self);

  was_visible = gtk_widget_get_visible (child);

  g_clear_object (&page->widget);

  if (self->visible_child == page)
    {
      if (in_dispose)
        self->visible_child = NULL;
      else
        set_visible_child (self, NULL, self->transition_type, self->transition_duration);
    }

  if (self->last_visible_child == page)
    self->last_visible_child = NULL;

  gtk_widget_unparent (child);

  g_object_unref (page);

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
  case PROP_PAGES:
    g_value_set_object (value, hdy_squeezer_get_pages (self));
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
hdy_squeezer_compute_expand (GtkWidget *widget,
                             gboolean  *hexpand_p,
                             gboolean  *vexpand_p)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  gboolean hexpand, vexpand;
  HdySqueezerPage *page;
  GtkWidget *child;
  GList *l;

  hexpand = FALSE;
  vexpand = FALSE;
  for (l = self->children; l != NULL; l = l->next) {
    page = l->data;
    child = page->widget;

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
hdy_squeezer_snapshot_crossfade (GtkWidget   *widget,
                                 GtkSnapshot *snapshot)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  gdouble progress = gtk_progress_tracker_get_progress (&self->tracker, FALSE);

  gtk_snapshot_push_cross_fade (snapshot, progress);

  if (self->last_visible_child) {
    gint width_diff = gtk_widget_get_width (widget) - self->last_visible_widget_width;
    gint height_diff = gtk_widget_get_height (widget) - self->last_visible_widget_height;

    gtk_snapshot_translate (snapshot,
                            &GRAPHENE_POINT_INIT (
                              width_diff * self->xalign,
                              height_diff * self->yalign
                            ));

    gtk_widget_snapshot_child (widget,
                               self->last_visible_child->widget,
                               snapshot);
  }

  gtk_snapshot_pop (snapshot);

  gtk_widget_snapshot_child (widget,
                             self->visible_child->widget,
                             snapshot);
  gtk_snapshot_pop (snapshot);
}


static void
hdy_squeezer_snapshot (GtkWidget   *widget,
                       GtkSnapshot *snapshot)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);

  if (self->visible_child) {
    if (gtk_progress_tracker_get_state (&self->tracker) != GTK_PROGRESS_STATE_AFTER) {
      gtk_snapshot_push_clip (snapshot,
                              &GRAPHENE_RECT_INIT(
                                  0, 0,
                                  gtk_widget_get_width (widget),
                                  gtk_widget_get_height (widget)
                              ));

      switch (self->active_transition_type)
        {
        case GTK_STACK_TRANSITION_TYPE_CROSSFADE:
          hdy_squeezer_snapshot_crossfade (widget, snapshot);
          break;
        case GTK_STACK_TRANSITION_TYPE_NONE:
        default:
          g_assert_not_reached ();
        }

      gtk_snapshot_pop (snapshot);
    } else
      gtk_widget_snapshot_child (widget,
                                 self->visible_child->widget,
                                 snapshot);
  }
}

static void
hdy_squeezer_size_allocate (GtkWidget *widget,
                            gint       width,
                            gint       height,
                            gint       baseline)
{
  HdySqueezer *self = HDY_SQUEEZER (widget);
  HdySqueezerPage *page = NULL;
  GtkWidget *child = NULL;
  gint child_min;
  GList *l;
  GtkAllocation child_allocation;

  for (l = self->children; l; l = l->next) {
    gint for_size = -1;

    page = l->data;
    child = page->widget;

    if (!gtk_widget_get_visible (child))
      continue;

    if (!page->enabled)
      continue;

    if (self->orientation == GTK_ORIENTATION_VERTICAL) {
      if (gtk_widget_get_request_mode (child) == GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH)
        for_size = width;

      gtk_widget_measure (child, self->orientation, for_size,
                          &child_min, NULL, NULL, NULL);

      if (child_min <= height)
        break;
    } else {
      if (gtk_widget_get_request_mode (child) == GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT)
        for_size = height;

      gtk_widget_measure (child, self->orientation, for_size,
                          &child_min, NULL, NULL, NULL);

      if (child_min <= width)
        break;
    }
  }

  set_visible_child (self, page,
                     self->transition_type,
                     self->transition_duration);

  child_allocation.x = 0;
  child_allocation.y = 0;

  if (self->last_visible_child) {
    gint min, nat;

    gtk_widget_measure (self->last_visible_child->widget, GTK_ORIENTATION_HORIZONTAL,
                        -1, &min, &nat, NULL, NULL);
    child_allocation.width = MAX (min, width);
    gtk_widget_measure (self->last_visible_child->widget, GTK_ORIENTATION_VERTICAL,
                        child_allocation.width, &min, &nat, NULL, NULL);
    child_allocation.height = MAX (min, height);

    gtk_widget_size_allocate (self->last_visible_child->widget, &child_allocation, -1);
  }

  child_allocation.width = width;
  child_allocation.height = height;

  if (self->visible_child) {
    gint min, nat;
    GtkAlign valign;

    gtk_widget_measure (self->visible_child->widget, GTK_ORIENTATION_VERTICAL,
                        width, &min, &nat, NULL, NULL);

    if (self->interpolate_size) {
      valign = gtk_widget_get_valign (self->visible_child->widget);
      child_allocation.height = MAX (nat, height);
      if (valign == GTK_ALIGN_END &&
          child_allocation.height > height)
        child_allocation.y -= nat - height;
      else if (valign == GTK_ALIGN_CENTER &&
               child_allocation.height > height)
        child_allocation.y -= (nat - height) / 2;
    }

    gtk_widget_size_allocate (self->visible_child->widget, &child_allocation, -1);
  }
}

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
  gint child_min, child_nat;
  GList *l;
  gint min = 0, nat = 0;

  for (l = self->children; l != NULL; l = l->next) {
    HdySqueezerPage *page = l->data;
    GtkWidget *child = page->widget;

    if (self->orientation != orientation && !self->homogeneous &&
        self->visible_child != page)
      continue;

    if (!gtk_widget_get_visible (child))
      continue;

    /* Disabled children are taken into account when measuring the widget, to
     * keep its size request and allocation consistent. This avoids the
     * appearant size and position of a child to changes suddenly when a larger
     * child gets enabled/disabled.
     */
    gtk_widget_measure (child, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);

    if (self->orientation == orientation)
      min = min == 0 ? child_min : MIN (min, child_min);
    else
      min = MAX (min, child_min);

    nat = MAX (nat, child_nat);
  }

  if (self->orientation != orientation && !self->homogeneous &&
      self->interpolate_size &&
      self->last_visible_child != NULL) {
    gdouble t = gtk_progress_tracker_get_ease_out_cubic (&self->tracker, FALSE);

    if (orientation == GTK_ORIENTATION_VERTICAL) {
      min = hdy_lerp (self->last_visible_widget_height, min, t);
      nat = hdy_lerp (self->last_visible_widget_height, nat, t);
    } else {
      min = hdy_lerp (self->last_visible_widget_width, min, t);
      nat = hdy_lerp (self->last_visible_widget_width, nat, t);
    }
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
hdy_squeezer_dispose (GObject *object)
{
  HdySqueezer *self = HDY_SQUEEZER (object);
  GtkWidget *child;

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), 0,
                                g_list_length (self->children), 0);

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (self))))
    squeezer_remove (self, child, TRUE);

  G_OBJECT_CLASS (hdy_squeezer_parent_class)->dispose (object);
}

static void
hdy_squeezer_finalize (GObject *object)
{
  HdySqueezer *self = HDY_SQUEEZER (object);

  if (self->pages)
    g_object_remove_weak_pointer (G_OBJECT (self->pages),
                                  (gpointer *) &self->pages);

  hdy_squeezer_unschedule_ticks (self);

  G_OBJECT_CLASS (hdy_squeezer_parent_class)->finalize (object);
}

static void
hdy_squeezer_class_init (HdySqueezerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_squeezer_get_property;
  object_class->set_property = hdy_squeezer_set_property;
  object_class->dispose = hdy_squeezer_dispose;
  object_class->finalize = hdy_squeezer_finalize;

  widget_class->size_allocate = hdy_squeezer_size_allocate;
  widget_class->snapshot = hdy_squeezer_snapshot;
  widget_class->measure = hdy_squeezer_measure;
  widget_class->compute_expand = hdy_squeezer_compute_expand;

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
   * The xalign property determines the horizontal alignment of the children
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
   * The yalign property determines the vertical alignment of the children inside
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

  props[PROP_PAGES] =
    g_param_spec_object ("pages",
                         _("Pages"),
                         _("A selection model with the squeezer's pages"),
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "squeezer");
}

static void
hdy_squeezer_init (HdySqueezer *self)
{
  self->homogeneous = TRUE;
  self->transition_duration = 200;
  self->transition_type = HDY_SQUEEZER_TRANSITION_TYPE_NONE;
  self->xalign = 0.5;
  self->yalign = 0.5;
}

static void
hdy_squeezer_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  if (GTK_IS_STACK_PAGE (child))
    add_page (HDY_SQUEEZER (buildable), HDY_SQUEEZER_PAGE (child));
  else if (GTK_IS_WIDGET (child))
    hdy_squeezer_add (HDY_SQUEEZER (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
hdy_squeezer_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = hdy_squeezer_buildable_add_child;
}

/**
 * hdy_squeezer_page_get_child:
 * @self: a #HdySqueezerPage
 *
 * Returns the squeezer child to which @self belongs.
 *
 * Returns: (transfer none): the child to which @self belongs
 */
GtkWidget *
hdy_squeezer_page_get_child (HdySqueezerPage *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER_PAGE (self), NULL);

  return self->widget;
}

/**
 * hdy_squeezer_page_get_enabled:
 * @self: a #HdySqueezerPage
 *
 * Returns whether @self is enabled in its #HdySqueezer. This is independent
 * from the #GtkWidget:visible value of its #GtkWidget.
 *
 * See hdy_squeezer_page_set_enabled().
 *
 * Returns: %TRUE if @self is enabled, %FALSE otherwise
 */
gboolean
hdy_squeezer_page_get_enabled (HdySqueezerPage *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER_PAGE (self), FALSE);

  return self->enabled;
}

/**
 * hdy_squeezer_page_set_enabled:
 * @self: a #HdySqueezerPage
 * @enabled: %TRUE to enable the child, %FALSE to disable it
 *
 * Make the squeezer enable or disable @child. If a child is disabled, it will
 * be ignored when looking for the child fitting the available size best. This
 * allows to programmatically and prematurely hide a child even if it fits in
 * the available space.
 *
 * This can be used e.g. to ensure a certain child is hidden below a certain
 * window width, or any other constraint you find suitable.
 *
 * Sets the new value of the #HdySqueezerPage:enabled property to @enabled.
 */
void
hdy_squeezer_page_set_enabled (HdySqueezerPage *self,
                               gboolean         enabled)
{
  g_return_if_fail (HDY_IS_SQUEEZER_PAGE (self));

  enabled = !!enabled;

  if (enabled == self->enabled)
    return;

  self->enabled = enabled;

  if (self->widget && gtk_widget_get_parent (self->widget)) {
    HdySqueezer *squeezer = HDY_SQUEEZER (gtk_widget_get_parent (self->widget));

    gtk_widget_queue_resize (GTK_WIDGET (squeezer));
    update_child_visible (squeezer, self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), page_props[PAGE_PROP_ENABLED]);
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
 * hdy_squeezer_add:
 * @self: a #HdySqueezer
 * @child: the widget to add
 *
 * Adds a child to @self.
 *
 * Returns: (transfer none): the #HdySqueezerPage for @child
 */
HdySqueezerPage *
hdy_squeezer_add (HdySqueezer *self,
                  GtkWidget   *child)
{
  HdySqueezerPage *page;

  g_return_val_if_fail (HDY_IS_SQUEEZER (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  page = g_object_new (HDY_TYPE_SQUEEZER_PAGE,
                       "child", child,
                       NULL);

  add_page (self, page);

  g_object_unref (page);

  return page;
}

/**
 * hdy_squeezer_remove:
 * @self: a #HdySqueezer
 * @child: the child to remove
 *
 * Removes a child widget from @self.
 */
void
hdy_squeezer_remove (HdySqueezer *self,
                     GtkWidget   *child)
{
  GList *l;
  guint position;

  g_return_if_fail (HDY_IS_SQUEEZER (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));

  for (l = self->children, position = 0; l; l = l->next, position++) {
    HdySqueezerPage *page = l->data;

    if (page->widget == child)
      break;
  }

  squeezer_remove (self, child, FALSE);

  if (self->pages)
    g_list_model_items_changed (G_LIST_MODEL (self->pages), position, 1, 0);
}

/**
 * hdy_squeezer_get_page:
 * @self: a #HdySqueezer
 * @child: a child of @self
 *
 * Returns the #HdySqueezerPage object for @child.
 *
 * Returns: (transfer none): the #HdySqueezerPage for @child
 */
HdySqueezerPage *
hdy_squeezer_get_page (HdySqueezer *self,
                       GtkWidget   *child)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (child), NULL);

  return find_page_for_widget (self, child);
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

/**
 * hdy_squeezer_get_pages:
 * @self: a #HdySqueezer
 *
 * Returns a #GListModel that contains the pages of the squeezer, and can be
 * used to keep an up-to-date view. The model also implements #GtkSelectionModel
 * and can be used to track the visible page.
 *
 * Returns: (transfer full): a #GtkSelectionModel for the squeezer's children
 */
GtkSelectionModel *
hdy_squeezer_get_pages (HdySqueezer *self)
{
  g_return_val_if_fail (HDY_IS_SQUEEZER (self), NULL);

  if (self->pages)
    return g_object_ref (self->pages);

  self->pages = GTK_SELECTION_MODEL (hdy_squeezer_pages_new (self));
  g_object_add_weak_pointer (G_OBJECT (self->pages), (gpointer *) &self->pages);

  return self->pages;
}
