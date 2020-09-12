/*
 * Copyright (C) 2020-2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-tab-box-private.h"
#include "adw-animation-private.h"
#include "adw-tab-private.h"
#include "adw-tab-bar-private.h"
#include "adw-tab-view-private.h"
#include <math.h>

/* Border collapsing without glitches */
#define OVERLAP 1
#define DND_THRESHOLD_MULTIPLIER 4
#define DROP_SWITCH_TIMEOUT 500

#define AUTOSCROLL_SPEED 2.5

#define OPEN_ANIMATION_DURATION 200
#define CLOSE_ANIMATION_DURATION 200
#define FOCUS_ANIMATION_DURATION 200
#define SCROLL_ANIMATION_DURATION 200
#define RESIZE_ANIMATION_DURATION 200
#define REORDER_ANIMATION_DURATION 250
#define ICON_RESIZE_ANIMATION_DURATION 200

#define MAX_TAB_WIDTH_NON_EXPAND 220

typedef enum {
  TAB_RESIZE_NORMAL,
  TAB_RESIZE_FIXED_TAB_WIDTH,
  TAB_RESIZE_FIXED_END_PADDING
} TabResizeMode;

typedef struct {
  GdkDrag *drag;

  AdwTab *tab;
  GtkBorder tab_margin;

  int hotspot_x;
  int hotspot_y;

  int width;
  int target_width;
  AdwAnimation *resize_animation;
} DragIcon;

typedef struct {
  AdwTabPage *page;
  AdwTab *tab;

  int pos;
  int width;
  int last_width;

  double end_reorder_offset;
  double reorder_offset;

  AdwAnimation *reorder_animation;
  gboolean reorder_ignore_bounds;

  double appear_progress;
  AdwAnimation *appear_animation;

  gulong notify_needs_attention_id;
} TabInfo;

struct _AdwTabBox
{
  GtkWidget parent_instance;

  gboolean pinned;
  AdwTabBar *tab_bar;
  AdwTabView *view;
  GtkAdjustment *adjustment;
  gboolean needs_attention_left;
  gboolean needs_attention_right;
  gboolean expand_tabs;
  gboolean inverted;

  GtkEventController *view_drop_target;
  GtkGesture *drag_gesture;

  GList *tabs;
  int n_tabs;

  GtkPopover *context_menu;

  int allocated_width;
  int last_width;
  int end_padding;
  int initial_end_padding;
  TabResizeMode tab_resize_mode;
  AdwAnimation *resize_animation;

  TabInfo *selected_tab;

  gboolean hovering;
  TabInfo *pressed_tab;
  TabInfo *reordered_tab;
  AdwAnimation *reorder_animation;

  int reorder_start_pos;
  int reorder_x;
  int reorder_y;
  int reorder_index;
  int reorder_window_x;
  gboolean continue_reorder;
  gboolean indirect_reordering;

  gboolean dragging;
  double drag_offset_x;
  double drag_offset_y;

  guint drag_autoscroll_cb_id;
  gint64 drag_autoscroll_prev_time;

  AdwTabPage *detached_page;
  int detached_index;
  TabInfo *reorder_placeholder;
  AdwTabPage *placeholder_page;
  int placeholder_scroll_offset;
  gboolean can_remove_placeholder;
  DragIcon *drag_icon;
  gboolean should_detach_into_new_window;

  TabInfo *drop_target_tab;
  guint drop_switch_timeout_id;
  guint reset_drop_target_tab_id;
  double drop_target_x;

  struct {
    TabInfo *info;
    int pos;
    gint64 duration;
    gboolean keep_selected_visible;
  } scheduled_scroll;

  AdwAnimation *scroll_animation;
  gboolean scroll_animation_done;
  double scroll_animation_from;
  double scroll_animation_offset;
  TabInfo *scroll_animation_tab;
  gboolean block_scrolling;
  double adjustment_prev_value;

  GdkDragAction extra_drag_actions;
  GType *extra_drag_types;
  gsize extra_drag_n_types;
};

G_DEFINE_TYPE_WITH_CODE (AdwTabBox, adw_tab_box, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

enum {
  PROP_0,
  PROP_PINNED,
  PROP_TAB_BAR,
  PROP_VIEW,
  PROP_NEEDS_ATTENTION_LEFT,
  PROP_NEEDS_ATTENTION_RIGHT,
  PROP_RESIZE_FROZEN,
  PROP_HADJUSTMENT,
  PROP_VADJUSTMENT,
  PROP_HSCROLL_POLICY,
  PROP_VSCROLL_POLICY,
  LAST_PROP = PROP_HADJUSTMENT
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_STOP_KINETIC_SCROLLING,
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

/* Helpers */

static void
remove_and_free_tab_info (TabInfo *info)
{
  gtk_widget_unparent (GTK_WIDGET (info->tab));

  g_free (info);
}

static inline int
get_tab_position (AdwTabBox *self,
                  TabInfo   *info)
{
  if (info == self->reordered_tab)
    return self->reorder_window_x;

  return info->pos;
}

static inline TabInfo *
find_tab_info_at (AdwTabBox *self,
                  double     x)
{
  GList *l;

  if (self->reordered_tab) {
    int pos = get_tab_position (self, self->reordered_tab);

    if (pos <= x && x < pos + self->reordered_tab->width)
      return self->reordered_tab;
  }

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info != self->reordered_tab &&
        info->pos <= x && x < info->pos + info->width)
      return info;
  }

  return NULL;
}

static inline GList *
find_link_for_page (AdwTabBox  *self,
                    AdwTabPage *page)
{
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->page == page)
      return l;
  }

  return NULL;
}

static inline TabInfo *
find_info_for_page (AdwTabBox  *self,
                    AdwTabPage *page)
{
  GList *l = find_link_for_page (self, page);

  return l ? l->data : NULL;
}

static GList *
find_nth_alive_tab (AdwTabBox *self,
                    guint      position)
{
  GList *l;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (!info->page)
        continue;

    if (!position--)
        return l;
  }

  return NULL;
}

static inline int
calculate_tab_width (TabInfo *info,
                     int      base_width)
{
  return OVERLAP + (int) floor ((base_width - OVERLAP) * info->appear_progress);
}

static int
get_base_tab_width (AdwTabBox *self,
                    gboolean   target)
{
  double max_progress = 0;
  double n = 0;
  double used_width;
  GList *l;
  int ret;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    max_progress = MAX (max_progress, info->appear_progress);
    n += info->appear_progress;
  }

  used_width = (self->allocated_width + (n + 1) * OVERLAP - (target ? 0 : self->end_padding)) * max_progress;

  ret = (int) ceil (used_width / n);

  if (!self->expand_tabs)
    ret = MIN (ret, MAX_TAB_WIDTH_NON_EXPAND + OVERLAP);

  return ret;
}

static int
predict_tab_width (AdwTabBox *self,
                   TabInfo   *info,
                   gboolean   assume_placeholder)
{
  int n;
  int width = self->allocated_width;
  int min;

  if (self->pinned)
    n = adw_tab_view_get_n_pinned_pages (self->view);
  else
    n = adw_tab_view_get_n_pages (self->view) - adw_tab_view_get_n_pinned_pages (self->view);

  if (assume_placeholder)
      n++;

  width += OVERLAP * (n + 1) - self->end_padding;

  /* Tabs have 0 minimum width, we need natural width instead */
  gtk_widget_measure (GTK_WIDGET (info->tab), GTK_ORIENTATION_HORIZONTAL, -1,
                      NULL, &min, NULL, NULL);

  if (self->expand_tabs)
    return MAX ((int) floor (width / (double) n), min);
  else
    return CLAMP ((int) floor (width / (double) n), min, MAX_TAB_WIDTH_NON_EXPAND);
}

static int
calculate_tab_offset (AdwTabBox *self,
                      TabInfo   *info,
                      gboolean target)
{
  int width;

  if (!self->reordered_tab)
      return 0;

  width = (target ? adw_tab_get_display_width (self->reordered_tab->tab) : self->reordered_tab->width) - OVERLAP;

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      width = -width;

  return (int) round (width * (target ? info->end_reorder_offset : info->reorder_offset));
}

static void
get_visible_range (AdwTabBox *self,
                   int       *lower,
                   int       *upper)
{
  int min = -OVERLAP;
  int max = self->allocated_width + OVERLAP;

  if (self->pinned) {
    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      min += OVERLAP;
    else
      max -= OVERLAP;
  }

  if (self->adjustment) {
    double value = gtk_adjustment_get_value (self->adjustment);
    double page_size = gtk_adjustment_get_page_size (self->adjustment);

    min = MAX (min, (int) floor (value) - OVERLAP);
    max = MIN (max, (int) ceil (value + page_size) + OVERLAP);
  }

  if (lower)
    *lower = min;

  if (upper)
    *upper = max;
}

static inline gboolean
is_touchscreen (GtkGesture *gesture)
{
  GtkEventController *controller = GTK_EVENT_CONTROLLER (gesture);
  GdkDevice *device = gtk_event_controller_get_current_event_device (controller);
  GdkInputSource input_source = gdk_device_get_source (device);

  return input_source == GDK_SOURCE_TOUCHSCREEN;
}

/* Tab resize delay */

static void
resize_animation_value_cb (double   value,
                           gpointer user_data)
{
  AdwTabBox *self = ADW_TAB_BOX (user_data);
  double target_end_padding = 0;

  if (!self->expand_tabs) {
    int predicted_tab_width = get_base_tab_width (self, TRUE);
    GList *l;

    target_end_padding = self->allocated_width + OVERLAP;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      target_end_padding -= calculate_tab_width (info, predicted_tab_width) - OVERLAP;
    }

    target_end_padding = MAX (target_end_padding, 0);
  }

  self->end_padding = (int) floor (adw_lerp (self->initial_end_padding, target_end_padding, value));

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
resize_animation_done_cb (gpointer user_data)
{
  AdwTabBox *self = ADW_TAB_BOX (user_data);

  self->end_padding = 0;
  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_clear_pointer (&self->resize_animation, adw_animation_unref);
}

static void
set_tab_resize_mode (AdwTabBox     *self,
                     TabResizeMode  mode)
{
  gboolean notify;

  if (self->tab_resize_mode == mode)
    return;

  if (mode == TAB_RESIZE_FIXED_TAB_WIDTH) {
    GList *l;

    self->last_width = self->allocated_width;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      if (info->appear_animation)
        info->last_width = adw_tab_get_display_width (info->tab);
      else
        info->last_width = info->width;
    }
  } else {
    self->last_width = 0;
  }

  if (mode == TAB_RESIZE_NORMAL) {
    self->initial_end_padding = self->end_padding;

    self->resize_animation =
      adw_animation_new (GTK_WIDGET (self), 0, 1,
                         RESIZE_ANIMATION_DURATION,
                         adw_ease_out_cubic,
                         resize_animation_value_cb,
                         resize_animation_done_cb,
                         self);

    adw_animation_start (self->resize_animation);
  }

  notify = (self->tab_resize_mode == TAB_RESIZE_NORMAL) !=
           (mode == TAB_RESIZE_NORMAL);

  self->tab_resize_mode = mode;

  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_RESIZE_FROZEN]);
}

/* Hover */

static void
update_hover (AdwTabBox *self)
{
  if (!self->dragging && !self->hovering)
    set_tab_resize_mode (self, TAB_RESIZE_NORMAL);
}

static void
motion_cb (AdwTabBox          *self,
           double              x,
           double              y,
           GtkEventController *controller)
{
  GdkDevice *device = gtk_event_controller_get_current_event_device (controller);
  GdkInputSource input_source = gdk_device_get_source (device);

  if (input_source == GDK_SOURCE_TOUCHSCREEN)
    return;

  if (self->hovering)
    return;

  self->hovering = TRUE;

  update_hover (self);
}

static void
leave_cb (AdwTabBox          *self,
          GtkEventController *controller)
{
  self->hovering = FALSE;

  update_hover (self);
}

/* Keybindings */

static void
focus_tab_cb (AdwTabBox *self,
              GVariant  *args)
{
  GtkDirectionType direction;
  gboolean last, is_rtl, success;

  if (!self->view || !self->selected_tab)
    return;

  g_variant_get (args, "(hb)", &direction, &last);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  success = last;

  if (direction == GTK_DIR_LEFT)
    direction = is_rtl ? GTK_DIR_TAB_FORWARD : GTK_DIR_TAB_BACKWARD;
  else if (direction == GTK_DIR_RIGHT)
    direction = is_rtl ? GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD;

  if (direction == GTK_DIR_TAB_BACKWARD) {
    if (last)
      success = adw_tab_view_select_first_page (self->view);
    else
      success = adw_tab_view_select_previous_page (self->view);
  } else if (direction == GTK_DIR_TAB_FORWARD) {
    if (last)
      success = adw_tab_view_select_last_page (self->view);
    else
      success = adw_tab_view_select_next_page (self->view);
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));
}

static void
reorder_tab_cb (AdwTabBox *self,
                GVariant  *args)
{
  GtkDirectionType direction;
  gboolean last, is_rtl, success;

  if (!self->view || !self->selected_tab || !self->selected_tab->page)
    return;

  g_variant_get (args, "(hb)", &direction, &last);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  success = last;

  if (direction == GTK_DIR_LEFT)
    direction = is_rtl ? GTK_DIR_TAB_FORWARD : GTK_DIR_TAB_BACKWARD;
  else if (direction == GTK_DIR_RIGHT)
    direction = is_rtl ? GTK_DIR_TAB_BACKWARD : GTK_DIR_TAB_FORWARD;

  if (direction == GTK_DIR_TAB_BACKWARD) {
    if (last)
      success = adw_tab_view_reorder_first (self->view, self->selected_tab->page);
    else
      success = adw_tab_view_reorder_backward (self->view, self->selected_tab->page);
  } else if (direction == GTK_DIR_TAB_FORWARD) {
    if (last)
      success = adw_tab_view_reorder_last (self->view, self->selected_tab->page);
    else
      success = adw_tab_view_reorder_forward (self->view, self->selected_tab->page);
  }

  if (!success)
    gtk_widget_error_bell (GTK_WIDGET (self));
}

static void
add_focus_bindings (GtkWidgetClass   *widget_class,
                    guint             keysym,
                    GtkDirectionType  direction,
                    gboolean          last)
{
  /* All keypad keysyms are aligned at the same order as non-keypad ones */
  guint keypad_keysym = keysym - GDK_KEY_Left + GDK_KEY_KP_Left;

  gtk_widget_class_add_binding (widget_class, keysym, 0,
                                (GtkShortcutFunc) focus_tab_cb,
                                "(hb)", direction, last);
  gtk_widget_class_add_binding (widget_class, keypad_keysym, 0,
                                (GtkShortcutFunc) focus_tab_cb,
                                "(hb)", direction, last);
}

static void
add_reorder_bindings (GtkWidgetClass   *widget_class,
                      guint             keysym,
                      GtkDirectionType  direction,
                      gboolean          last)
{
  /* All keypad keysyms are aligned at the same order as non-keypad ones */
  guint keypad_keysym = keysym - GDK_KEY_Left + GDK_KEY_KP_Left;

  gtk_widget_class_add_binding (widget_class, keysym, GDK_SHIFT_MASK,
                                (GtkShortcutFunc) reorder_tab_cb,
                                "(hb)", direction, last);
  gtk_widget_class_add_binding (widget_class, keypad_keysym, GDK_SHIFT_MASK,
                                (GtkShortcutFunc) reorder_tab_cb,
                                "(hb)", direction, last);
}

static void
activate_tab (AdwTabBox *self)
{
  GtkWidget *child;

  if (!self->selected_tab || !self->selected_tab->page)
    return;

  child = adw_tab_page_get_child (self->selected_tab->page);

  gtk_widget_grab_focus (child);
}

/* Scrolling */

static void
update_visible (AdwTabBox *self)
{
  gboolean left = FALSE, right = FALSE;
  GList *l;
  double value, page_size;

  if (!self->adjustment)
    return;

  value = gtk_adjustment_get_value (self->adjustment);
  page_size = gtk_adjustment_get_page_size (self->adjustment);

  if (!self->adjustment)
      return;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    int pos;

    if (!info->page)
      continue;

    pos = get_tab_position (self, info);

    adw_tab_set_fully_visible (info->tab,
                               pos + OVERLAP >= value &&
                               pos + info->width - OVERLAP <= value + page_size);

    if (!adw_tab_page_get_needs_attention (info->page))
      continue;

    if (pos + info->width / 2.0 <= value)
      left = TRUE;

    if (pos + info->width / 2.0 >= value + page_size)
      right = TRUE;
  }

  if (self->needs_attention_left != left) {
    self->needs_attention_left = left;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NEEDS_ATTENTION_LEFT]);
  }

  if (self->needs_attention_right != right) {
    self->needs_attention_right = right;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NEEDS_ATTENTION_RIGHT]);
  }
}

static double
get_scroll_animation_value (AdwTabBox *self)
{
  double to, value;

  g_assert (self->scroll_animation);

  to = self->scroll_animation_offset;

  if (self->scroll_animation_tab) {
    double page_size = gtk_adjustment_get_page_size (self->adjustment);

    to += get_tab_position (self, self->scroll_animation_tab);
    to = CLAMP (to, 0, self->allocated_width - page_size);
  }

  value = adw_animation_get_value (self->scroll_animation);

  return round (adw_lerp (self->scroll_animation_from, to, value));
}

static gboolean
drop_switch_timeout_cb (AdwTabBox *self)
{
  self->drop_switch_timeout_id = 0;
  adw_tab_view_set_selected_page (self->view,
                                  self->drop_target_tab->page);

  return G_SOURCE_REMOVE;
}

static void
set_drop_target_tab (AdwTabBox *self,
                     TabInfo   *info)
{
  if (self->drop_target_tab == info)
    return;

  if (self->drop_target_tab)
    g_clear_handle_id (&self->drop_switch_timeout_id, g_source_remove);

  self->drop_target_tab = info;

  if (self->drop_target_tab) {
    self->drop_switch_timeout_id =
      g_timeout_add (DROP_SWITCH_TIMEOUT,
                     (GSourceFunc) drop_switch_timeout_cb,
                     self);
  }
}

static void
adjustment_value_changed_cb (AdwTabBox *self)
{
  double value = gtk_adjustment_get_value (self->adjustment);

  update_visible (self);

  if (self->drop_target_tab) {
    self->drop_target_x += (value - self->adjustment_prev_value);
    set_drop_target_tab (self, find_tab_info_at (self, self->drop_target_x));
  }

  self->adjustment_prev_value = value;

  if (self->block_scrolling)
      return;

  if (self->scroll_animation)
    adw_animation_stop (self->scroll_animation);

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
scroll_animation_value_cb (double   value,
                           gpointer user_data)
{
  gtk_widget_queue_resize (GTK_WIDGET (user_data));
}

static void
scroll_animation_done_cb (gpointer user_data)
{
  AdwTabBox *self = ADW_TAB_BOX (user_data);

  self->scroll_animation_done = TRUE;
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
animate_scroll (AdwTabBox *self,
                TabInfo   *info,
                double     offset,
                gint64     duration)
{
  if (!self->adjustment)
    return;

  g_signal_emit (self, signals[SIGNAL_STOP_KINETIC_SCROLLING], 0);

  if (self->scroll_animation)
    adw_animation_stop (self->scroll_animation);

  g_clear_pointer (&self->scroll_animation, adw_animation_unref);
  self->scroll_animation_done = FALSE;
  self->scroll_animation_from = gtk_adjustment_get_value (self->adjustment);
  self->scroll_animation_tab = info;
  self->scroll_animation_offset = offset;

  /* The actual update will be done in size_allocate(). After the animation
   * finishes, don't remove it right away, it will be done in size-allocate as
   * well after one last update, so that we don't miss the last frame.
   */

  self->scroll_animation =
    adw_animation_new (GTK_WIDGET (self), 0, 1, duration,
                       adw_ease_out_cubic,
                       scroll_animation_value_cb,
                       scroll_animation_done_cb,
                       self);

  adw_animation_start (self->scroll_animation);
}

static void
animate_scroll_relative (AdwTabBox *self,
                         double     delta,
                         gint64     duration)
{
  double current_value = gtk_adjustment_get_value (self->adjustment);

  if (self->scroll_animation) {
    current_value = self->scroll_animation_offset;

    if (self->scroll_animation_tab)
      current_value += get_tab_position (self, self->scroll_animation_tab);
  }

  animate_scroll (self, NULL, current_value + delta, duration);
}

static void
scroll_to_tab_full (AdwTabBox *self,
                    TabInfo   *info,
                    int        pos,
                    gint64     duration,
                    gboolean   keep_selected_visible)
{
  int tab_width;
  double padding, value, page_size;

  if (!self->adjustment)
    return;

  tab_width = info->width;

  if (tab_width < 0) {
    self->scheduled_scroll.info = info;
    self->scheduled_scroll.pos = pos;
    self->scheduled_scroll.duration = duration;
    self->scheduled_scroll.keep_selected_visible = keep_selected_visible;

    gtk_widget_queue_allocate (GTK_WIDGET (self));

    return;
  }

  if (info->appear_animation)
    tab_width = adw_tab_get_display_width (info->tab);

  value = gtk_adjustment_get_value (self->adjustment);
  page_size = gtk_adjustment_get_page_size (self->adjustment);

  padding = MIN (tab_width, page_size - tab_width) / 2.0;

  if (pos < 0)
    pos = get_tab_position (self, info);

  if (pos + OVERLAP < value)
    animate_scroll (self, info, -padding, duration);
  else if (pos + tab_width - OVERLAP > value + page_size)
    animate_scroll (self, info, tab_width + padding - page_size, duration);
}

static void
scroll_to_tab (AdwTabBox *self,
               TabInfo   *info,
               gint64     duration)
{
  scroll_to_tab_full (self, info, -1, duration, FALSE);
}

static gboolean
scroll_cb (AdwTabBox          *self,
           double              dx,
           double              dy,
           GtkEventController *controller)
{
  double page_size, pow_unit, scroll_unit;
  GdkDevice *source_device;
  GdkInputSource input_source;

  if (!self->adjustment)
    return GDK_EVENT_PROPAGATE;

  source_device = gtk_event_controller_get_current_event_device (controller);
  input_source = gdk_device_get_source (source_device);

  if (input_source != GDK_SOURCE_MOUSE)
    return GDK_EVENT_PROPAGATE;

  page_size = gtk_adjustment_get_page_size (self->adjustment);

  /* Copied from gtkrange.c, _gtk_range_get_wheel_delta() */
  pow_unit = pow (page_size, 2.0 / 3.0);
  scroll_unit = MIN (pow_unit, page_size / 2.0);

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    dy = -dy;

  animate_scroll_relative (self, dy * scroll_unit, SCROLL_ANIMATION_DURATION);

  return GDK_EVENT_STOP;
}

/* Reordering */

static void
force_end_reordering (AdwTabBox *self)
{
  GList *l;

  if (self->dragging || !self->reordered_tab)
    return;

  if (self->reorder_animation)
    adw_animation_stop (self->reorder_animation);

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->reorder_animation)
      adw_animation_stop (info->reorder_animation);
  }
}

static void
check_end_reordering (AdwTabBox *self)
{
  GList *l;

  if (self->dragging || !self->reordered_tab || self->continue_reorder)
    return;

  if (self->reorder_animation)
    return;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (info->reorder_animation)
      return;
  }

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    info->end_reorder_offset = 0;
    info->reorder_offset = 0;
  }

  self->reordered_tab->reorder_ignore_bounds = FALSE;

  self->tabs = g_list_remove (self->tabs, self->reordered_tab);
  self->tabs = g_list_insert (self->tabs, self->reordered_tab, self->reorder_index);

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  self->reordered_tab = NULL;
}

static void
start_reordering (AdwTabBox *self,
                  TabInfo   *info)
{
  self->reordered_tab = info;

  /* The reordered tab should be displayed above everything else */
  gtk_widget_insert_before (GTK_WIDGET (self->reordered_tab->tab),
                            GTK_WIDGET (self), NULL);

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static int
get_reorder_position (AdwTabBox *self)
{
  int lower, upper;

  if (self->reordered_tab->reorder_ignore_bounds)
    return self->reorder_x;

  get_visible_range (self, &lower, &upper);

  return CLAMP (self->reorder_x, lower, upper - self->reordered_tab->width);
}

static void
reorder_animation_value_cb (double   value,
                            gpointer user_data)
{
  TabInfo *dest_tab = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (dest_tab->tab));
  AdwTabBox *self = ADW_TAB_BOX (parent);
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  double x1, x2;

  x1 = get_reorder_position (self);
  x2 = dest_tab->pos - calculate_tab_offset (self, dest_tab, FALSE);

  if (dest_tab->end_reorder_offset * (is_rtl ? 1 : -1) > 0)
    x2 += dest_tab->width - self->reordered_tab->width;

  self->reorder_window_x = (int) round (adw_lerp (x1, x2, value));

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
reorder_animation_done_cb (gpointer user_data)
{
  TabInfo *dest_tab = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (dest_tab->tab));
  AdwTabBox *self = ADW_TAB_BOX (parent);

  g_clear_pointer (&self->reorder_animation, adw_animation_unref);
  check_end_reordering (self);
}

static void
animate_reordering (AdwTabBox *self,
                    TabInfo   *dest_tab)
{
  if (self->reorder_animation)
    adw_animation_stop (self->reorder_animation);

  self->reorder_animation =
    adw_animation_new (GTK_WIDGET (self), 0, 1,
                       REORDER_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       reorder_animation_value_cb,
                       reorder_animation_done_cb,
                       dest_tab);

  adw_animation_start (self->reorder_animation);

  check_end_reordering (self);
}

static void
reorder_offset_animation_value_cb (double   value,
                                   gpointer user_data)
{
  TabInfo *info = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (info->tab));

  info->reorder_offset = value;
  gtk_widget_queue_allocate (parent);
}

static void
reorder_offset_animation_done_cb (gpointer user_data)
{
  TabInfo *info = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (info->tab));
  AdwTabBox *self = ADW_TAB_BOX (parent);

  g_clear_pointer (&info->reorder_animation, adw_animation_unref);
  check_end_reordering (self);
}

static void
animate_reorder_offset (AdwTabBox *self,
                        TabInfo   *info,
                        double     offset)
{
  gboolean is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  offset *= (is_rtl ? -1 : 1);

  if (info->end_reorder_offset == offset)
    return;

  info->end_reorder_offset = offset;

  if (info->reorder_animation)
    adw_animation_stop (info->reorder_animation);

  info->reorder_animation =
    adw_animation_new (GTK_WIDGET (self), info->reorder_offset, offset,
                       REORDER_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       reorder_offset_animation_value_cb,
                       reorder_offset_animation_done_cb,
                       info);

  adw_animation_start (info->reorder_animation);
}

static void
reset_reorder_animations (AdwTabBox *self)
{
  int i, original_index;
  GList *l;

  if (!adw_get_enable_animations (GTK_WIDGET (self)))
      return;

  l = find_link_for_page (self, self->reordered_tab->page);
  original_index = g_list_position (self->tabs, l);

  if (self->reorder_index > original_index)
    for (i = 0; i < self->reorder_index - original_index; i++) {
      l = l->next;
      animate_reorder_offset (self, l->data, 0);
    }

  if (self->reorder_index < original_index)
    for (i = 0; i < original_index - self->reorder_index; i++) {
      l = l->prev;
      animate_reorder_offset (self, l->data, 0);
    }
}

static void
page_reordered_cb (AdwTabBox  *self,
                   AdwTabPage *page,
                   int         index)
{
  GList *link;
  int original_index;
  TabInfo *info, *dest_tab;
  gboolean is_rtl;

  if (adw_tab_page_get_pinned (page) != self->pinned)
    return;

  self->continue_reorder = self->reordered_tab && page == self->reordered_tab->page;

  if (self->continue_reorder)
    reset_reorder_animations (self);
  else
    force_end_reordering (self);

  link = find_link_for_page (self, page);
  info = link->data;
  original_index = g_list_position (self->tabs, link);

  if (!self->continue_reorder)
    start_reordering (self, info);

  if (self->continue_reorder)
    self->reorder_x = self->reorder_window_x;
  else
    self->reorder_x = info->pos;

  self->reorder_index = index;

  if (!self->pinned)
    self->reorder_index -= adw_tab_view_get_n_pinned_pages (self->view);

  dest_tab = g_list_nth_data (self->tabs, self->reorder_index);

  if (info == self->selected_tab)
    scroll_to_tab_full (self, self->selected_tab, dest_tab->pos, REORDER_ANIMATION_DURATION, FALSE);

  animate_reordering (self, dest_tab);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  /* If animations are disabled, animate_reordering() animation will have
   * already finished and called check_end_reordering () by this point, so
   * it's too late to animate these, so we get a crash.
   */

  if (adw_get_enable_animations (GTK_WIDGET (self)) &&
      gtk_widget_get_mapped (GTK_WIDGET (self))) {
    int i;

    if (self->reorder_index > original_index)
      for (i = 0; i < self->reorder_index - original_index; i++) {
        link = link->next;
        animate_reorder_offset (self, link->data, is_rtl ? 1 : -1);
      }

    if (self->reorder_index < original_index)
      for (i = 0; i < original_index - self->reorder_index; i++) {
        link = link->prev;
        animate_reorder_offset (self, link->data, is_rtl ? -1 : 1);
      }
  }

  self->continue_reorder = FALSE;
}

static void
update_drag_reodering (AdwTabBox *self)
{
  gboolean is_rtl, after_selected, found_index;
  int x;
  int i = 0;
  int width;
  GList *l;

  if (!self->dragging)
    return;

  x = get_reorder_position (self);

  width = adw_tab_get_display_width (self->reordered_tab->tab);

  self->reorder_window_x = x;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;
  after_selected = FALSE;
  found_index = FALSE;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    int center = info->pos - calculate_tab_offset (self, info, FALSE) + info->width / 2;
    double offset = 0;

    if (x + width > center && center > x &&
        (!found_index || after_selected)) {
      self->reorder_index = i;
      found_index = TRUE;
    }

    i++;

    if (info == self->reordered_tab) {
      after_selected = TRUE;
      continue;
    }

    if (after_selected != is_rtl && x + width > center)
      offset = -1;
    else if (after_selected == is_rtl && x < center)
      offset = 1;

    animate_reorder_offset (self, info, offset);
  }
}

static gboolean
drag_autoscroll_cb (GtkWidget     *widget,
                    GdkFrameClock *frame_clock,
                    AdwTabBox     *self)
{
  double value, page_size;
  double x, delta_ms, start_threshold, end_threshold, autoscroll_factor;
  gint64 time;
  int offset = 0;
  int tab_width = 0;
  int autoscroll_area = 0;

  if (self->reordered_tab) {
    gtk_widget_measure (GTK_WIDGET (self->reordered_tab->tab),
                        GTK_ORIENTATION_HORIZONTAL, -1,
                        NULL, &tab_width, NULL, NULL);
    tab_width -= 2 * OVERLAP;
    x = (double) self->reorder_x + OVERLAP;
  } else if (self->drop_target_tab) {
    gtk_widget_measure (GTK_WIDGET (self->drop_target_tab->tab),
                        GTK_ORIENTATION_HORIZONTAL, -1,
                        NULL, &tab_width, NULL, NULL);
    tab_width -= 2 * OVERLAP;
    x = (double) self->drop_target_x + OVERLAP - tab_width / 2;
  } else {
    return G_SOURCE_CONTINUE;
  }

  value = gtk_adjustment_get_value (self->adjustment);
  page_size = gtk_adjustment_get_page_size (self->adjustment);
  autoscroll_area = tab_width / 2;

  x = CLAMP (x,
             autoscroll_area,
             self->allocated_width - tab_width - autoscroll_area);

  time = gdk_frame_clock_get_frame_time (frame_clock);
  delta_ms = (time - self->drag_autoscroll_prev_time) / 1000.0;

  start_threshold = value + autoscroll_area;
  end_threshold = value + page_size - tab_width - autoscroll_area;
  autoscroll_factor = 0;

  if (x < start_threshold)
    autoscroll_factor = -(start_threshold - x) / autoscroll_area;
  else if (x > end_threshold)
    autoscroll_factor = (x - end_threshold) / autoscroll_area;

  autoscroll_factor = CLAMP (autoscroll_factor, -1, 1);
  autoscroll_factor = adw_ease_in_cubic (autoscroll_factor);
  self->drag_autoscroll_prev_time = time;

  if (autoscroll_factor == 0)
    return G_SOURCE_CONTINUE;

  if (autoscroll_factor > 0)
    offset = (int) ceil (autoscroll_factor * delta_ms * AUTOSCROLL_SPEED);
  else
    offset = (int) floor (autoscroll_factor * delta_ms * AUTOSCROLL_SPEED);

  self->reorder_x += offset;
  gtk_adjustment_set_value (self->adjustment, value + offset);
  update_drag_reodering (self);

  return G_SOURCE_CONTINUE;
}

static void
start_autoscroll (AdwTabBox *self)
{
  GdkFrameClock *frame_clock;

  if (!self->adjustment)
    return;

  if (self->drag_autoscroll_cb_id)
    return;

  frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));

  self->drag_autoscroll_prev_time = gdk_frame_clock_get_frame_time (frame_clock);
  self->drag_autoscroll_cb_id =
    gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                  (GtkTickCallback) drag_autoscroll_cb,
                                  self, NULL);
}

static void
end_autoscroll (AdwTabBox *self)
{
  if (self->drag_autoscroll_cb_id) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self),
                                     self->drag_autoscroll_cb_id);
    self->drag_autoscroll_cb_id = 0;
  }
}

static void
start_drag_reodering (AdwTabBox *self,
                      TabInfo   *info,
                      double     x,
                      double     y)
{
  if (self->dragging)
    return;

  if (!info)
    return;

  self->continue_reorder = info == self->reordered_tab;

  if (self->continue_reorder) {
    if (self->reorder_animation)
      adw_animation_stop (self->reorder_animation);

    reset_reorder_animations (self);

    self->reorder_x = (int) round (x - self->drag_offset_x);
    self->reorder_y = (int) round (y - self->drag_offset_y);
  } else
    force_end_reordering (self);

  start_autoscroll (self);
  self->dragging = TRUE;

  if (!self->continue_reorder)
    start_reordering (self, info);
}

static void
end_drag_reodering (AdwTabBox *self)
{
  TabInfo *dest_tab;

  if (!self->dragging)
    return;

  self->dragging = FALSE;

  end_autoscroll (self);

  dest_tab = g_list_nth_data (self->tabs, self->reorder_index);

  if (!self->indirect_reordering) {
    int index = self->reorder_index;

    if (!self->pinned)
      index += adw_tab_view_get_n_pinned_pages (self->view);

    /* We've already reordered the tab here, no need to do it again */
    g_signal_handlers_block_by_func (self->view, page_reordered_cb, self);

    adw_tab_view_reorder_page (self->view, self->reordered_tab->page, index);

    g_signal_handlers_unblock_by_func (self->view, page_reordered_cb, self);
  }

  animate_reordering (self, dest_tab);

  self->continue_reorder = FALSE;
}

static void
reorder_begin_cb (AdwTabBox  *self,
                  double      start_x,
                  double      start_y,
                  GtkGesture *gesture)
{
  self->reorder_start_pos = gtk_adjustment_get_value (self->adjustment);

  start_x += self->reorder_start_pos;

  self->pressed_tab = find_tab_info_at (self, start_x);

  self->drag_offset_x = start_x - get_tab_position (self, self->pressed_tab);
  self->drag_offset_y = start_y;

  if (!self->reorder_animation) {
    self->reorder_x = (int) round (start_x - self->drag_offset_x);
    self->reorder_y = (int) round (start_y - self->drag_offset_y);
  }
}

/* Copied from gtkdragsource.c */
static gboolean
gtk_drag_check_threshold_double (GtkWidget *widget,
                                 double     start_x,
                                 double     start_y,
                                 double     current_x,
                                 double     current_y)
{
  int drag_threshold;

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-dnd-drag-threshold", &drag_threshold,
                NULL);

  return (ABS (current_x - start_x) > drag_threshold ||
          ABS (current_y - start_y) > drag_threshold);
}

static gboolean
check_dnd_threshold (AdwTabBox *self,
                     double     x,
                     double     y)
{
  int threshold;
  graphene_rect_t rect;

  g_object_get (gtk_widget_get_settings (GTK_WIDGET (self)),
                "gtk-dnd-drag-threshold", &threshold,
                NULL);

  threshold *= DND_THRESHOLD_MULTIPLIER;

  graphene_rect_init (&rect, 0, 0,
                      self->allocated_width,
                      gtk_widget_get_height (GTK_WIDGET (self)));
  graphene_rect_inset (&rect, -threshold, -threshold);

  return !graphene_rect_contains_point (&rect, &GRAPHENE_POINT_INIT (x, y));
}

static void begin_drag (AdwTabBox *self,
                        GdkDevice *device);

static void
reorder_update_cb (AdwTabBox  *self,
                   double      offset_x,
                   double      offset_y,
                   GtkGesture *gesture)
{
  double start_x, start_y, x, y;
  GdkDevice *device;

  if (!self->pressed_tab) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (!self->dragging &&
      !gtk_drag_check_threshold_double (GTK_WIDGET (self), 0, 0,
                                        offset_x, offset_y))
    return;

  gtk_gesture_drag_get_start_point (GTK_GESTURE_DRAG (gesture),
                                    &start_x, &start_y);

  x = start_x + gtk_adjustment_get_value (self->adjustment) + offset_x;
  y = start_y + offset_y;

  start_drag_reodering (self, self->pressed_tab, x, y);

  if (self->dragging) {
    adw_tab_view_set_selected_page (self->view, self->pressed_tab->page);
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
  } else {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  self->reorder_x = (int) round (x - self->drag_offset_x);
  self->reorder_y = (int) round (y - self->drag_offset_y);

  device = gtk_event_controller_get_current_event_device (GTK_EVENT_CONTROLLER (gesture));

  if (!self->pinned &&
      self->pressed_tab &&
      self->pressed_tab != self->reorder_placeholder &&
      self->pressed_tab->page &&
      !is_touchscreen (gesture) &&
      adw_tab_view_get_n_pages (self->view) > 1 &&
      check_dnd_threshold (self, x, y)) {
    begin_drag (self, device);

    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  update_drag_reodering (self);
}

static void
reorder_end_cb (AdwTabBox  *self,
                double      offset_x,
                double      offset_y,
                GtkGesture *gesture)
{
  end_drag_reodering (self);
}

/* Selection */

static void
reset_focus (AdwTabBox *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  gtk_widget_set_focus_child (GTK_WIDGET (self), NULL);

  if (root)
    gtk_root_set_focus (root, NULL);
}

static void
select_page (AdwTabBox  *self,
             AdwTabPage *page)
{
  if (!page) {
    self->selected_tab = NULL;

    reset_focus (self);

    return;
  }

  self->selected_tab = find_info_for_page (self, page);

  if (!self->selected_tab) {
    if (gtk_widget_get_focus_child (GTK_WIDGET (self)))
      reset_focus (self);

    return;
  }

  if (adw_tab_bar_tabs_have_visible_focus (self->tab_bar))
    gtk_widget_grab_focus (GTK_WIDGET (self->selected_tab->tab));

  gtk_widget_set_focus_child (GTK_WIDGET (self),
                              GTK_WIDGET (self->selected_tab->tab));

  if (self->selected_tab->width >= 0)
    scroll_to_tab (self, self->selected_tab, FOCUS_ANIMATION_DURATION);
}

/* Opening */

static gboolean
extra_drag_drop_cb (AdwTab    *tab,
                    GValue    *value,
                    AdwTabBox *self)
{
  gboolean ret = GDK_EVENT_PROPAGATE;
  AdwTabPage *page = adw_tab_get_page (tab);

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, page, value, &ret);

  return ret;
}

static void
appear_animation_value_cb (double   value,
                           gpointer user_data)
{
  TabInfo *info = user_data;

  info->appear_progress = value;

  if (GTK_IS_WIDGET (info->tab))
    gtk_widget_queue_resize (GTK_WIDGET (info->tab));
}

static void
open_animation_done_cb (gpointer user_data)
{
  TabInfo *info = user_data;

  g_clear_pointer (&info->appear_animation, adw_animation_unref);
}

static TabInfo *
create_tab_info (AdwTabBox  *self,
                 AdwTabPage *page)
{
  TabInfo *info;

  info = g_new0 (TabInfo, 1);
  info->page = page;
  info->pos = -1;
  info->width = -1;
  info->tab = adw_tab_new (self->view, self->pinned);

  adw_tab_set_page (info->tab, page);
  adw_tab_set_inverted (info->tab, self->inverted);
  adw_tab_setup_extra_drop_target (info->tab,
                                   self->extra_drag_actions,
                                   self->extra_drag_types,
                                   self->extra_drag_n_types);

  gtk_widget_set_parent (GTK_WIDGET (info->tab), GTK_WIDGET (self));

  g_signal_connect_object (info->tab, "extra-drag-drop", G_CALLBACK (extra_drag_drop_cb), self, 0);

  return info;
}

static void
page_attached_cb (AdwTabBox  *self,
                  AdwTabPage *page,
                  int         position)
{
  TabInfo *info;
  GList *l;

  if (adw_tab_page_get_pinned (page) != self->pinned)
    return;

  if (!self->pinned)
    position -= adw_tab_view_get_n_pinned_pages (self->view);

  set_tab_resize_mode (self, TAB_RESIZE_NORMAL);
  force_end_reordering (self);

  info = create_tab_info (self, page);

  info->notify_needs_attention_id =
    g_signal_connect_object (page,
                             "notify::needs-attention",
                             G_CALLBACK (update_visible),
                             self,
                             G_CONNECT_SWAPPED);

  info->appear_animation =
    adw_animation_new (GTK_WIDGET (self), 0, 1,
                       OPEN_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       appear_animation_value_cb,
                       open_animation_done_cb,
                       info);

  l = find_nth_alive_tab (self, position);
  self->tabs = g_list_insert_before (self->tabs, l, info);

  self->n_tabs++;

  adw_animation_start (info->appear_animation);

  if (page == adw_tab_view_get_selected_page (self->view))
    adw_tab_box_select_page (self, page);
  else
    scroll_to_tab_full (self, info, -1, FOCUS_ANIMATION_DURATION, TRUE);
}

/* Closing */

static void
close_animation_done_cb (gpointer user_data)
{
  TabInfo *info = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (info->tab));
  AdwTabBox *self = ADW_TAB_BOX (parent);

  g_clear_pointer (&info->appear_animation, adw_animation_unref);

  self->tabs = g_list_remove (self->tabs, info);

  if (info->reorder_animation)
    adw_animation_stop (info->reorder_animation);

  if (self->reorder_animation)
    adw_animation_stop (self->reorder_animation);

  if (self->pressed_tab == info)
    self->pressed_tab = NULL;

  if (self->reordered_tab == info)
    self->reordered_tab = NULL;

  remove_and_free_tab_info (info);

  self->n_tabs--;
}

static void
page_detached_cb (AdwTabBox  *self,
                  AdwTabPage *page)
{
  TabInfo *info;
  GList *page_link;

  page_link = find_link_for_page (self, page);

  if (!page_link)
    return;

  info = page_link->data;
  page_link = page_link->next;

  force_end_reordering (self);

  if (self->hovering && !self->pinned) {
    gboolean is_last = TRUE;

    while (page_link) {
      TabInfo *i = page_link->data;
      page_link = page_link->next;

      if (i->page) {
        is_last = FALSE;
        break;
      }
    }

    if (is_last)
      set_tab_resize_mode (self, self->inverted ? TAB_RESIZE_NORMAL : TAB_RESIZE_FIXED_END_PADDING);
    else
      set_tab_resize_mode (self, TAB_RESIZE_FIXED_TAB_WIDTH);
  }

  g_assert (info->page);

  if (gtk_widget_is_focus (GTK_WIDGET (info->tab)))
    adw_tab_box_try_focus_selected_tab (self);

  if (info == self->selected_tab)
    adw_tab_box_select_page (self, NULL);

  adw_tab_set_page (info->tab, NULL);

  if (info->notify_needs_attention_id > 0) {
    g_signal_handler_disconnect (info->page, info->notify_needs_attention_id);
    info->notify_needs_attention_id = 0;
  }

  info->page = NULL;

  if (info->appear_animation)
    adw_animation_stop (info->appear_animation);

  info->appear_animation =
    adw_animation_new (GTK_WIDGET (self), info->appear_progress, 0,
                       CLOSE_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       appear_animation_value_cb,
                       close_animation_done_cb,
                       info);

  adw_animation_start (info->appear_animation);
}

/* Tab DND */

#define ADW_TYPE_TAB_BOX_ROOT_CONTENT (adw_tab_box_root_content_get_type ())

G_DECLARE_FINAL_TYPE (AdwTabBoxRootContent, adw_tab_box_root_content, ADW, TAB_BOX_ROOT_CONTENT, GdkContentProvider)

struct _AdwTabBoxRootContent
{
  GdkContentProvider parent_instance;

  AdwTabBox *tab_box;
};

G_DEFINE_TYPE (AdwTabBoxRootContent, adw_tab_box_root_content, GDK_TYPE_CONTENT_PROVIDER)

static GdkContentFormats *
adw_tab_box_root_content_ref_formats (GdkContentProvider *provider)
{
  return gdk_content_formats_new ((const char *[1]) { "application/x-rootwindow-drop" }, 1);
}

static void
adw_tab_box_root_content_write_mime_type_async (GdkContentProvider  *provider,
                                                const char          *mime_type,
                                                GOutputStream       *stream,
                                                int                  io_priority,
                                                GCancellable        *cancellable,
                                                GAsyncReadyCallback  callback,
                                                gpointer             user_data)
{
  AdwTabBoxRootContent *self = ADW_TAB_BOX_ROOT_CONTENT (provider);
  g_autoptr (GTask) task = NULL;

  self->tab_box->should_detach_into_new_window = TRUE;

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_priority (task, io_priority);
  g_task_set_source_tag (task, adw_tab_box_root_content_write_mime_type_async);
  g_task_return_boolean (task, TRUE);
}

static gboolean
adw_tab_box_root_content_write_mime_type_finish (GdkContentProvider  *provider,
                                                 GAsyncResult        *result,
                                                 GError             **error)
{
  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
adw_tab_box_root_content_finalize (GObject *object)
{
  AdwTabBoxRootContent *self = ADW_TAB_BOX_ROOT_CONTENT (object);

  g_clear_object (&self->tab_box);

  G_OBJECT_CLASS (adw_tab_box_root_content_parent_class)->finalize (object);
}

static void
adw_tab_box_root_content_class_init (AdwTabBoxRootContentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GdkContentProviderClass *provider_class = GDK_CONTENT_PROVIDER_CLASS (klass);

  object_class->finalize = adw_tab_box_root_content_finalize;

  provider_class->ref_formats = adw_tab_box_root_content_ref_formats;
  provider_class->write_mime_type_async = adw_tab_box_root_content_write_mime_type_async;
  provider_class->write_mime_type_finish = adw_tab_box_root_content_write_mime_type_finish;
}

static void
adw_tab_box_root_content_init (AdwTabBoxRootContent *self)
{
}

static GdkContentProvider *
adw_tab_box_root_content_new (AdwTabBox *tab_box)
{
  AdwTabBoxRootContent *self = g_object_new (ADW_TYPE_TAB_BOX_ROOT_CONTENT, NULL);

  self->tab_box = g_object_ref (tab_box);

  return GDK_CONTENT_PROVIDER (self);
}

static int
calculate_placeholder_index (AdwTabBox *self,
                             int        x)
{
  int lower, upper, pos, i;
  gboolean is_rtl;
  GList *l;

  get_visible_range (self, &lower, &upper);

  x = CLAMP (x, lower, upper);

  is_rtl = gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  pos = (is_rtl ? self->allocated_width + OVERLAP : -OVERLAP);
  i = 0;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;
    int tab_width = predict_tab_width (self, info, TRUE) * (is_rtl ? -1 : 1);

    int end = pos + tab_width + calculate_tab_offset (self, info, FALSE);

    if ((x <= end && !is_rtl) || (x >= end && is_rtl))
      break;

    pos += tab_width + (is_rtl ? OVERLAP : -OVERLAP);
    i++;
  }

  return i;
}

static void
insert_animation_value_cb (double   value,
                           gpointer user_data)
{
  TabInfo *info = user_data;
  AdwTabBox *self = ADW_TAB_BOX (gtk_widget_get_parent (GTK_WIDGET (info->tab)));

  appear_animation_value_cb (value, info);

  update_drag_reodering (self);
}

static void
insert_placeholder (AdwTabBox  *self,
                    AdwTabPage *page,
                    int         pos)
{
  TabInfo *info = self->reorder_placeholder;
  double initial_progress = 0;

  if (info) {
    initial_progress = info->appear_progress;

    if (info->appear_animation)
      adw_animation_stop (info->appear_animation);
  } else {
    int index;

    self->placeholder_page = page;

    info = create_tab_info (self, page);

    gtk_widget_set_opacity (GTK_WIDGET (info->tab), 0);

    adw_tab_set_dragging (info->tab, TRUE);

    info->reorder_ignore_bounds = TRUE;

    if (self->adjustment) {
      double page_size = gtk_adjustment_get_page_size (self->adjustment);

      if (self->allocated_width > page_size) {
        gtk_widget_measure (GTK_WIDGET (info->tab), GTK_ORIENTATION_HORIZONTAL, -1,
                            NULL, &self->placeholder_scroll_offset, NULL, NULL);

        self->placeholder_scroll_offset /= 2;
      } else {
        self->placeholder_scroll_offset = 0;
      }
    }

    index = calculate_placeholder_index (self, pos + self->placeholder_scroll_offset);

    self->tabs = g_list_insert (self->tabs, info, index);
    self->n_tabs++;

    self->reorder_placeholder = info;
    self->reorder_index = g_list_index (self->tabs, info);

    animate_scroll_relative (self, self->placeholder_scroll_offset, OPEN_ANIMATION_DURATION);
  }

  info->appear_animation =
    adw_animation_new (GTK_WIDGET (self), initial_progress, 1,
                       OPEN_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       insert_animation_value_cb,
                       open_animation_done_cb,
                       info);

  adw_animation_start (info->appear_animation);
}

static void
replace_animation_done_cb (gpointer user_data)
{
  TabInfo *info = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (info->tab));
  AdwTabBox *self = ADW_TAB_BOX (parent);

  g_clear_pointer (&info->appear_animation, adw_animation_unref);
  self->reorder_placeholder = NULL;
  self->can_remove_placeholder = TRUE;
}

static void
replace_placeholder (AdwTabBox  *self,
                     AdwTabPage *page)
{
  TabInfo *info = self->reorder_placeholder;
  double initial_progress;

  self->placeholder_scroll_offset = 0;
  gtk_widget_set_opacity (GTK_WIDGET (self->reorder_placeholder->tab), 1);
  adw_tab_set_dragging (info->tab, FALSE);

  if (!info->appear_animation) {
    self->reorder_placeholder = NULL;

    return;
  }

  initial_progress = info->appear_progress;

  self->can_remove_placeholder = FALSE;

  adw_tab_set_page (info->tab, page);
  info->page = page;

  adw_animation_stop (info->appear_animation);

  info->appear_animation =
    adw_animation_new (GTK_WIDGET (self), initial_progress, 1,
                       OPEN_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       appear_animation_value_cb,
                       replace_animation_done_cb,
                       info);

  adw_animation_start (info->appear_animation);
}

static void
remove_animation_done_cb (gpointer user_data)
{
  TabInfo *info = user_data;
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (info->tab));
  AdwTabBox *self = ADW_TAB_BOX (parent);

  g_clear_pointer (&info->appear_animation, adw_animation_unref);

  if (!self->can_remove_placeholder) {
    adw_tab_set_page (info->tab, self->placeholder_page);
    info->page = self->placeholder_page;

    return;
  }

  if (self->reordered_tab == info) {
    force_end_reordering (self);

    if (self->reorder_animation)
      adw_animation_stop (info->reorder_animation);

    self->reordered_tab = NULL;
  }

  if (self->pressed_tab == info)
    self->pressed_tab = NULL;

  self->tabs = g_list_remove (self->tabs, info);

  remove_and_free_tab_info (info);

  self->n_tabs--;

  self->reorder_placeholder = NULL;
}

static gboolean
remove_placeholder_scroll_cb (AdwTabBox *self)
{
  animate_scroll_relative (self, -self->placeholder_scroll_offset, CLOSE_ANIMATION_DURATION);
  self->placeholder_scroll_offset = 0;

  return G_SOURCE_REMOVE;
}

static void
remove_placeholder (AdwTabBox *self)
{
  TabInfo *info = self->reorder_placeholder;

  if (!info || !info->page)
    return;

  adw_tab_set_page (info->tab, NULL);
  info->page = NULL;

  if (info->appear_animation)
    adw_animation_stop (info->appear_animation);

  g_idle_add ((GSourceFunc) remove_placeholder_scroll_cb, self);

  info->appear_animation =
    adw_animation_new (GTK_WIDGET (self), info->appear_progress, 0,
                       CLOSE_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       appear_animation_value_cb,
                       remove_animation_done_cb,
                       info);

  adw_animation_start (info->appear_animation);
}

static inline AdwTabBox *
get_source_tab_box (GtkDropTarget *target)
{
  GdkDrop *drop = gtk_drop_target_get_drop (target);
  GdkDrag *drag = gdk_drop_get_drag (drop);

  if (!drag)
    return NULL;

  return ADW_TAB_BOX (g_object_get_data (G_OBJECT (drag),
                      "adw-tab-bar-drag-origin"));
}

static void
do_drag_drop (AdwTabBox *self,
              AdwTabBox *source_tab_box)
{
  AdwTabPage *page = source_tab_box->detached_page;
  int offset = (self->pinned ? 0 : adw_tab_view_get_n_pinned_pages (self->view));

  if (self->reorder_placeholder) {
    replace_placeholder (self, page);
    end_drag_reodering (self);

    g_signal_handlers_block_by_func (self->view, page_attached_cb, self);

    adw_tab_view_attach_page (self->view, page, self->reorder_index + offset);

    g_signal_handlers_unblock_by_func (self->view, page_attached_cb, self);
  } else {
    adw_tab_view_attach_page (self->view, page, self->reorder_index + offset);
  }

  source_tab_box->should_detach_into_new_window = FALSE;
  source_tab_box->detached_page = NULL;

  self->indirect_reordering = FALSE;
}

static void
detach_into_new_window (AdwTabBox *self)
{
  AdwTabPage *page;
  AdwTabView *new_view;

  page = self->detached_page;

  new_view = adw_tab_view_create_window (self->view);

  if (ADW_IS_TAB_VIEW (new_view))
    adw_tab_view_attach_page (new_view, page, 0);
  else
    adw_tab_view_attach_page (self->view, page, self->detached_index);

  self->should_detach_into_new_window = FALSE;
}

static gboolean
is_view_in_the_same_group (AdwTabBox  *self,
                           AdwTabView *other_view)
{
  /* TODO when we have groups, this should do the actual check */
  return TRUE;
}

static void
drag_end (AdwTabBox *self,
          GdkDrag   *drag,
          gboolean   success)
{
  g_signal_handlers_disconnect_by_data (drag, self);

  gdk_drag_drop_done (drag, success);

  if (!success) {
    adw_tab_view_attach_page (self->view,
                              self->detached_page,
                              self->detached_index);

    self->indirect_reordering = FALSE;
  }

  self->detached_page = NULL;

  if (self->drag_icon)
    g_clear_pointer (&self->drag_icon, g_free);

  g_object_unref (drag);
}

static void
tab_drop_performed_cb (AdwTabBox *self,
                       GdkDrag   *drag)
{
  /* Catch drops into our windows, but outside of tab views. If this is a false
   * positive, it will be set to FALSE in do_drag_drop(). */
  self->should_detach_into_new_window = TRUE;
}

static void
tab_dnd_finished_cb (AdwTabBox *self,
                     GdkDrag   *drag)
{
  if (self->should_detach_into_new_window)
    detach_into_new_window (self);

  drag_end (self, drag, TRUE);
}

static void
tab_drag_cancel_cb (AdwTabBox           *self,
                    GdkDragCancelReason  reason,
                    GdkDrag             *drag)
{
  if (reason == GDK_DRAG_CANCEL_NO_TARGET) {
    detach_into_new_window (self);
    drag_end (self, drag, TRUE);

    return;
  }

  self->should_detach_into_new_window = FALSE;
  drag_end (self, drag, FALSE);
}

static void
create_drag_icon (AdwTabBox *self,
                  GdkDrag   *drag)
{
  DragIcon *icon;

  icon = g_new0 (DragIcon, 1);

  icon->drag = drag;

  icon->width = predict_tab_width (self, self->reordered_tab, FALSE);
  icon->target_width = icon->width;

  icon->tab = adw_tab_new (self->view, FALSE);
  adw_tab_set_page (icon->tab, self->reordered_tab->page);
  adw_tab_set_dragging (icon->tab, TRUE);
  adw_tab_set_inverted (icon->tab, self->inverted);
  adw_tab_set_display_width (icon->tab, icon->width);
  gtk_widget_set_halign (GTK_WIDGET (icon->tab), GTK_ALIGN_START);

  gtk_drag_icon_set_child (GTK_DRAG_ICON (gtk_drag_icon_get_for_drag (drag)),
                           GTK_WIDGET (icon->tab));

  gtk_style_context_get_margin (gtk_widget_get_style_context (GTK_WIDGET (icon->tab)),
                                &icon->tab_margin);

  gtk_widget_set_size_request (GTK_WIDGET (icon->tab),
                               icon->width + icon->tab_margin.left + icon->tab_margin.right,
                               -1);

  icon->hotspot_x = (int) self->drag_offset_x;
  icon->hotspot_y = (int) self->drag_offset_y;

  gdk_drag_set_hotspot (drag,
                        icon->hotspot_x + icon->tab_margin.left,
                        icon->hotspot_y + icon->tab_margin.top);

  self->drag_icon = icon;
}

static void
icon_resize_animation_value_cb (double   value,
                                gpointer user_data)
{
  DragIcon *icon = user_data;
  double relative_pos;

  relative_pos = (double) icon->hotspot_x / icon->width;

  icon->width = (int) round (value);

  adw_tab_set_display_width (icon->tab, icon->width);
  gtk_widget_set_size_request (GTK_WIDGET (icon->tab),
                               icon->width + icon->tab_margin.left + icon->tab_margin.right,
                               -1);

  icon->hotspot_x = (int) round (icon->width * relative_pos);

  gdk_drag_set_hotspot (icon->drag,
                        icon->hotspot_x + icon->tab_margin.left,
                        icon->hotspot_y + icon->tab_margin.top);

  gtk_widget_queue_resize (GTK_WIDGET (icon->tab));
}

static void
icon_resize_animation_done_cb (gpointer user_data)
{
  DragIcon *icon = user_data;

  g_clear_pointer (&icon->resize_animation, adw_animation_unref);
}

static void
resize_drag_icon (AdwTabBox *self,
                  int        width)
{
  DragIcon *icon = self->drag_icon;

  if (width == icon->target_width)
    return;

  if (icon->resize_animation)
    adw_animation_stop (icon->resize_animation);

  icon->target_width = width;

  icon->resize_animation =
    adw_animation_new (GTK_WIDGET (icon->tab), icon->width, width,
                       ICON_RESIZE_ANIMATION_DURATION,
                       adw_ease_out_cubic,
                       icon_resize_animation_value_cb,
                       icon_resize_animation_done_cb,
                       icon);

  adw_animation_start (icon->resize_animation);
}

static void
begin_drag (AdwTabBox *self,
            GdkDevice *device)
{
  GtkNative *native;
  GdkSurface *surface;
  GdkContentProvider *content;
  GdkDrag *drag;
  TabInfo *detached_info;
  AdwTab *detached_tab;

  native = gtk_widget_get_native (GTK_WIDGET (self));
  surface = gtk_native_get_surface (native);

  self->hovering = TRUE;
  self->pressed_tab = NULL;

  detached_info = self->reordered_tab;
  detached_tab = g_object_ref (detached_info->tab);
  self->detached_page = detached_info->page;

  self->indirect_reordering = TRUE;

  content = gdk_content_provider_new_union ((GdkContentProvider *[2]) {
                                              adw_tab_box_root_content_new (self),
                                              gdk_content_provider_new_typed (ADW_TYPE_TAB_PAGE, detached_info->page)
                                            }, 2);

  drag = gdk_drag_begin (surface, device, content, GDK_ACTION_MOVE,
                         self->reorder_x, self->reorder_y);

  g_object_set_data (G_OBJECT (drag), "adw-tab-bar-drag-origin", self);

  g_signal_connect_swapped (drag, "drop-performed",
                            G_CALLBACK (tab_drop_performed_cb), self);
  g_signal_connect_swapped (drag, "dnd-finished",
                            G_CALLBACK (tab_dnd_finished_cb), self);
  g_signal_connect_swapped (drag, "cancel",
                            G_CALLBACK (tab_drag_cancel_cb), self);

  create_drag_icon (self, drag);

  end_drag_reodering (self);
  update_hover (self);

  gtk_widget_set_opacity (GTK_WIDGET (detached_tab), 0);
  self->detached_index = adw_tab_view_get_page_position (self->view, self->detached_page);

  adw_tab_view_detach_page (self->view, self->detached_page);

  self->indirect_reordering = FALSE;

  gtk_widget_measure (GTK_WIDGET (detached_tab),
                      GTK_ORIENTATION_HORIZONTAL, -1,
                      NULL, &self->placeholder_scroll_offset, NULL, NULL);
  self->placeholder_scroll_offset /= 2;

  animate_scroll_relative (self, -self->placeholder_scroll_offset, CLOSE_ANIMATION_DURATION);

  g_object_unref (detached_tab);
}

static GdkDragAction
tab_drag_enter_motion_cb (AdwTabBox     *self,
                          double         x,
                          double         y,
                          GtkDropTarget *target)
{
  AdwTabBox *source_tab_box;

  if (self->pinned)
    return 0;

  source_tab_box = get_source_tab_box (target);

  if (!source_tab_box)
    return 0;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_box->view))
    return 0;

  x += gtk_adjustment_get_value (self->adjustment);

  self->can_remove_placeholder = FALSE;

  if (!self->reorder_placeholder || !self->reorder_placeholder->page) {
    AdwTabPage *page = source_tab_box->detached_page;
    double center = x - source_tab_box->drag_icon->hotspot_x + source_tab_box->drag_icon->width / 2;

    insert_placeholder (self, page, center);

    self->indirect_reordering = TRUE;

    resize_drag_icon (source_tab_box, predict_tab_width (self, self->reorder_placeholder, TRUE));
    adw_tab_set_display_width (self->reorder_placeholder->tab, source_tab_box->drag_icon->target_width);
    adw_tab_set_inverted (source_tab_box->drag_icon->tab, self->inverted);

    self->drag_offset_x = source_tab_box->drag_icon->hotspot_x;
    self->drag_offset_y = source_tab_box->drag_icon->hotspot_y;

    self->reorder_x = (int) round (x - source_tab_box->drag_icon->hotspot_x);

    start_drag_reodering (self, self->reorder_placeholder, x, y);

    return GDK_ACTION_MOVE;
  }

  self->reorder_x = (int) round (x - source_tab_box->drag_icon->hotspot_x);

  update_drag_reodering (self);

  return GDK_ACTION_MOVE;
}

static void
tab_drag_leave_cb (AdwTabBox     *self,
                   GtkDropTarget *target)
{
  AdwTabBox *source_tab_box;

  if (!self->indirect_reordering)
    return;

  if (self->pinned)
    return;

  source_tab_box = get_source_tab_box (target);

  if (!source_tab_box)
    return;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_box->view))
    return;

  self->can_remove_placeholder = TRUE;

  end_drag_reodering (self);
  remove_placeholder (self);

  self->indirect_reordering = FALSE;
}

static gboolean
tab_drag_drop_cb (AdwTabBox     *self,
                  const GValue  *value,
                  double         x,
                  double         y,
                  GtkDropTarget *target)
{
  AdwTabBox *source_tab_box;

  if (self->pinned)
    return GDK_EVENT_PROPAGATE;

  source_tab_box = get_source_tab_box (target);

  if (!source_tab_box)
    return GDK_EVENT_PROPAGATE;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_box->view))
    return GDK_EVENT_PROPAGATE;

  do_drag_drop (self, source_tab_box);

  return GDK_EVENT_STOP;
}

static gboolean
view_drag_drop_cb (AdwTabBox      *self,
                   const GValue  *value,
                   double         x,
                   double         y,
                   GtkDropTarget *target)
{
  AdwTabBox *source_tab_box;

  if (self->pinned)
    return GDK_EVENT_PROPAGATE;

  source_tab_box = get_source_tab_box (target);

  if (!source_tab_box)
    return GDK_EVENT_PROPAGATE;

  if (!self->view || !is_view_in_the_same_group (self, source_tab_box->view))
    return GDK_EVENT_PROPAGATE;

  self->reorder_index = adw_tab_view_get_n_pages (self->view) -
                        adw_tab_view_get_n_pinned_pages (self->view);

  do_drag_drop (self, source_tab_box);

  return GDK_EVENT_STOP;
}

/* DND autoscrolling */

static gboolean
reset_drop_target_tab_cb (AdwTabBox *self)
{
  self->reset_drop_target_tab_id = 0;
  set_drop_target_tab (self, NULL);

  return G_SOURCE_REMOVE;
}

static void
drag_leave_cb (AdwTabBox               *self,
               GtkDropControllerMotion *controller)
{
  GdkDrop *drop = gtk_drop_controller_motion_get_drop (controller);
  GdkDrag *drag = gdk_drop_get_drag (drop);
  AdwTabBox *source = ADW_TAB_BOX (g_object_get_data (G_OBJECT (drag),
                                                      "adw-tab-bar-drag-origin"));

  if (source)
    return;

  if (!self->reset_drop_target_tab_id)
    self->reset_drop_target_tab_id =
      g_idle_add ((GSourceFunc) reset_drop_target_tab_cb, self);

  end_autoscroll (self);
}

static void
drag_enter_motion_cb (AdwTabBox               *self,
                      double                   x,
                      double                   y,
                      GtkDropControllerMotion *controller)
{
  TabInfo *info;
  GdkDrop *drop = gtk_drop_controller_motion_get_drop (controller);
  GdkDrag *drag = gdk_drop_get_drag (drop);
  AdwTabBox *source = ADW_TAB_BOX (g_object_get_data (G_OBJECT (drag),
                                                      "adw-tab-bar-drag-origin"));

  if (source)
    return;

  x += gtk_adjustment_get_value (self->adjustment);

  info = find_tab_info_at (self, x);

  if (!info) {
    drag_leave_cb (self, controller);

    return;
  }

  self->drop_target_x = x;
  set_drop_target_tab (self, info);

  start_autoscroll (self);
}

/* Context menu */

static gboolean
reset_setup_menu_cb (AdwTabBox *self)
{
  g_signal_emit_by_name (self->view, "setup-menu", NULL);

  return G_SOURCE_REMOVE;
}

static void
touch_menu_notify_visible_cb (AdwTabBox *self)
{
  if (!self->context_menu || gtk_widget_get_visible (GTK_WIDGET (self->context_menu)))
    return;

  self->hovering = FALSE;
  update_hover (self);

  g_idle_add ((GSourceFunc) reset_setup_menu_cb, self);
}

static void
do_popup (AdwTabBox *self,
          TabInfo   *info,
          double     x,
          double     y)
{
  GMenuModel *model = adw_tab_view_get_menu_model (self->view);
  GdkRectangle rect;

  if (!G_IS_MENU_MODEL (model))
    return;

  g_signal_emit_by_name (self->view, "setup-menu", info->page);

  if (!self->context_menu) {
    self->context_menu = GTK_POPOVER (gtk_popover_menu_new_from_model (model));
    gtk_widget_set_parent (GTK_WIDGET (self->context_menu), GTK_WIDGET (self));
    gtk_popover_set_position (self->context_menu, GTK_POS_BOTTOM);
    gtk_popover_set_has_arrow (self->context_menu, FALSE);

    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      gtk_widget_set_halign (GTK_WIDGET (self->context_menu), GTK_ALIGN_END);
    else
      gtk_widget_set_halign (GTK_WIDGET (self->context_menu), GTK_ALIGN_START);

    g_signal_connect_object (self->context_menu, "notify::visible",
                             G_CALLBACK (touch_menu_notify_visible_cb), self,
                             G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  }

  if (x >= 0 && y >= 0) {
    rect.x = x;
    rect.y = y;
  } else {
    rect.x = info->pos;
    rect.y = gtk_widget_get_allocated_height (GTK_WIDGET (info->tab));

    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      rect.x += info->width;
  }

  rect.x -= gtk_adjustment_get_value (self->adjustment);
  rect.width = 0;
  rect.height = 0;

  gtk_popover_set_pointing_to (self->context_menu, &rect);

  gtk_popover_popup (self->context_menu);
}

static void
long_pressed_cb (AdwTabBox  *self,
                 double      x,
                 double      y,
                 GtkGesture *gesture)
{
  TabInfo *info = find_tab_info_at (self, x);

  gtk_gesture_set_state (self->drag_gesture, GTK_EVENT_SEQUENCE_DENIED);

  if (!info || !info->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  x += gtk_adjustment_get_value (self->adjustment);

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
  do_popup (self, self->pressed_tab, x, y);
}

static void
popup_menu_cb (GtkWidget  *widget,
               const char *action_name,
               GVariant   *parameter)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);

  if (self->selected_tab && self->selected_tab->page)
    do_popup (self, self->selected_tab, -1, -1);
}

/* Clicking */

static void
handle_click (AdwTabBox  *self,
              TabInfo    *info,
              GtkGesture *gesture)
{
  gboolean can_grab_focus;

  if (self->adjustment) {
    int pos = get_tab_position (self, info);
    double value = gtk_adjustment_get_value (self->adjustment);
    double page_size = gtk_adjustment_get_page_size (self->adjustment);

    if (pos + OVERLAP < value ||
        pos + info->width - OVERLAP > value + page_size) {
      gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);

      scroll_to_tab (self, info, SCROLL_ANIMATION_DURATION);

      return;
    }
  }

  can_grab_focus = adw_tab_bar_tabs_have_visible_focus (self->tab_bar);

  if (info == self->selected_tab)
    can_grab_focus = TRUE;
  else
    adw_tab_view_set_selected_page (self->view, info->page);

  if (can_grab_focus)
    gtk_widget_grab_focus (GTK_WIDGET (info->tab));
  else
    activate_tab (self);
}

static void
pressed_cb (AdwTabBox  *self,
            int         n_press,
            double      x,
            double      y,
            GtkGesture *gesture)
{
  TabInfo *info;
  GdkEvent *event;
  GdkEventSequence *current;
  guint button;

  if (is_touchscreen (gesture))
    return;

  x += gtk_adjustment_get_value (self->adjustment);

  info = find_tab_info_at (self, x);

  if (!info || !info->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  current = gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  event = gtk_gesture_get_last_event (gesture, current);

   if (gdk_event_triggers_context_menu (event)) {
    do_popup (self, info, x, y);
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));

    return;
  }

  button = gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

  if (button == GDK_BUTTON_MIDDLE) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
    adw_tab_view_close_page (self->view, info->page);

    return;
  }

  if (button != GDK_BUTTON_PRIMARY) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  handle_click (self, info, gesture);
}

static void
released_cb (AdwTabBox  *self,
             int         n_press,
             double      x,
             double      y,
             GtkGesture *gesture)
{
  TabInfo *info;

  if (!is_touchscreen (gesture))
    return;

  x += gtk_adjustment_get_value (self->adjustment);

  info = find_tab_info_at (self, x);

  if (!info || !info->page) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  handle_click (self, info, gesture);
}

/* Overrides */

static void
adw_tab_box_measure (GtkWidget      *widget,
                     GtkOrientation  orientation,
                     int             for_size,
                     int            *minimum,
                     int            *natural,
                     int            *minimum_baseline,
                     int            *natural_baseline)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);
  int min, nat;

  if (self->n_tabs == 0) {
    if (minimum)
      *minimum = 0;

    if (natural)
      *natural = 0;

    if (minimum_baseline)
      *minimum_baseline = -1;

    if (natural_baseline)
      *natural_baseline = -1;

    return;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    int width = self->end_padding;
    GList *l;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;
      int child_width;

      gtk_widget_measure (GTK_WIDGET (info->tab), orientation, -1,
                          NULL, &child_width, NULL, NULL);

      width += calculate_tab_width (info, child_width) - OVERLAP;
    }

    if (!self->pinned)
      width -= OVERLAP;

    min = nat = MAX (self->last_width, width);
  } else {
    GList *l;

    min = nat = 0;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;
      int child_min, child_nat;

      gtk_widget_measure (GTK_WIDGET (info->tab), orientation, -1,
                          &child_min, &child_nat, NULL, NULL);

      if (child_min > min)
        min = child_min;

      if (child_nat > nat)
        nat = child_nat;
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
adw_tab_box_size_allocate (GtkWidget *widget,
                           int        width,
                           int        height,
                           int        baseline)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);
  gboolean is_rtl;
  GList *l;
  GtkAllocation child_allocation;
  int pos;
  double value;

  adw_tab_box_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                       &self->allocated_width, NULL, NULL, NULL);
  self->allocated_width = MAX (self->allocated_width, width);

  value = gtk_adjustment_get_value (self->adjustment);

  gtk_adjustment_configure (self->adjustment,
                            value,
                            0,
                            self->allocated_width,
                            width * 0.1,
                            width * 0.9,
                            width);

  if (self->context_menu)
    gtk_popover_present (self->context_menu);

  if (!self->n_tabs)
    return;

  is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;

  if (self->pinned) {
    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;
      int child_width;

      gtk_widget_measure (GTK_WIDGET (info->tab), GTK_ORIENTATION_HORIZONTAL, -1,
                          NULL, &child_width, NULL, NULL);

      info->width = calculate_tab_width (info, child_width);
    }
  } else if (self->tab_resize_mode == TAB_RESIZE_FIXED_TAB_WIDTH) {
    self->end_padding = self->allocated_width + OVERLAP;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      info->width = calculate_tab_width (info, info->last_width);
      self->end_padding -= info->width - OVERLAP;
    }
  } else {
    int tab_width = get_base_tab_width (self, FALSE);
    int excess = self->allocated_width + OVERLAP - self->end_padding;

    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      info->width = calculate_tab_width (info, tab_width);
      excess -= info->width - OVERLAP;
    }

    /* Now spread excess width across the tabs */
    for (l = self->tabs; l; l = l->next) {
      TabInfo *info = l->data;

      if (excess >= 0)
          break;

      info->width--;
      excess++;
    }
  }

  pos = is_rtl ? self->allocated_width + OVERLAP : -OVERLAP;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    if (!info->appear_animation)
      adw_tab_set_display_width (info->tab, info->width);
    else if (info->page && info != self->reorder_placeholder)
      adw_tab_set_display_width (info->tab, predict_tab_width (self, info, FALSE));

    info->pos = pos + calculate_tab_offset (self, info, FALSE);

    if (is_rtl)
      info->pos -= info->width;

    child_allocation.x = ((info == self->reordered_tab) ? self->reorder_window_x : info->pos) - value;
    child_allocation.y = 0;
    child_allocation.width = info->width;
    child_allocation.height = height;

    gtk_widget_size_allocate (GTK_WIDGET (info->tab), &child_allocation, baseline);

    pos += (is_rtl ? -1 : 1) * (info->width - OVERLAP);
  }

  if (self->scheduled_scroll.info) {
    scroll_to_tab_full (self,
                        self->scheduled_scroll.info,
                        self->scheduled_scroll.pos,
                        self->scheduled_scroll.duration,
                        self->scheduled_scroll.keep_selected_visible);
    self->scheduled_scroll.info = NULL;
  }

  if (self->scroll_animation) {
    self->block_scrolling = TRUE;
    gtk_adjustment_set_value (self->adjustment,
                              get_scroll_animation_value (self));
    self->block_scrolling = FALSE;

    if (self->scroll_animation_done) {
        self->scroll_animation_done = FALSE;
        self->scroll_animation_tab = NULL;
        g_clear_pointer (&self->scroll_animation, adw_animation_unref);
    }
  }

  update_visible (self);
}

static gboolean
adw_tab_box_focus (GtkWidget        *widget,
                   GtkDirectionType  direction)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);

  if (!self->selected_tab)
    return GDK_EVENT_PROPAGATE;

  return gtk_widget_child_focus (GTK_WIDGET (self->selected_tab->tab), direction);
}

static void
adw_tab_box_unrealize (GtkWidget *widget)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);

  g_clear_pointer ((GtkWidget **) &self->context_menu, gtk_widget_unparent);

  GTK_WIDGET_CLASS (adw_tab_box_parent_class)->unrealize (widget);
}

static void
adw_tab_box_unmap (GtkWidget *widget)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);

  force_end_reordering (self);

  if (self->drag_autoscroll_cb_id) {
    gtk_widget_remove_tick_callback (widget, self->drag_autoscroll_cb_id);
    self->drag_autoscroll_cb_id = 0;
  }

  self->hovering = FALSE;
  update_hover (self);

  GTK_WIDGET_CLASS (adw_tab_box_parent_class)->unmap (widget);
}

static void
adw_tab_box_direction_changed (GtkWidget        *widget,
                               GtkTextDirection  previous_direction)
{
  AdwTabBox *self = ADW_TAB_BOX (widget);
  double upper, page_size;

  if (!self->adjustment)
    return;

  if (gtk_widget_get_direction (widget) == previous_direction)
    return;

  upper = gtk_adjustment_get_upper (self->adjustment);
  page_size = gtk_adjustment_get_page_size (self->adjustment);

  gtk_adjustment_set_value (self->adjustment,
                            upper - page_size - self->adjustment_prev_value);

  if (self->context_menu) {
    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      gtk_widget_set_halign (GTK_WIDGET (self->context_menu), GTK_ALIGN_END);
    else
      gtk_widget_set_halign (GTK_WIDGET (self->context_menu), GTK_ALIGN_START);
  }
}

static void
adw_tab_box_dispose (GObject *object)
{
  AdwTabBox *self = ADW_TAB_BOX (object);

  g_clear_handle_id (&self->drop_switch_timeout_id, g_source_remove);

  self->drag_gesture = NULL;
  self->tab_bar = NULL;
  adw_tab_box_set_view (self, NULL);
  adw_tab_box_set_adjustment (self, NULL);

  G_OBJECT_CLASS (adw_tab_box_parent_class)->dispose (object);
}

static void
adw_tab_box_finalize (GObject *object)
{
  AdwTabBox *self = (AdwTabBox *) object;

  g_clear_pointer (&self->extra_drag_types, g_free);

  G_OBJECT_CLASS (adw_tab_box_parent_class)->finalize (object);
}

static void
adw_tab_box_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  AdwTabBox *self = ADW_TAB_BOX (object);

  switch (prop_id) {
  case PROP_PINNED:
    g_value_set_boolean (value, self->pinned);
    break;

  case PROP_TAB_BAR:
    g_value_set_object (value, self->tab_bar);
    break;

  case PROP_VIEW:
    g_value_set_object (value, self->view);
    break;

  case PROP_NEEDS_ATTENTION_LEFT:
    g_value_set_boolean (value, self->needs_attention_left);
    break;

  case PROP_NEEDS_ATTENTION_RIGHT:
    g_value_set_boolean (value, self->needs_attention_right);
    break;

  case PROP_RESIZE_FROZEN:
    g_value_set_boolean (value, self->tab_resize_mode != TAB_RESIZE_NORMAL);
    break;

  case PROP_HADJUSTMENT:
    g_value_set_object (value, self->adjustment);
    break;

  case PROP_VADJUSTMENT:
  case PROP_HSCROLL_POLICY:
  case PROP_VSCROLL_POLICY:
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_box_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  AdwTabBox *self = ADW_TAB_BOX (object);

  switch (prop_id) {
  case PROP_PINNED:
    self->pinned = g_value_get_boolean (value);
    break;

  case PROP_TAB_BAR:
    self->tab_bar = g_value_get_object (value);
    break;

  case PROP_VIEW:
    adw_tab_box_set_view (self, g_value_get_object (value));
    break;

  case PROP_HADJUSTMENT:
    adw_tab_box_set_adjustment (self, g_value_get_object (value));
    break;

  case PROP_VADJUSTMENT:
  case PROP_HSCROLL_POLICY:
  case PROP_VSCROLL_POLICY:
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_box_class_init (AdwTabBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_tab_box_dispose;
  object_class->finalize = adw_tab_box_finalize;
  object_class->get_property = adw_tab_box_get_property;
  object_class->set_property = adw_tab_box_set_property;

  widget_class->measure = adw_tab_box_measure;
  widget_class->size_allocate = adw_tab_box_size_allocate;
  widget_class->focus = adw_tab_box_focus;
  widget_class->unrealize = adw_tab_box_unrealize;
  widget_class->unmap = adw_tab_box_unmap;
  widget_class->direction_changed = adw_tab_box_direction_changed;

  props[PROP_PINNED] =
    g_param_spec_boolean ("pinned",
                          "Pinned",
                          "Pinned",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  props[PROP_TAB_BAR] =
    g_param_spec_object ("tab-bar",
                         "Tab Bar",
                         "Tab Bar",
                         ADW_TYPE_TAB_BAR,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  props[PROP_VIEW] =
    g_param_spec_object ("view",
                         "View",
                         "View",
                         ADW_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_NEEDS_ATTENTION_LEFT] =
    g_param_spec_boolean ("needs-attention-left",
                          "Needs Attention Left",
                          "Needs Attention Left",
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_NEEDS_ATTENTION_RIGHT] =
    g_param_spec_boolean ("needs-attention-right",
                          "Needs Attention Right",
                          "Needs Attention Right",
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_RESIZE_FROZEN] =
    g_param_spec_boolean ("resize-frozen",
                          "Resize Frozen",
                          "Resize Frozen",
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  g_object_class_override_property (object_class, PROP_HADJUSTMENT, "hadjustment");
  g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
  g_object_class_override_property (object_class, PROP_HSCROLL_POLICY, "hscroll-policy");
  g_object_class_override_property (object_class, PROP_VSCROLL_POLICY, "vscroll-policy");

  signals[SIGNAL_STOP_KINETIC_SCROLLING] =
    g_signal_new ("stop-kinetic-scrolling",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins, NULL, NULL,
                  G_TYPE_BOOLEAN,
                  2,
                  ADW_TYPE_TAB_PAGE,
                  G_TYPE_VALUE);

  gtk_widget_class_install_action (widget_class, "menu.popup", NULL, popup_menu_cb);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_F10, GDK_SHIFT_MASK, "menu.popup", NULL);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Menu, 0, "menu.popup", NULL);

  add_focus_bindings (widget_class, GDK_KEY_Page_Up,   GTK_DIR_TAB_BACKWARD, FALSE);
  add_focus_bindings (widget_class, GDK_KEY_Page_Down, GTK_DIR_TAB_FORWARD,  FALSE);
  add_focus_bindings (widget_class, GDK_KEY_Home,      GTK_DIR_TAB_BACKWARD, TRUE);
  add_focus_bindings (widget_class, GDK_KEY_End,       GTK_DIR_TAB_FORWARD,  TRUE);

  add_reorder_bindings (widget_class, GDK_KEY_Left,      GTK_DIR_LEFT,         FALSE);
  add_reorder_bindings (widget_class, GDK_KEY_Right,     GTK_DIR_RIGHT,        FALSE);
  add_reorder_bindings (widget_class, GDK_KEY_Page_Up,   GTK_DIR_TAB_BACKWARD, FALSE);
  add_reorder_bindings (widget_class, GDK_KEY_Page_Down, GTK_DIR_TAB_FORWARD,  FALSE);
  add_reorder_bindings (widget_class, GDK_KEY_Home,      GTK_DIR_TAB_BACKWARD, TRUE);
  add_reorder_bindings (widget_class, GDK_KEY_End,       GTK_DIR_TAB_FORWARD,  TRUE);

  gtk_widget_class_set_css_name (widget_class, "tabbox");
}

static void
adw_tab_box_init (AdwTabBox *self)
{
  GtkEventController *controller;

  self->can_remove_placeholder = TRUE;
  self->expand_tabs = TRUE;

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  controller = gtk_event_controller_motion_new ();
  g_signal_connect_swapped (controller, "motion", G_CALLBACK (motion_cb), self);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (leave_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
  g_signal_connect_swapped (controller, "scroll", G_CALLBACK (scroll_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), 0);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "pressed", G_CALLBACK (pressed_cb), self);
  g_signal_connect_swapped (controller, "released", G_CALLBACK (released_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_long_press_new ());
  gtk_gesture_long_press_set_delay_factor (GTK_GESTURE_LONG_PRESS (controller), 2);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "pressed", G_CALLBACK (long_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), GDK_BUTTON_PRIMARY);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "drag-begin", G_CALLBACK (reorder_begin_cb), self);
  g_signal_connect_swapped (controller, "drag-update", G_CALLBACK (reorder_update_cb), self);
  g_signal_connect_swapped (controller, "drag-end", G_CALLBACK (reorder_end_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);
  self->drag_gesture = GTK_GESTURE (controller);

  controller = gtk_drop_controller_motion_new ();
  g_signal_connect_swapped (controller, "enter", G_CALLBACK (drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "motion", G_CALLBACK (drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (drag_leave_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_drop_target_new (ADW_TYPE_TAB_PAGE, GDK_ACTION_MOVE));
  gtk_drop_target_set_preload (GTK_DROP_TARGET (controller), TRUE);
  g_signal_connect_swapped (controller, "enter", G_CALLBACK (tab_drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "motion", G_CALLBACK (tab_drag_enter_motion_cb), self);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (tab_drag_leave_cb), self);
  g_signal_connect_swapped (controller, "drop", G_CALLBACK (tab_drag_drop_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);
}

void
adw_tab_box_set_view (AdwTabBox  *self,
                      AdwTabView *view)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));
  g_return_if_fail (ADW_IS_TAB_VIEW (view) || view == NULL);

  if (view == self->view)
    return;

  if (self->view) {
    force_end_reordering (self);
    g_signal_handlers_disconnect_by_func (self->view, page_attached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_detached_cb, self);
    g_signal_handlers_disconnect_by_func (self->view, page_reordered_cb, self);

    if (!self->pinned) {
      gtk_widget_remove_controller (GTK_WIDGET (self->view), self->view_drop_target);
      self->view_drop_target = NULL;
    }

    g_list_free_full (self->tabs, (GDestroyNotify) remove_and_free_tab_info);

    self->tabs = NULL;
    self->n_tabs = 0;
  }

  self->view = view;

  if (self->view) {
    int i, n_pages = adw_tab_view_get_n_pages (self->view);

    for (i = n_pages - 1; i >= 0; i--)
      page_attached_cb (self, adw_tab_view_get_nth_page (self->view, i), 0);

    g_signal_connect_object (self->view, "page-attached", G_CALLBACK (page_attached_cb), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-detached", G_CALLBACK (page_detached_cb), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->view, "page-reordered", G_CALLBACK (page_reordered_cb), self, G_CONNECT_SWAPPED);

    if (!self->pinned) {
      self->view_drop_target = GTK_EVENT_CONTROLLER (gtk_drop_target_new (ADW_TYPE_TAB_PAGE, GDK_ACTION_MOVE));

      g_signal_connect_object (self->view_drop_target, "drop", G_CALLBACK (view_drag_drop_cb), self, G_CONNECT_SWAPPED);

      gtk_widget_add_controller (GTK_WIDGET (self->view), self->view_drop_target);
    }
  }

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW]);
}

void
adw_tab_box_set_adjustment (AdwTabBox     *self,
                            GtkAdjustment *adjustment)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));
  g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment) || adjustment == NULL);

  if (adjustment == self->adjustment)
    return;

  if (self->adjustment) {
    g_signal_handlers_disconnect_by_func (self->adjustment, adjustment_value_changed_cb, self);
    g_signal_handlers_disconnect_by_func (self->adjustment, update_visible, self);
  }

  g_set_object (&self->adjustment, adjustment);

  if (self->adjustment) {
    g_signal_connect_object (self->adjustment, "value-changed", G_CALLBACK (adjustment_value_changed_cb), self, G_CONNECT_SWAPPED);
    g_signal_connect_object (self->adjustment, "notify::page-size", G_CALLBACK (update_visible), self, G_CONNECT_SWAPPED);
  }

  g_object_notify (G_OBJECT (self), "hadjustment");
}

void
adw_tab_box_attach_page (AdwTabBox  *self,
                         AdwTabPage *page,
                         int         position)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));

  page_attached_cb (self, page, position);
}

void
adw_tab_box_detach_page (AdwTabBox  *self,
                         AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page));

  page_detached_cb (self, page);
}

void
adw_tab_box_select_page (AdwTabBox  *self,
                         AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));
  g_return_if_fail (ADW_IS_TAB_PAGE (page) || page == NULL);

  select_page (self, page);
}

void
adw_tab_box_try_focus_selected_tab (AdwTabBox *self)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));

  if (self->selected_tab)
    gtk_widget_grab_focus (GTK_WIDGET (self->selected_tab->tab));
}

gboolean
adw_tab_box_is_page_focused (AdwTabBox  *self,
                             AdwTabPage *page)
{
  TabInfo *info;

  g_return_val_if_fail (ADW_IS_TAB_BOX (self), FALSE);
  g_return_val_if_fail (ADW_IS_TAB_PAGE (page), FALSE);

  info = find_info_for_page (self, page);

  return info && gtk_widget_is_focus (GTK_WIDGET (info->tab));
}

void
adw_tab_box_setup_extra_drop_target (AdwTabBox     *self,
                                     GdkDragAction  actions,
                                     GType         *types,
                                     gsize          n_types)
{
  GList *l;

  g_return_if_fail (ADW_IS_TAB_BOX (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  g_clear_pointer (&self->extra_drag_types, g_free);

  self->extra_drag_actions = actions;
#if GLIB_CHECK_VERSION(2, 67, 3)
  self->extra_drag_types = g_memdup2 (types, sizeof (GType) * n_types);
#else
  self->extra_drag_types = g_memdup (types, sizeof (GType) * n_types);
#endif
  self->extra_drag_n_types = n_types;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    adw_tab_setup_extra_drop_target (info->tab,
                                     self->extra_drag_actions,
                                     self->extra_drag_types,
                                     self->extra_drag_n_types);
  }
}

gboolean
adw_tab_box_get_expand_tabs (AdwTabBox *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BOX (self), FALSE);

  return self->expand_tabs;
}

void
adw_tab_box_set_expand_tabs (AdwTabBox *self,
                             gboolean   expand_tabs)
{
  g_return_if_fail (ADW_IS_TAB_BOX (self));

  expand_tabs = !!expand_tabs;

  if (expand_tabs == self->expand_tabs)
    return;

  self->expand_tabs = expand_tabs;

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

gboolean
adw_tab_box_get_inverted (AdwTabBox *self)
{
  g_return_val_if_fail (ADW_IS_TAB_BOX (self), FALSE);

  return self->inverted;
}

void
adw_tab_box_set_inverted (AdwTabBox *self,
                          gboolean   inverted)
{
  GList *l;

  g_return_if_fail (ADW_IS_TAB_BOX (self));

  inverted = !!inverted;

  if (inverted == self->inverted)
    return;

  self->inverted = inverted;

  for (l = self->tabs; l; l = l->next) {
    TabInfo *info = l->data;

    adw_tab_set_inverted (info->tab, inverted);
  }
}
