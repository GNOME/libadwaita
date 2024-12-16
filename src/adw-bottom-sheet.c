/*
 * Copyright (C) 2023-2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-bottom-sheet-private.h"

#include <math.h>

#include "adw-animation-target.h"
#include "adw-animation-util.h"
#include "adw-bin.h"
#include "adw-gizmo-private.h"
#include "adw-marshalers.h"
#include "adw-spring-animation.h"
#include "adw-swipeable.h"
#include "adw-swipe-tracker-private.h"
#include "adw-timed-animation.h"
#include "adw-widget-utils-private.h"

#define TOP_PADDING_MIN_HEIGHT 720
#define TOP_PADDING_MIN_VALUE 30
#define TOP_PADDING_TARGET_HEIGHT 1440
#define TOP_PADDING_TARGET_VALUE 120
#define CHILD_SWITCH_THRESHOLD 0.15
#define REVEAL_BOTTOM_BAR_DURATION 250

/**
 * AdwBottomSheet:
 *
 * A bottom sheet with an optional bottom bar.
 *
 * <picture>
 *   <source srcset="bottom-sheet-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="bottom-sheet.png" alt="bottom-sheet">
 * </picture>
 *
 * `AdwBottomSheet` has three child widgets. [property@BottomSheet:content] is
 * shown persistently. [property@BottomSheet:sheet] is displayed above it when
 * it's open, and [property@BottomSheet:bottom-bar] is displayed when it's not.
 *
 * Bottom sheet and bottom bar are attached to the bottom edge of the widget.
 * They take the full width by default, but can only take a portion of it if
 * [property@BottomSheet:full-width] is set to `FALSE`. In this case,
 * [property@BottomSheet:align] determines where along the bottom edge they are
 * placed.
 *
 * Bottom bar can be hidden using the [property@BottomSheet:reveal-bottom-bar]
 * property.
 *
 * `AdwBottomSheet` can be useful for applications such as music players, that
 * want to have a persistent bottom bar that expands into a bottom sheet when
 * clicked. It's meant for cases where a bottom sheet is tightly integrated into
 * the UI. For more transient bottom sheets, see [class@Dialog].
 *
 * To open or close the bottom sheet, use the [property@BottomSheet:open]
 * property.
 *
 * By default, the bottom sheet has an overlaid drag handle. It can be disabled
 * by setting [property@BottomSheet:show-drag-handle] to `FALSE`. Note that the
 * handle also controls whether the sheet can be dragged using a pointer.
 *
 * Bottom sheets are modal by default, meaning that the content is dimmed and
 * cannot be accessed while the sheet is open. Set [property@BottomSheet:modal]
 * to `FALSE` if this behavior is unwanted.
 *
 * To disable user interactions for opening or closing the bottom sheet (such as
 * swipes or clicking the bottom bar or close button), set
 * [property@BottomSheet:can-open] or [property@BottomSheet:can-close] to
 * `FALSE`.
 *
 * In some cases, particularly when using a full-width bottom bar, it may be
 * necessary to shift [property@BottomSheet:content] upwards. Use the
 * [property@BottomSheet:bottom-bar-height] and
 * [property@BottomSheet:sheet-height] for that.
 *
 * `AdwBottomSheet` is not adaptive, and for larger window sizes applications
 * may want to replace it with another UI, such as a sidebar. This can be done
 * using [class@MultiLayoutView].
 *
 * ## Sizing
 *
 * Unlike [class@Dialog] presented as a bottom sheet, `AdwBottomSheet` just
 * follows the content's natural size, and it's up to the applications to make
 * sure their content provides one. For example, when using
 * [class@Gtk.ScrolledWindow], make sure to set
 * [property@Gtk.ScrolledWindow:propagate-natural-height] to `TRUE`.
 *
 * ## Header Bar Integration
 *
 * When placed inside an `AdwBottomSheet`, [class@HeaderBar] will not show the
 * title when [property@BottomSheet:show-drag-handle] is `TRUE`, regardless of
 * [property@HeaderBar:show-title]. This only applies to the default title,
 * titles set with [property@HeaderBar:title-widget] will still be shown.
 *
 * ## `AdwBottomSheet` as `GtkBuildable`:
 *
 * The `AdwBottomSheet` implementation of the [iface@Gtk.Buildable] interface
 * supports setting the sheet widget by specifying “sheet” as the “type”
 * attribute of a `<child>` element, and the bottom bar by specifying
 * “bottom-bar”. Specifying “content” or omitting the child type results in
 * setting the content child.
 *
 * Since: 1.6
 */

struct _AdwBottomSheet
{
  GtkWidget parent_instance;

  GtkWidget *content;
  GtkWidget *sheet;
  GtkWidget *bottom_bar;

  GtkWidget *content_bin;
  GtkWidget *sheet_page;
  GtkWidget *sheet_stack;
  GtkWidget *sheet_bin;
  GtkWidget *dimming;
  GtkWidget *bottom_bar_bin;

  GtkWidget *drag_handle;
  GtkWidget *outline;

  gboolean open;

  AdwAnimation *open_animation;
  double progress;
  float align;
  gboolean full_width;

  gboolean switch_child;
  gboolean showing_bottom_bar;

  AdwAnimation *reveal_bottom_bar_animation;
  double reveal_bottom_bar_progress;
  gboolean reveal_bottom_bar;

  gboolean show_drag_handle;
  gboolean modal;
  gboolean can_open;
  gboolean can_close;

  gboolean has_been_open;

  AdwSwipeTracker *swipe_tracker;
  gboolean swipe_detected;
  gboolean swipe_active;

  int sheet_height;
  int bottom_bar_height;

  GtkWidget *last_content_focus;
  GtkWidget *last_sheet_focus;

  int min_natural_width;

  GFunc closing_callback;
  GFunc closed_callback;
  gpointer user_data;
};

static void adw_bottom_sheet_buildable_init (GtkBuildableIface *iface);
static void adw_bottom_sheet_swipeable_init (AdwSwipeableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwBottomSheet, adw_bottom_sheet, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_bottom_sheet_buildable_init)
                               G_IMPLEMENT_INTERFACE (ADW_TYPE_SWIPEABLE, adw_bottom_sheet_swipeable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_SHEET,
  PROP_BOTTOM_BAR,
  PROP_OPEN,
  PROP_ALIGN,
  PROP_FULL_WIDTH,
  PROP_SHOW_DRAG_HANDLE,
  PROP_MODAL,
  PROP_CAN_OPEN,
  PROP_CAN_CLOSE,
  PROP_SHEET_HEIGHT,
  PROP_BOTTOM_BAR_HEIGHT,
  PROP_REVEAL_BOTTOM_BAR,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_CLOSE_ATTEMPT,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
released_cb (GtkGestureClick *gesture,
             int              n_press,
             double           x,
             double           y,
             AdwBottomSheet  *self)
{
  if (self->swipe_active) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (!self->can_close)
    g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
  else
    adw_bottom_sheet_set_open (self, FALSE);

  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
  gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));
}

static void
bottom_bar_pressed_cb (GtkGestureClick *gesture,
                       int              n_press,
                       double           x,
                       double           y,
                       AdwBottomSheet  *self)
{
  if (self->swipe_active || !self->can_open)
    return;

  if (gtk_widget_has_focus (GTK_WIDGET (self->bottom_bar_bin)))
    return;

  gtk_widget_grab_focus (GTK_WIDGET (self->bottom_bar_bin));
}

static void
bottom_bar_released_cb (GtkGestureClick *gesture,
                        int              n_press,
                        double           x,
                        double           y,
                        AdwBottomSheet  *self)
{
  if (self->swipe_active || !self->can_open) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (!gtk_widget_contains (GTK_WIDGET (self->bottom_bar_bin), x, y)) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (self->can_open)
    adw_bottom_sheet_set_open (self, TRUE);

  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
  gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));
}

static void
show_bottom_bar (AdwBottomSheet *self,
                 gboolean        show)
{
  if (show == self->showing_bottom_bar)
    return;

  self->showing_bottom_bar = show;

  if (!self->bottom_bar)
    return;

  gtk_stack_set_visible_child (GTK_STACK (self->sheet_stack),
                               show ? self->bottom_bar_bin : self->sheet_page);

  if (show)
    gtk_widget_add_css_class (self->sheet_bin, "bottom-bar");
  else
    gtk_widget_remove_css_class (self->sheet_bin, "bottom-bar");
}

static void
open_animation_cb (double          value,
                   AdwBottomSheet *self)
{
  double last_progress = self->progress;

  self->progress = value;

  gtk_widget_set_opacity (self->dimming, CLAMP (value, 0, 1));
  gtk_widget_queue_allocate (GTK_WIDGET (self));

  if (self->switch_child || self->swipe_active) {
    if (last_progress < CHILD_SWITCH_THRESHOLD && value >= CHILD_SWITCH_THRESHOLD) {
      show_bottom_bar (self, FALSE);

      self->switch_child = FALSE;
    } else if (last_progress >= CHILD_SWITCH_THRESHOLD && value < CHILD_SWITCH_THRESHOLD) {
      show_bottom_bar (self, TRUE);

      self->switch_child = FALSE;
    }
  }
}

static void
open_animation_done_cb (AdwBottomSheet *self)
{
  if (self->progress < 0.5) {
    gtk_widget_set_child_visible (self->dimming, FALSE);
    gtk_widget_set_child_visible (self->sheet_bin,
                                  self->bottom_bar != NULL &&
                                  self->reveal_bottom_bar);

    if (self->closed_callback)
      self->closed_callback (self, self->user_data);
  }

  self->switch_child = FALSE;
}

static void
reveal_animation_cb (double          value,
                     AdwBottomSheet *self)
{
  self->reveal_bottom_bar_progress = value;

  gtk_widget_queue_allocate (GTK_WIDGET (self));
}

static void
reveal_animation_done_cb (AdwBottomSheet *self)
{
  if (!self->reveal_bottom_bar && G_APPROX_VALUE (self->progress, 0, DBL_EPSILON))
    gtk_widget_set_child_visible (self->sheet_bin, FALSE);
}

static void
sheet_close_cb (AdwBottomSheet *self)
{
  GtkWidget *parent;

  if (!self->can_close) {
    g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
    return;
  }

  if (self->open) {
    adw_bottom_sheet_set_open (self, FALSE);
    return;
  }

  parent = gtk_widget_get_parent (GTK_WIDGET (self));

  if (parent)
    gtk_widget_activate_action (parent, "sheet.close", NULL);
}

static gboolean
maybe_close_cb (GtkWidget      *widget,
                GVariant       *args,
                AdwBottomSheet *self)
{
  if (self->can_close && self->open) {
    adw_bottom_sheet_set_open (self, FALSE);
    return GDK_EVENT_STOP;
  }

  g_signal_emit (self, signals[SIGNAL_CLOSE_ATTEMPT], 0);
  return GDK_EVENT_STOP;
}

static void
bottom_bar_clicked_cb (AdwBottomSheet *self)
{
  if (self->can_open)
    adw_bottom_sheet_set_open (self, TRUE);
}

static void
set_heights (AdwBottomSheet *self,
             int             sheet_height,
             int             bottom_bar_height)
{
  if (self->sheet_height != sheet_height) {
    self->sheet_height = sheet_height;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHEET_HEIGHT]);
  }

  if (self->bottom_bar_height != bottom_bar_height) {
    self->bottom_bar_height = bottom_bar_height;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BOTTOM_BAR_HEIGHT]);
  }
}

static void
disable_button_click (GtkWidget *widget)
{
  GListModel *controllers;
  guint i, n;

  /* XXX: We shouldn't be doing this, but button click gesture has capture phase
   * and so clicks are handler even on e.g. nested buttons and entries */
  controllers = gtk_widget_observe_controllers (widget);

  n = g_list_model_get_n_items (controllers);
  for (i = 0; i < n; i++) {
    GtkEventController *controller = g_list_model_get_item (controllers, i);

    if (GTK_IS_GESTURE_CLICK (controller))
      gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_NONE);

    g_object_unref (controller);
  }

  g_object_unref (controllers);
}

static void
update_swipe_tracker (AdwBottomSheet *self)
{
  adw_swipe_tracker_set_enabled (self->swipe_tracker,
                                 (self->can_open && self->bottom_bar != NULL) ||
                                  self->can_close);

  adw_swipe_tracker_set_allow_mouse_drag (self->swipe_tracker,
                                          self->show_drag_handle ||
                                          self->bottom_bar != NULL);

  adw_swipe_tracker_set_lower_overshoot (self->swipe_tracker,
                                         self->bottom_bar != NULL);
}

static void
measure_sheet (GtkWidget      *widget,
               GtkOrientation  orientation,
               int             for_size,
               int            *minimum,
               int            *natural,
               int            *minimum_baseline,
               int            *natural_baseline)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (gtk_widget_get_ancestor (widget, ADW_TYPE_BOTTOM_SHEET));
  int sheet_min, sheet_nat, handle_min, handle_nat;

  if (self->sheet && gtk_widget_should_layout (self->sheet)) {
    gtk_widget_measure (self->sheet, orientation, for_size,
                        &sheet_min, &sheet_nat, NULL, NULL);
  } else {
    sheet_min = sheet_nat = 0;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL && self->min_natural_width >= 0)
    sheet_nat = MAX (sheet_nat, self->min_natural_width);

  if (gtk_widget_should_layout (self->drag_handle)) {
    gtk_widget_measure (self->drag_handle, orientation, for_size,
                        &handle_min, &handle_nat, NULL, NULL);
  } else {
    handle_min = handle_nat = 0;
  }

  if (minimum)
    *minimum = MAX (sheet_min, handle_min);
  if (natural)
    *natural = MAX (sheet_nat, handle_nat);
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_sheet (GtkWidget *widget,
                int        width,
                int        height,
                int        baseline)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (gtk_widget_get_ancestor (widget, ADW_TYPE_BOTTOM_SHEET));

  if (gtk_widget_should_layout (self->drag_handle)) {
    int handle_width, handle_height, handle_x;
    GskTransform *transform;

    gtk_widget_measure (self->drag_handle, GTK_ORIENTATION_HORIZONTAL, -1,
                        NULL, &handle_width, NULL, NULL);
    gtk_widget_measure (self->drag_handle, GTK_ORIENTATION_VERTICAL, -1,
                        NULL, &handle_height, NULL, NULL);

    handle_width = MIN (handle_width, width);
    handle_height = MIN (handle_height, height);

    handle_x = round ((width - handle_width) / 2);

    transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (handle_x, 0));

    gtk_widget_allocate (self->drag_handle, handle_width, handle_height,
                         baseline, transform);
  }

  if (self->sheet && gtk_widget_should_layout (self->sheet))
    gtk_widget_allocate (self->sheet, width, height, baseline, NULL);
}

static void
adw_bottom_sheet_measure (GtkWidget      *widget,
                          GtkOrientation  orientation,
                          int             for_size,
                          int            *minimum,
                          int            *natural,
                          int            *minimum_baseline,
                          int            *natural_baseline)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (widget);
  int content_min, content_nat, dim_min, dim_nat, sheet_min, sheet_nat;

  if (gtk_widget_should_layout (self->content_bin)) {
    gtk_widget_measure (self->content_bin, orientation, for_size,
                        &content_min, &content_nat, NULL, NULL);
  } else {
    content_min = content_nat = 0;
  }

  gtk_widget_measure (self->dimming, orientation, for_size,
                      &dim_min, &dim_nat, NULL, NULL);

  gtk_widget_measure (self->sheet_bin, orientation, for_size,
                      &sheet_min, &sheet_nat, NULL, NULL);

  if (minimum)
    *minimum = MAX (content_min, MAX (dim_min, sheet_min));
  if (natural)
    *natural = MAX (content_nat, MAX (dim_nat, sheet_nat));
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
adw_bottom_sheet_size_allocate (GtkWidget *widget,
                                int        width,
                                int        height,
                                int        baseline)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (widget);
  GskTransform *transform;
  int sheet_x, sheet_y, sheet_min_width, sheet_width, sheet_min_height, sheet_height;
  int top_padding;
  int bottom_bar_height;
  float align;

  if (width == 0 && height == 0)
    return;

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sheet_min_width, &sheet_width, NULL, NULL);

  if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    align = 1 - self->align;
  else
    align = self->align;

  if (self->full_width)
    sheet_width = MAX (width, sheet_min_width);
  else
    sheet_width = MAX (MIN (sheet_width, width), sheet_min_width);

  sheet_x = round ((width - sheet_width) * align);

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  if (self->bottom_bar) {
    int bottom_bar_min_height;

    gtk_widget_measure (self->bottom_bar_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                        &bottom_bar_min_height, &bottom_bar_height, NULL, NULL);

    bottom_bar_height = MAX (MIN (bottom_bar_height, height), bottom_bar_min_height);

    bottom_bar_height = round (adw_lerp (0, bottom_bar_height, self->reveal_bottom_bar_progress));
  } else {
    bottom_bar_height = 0;
  }

  top_padding = adw_lerp (TOP_PADDING_MIN_VALUE,
                          TOP_PADDING_TARGET_VALUE,
                          MAX (0, (height - TOP_PADDING_MIN_HEIGHT) /
                                  (double) (TOP_PADDING_TARGET_HEIGHT -
                                            TOP_PADDING_MIN_HEIGHT)));

  sheet_height = MAX (MIN (sheet_height, height - top_padding), sheet_min_height);

  sheet_y = height - round (adw_lerp (bottom_bar_height, sheet_height, self->progress));

  set_heights (self,
               MAX (MIN (sheet_height, height - sheet_y), bottom_bar_height),
               bottom_bar_height);

  sheet_height = MAX (sheet_height, height - sheet_y);

  if (sheet_x == 0)
    gtk_widget_add_css_class (self->sheet_bin, "flush-left");
  else
    gtk_widget_remove_css_class (self->sheet_bin, "flush-left");

  if (sheet_x == width - sheet_width)
    gtk_widget_add_css_class (self->sheet_bin, "flush-right");
  else
    gtk_widget_remove_css_class (self->sheet_bin, "flush-right");

  if (gtk_widget_should_layout (self->content_bin))
    gtk_widget_allocate (self->content_bin, width, height, baseline, NULL);

  gtk_widget_allocate (self->dimming, width, height, baseline, NULL);

  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sheet_x, sheet_y));
  gtk_widget_allocate (self->sheet_bin, sheet_width, sheet_height, baseline, transform);
}

static void
adw_bottom_sheet_dispose (GObject *object)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (object);

  g_clear_weak_pointer (&self->last_content_focus);
  g_clear_weak_pointer (&self->last_sheet_focus);

  g_clear_object (&self->swipe_tracker);
  g_clear_pointer (&self->content_bin, gtk_widget_unparent);
  g_clear_pointer (&self->dimming, gtk_widget_unparent);
  g_clear_pointer (&self->sheet_bin, gtk_widget_unparent);
  g_clear_object (&self->open_animation);
  self->content = NULL;
  self->sheet = NULL;
  self->sheet_stack = NULL;
  self->sheet_page = NULL;
  self->drag_handle = NULL;
  self->outline = NULL;
  self->bottom_bar_bin = NULL;

  G_OBJECT_CLASS (adw_bottom_sheet_parent_class)->dispose (object);
}

static void
adw_bottom_sheet_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_bottom_sheet_get_content (self));
    break;
  case PROP_SHEET:
    g_value_set_object (value, adw_bottom_sheet_get_sheet (self));
    break;
  case PROP_BOTTOM_BAR:
    g_value_set_object (value, adw_bottom_sheet_get_bottom_bar (self));
    break;
  case PROP_OPEN:
    g_value_set_boolean (value, adw_bottom_sheet_get_open (self));
    break;
  case PROP_ALIGN:
    g_value_set_float (value, adw_bottom_sheet_get_align (self));
    break;
  case PROP_FULL_WIDTH:
    g_value_set_boolean (value, adw_bottom_sheet_get_full_width (self));
    break;
  case PROP_SHOW_DRAG_HANDLE:
    g_value_set_boolean (value, adw_bottom_sheet_get_show_drag_handle (self));
    break;
  case PROP_MODAL:
    g_value_set_boolean (value, adw_bottom_sheet_get_modal (self));
    break;
  case PROP_CAN_OPEN:
    g_value_set_boolean (value, adw_bottom_sheet_get_can_open (self));
    break;
  case PROP_CAN_CLOSE:
    g_value_set_boolean (value, adw_bottom_sheet_get_can_close (self));
    break;
  case PROP_SHEET_HEIGHT:
    g_value_set_int (value, adw_bottom_sheet_get_sheet_height (self));
    break;
  case PROP_BOTTOM_BAR_HEIGHT:
    g_value_set_int (value, adw_bottom_sheet_get_bottom_bar_height (self));
    break;
  case PROP_REVEAL_BOTTOM_BAR:
    g_value_set_boolean (value, adw_bottom_sheet_get_reveal_bottom_bar (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_bottom_sheet_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (object);

  switch (prop_id) {
  case PROP_CONTENT:
    adw_bottom_sheet_set_content (self, g_value_get_object (value));
    break;
  case PROP_SHEET:
    adw_bottom_sheet_set_sheet (self, g_value_get_object (value));
    break;
  case PROP_BOTTOM_BAR:
    adw_bottom_sheet_set_bottom_bar (self, g_value_get_object (value));
    break;
  case PROP_OPEN:
    adw_bottom_sheet_set_open (self, g_value_get_boolean (value));
    break;
  case PROP_ALIGN:
    adw_bottom_sheet_set_align (self, g_value_get_float (value));
    break;
  case PROP_FULL_WIDTH:
    adw_bottom_sheet_set_full_width (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_DRAG_HANDLE:
    adw_bottom_sheet_set_show_drag_handle (self, g_value_get_boolean (value));
    break;
  case PROP_MODAL:
    adw_bottom_sheet_set_modal (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_OPEN:
    adw_bottom_sheet_set_can_open (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_CLOSE:
    adw_bottom_sheet_set_can_close (self, g_value_get_boolean (value));
    break;
  case PROP_REVEAL_BOTTOM_BAR:
    adw_bottom_sheet_set_reveal_bottom_bar (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_bottom_sheet_class_init (AdwBottomSheetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_bottom_sheet_dispose;
  object_class->get_property = adw_bottom_sheet_get_property;
  object_class->set_property = adw_bottom_sheet_set_property;

  widget_class->contains = adw_widget_contains_passthrough;
  widget_class->measure = adw_bottom_sheet_measure;
  widget_class->size_allocate = adw_bottom_sheet_size_allocate;
  widget_class->get_request_mode = adw_widget_get_request_mode;
  widget_class->compute_expand = adw_widget_compute_expand;
  widget_class->focus = adw_widget_focus_child;
  widget_class->grab_focus = adw_widget_grab_focus_child;

  /**
   * AdwBottomSheet:content:
   *
   * The content widget.
   *
   * It's always shown, and the bottom sheet is overlaid over it.
   *
   * Since: 1.6
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:sheet:
   *
   * The bottom sheet widget.
   *
   * Only shown when [property@BottomSheet:open] is `TRUE`.
   *
   * Since: 1.6
   */
  props[PROP_SHEET] =
    g_param_spec_object ("sheet", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:bottom-bar:
   *
   * The bottom bar widget.
   *
   * Shown when [property@BottomSheet:open] is `FALSE`. When open, morphs into
   * the [property@BottomSheet:sheet].
   *
   * Bottom bar can be temporarily hidden using the
   * [property@BottomSheet:reveal-bottom-bar] property.
   *
   * Since: 1.6
   */
  props[PROP_BOTTOM_BAR] =
    g_param_spec_object ("bottom-bar", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:open:
   *
   * Whether the bottom sheet is open.
   *
   * Since: 1.6
   */
  props[PROP_OPEN] =
    g_param_spec_boolean ("open", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:align:
   *
   * Horizontal alignment of the bottom sheet.
   *
   * 0 means the bottom sheet is flush with the start edge, 1 means it's flush
   * with the end edge. 0.5 means it's centered.
   *
   * Only used when [property@BottomSheet:full-width] is set to `FALSE`.
   *
   * Since: 1.6
   */
  props[PROP_ALIGN] =
    g_param_spec_float ("align", NULL, NULL,
                        0, 1, 0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:full-width:
   *
   * Whether the bottom sheet takes the full width.
   *
   * When full width, [property@BottomSheet:align] is ignored.
   *
   * Since: 1.6
   */
  props[PROP_FULL_WIDTH] =
    g_param_spec_boolean ("full-width", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:show-drag-handle:
   *
   * Whether to overlay a drag handle in the bottom sheet.
   *
   * The handle will be overlaid over [property@BottomSheet:sheet].
   *
   * When the handle is shown, [class@HeaderBar] will hide its default title,
   * and [class@ToolbarView] will reserve space if there are no top bars.
   *
   * Showing drag handle also allows to swipe the bottom sheet down (and to
   * swipe the bottom bar up) with a pointer, instead of just touchscreen.
   *
   * Since: 1.6
   */
  props[PROP_SHOW_DRAG_HANDLE] =
    g_param_spec_boolean ("show-drag-handle", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:modal:
   *
   * Whether the bottom sheet is modal.
   *
   * When modal, [property@BottomSheet:content] will be dimmed when the bottom
   * sheet is open, and clicking it will close the bottom sheet. It also cannot
   * be focused with keyboard.
   *
   * Otherwise, the content is accessible even when the bottom sheet is open.
   *
   * Since: 1.6
   */
  props[PROP_MODAL] =
    g_param_spec_boolean ("modal", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:can-open:
   *
   * Whether the bottom sheet can be opened by user.
   *
   * It can be opened via clicking or swiping up from the bottom bar.
   *
   * Does nothing if [property@BottomSheet:bottom-bar] is not set.
   *
   * Bottom sheet can still be opened using [property@BottomSheet:open].
   *
   * Since: 1.6
   */
  props[PROP_CAN_OPEN] =
    g_param_spec_boolean ("can-open", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:can-close:
   *
   * Whether the bottom sheet can be closed by user.
   *
   * It can be closed via the close button, swiping down, pressing
   * <kbd>Escape</kbd> or clicking the content dimming (when modal).
   *
   * Bottom sheet can still be closed using [property@BottomSheet:open].
   *
   * Since: 1.6
   */
  props[PROP_CAN_CLOSE] =
    g_param_spec_boolean ("can-close", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwBottomSheet:sheet-height:
   *
   * The current bottom sheet height.
   *
   * It can be used to shift the content upwards when the bottom sheet is open.
   *
   * Since: 1.6
   */
  props[PROP_SHEET_HEIGHT] =
    g_param_spec_int ("sheet-height", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwBottomSheet:bottom-bar-height:
   *
   * The current bottom bar height.
   *
   * It can be used to shift the content upwards permanently to accommodate for
   * the bottom bar.
   *
   * Since: 1.6
   */
  props[PROP_BOTTOM_BAR_HEIGHT] =
    g_param_spec_int ("bottom-bar-height", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwBottomSheet:reveal-bottom-bar:
   *
   * Whether to reveal the bottom bar.
   *
   * The transition will be animated.
   *
   * See [property@BottomSheet:bottom-bar] and
   * [property@BottomSheet:bottom-bar-height].
   *
   * Since: 1.7
   */
  props[PROP_REVEAL_BOTTOM_BAR] =
    g_param_spec_boolean ("reveal-bottom-bar", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwBottomSheet::close-attempt:
   * @self: a bottom sheet
   *
   * Emitted when the close button or shortcut is used while
   * [property@Dialog:can-close] is set to `FALSE`.
   *
   * Since: 1.6
   */
  signals[SIGNAL_CLOSE_ATTEMPT] =
    g_signal_new ("close-attempt",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL,
                  adw_marshal_VOID__VOID,
                  G_TYPE_NONE,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_CLOSE_ATTEMPT],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__VOIDv);

  gtk_widget_class_install_action (widget_class, "sheet.close", NULL,
                                   (GtkWidgetActionActivateFunc) sheet_close_cb);

  gtk_widget_class_set_css_name (widget_class, "bottom-sheet");
}

static void
prepare_cb (AdwSwipeTracker        *tracker,
            AdwNavigationDirection  direction,
            AdwBottomSheet         *self)
{
  self->swipe_detected = FALSE;

  if (!self->bottom_bar &&
      adw_animation_get_state (self->open_animation) == ADW_ANIMATION_PLAYING &&
      adw_spring_animation_get_value_to (ADW_SPRING_ANIMATION (self->open_animation)) < 0.5)
    return;

  if (self->open && !self->can_close)
    return;

  if (!self->open && !self->can_open)
    return;

  self->swipe_detected = TRUE;
}

static void
begin_swipe_cb (AdwSwipeTracker *tracker,
                AdwBottomSheet  *self)
{
  if (!self->swipe_detected)
    return;

  adw_animation_pause (self->open_animation);

  if (!self->open)
    gtk_widget_set_child_visible (self->dimming, self->modal);

  self->swipe_detected = FALSE;
  self->swipe_active = TRUE;
}

static void
update_swipe_cb (AdwSwipeTracker *tracker,
                 double           progress,
                 AdwBottomSheet  *self)
{
  if (!self->swipe_active)
    return;

  open_animation_cb (progress, self);
}

static void
end_swipe_cb (AdwSwipeTracker *tracker,
              double           velocity,
              double           to,
              AdwBottomSheet  *self)
{
  if (!self->swipe_active)
    return;

  self->swipe_active = FALSE;

  adw_spring_animation_set_initial_velocity (ADW_SPRING_ANIMATION (self->open_animation),
                                             velocity);

  if ((to > 0.5) != self->open) {
    adw_bottom_sheet_set_open (self, to > 0.5);
    return;
  }

  self->switch_child = TRUE;

  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->open_animation),
                                       self->progress);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->open_animation),
                                     to);
  adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->open_animation),
                                  to < 0.5 && !self->bottom_bar);
  adw_animation_play (self->open_animation);
}

static void
adw_bottom_sheet_init (AdwBottomSheet *self)
{
  AdwAnimationTarget *target;
  GtkEventController *gesture, *shortcut_controller;
  GtkShortcut *shortcut;

  self->align = 0.5;
  self->full_width = TRUE;
  self->show_drag_handle = TRUE;
  self->modal = TRUE;
  self->can_open = TRUE;
  self->can_close = TRUE;
  self->showing_bottom_bar = TRUE;
  self->reveal_bottom_bar = TRUE;
  self->reveal_bottom_bar_progress = 1;

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  /* Content */

  self->content_bin = adw_bin_new ();
  gtk_widget_set_parent (self->content_bin, GTK_WIDGET (self));

  /* Dimming */

  self->dimming = adw_gizmo_new ("dimming", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_opacity (self->dimming, 0);
  gtk_widget_set_child_visible (self->dimming, FALSE);
  gtk_widget_set_can_focus (self->dimming, FALSE);
  gtk_widget_set_can_target (self->dimming, FALSE);
  gtk_widget_set_parent (self->dimming, GTK_WIDGET (self));

  gesture = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (gesture), TRUE);
  gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (gesture),
                                              GTK_PHASE_CAPTURE);
  g_signal_connect_object (gesture, "released", G_CALLBACK (released_cb), self, 0);
  gtk_widget_add_controller (self->dimming, gesture);

  /* Sheet */

  self->sheet_bin = adw_gizmo_new ("sheet", NULL, NULL, NULL, NULL,
                                   (AdwGizmoFocusFunc) adw_widget_focus_child,
                                   (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child_or_self);
  gtk_widget_set_layout_manager (self->sheet_bin, gtk_bin_layout_new ());
  gtk_widget_add_css_class (self->sheet_bin, "background");
  gtk_widget_add_css_class (self->sheet_bin, "has-drag-handle");
  gtk_widget_set_focusable (self->sheet_bin, TRUE);
  gtk_widget_set_child_visible (self->sheet_bin, FALSE);
  gtk_widget_set_parent (self->sheet_bin, GTK_WIDGET (self));

  self->sheet_stack = gtk_stack_new ();
  gtk_stack_set_hhomogeneous (GTK_STACK (self->sheet_stack), TRUE);
  gtk_stack_set_transition_type (GTK_STACK (self->sheet_stack),
                                 GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  gtk_stack_set_transition_duration (GTK_STACK (self->sheet_stack), 100);
  gtk_widget_set_parent (self->sheet_stack, self->sheet_bin);

  self->outline = adw_gizmo_new ("outline", NULL, NULL, NULL,
                                 (AdwGizmoContainsFunc) adw_widget_contains_passthrough,
                                 NULL, NULL);
  gtk_widget_set_can_target (self->outline, FALSE);
  gtk_widget_set_can_focus (self->outline, FALSE);
  gtk_widget_set_parent (self->outline, self->sheet_bin);

  /* Sheet child */

  self->sheet_page = adw_gizmo_new ("widget", NULL, NULL, NULL, NULL,
                                    (AdwGizmoFocusFunc) adw_widget_focus_child,
                                    (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child_or_self);
  gtk_widget_set_overflow (self->sheet_page, GTK_OVERFLOW_HIDDEN);
  gtk_widget_set_layout_manager (self->sheet_page,
                                 gtk_custom_layout_new (adw_widget_get_request_mode,
                                                        measure_sheet,
                                                        allocate_sheet));
  gtk_stack_add_child (GTK_STACK (self->sheet_stack), self->sheet_page);

  self->drag_handle = adw_gizmo_new ("drag-handle", NULL, NULL, NULL, NULL, NULL, NULL);
  gtk_widget_set_can_focus (self->drag_handle, FALSE);
  gtk_widget_set_can_target (self->drag_handle, FALSE);
  gtk_widget_set_parent (self->drag_handle, self->sheet_page);

  /* Bottom bar */

  self->bottom_bar_bin = gtk_button_new ();
  gtk_widget_set_valign (self->bottom_bar_bin, GTK_ALIGN_START);
  gtk_widget_set_overflow (self->bottom_bar_bin, GTK_OVERFLOW_HIDDEN);
  gtk_stack_add_child (GTK_STACK (self->sheet_stack), self->bottom_bar_bin);
  g_signal_connect_swapped (self->bottom_bar_bin, "clicked", G_CALLBACK (bottom_bar_clicked_cb), self);

  disable_button_click (self->bottom_bar_bin);

  gesture = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (gesture), TRUE);
  g_signal_connect_object (gesture, "pressed",
                           G_CALLBACK (bottom_bar_pressed_cb), self, 0);
  g_signal_connect_object (gesture, "released",
                           G_CALLBACK (bottom_bar_released_cb), self, 0);
  gtk_widget_add_controller (self->bottom_bar_bin, gesture);

  /* Animations */

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) open_animation_cb,
                                              self,
                                              NULL);

  self->open_animation = adw_spring_animation_new (GTK_WIDGET (self),
                                                   0,
                                                   1,
                                                   adw_spring_params_new (0.8, 1, 400),
                                                   target);
  adw_spring_animation_set_epsilon (ADW_SPRING_ANIMATION (self->open_animation), 0.0001);
  g_signal_connect_swapped (self->open_animation, "done",
                            G_CALLBACK (open_animation_done_cb), self);

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc) reveal_animation_cb,
                                              self,
                                              NULL);

  self->reveal_bottom_bar_animation = adw_timed_animation_new (GTK_WIDGET (self),
                                                               0, 1,
                                                               REVEAL_BOTTOM_BAR_DURATION,
                                                               target);
  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->reveal_bottom_bar_animation),
                                  ADW_EASE);
  g_signal_connect_swapped (self->reveal_bottom_bar_animation, "done",
                            G_CALLBACK (reveal_animation_done_cb), self);

  /* Swipes */

  self->swipe_tracker = adw_swipe_tracker_new (ADW_SWIPEABLE (self));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->swipe_tracker),
                                  GTK_ORIENTATION_VERTICAL);
  adw_swipe_tracker_set_upper_overshoot (self->swipe_tracker, TRUE);
  adw_swipe_tracker_set_allow_window_handle (self->swipe_tracker, TRUE);
  adw_swipe_tracker_set_allow_mouse_drag (self->swipe_tracker, TRUE);
  adw_swipe_tracker_set_ignore_direction (self->swipe_tracker, TRUE);

  g_signal_connect (self->swipe_tracker, "prepare", G_CALLBACK (prepare_cb), self);
  g_signal_connect (self->swipe_tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self);
  g_signal_connect (self->swipe_tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self);

  /* Esc to close */

  shortcut = gtk_shortcut_new (gtk_keyval_trigger_new (GDK_KEY_Escape, 0),
                               gtk_callback_action_new ((GtkShortcutFunc) maybe_close_cb, self, NULL));

  shortcut_controller = gtk_shortcut_controller_new ();
  gtk_shortcut_controller_add_shortcut (GTK_SHORTCUT_CONTROLLER (shortcut_controller), shortcut);
  gtk_widget_add_controller (self->sheet_bin, shortcut_controller);
}

static void
adw_bottom_sheet_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  if (!g_strcmp0 (type, "sheet"))
    adw_bottom_sheet_set_sheet (ADW_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "bottom-bar"))
    adw_bottom_sheet_set_bottom_bar (ADW_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
  else if (!g_strcmp0 (type, "content"))
    adw_bottom_sheet_set_content (ADW_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_bottom_sheet_set_content (ADW_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_bottom_sheet_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_bottom_sheet_buildable_add_child;
}

static int
get_sheet_width (AdwBottomSheet *self)
{
  int sheet_width, sheet_min_width, width;

  width = gtk_widget_get_width (GTK_WIDGET (self));

  if (!self->sheet_bin)
    return width;

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_HORIZONTAL, -1,
                      &sheet_min_width, &sheet_width, NULL, NULL);

  if (self->full_width)
    return MAX (width, sheet_min_width);

  return MAX (MIN (sheet_width, width), sheet_min_width);
}

static double
adw_bottom_sheet_get_distance (AdwSwipeable *swipeable)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (swipeable);
  int height, sheet_width, sheet_min_height, sheet_height, bottom_bar_height;

  height = gtk_widget_get_height (GTK_WIDGET (self));

  sheet_width = get_sheet_width (self);

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  if (self->bottom_bar) {
    int bottom_bar_min_height;

    gtk_widget_measure (self->bottom_bar_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                        &bottom_bar_min_height, &bottom_bar_height, NULL, NULL);

    bottom_bar_height = MAX (MIN (bottom_bar_height, height), bottom_bar_min_height);
  } else {
    bottom_bar_height = 0;
  }

  return MAX (MIN (sheet_height, height), sheet_min_height) - bottom_bar_height;
}

static double *
adw_bottom_sheet_get_snap_points (AdwSwipeable *swipeable,
                                  int          *n_snap_points)
{
  double *points;

  points = g_new0 (double, 2);

  if (n_snap_points)
    *n_snap_points = 2;

  points[0] = 0;
  points[1] = 1;

  return points;
}

static double
adw_bottom_sheet_get_progress (AdwSwipeable *swipeable)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (swipeable);

  return self->progress;
}

static double
adw_bottom_sheet_get_cancel_progress (AdwSwipeable *swipeable)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (swipeable);

  return round (self->progress);
}

static void
adw_bottom_sheet_get_swipe_area (AdwSwipeable           *swipeable,
                                 AdwNavigationDirection  navigation_direction,
                                 gboolean                is_drag,
                                 GdkRectangle           *rect)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (swipeable);
  int width, height, sheet_width, sheet_min_height, sheet_height;
  int sheet_x, sheet_y, bottom_bar_height;
  float align;

  if (!is_drag) {
    rect->x = 0;
    rect->y = 0;
    rect->width = 0;
    rect->height = 0;

    return;
  }

  sheet_width = get_sheet_width (self);
  width = gtk_widget_get_width (GTK_WIDGET (self));
  height = gtk_widget_get_height (GTK_WIDGET (self));

  gtk_widget_measure (self->sheet_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                      &sheet_min_height, &sheet_height, NULL, NULL);

  if (self->bottom_bar) {
    int bottom_bar_min_height;

    gtk_widget_measure (self->bottom_bar_bin, GTK_ORIENTATION_VERTICAL, sheet_width,
                        &bottom_bar_min_height, &bottom_bar_height, NULL, NULL);

    bottom_bar_height = MAX (MIN (bottom_bar_height, height), bottom_bar_min_height);
  } else {
    bottom_bar_height = 0;
  }

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    align = 1 - self->align;
  else
    align = self->align;

  sheet_x = round ((width - sheet_width) * align);

  sheet_height = MAX (MIN (sheet_height, height), sheet_min_height);
  sheet_y = height - round (adw_lerp (bottom_bar_height, sheet_height, self->progress));

  rect->x = sheet_x;
  rect->y = sheet_y;
  rect->width = sheet_width;
  rect->height = height - sheet_y;
}

static void
adw_bottom_sheet_swipeable_init (AdwSwipeableInterface *iface)
{
  iface->get_distance = adw_bottom_sheet_get_distance;
  iface->get_snap_points = adw_bottom_sheet_get_snap_points;
  iface->get_progress = adw_bottom_sheet_get_progress;
  iface->get_cancel_progress = adw_bottom_sheet_get_cancel_progress;
  iface->get_swipe_area = adw_bottom_sheet_get_swipe_area;
}

/**
 * adw_bottom_sheet_new:
 *
 * Creates a new `AdwBottomSheet`.
 *
 * Returns: the new created `AdwBottomSheet`
 *
 * Since: 1.6
 */
GtkWidget *
adw_bottom_sheet_new (void)
{
  return g_object_new (ADW_TYPE_BOTTOM_SHEET, NULL);
}

/**
 * adw_bottom_sheet_get_content:
 * @self: a bottom sheet
 *
 * Gets the content widget for @self.
 *
 * Returns: (nullable) (transfer none): the content widget
 *
 * Since: 1.6
 */
GtkWidget *
adw_bottom_sheet_get_content (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), NULL);

  return self->content;
}

/**
 * adw_bottom_sheet_set_content:
 * @self: a bottom sheet
 * @content: (nullable): the content widget
 *
 * Sets the content widget for @self.
 *
 * It's always shown, and the bottom sheet is overlaid over it.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_content (AdwBottomSheet *self,
                              GtkWidget      *content)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));
  g_return_if_fail (content == NULL || GTK_IS_WIDGET (content));

  if (self->content == content)
    return;

  self->content = content;

  if (content)
    g_return_if_fail (gtk_widget_get_parent (content) == NULL);

  adw_bin_set_child (ADW_BIN (self->content_bin), content);
  gtk_widget_set_visible (self->content_bin, content != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CONTENT]);
}

/**
 * adw_bottom_sheet_get_sheet:
 * @self: a bottom sheet
 *
 * Gets the bottom sheet widget for @self.
 *
 * Returns: (nullable) (transfer none): the sheet widget
 *
 * Since: 1.6
 */
GtkWidget *
adw_bottom_sheet_get_sheet (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), NULL);

  return self->sheet;
}

/**
 * adw_bottom_sheet_set_sheet:
 * @self: a bottom sheet
 * @sheet: (nullable): the sheet widget
 *
 * Sets the bottom sheet widget for @self.
 *
 * Only shown when [property@BottomSheet:open] is `TRUE`.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_sheet (AdwBottomSheet *self,
                            GtkWidget      *sheet)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));
  g_return_if_fail (sheet == NULL || GTK_IS_WIDGET (sheet));

  if (self->sheet == sheet)
    return;

  if (sheet)
    g_return_if_fail (gtk_widget_get_parent (sheet) == NULL);

  if (self->sheet)
    gtk_widget_unparent (self->sheet);

  self->sheet = sheet;

  if (self->sheet)
    gtk_widget_insert_before (self->sheet, self->sheet_page, self->drag_handle);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHEET]);
}

/**
 * adw_bottom_sheet_get_bottom_bar:
 * @self: a bottom sheet
 *
 * Gets the bottom bar widget for @self.
 *
 * Returns: (nullable) (transfer none): the bottom bar widget
 *
 * Since: 1.6
 */
GtkWidget *
adw_bottom_sheet_get_bottom_bar (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), NULL);

  return self->bottom_bar;
}

/**
 * adw_bottom_sheet_set_bottom_bar:
 * @self: a bottom sheet
 * @bottom_bar: (nullable): the bottom bar widget
 *
 * Sets the bottom bar widget for @self.
 *
 * Shown when [property@BottomSheet:open] is `FALSE`. When open, morphs into
 * the [property@BottomSheet:sheet].
 *
 * Bottom bar can be temporarily hidden using the
 * [property@BottomSheet:reveal-bottom-bar] property.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_bottom_bar (AdwBottomSheet *self,
                                 GtkWidget      *bottom_bar)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));
  g_return_if_fail (bottom_bar == NULL || GTK_IS_WIDGET (bottom_bar));

  if (self->bottom_bar == bottom_bar)
    return;

  if (bottom_bar)
    g_return_if_fail (gtk_widget_get_parent (bottom_bar) == NULL);

  self->bottom_bar = bottom_bar;

  gtk_button_set_child (GTK_BUTTON (self->bottom_bar_bin), self->bottom_bar);

  if (self->showing_bottom_bar) {
    gtk_stack_set_visible_child (GTK_STACK (self->sheet_stack),
                                 bottom_bar ? self->bottom_bar_bin : self->sheet_page);

    gtk_widget_add_css_class (self->sheet_bin, "bottom-bar");
  } else {
    gtk_widget_remove_css_class (self->sheet_bin, "bottom-bar");
  }

  if (G_APPROX_VALUE (self->progress, 0, DBL_EPSILON)) {
    gtk_widget_set_child_visible (self->sheet_bin,
                                  self->bottom_bar != NULL &&
                                  self->reveal_bottom_bar);
  }

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BOTTOM_BAR]);
}

/**
 * adw_bottom_sheet_get_open:
 * @self: a bottom sheet
 *
 * Gets whether the bottom sheet is open.
 *
 * Returns: whether the sheet is open
 *
 * Since: 1.6
 */
gboolean
adw_bottom_sheet_get_open (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->open;
}

/**
 * adw_bottom_sheet_set_open:
 * @self: a bottom sheet
 * @open: whether to open the sheet
 *
 * Sets whether the bottom sheet is open.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_open (AdwBottomSheet *self,
                           gboolean        open)
{
  GtkRoot *root;
  GtkWidget *focus = NULL;

  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  open = !!open;

  if (self->open == open) {
    if (!self->has_been_open && !open) {
      if (self->closing_callback)
        self->closing_callback (self, self->user_data);

      if (self->closed_callback)
        self->closed_callback (self, self->user_data);
    }

    return;
  }

  self->open = open;

  if (open) {
    gtk_widget_set_child_visible (self->dimming, self->modal);
    gtk_widget_set_child_visible (self->sheet_bin, TRUE);
    self->has_been_open = true;
  }

  gtk_widget_set_can_target (self->dimming, open);

  if (!open) {
    if (self->closing_callback)
      self->closing_callback (self, self->user_data);

    if (self->open != open)
      return;
  }

  if (open)
    show_bottom_bar (self, FALSE);
  else if (self->progress < CHILD_SWITCH_THRESHOLD)
    show_bottom_bar (self, TRUE);
  else
    self->switch_child = TRUE;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  if (gtk_widget_get_mapped (GTK_WIDGET (self))) {
    if (root)
      focus = gtk_root_get_focus (root);

    if (open) {
      if (focus && !gtk_widget_is_ancestor (focus, self->content_bin))
        focus = NULL;

      g_set_weak_pointer (&self->last_content_focus, focus);
    } else {
      if (focus && focus != self->sheet_bin && !gtk_widget_is_ancestor (focus, self->sheet_bin))
        focus = NULL;

      g_set_weak_pointer (&self->last_sheet_focus, focus);
    }
  }

  if (self->modal)
    gtk_widget_set_can_focus (self->content_bin, !open);

  if (gtk_widget_get_mapped (GTK_WIDGET (self))) {
    if (open) {
      if (self->last_sheet_focus) {
        gtk_widget_grab_focus (self->last_sheet_focus);
      } else {
        g_signal_emit_by_name (self->sheet_bin, "move-focus", GTK_DIR_TAB_FORWARD);

        if (root)
          focus = gtk_root_get_focus (root);

        if (!focus || !gtk_widget_is_ancestor (focus, self->sheet_bin))
          gtk_widget_grab_focus (self->sheet_bin);
      }

      g_clear_weak_pointer (&self->last_sheet_focus);
    } else if (self->content) {
      if (self->last_content_focus) {
        gtk_widget_grab_focus (self->last_content_focus);
      } else {
        g_signal_emit_by_name (self->content_bin, "move-focus", GTK_DIR_TAB_FORWARD);

        if (root)
          focus = gtk_root_get_focus (root);

        if (!focus || !gtk_widget_is_ancestor (focus, self->content_bin))
          gtk_widget_grab_focus (self->content_bin);
      }

      g_clear_weak_pointer (&self->last_content_focus);
    }
  }

  adw_spring_animation_set_value_from (ADW_SPRING_ANIMATION (self->open_animation),
                                       self->progress);
  adw_spring_animation_set_value_to (ADW_SPRING_ANIMATION (self->open_animation),
                                     open ? 1 : 0);
  adw_spring_animation_set_clamp (ADW_SPRING_ANIMATION (self->open_animation),
                                  !open && !self->bottom_bar);
  adw_animation_play (self->open_animation);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_OPEN]);
}

/**
 * adw_bottom_sheet_get_align:
 * @self: a bottom sheet
 *
 * Gets horizontal alignment of the bottom sheet.
 *
 * Returns: the horizontal alignment
 *
 * Since: 1.6
 */
float
adw_bottom_sheet_get_align (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), 0.0f);

  return self->align;
}

/**
 * adw_bottom_sheet_set_align:
 * @self: a bottom sheet
 * @align: the new alignment
 *
 * Sets horizontal alignment of the bottom sheet.
 *
 * 0 means the bottom sheet is flush with the start edge, 1 means it's flush
 * with the end edge. 0.5 means it's centered.
 *
 * Only used when [property@BottomSheet:full-width] is set to `FALSE`.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_align (AdwBottomSheet *self,
                            float           align)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  if (G_APPROX_VALUE (align, self->align, FLT_EPSILON))
    return;

  self->align = align;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALIGN]);
}

/**
 * adw_bottom_sheet_get_full_width:
 * @self: a bottom sheet
 *
 * Gets whether the bottom sheet takes the full width.
 *
 * Returns: whether the sheet takes up the full width
 *
 * Since: 1.6
 */
gboolean
adw_bottom_sheet_get_full_width (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->full_width;
}

/**
 * adw_bottom_sheet_set_full_width:
 * @self: a bottom sheet
 * @full_width: whether the sheet takes up the full width
 *
 * Sets whether the bottom sheet takes the full width.
 *
 * When full width, [property@BottomSheet:align] is ignored.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_full_width (AdwBottomSheet *self,
                                 gboolean        full_width)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  full_width = !!full_width;

  if (full_width == self->full_width)
    return;

  self->full_width = full_width;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_FULL_WIDTH]);
}

/**
 * adw_bottom_sheet_get_show_drag_handle:
 * @self: a bottom sheet
 *
 * Gets whether to show a drag handle in the bottom sheet.
 *
 * Returns: whether to show the drag handle
 *
 * Since: 1.6
 */
gboolean
adw_bottom_sheet_get_show_drag_handle (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->show_drag_handle;
}

/**
 * adw_bottom_sheet_set_show_drag_handle
 * @self: a bottom sheet
 * @show_drag_handle: whether to show the drag handle
 *
 * Sets whether to show a drag handle in the bottom sheet.
 *
 * The handle will be overlaid over [property@BottomSheet:sheet].
 *
 * When the handle is shown, [class@HeaderBar] will hide its default title, and
 * [class@ToolbarView] will reserve space if there are no top bars.
 *
 * Showing drag handle also allows to swipe the bottom sheet down (and to swipe
 * the bottom bar up) with a pointer, instead of just touchscreen.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_show_drag_handle (AdwBottomSheet *self,
                                       gboolean        show_drag_handle)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  show_drag_handle = !!show_drag_handle;

  if (self->show_drag_handle == show_drag_handle)
    return;

  self->show_drag_handle = show_drag_handle;

  gtk_widget_set_visible (self->drag_handle, show_drag_handle);

  if (show_drag_handle)
    gtk_widget_add_css_class (self->sheet_bin, "has-drag-handle");
  else
    gtk_widget_remove_css_class (self->sheet_bin, "has-drag-handle");

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_DRAG_HANDLE]);
}

/**
 * adw_bottom_sheet_get_modal:
 * @self: a bottom sheet
 *
 * Gets whether the bottom sheet is modal.
 *
 * Returns: whether the sheet is modal
 *
 * Since: 1.6
 */
gboolean
adw_bottom_sheet_get_modal (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->modal;
}

/**
 * adw_bottom_sheet_set_modal:
 * @self: a bottom sheet
 * @modal: whether the sheet is modal
 *
 * Sets whether the bottom sheet is modal.
 *
 * When modal, [property@BottomSheet:content] will be dimmed when the bottom
 * sheet is open, and clicking it will close the bottom sheet. It also cannot be
 * focused with keyboard.
 *
 * Otherwise, the content is accessible even when the bottom sheet is open.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_modal (AdwBottomSheet *self,
                            gboolean        modal)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  modal = !!modal;

  if (self->modal == modal)
    return;

  self->modal = modal;

  if (!G_APPROX_VALUE (self->progress, 0, DBL_EPSILON))
    gtk_widget_set_child_visible (self->dimming, self->modal);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MODAL]);
}

/**
 * adw_bottom_sheet_get_can_open:
 * @self: a bottom sheet
 *
 * Gets whether the bottom sheet can be opened by user.
 *
 * Returns: whether the sheet can be opened by user.
 *
 * Since: 1.6
 */
gboolean
adw_bottom_sheet_get_can_open (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->can_open;
}

/**
 * adw_bottom_sheet_set_can_open:
 * @self: a bottom sheet
 * @can_open: whether the sheet can be opened by user.
 *
 * Sets whether the bottom sheet can be opened by user.
 *
 * It can be opened via clicking or swiping up from the bottom bar.
 *
 * Does nothing if [property@BottomSheet:bottom-bar] is not set.
 *
 * Bottom sheet can still be opened using [property@BottomSheet:open].
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_can_open (AdwBottomSheet *self,
                               gboolean        can_open)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  can_open = !!can_open;

  if (self->can_open == can_open)
    return;

  self->can_open = can_open;

  if (can_open)
    gtk_widget_remove_css_class (self->bottom_bar_bin, "inert");
  else
    gtk_widget_add_css_class (self->bottom_bar_bin, "inert");

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_OPEN]);
}

/**
 * adw_bottom_sheet_get_can_close:
 * @self: a bottom sheet
 *
 * Gets whether the bottom sheet can be closed by user.
 *
 * Returns: whether the sheet can be closed by user
 *
 * Since: 1.6
 */
gboolean
adw_bottom_sheet_get_can_close (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->can_close;
}

/**
 * adw_bottom_sheet_set_can_close:
 * @self: a bottom sheet
 * @can_close: whether the sheet can be closed by user
 *
 * Sets whether the bottom sheet can be closed by user.
 *
 * It can be closed via the close button, swiping down, pressing
 * <kbd>Escape</kbd> or clicking the content dimming (when modal).
 *
 * Bottom sheet can still be closed using [property@BottomSheet:open].
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_can_close (AdwBottomSheet *self,
                                gboolean        can_close)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  can_close = !!can_close;

  if (self->can_close == can_close)
    return;

  self->can_close = can_close;

  update_swipe_tracker (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_CLOSE]);
}

/**
 * adw_bottom_sheet_get_sheet_height:
 * @self: a bottom sheet
 *
 * Gets the current bottom sheet height.
 *
 * It can be used to shift the content upwards when the bottom sheet is open.
 *
 * Returns: the sheet height
 *
 * Since: 1.6
 */
int
adw_bottom_sheet_get_sheet_height (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), 0);

  return self->sheet_height;
}

/**
 * adw_bottom_sheet_get_bottom_bar_height:
 * @self: a bottom sheet
 *
 * Gets the current bottom bar height.
 *
 * It can be used to shift the content upwards permanently to accommodate for
 * the bottom bar.
 *
 * Returns: the bottom bar height
 *
 * Since: 1.6
 */
int
adw_bottom_sheet_get_bottom_bar_height (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), 0);

  return self->bottom_bar_height;
}

/**
 * adw_bottom_sheet_get_reveal_bottom_bar:
 * @self: a bottom sheet
 *
 * Gets whether the bottom bar is revealed.
 *
 * Returns: whether the bottom bar is revealed
 *
 * Since: 1.7
 */
gboolean
adw_bottom_sheet_get_reveal_bottom_bar (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->reveal_bottom_bar;
}

/**
 * adw_bottom_sheet_set_reveal_bottom_bar:
 * @self: a bottom sheet
 * @reveal: whether to reveal the bottom bar
 *
 * Sets whether to reveal the bottom bar.
 *
 * The transition will be animated.
 *
 * See [property@BottomSheet:bottom-bar] and
 * [property@BottomSheet:bottom-bar-height].
 *
 * Since: 1.7
 */
void
adw_bottom_sheet_set_reveal_bottom_bar (AdwBottomSheet *self,
                                        gboolean        reveal)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  reveal = !!reveal;

  if (self->reveal_bottom_bar == reveal)
    return;

  self->reveal_bottom_bar = reveal;

  if (self->bottom_bar) {
    adw_timed_animation_set_value_from (ADW_TIMED_ANIMATION (self->reveal_bottom_bar_animation),
                                        self->reveal_bottom_bar_progress);
    adw_timed_animation_set_value_to (ADW_TIMED_ANIMATION (self->reveal_bottom_bar_animation),
                                      reveal ? 1 : 0);

    if (reveal)
      gtk_widget_set_child_visible (self->sheet_bin, TRUE);

    adw_animation_play (self->reveal_bottom_bar_animation);
  } else {
    self->reveal_bottom_bar_progress = reveal ? 1 : 0;
  }

  if (reveal)
    gtk_widget_remove_css_class (self->sheet_bin, "hidden");
  else
    gtk_widget_add_css_class (self->sheet_bin, "hidden");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_BOTTOM_BAR]);
}

void
adw_bottom_sheet_set_min_natural_width (AdwBottomSheet *self,
                                        int             min_natural_width)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  self->min_natural_width = min_natural_width;
}

GtkWidget *
adw_bottom_sheet_get_sheet_bin (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), NULL);

  return self->sheet_bin;
}

void
adw_bottom_sheet_set_callbacks (AdwBottomSheet *self,
                                GFunc           closing_callback,
                                GFunc           closed_callback,
                                gpointer        user_data)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  self->closing_callback = closing_callback;
  self->closed_callback = closed_callback;
  self->user_data = user_data;
}
