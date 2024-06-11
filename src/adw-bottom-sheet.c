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
#include "adw-widget-utils-private.h"

#define TOP_PADDING_MIN_HEIGHT 720
#define TOP_PADDING_MIN_VALUE 30
#define TOP_PADDING_TARGET_HEIGHT 1440
#define TOP_PADDING_TARGET_VALUE 120
#define CHILD_SWITCH_THRESHOLD 0.15

struct _AdwBottomSheet
{
  GtkWidget parent_instance;

  GtkWidget *child;
  GtkWidget *sheet;
  GtkWidget *bottom_bar;

  GtkWidget *child_bin;
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

  gboolean show_drag_handle;
  gboolean modal;
  gboolean can_close;

  AdwSwipeTracker *swipe_tracker;
  gboolean swipe_active;

  GtkWidget *last_child_focus;
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
  PROP_CHILD,
  PROP_SHEET,
  PROP_BOTTOM_BAR,
  PROP_OPEN,
  PROP_ALIGN,
  PROP_FULL_WIDTH,
  PROP_SHOW_DRAG_HANDLE,
  PROP_MODAL,
  PROP_CAN_CLOSE,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
released_cb (GtkGestureClick *gesture,
             int              n_press,
             double           x,
             double           y,
             AdwBottomSheet  *self)
{
  if (self->swipe_active)
    return;

  if (!self->can_close)
    return;

  adw_bottom_sheet_set_open (self, FALSE);
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
    gtk_widget_set_child_visible (self->sheet_bin, self->bottom_bar != NULL);

    if (self->closed_callback)
      self->closed_callback (self, self->user_data);
  }

  self->switch_child = FALSE;
}

static void
sheet_close_cb (AdwBottomSheet *self)
{
  GtkWidget *parent;

  if (!self->can_close)
    return;

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

  return GDK_EVENT_PROPAGATE;
}

static void
bottom_bar_clicked_cb (AdwBottomSheet *self)
{
  adw_bottom_sheet_set_open (self, TRUE);
}

static void
fix_button_click_propagation_phase (GtkWidget *widget)
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
      gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_BUBBLE);

    g_object_unref (controller);
  }

  g_object_unref (controllers);
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

  if (self->min_natural_width >= 0)
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
  int child_min, child_nat, dim_min, dim_nat, sheet_min, sheet_nat;

  if (gtk_widget_should_layout (self->child_bin)) {
    gtk_widget_measure (self->child_bin, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
  } else {
    child_min = child_nat = 0;
  }

  gtk_widget_measure (self->dimming, orientation, for_size,
                      &dim_min, &dim_nat, NULL, NULL);

  gtk_widget_measure (self->sheet_bin, orientation, for_size,
                      &sheet_min, &sheet_nat, NULL, NULL);

  if (minimum)
    *minimum = MAX (child_min, MAX (dim_min, sheet_min));
  if (natural)
    *natural = MAX (child_nat, MAX (dim_nat, sheet_nat));
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
  sheet_height = MAX (sheet_height, height - sheet_y);

  if (sheet_x == 0)
    gtk_widget_add_css_class (self->sheet_bin, "flush-left");
  else
    gtk_widget_remove_css_class (self->sheet_bin, "flush-left");

  if (sheet_x == width - sheet_width)
    gtk_widget_add_css_class (self->sheet_bin, "flush-right");
  else
    gtk_widget_remove_css_class (self->sheet_bin, "flush-right");

  if (gtk_widget_should_layout (self->child_bin))
    gtk_widget_allocate (self->child_bin, width, height, baseline, NULL);

  gtk_widget_allocate (self->dimming, width, height, baseline, NULL);

  transform = gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (sheet_x, sheet_y));
  gtk_widget_allocate (self->sheet_bin, sheet_width, sheet_height, baseline, transform);
}

static void
adw_bottom_sheet_dispose (GObject *object)
{
  AdwBottomSheet *self = ADW_BOTTOM_SHEET (object);

  g_clear_weak_pointer (&self->last_child_focus);
  g_clear_weak_pointer (&self->last_sheet_focus);

  g_clear_pointer (&self->child_bin, gtk_widget_unparent);
  g_clear_pointer (&self->dimming, gtk_widget_unparent);
  g_clear_pointer (&self->sheet_bin, gtk_widget_unparent);
  self->child = NULL;
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
  case PROP_CHILD:
    g_value_set_object (value, adw_bottom_sheet_get_child (self));
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
  case PROP_CAN_CLOSE:
    g_value_set_boolean (value, adw_bottom_sheet_get_can_close (self));
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
  case PROP_CHILD:
    adw_bottom_sheet_set_child (self, g_value_get_object (value));
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
  case PROP_CAN_CLOSE:
    adw_bottom_sheet_set_can_close (self, g_value_get_boolean (value));
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

  props[PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_SHEET] =
    g_param_spec_object ("sheet", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_BOTTOM_BAR] =
    g_param_spec_object ("bottom-bar", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_OPEN] =
    g_param_spec_boolean ("open", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ALIGN] =
    g_param_spec_float ("align", NULL, NULL,
                        0, 1, 0.5,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_FULL_WIDTH] =
    g_param_spec_boolean ("full-width", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_SHOW_DRAG_HANDLE] =
    g_param_spec_boolean ("show-drag-handle", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_MODAL] =
    g_param_spec_boolean ("modal", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CAN_CLOSE] =
    g_param_spec_boolean ("can-close", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_install_action (widget_class, "sheet.close", NULL,
                                   (GtkWidgetActionActivateFunc) sheet_close_cb);

  gtk_widget_class_set_css_name (widget_class, "bottom-sheet");
}

static void
prepare_cb (AdwSwipeTracker        *tracker,
            AdwNavigationDirection  direction,
            AdwBottomSheet         *self)
{
  if (adw_animation_get_state (self->open_animation) == ADW_ANIMATION_PLAYING &&
      adw_spring_animation_get_value_to (ADW_SPRING_ANIMATION (self->open_animation)) < 0.5)
    return;

  self->swipe_active = TRUE;
}

static void
begin_swipe_cb (AdwSwipeTracker *tracker,
                AdwBottomSheet  *self)
{
  if (!self->swipe_active)
    return;

  adw_animation_pause (self->open_animation);

  if (!self->open)
    gtk_widget_set_child_visible (self->dimming, self->modal);
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
  self->can_close = TRUE;
  self->showing_bottom_bar = TRUE;

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  /* Child */

  self->child_bin = adw_bin_new ();
  gtk_widget_set_parent (self->child_bin, GTK_WIDGET (self));

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

  fix_button_click_propagation_phase (self->bottom_bar_bin);

  /* Animation */

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
  else if (!type && GTK_IS_WIDGET (child))
    adw_bottom_sheet_set_child (ADW_BOTTOM_SHEET (buildable), GTK_WIDGET (child));
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
 * Since: 1.5
 */
GtkWidget *
adw_bottom_sheet_new (void)
{
  return g_object_new (ADW_TYPE_BOTTOM_SHEET, NULL);
}

/**
 * adw_bottom_sheet_get_child:
 * @self: a bottom sheet
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self
 *
 * Since: 1.5
 */
GtkWidget *
adw_bottom_sheet_get_child (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), NULL);

  return self->child;
}

/**
 * adw_bottom_sheet_set_child:
 * @self: a bottom sheet
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * Since: 1.5
 */
void
adw_bottom_sheet_set_child (AdwBottomSheet *self,
                            GtkWidget      *child)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (self->child == child)
    return;

  self->child = child;

  adw_bin_set_child (ADW_BIN (self->child_bin), child);
  gtk_widget_set_visible (self->child_bin, child != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD]);
}

GtkWidget *
adw_bottom_sheet_get_sheet (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), NULL);

  return self->sheet;
}

void
adw_bottom_sheet_set_sheet (AdwBottomSheet *self,
                            GtkWidget      *sheet)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));
  g_return_if_fail (sheet == NULL || GTK_IS_WIDGET (sheet));

  if (sheet)
    g_return_if_fail (gtk_widget_get_parent (sheet) == NULL);

  if (self->sheet == sheet)
    return;

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
 * Gets the bottom bar widget of @self.
 *
 * Returns: (nullable) (transfer none): the bottom bar widget of @self
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
 * Sets the bottom bar widget of @self.
 *
 * Since: 1.6
 */
void
adw_bottom_sheet_set_bottom_bar (AdwBottomSheet *self,
                                 GtkWidget      *bottom_bar)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));
  g_return_if_fail (bottom_bar == NULL || GTK_IS_WIDGET (bottom_bar));

  if (bottom_bar)
    g_return_if_fail (gtk_widget_get_parent (bottom_bar) == NULL);

  if (self->bottom_bar == bottom_bar)
    return;

  self->bottom_bar = bottom_bar;

  gtk_button_set_child (GTK_BUTTON (self->bottom_bar_bin), self->bottom_bar);

  if (self->showing_bottom_bar) {
    gtk_stack_set_visible_child (GTK_STACK (self->sheet_stack),
                                 bottom_bar ? self->bottom_bar_bin : self->sheet_page);

    gtk_widget_add_css_class (self->sheet_bin, "bottom-bar");
  } else {
    gtk_widget_remove_css_class (self->sheet_bin, "bottom-bar");
  }

  if (G_APPROX_VALUE (self->progress, 0, DBL_EPSILON))
    gtk_widget_set_child_visible (self->sheet_bin, self->bottom_bar != NULL);

  adw_swipe_tracker_set_lower_overshoot (self->swipe_tracker, bottom_bar != NULL);
  adw_swipe_tracker_set_allow_mouse_drag (self->swipe_tracker,
                                          self->show_drag_handle || bottom_bar != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BOTTOM_BAR]);
}

gboolean
adw_bottom_sheet_get_open (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->open;
}

void
adw_bottom_sheet_set_open (AdwBottomSheet *self,
                           gboolean        open)
{
  GtkRoot *root;
  GtkWidget *focus;

  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  open = !!open;

  if (self->open == open)
    return;

  self->open = open;

  if (open) {
    gtk_widget_set_child_visible (self->dimming, self->modal);
    gtk_widget_set_child_visible (self->sheet_bin, TRUE);
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
  focus = gtk_root_get_focus (root);

  if (open) {
    if (focus && !gtk_widget_is_ancestor (focus, self->child_bin))
      focus = NULL;

    g_set_weak_pointer (&self->last_child_focus, focus);
  } else {
    if (focus && focus != self->sheet_bin && !gtk_widget_is_ancestor (focus, self->sheet_bin))
      focus = NULL;

    g_set_weak_pointer (&self->last_sheet_focus, focus);
  }

  if (self->modal)
    gtk_widget_set_can_focus (self->child_bin, !open);

  if (open) {
    if (root && self->last_sheet_focus) {
      gtk_root_set_focus (root, self->last_sheet_focus);
    } else {
      GTK_WIDGET_GET_CLASS (self->sheet_bin)->move_focus (self->sheet_bin,
                                                          GTK_DIR_TAB_FORWARD);

      focus = gtk_root_get_focus (root);
      if (!focus || !gtk_widget_is_ancestor (focus, self->sheet_bin))
        gtk_widget_grab_focus (self->sheet_bin);
    }

    g_clear_weak_pointer (&self->last_sheet_focus);
  } else if (self->child) {
    if (root && self->last_child_focus) {
      gtk_root_set_focus (root, self->last_child_focus);
    } else {
      GTK_WIDGET_GET_CLASS (self->child_bin)->move_focus (self->child_bin,
                                                          GTK_DIR_TAB_FORWARD);

      focus = gtk_root_get_focus (root);
      if (!focus || !gtk_widget_is_ancestor (focus, self->child_bin))
        gtk_widget_grab_focus (self->child_bin);
    }

    g_clear_weak_pointer (&self->last_child_focus);
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

float
adw_bottom_sheet_get_align (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), 0.0f);

  return self->align;
}

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

gboolean
adw_bottom_sheet_get_full_width (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->full_width;
}

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

gboolean
adw_bottom_sheet_get_show_drag_handle (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->show_drag_handle;
}

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

  adw_swipe_tracker_set_allow_mouse_drag (self->swipe_tracker,
                                          show_drag_handle || self->bottom_bar != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_DRAG_HANDLE]);
}

gboolean
adw_bottom_sheet_get_modal (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->modal;
}

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

gboolean
adw_bottom_sheet_get_can_close (AdwBottomSheet *self)
{
  g_return_val_if_fail (ADW_IS_BOTTOM_SHEET (self), FALSE);

  return self->can_close;
}

void
adw_bottom_sheet_set_can_close (AdwBottomSheet *self,
                                gboolean        can_close)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  can_close = !!can_close;

  if (self->can_close == can_close)
    return;

  self->can_close = can_close;

  adw_swipe_tracker_set_enabled (self->swipe_tracker, can_close); // TODO handler bottom bar

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_CLOSE]);
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
adw_bottom_sheet_set_sheet_overflow (AdwBottomSheet *self,
                                     GtkOverflow     overflow)
{
  g_return_if_fail (ADW_IS_BOTTOM_SHEET (self));

  gtk_widget_set_overflow (self->sheet_page, overflow);
  gtk_widget_set_overflow (self->bottom_bar_bin, overflow);
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
