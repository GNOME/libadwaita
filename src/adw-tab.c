/*
 * Copyright (C) 2020-2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include "config.h"
#include "adw-tab-private.h"

#include "adw-animation-util-private.h"
#include "adw-animation-private.h"
#include "adw-bidi-private.h"
#include "adw-fading-label-private.h"
#include "adw-macros-private.h"

#define FADE_WIDTH 18
#define CLOSE_BTN_ANIMATION_DURATION 150

#define BASE_WIDTH 118
#define BASE_WIDTH_PINNED 28

struct _AdwTab
{
  GtkWidget parent_instance;

  GtkWidget *title;
  GtkWidget *icon_stack;
  GtkImage *icon;
  GtkSpinner *spinner;
  GtkImage *indicator_icon;
  GtkWidget *indicator_btn;
  GtkWidget *close_btn;
  GtkDropTarget *drop_target;

  AdwTabView *view;
  AdwTabPage *page;
  gboolean pinned;
  gboolean dragging;
  int display_width;

  gboolean hovering;
  gboolean selected;
  gboolean inverted;
  gboolean title_inverted;
  gboolean close_overlap;
  gboolean show_close;
  gboolean fully_visible;

  AdwAnimation *close_btn_animation;

  GskGLShader *shader;
  gboolean shader_compiled;
};

G_DEFINE_FINAL_TYPE (AdwTab, adw_tab, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_VIEW,
  PROP_PINNED,
  PROP_DRAGGING,
  PROP_PAGE,
  PROP_DISPLAY_WIDTH,
  PROP_INVERTED,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_EXTRA_DRAG_DROP,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static inline void
set_style_class (GtkWidget  *widget,
                 const char *style_class,
                 gboolean    enabled)
{
  if (enabled)
    gtk_widget_add_css_class (widget, style_class);
  else
    gtk_widget_remove_css_class (widget, style_class);
}

static void
close_btn_animation_value_cb (AdwTab *self,
                              double  value)
{
  gtk_widget_set_opacity (self->close_btn, value);
  gtk_widget_set_can_target (self->close_btn, value > 0);
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
close_btn_animation_done_cb (AdwTab *self)
{
  gtk_widget_set_opacity (self->close_btn, self->show_close ? 1 : 0);
  gtk_widget_set_can_target (self->close_btn, self->show_close);
  g_clear_object (&self->close_btn_animation);
}

static void
update_state (AdwTab *self)
{
  GtkStateFlags new_state;
  gboolean show_close;
  AdwAnimationTarget *target;

  new_state = gtk_widget_get_state_flags (GTK_WIDGET (self)) &
    ~GTK_STATE_FLAG_CHECKED;

  if (self->selected || self->dragging)
    new_state |= GTK_STATE_FLAG_CHECKED;

  gtk_widget_set_state_flags (GTK_WIDGET (self), new_state, TRUE);

  show_close = (self->hovering && self->fully_visible) || self->selected || self->dragging;

  if (self->show_close != show_close) {
    double opacity = gtk_widget_get_opacity (self->close_btn);

    if (self->close_btn_animation)
      adw_animation_stop (self->close_btn_animation);

    self->show_close = show_close;

    target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                                close_btn_animation_value_cb,
                                                self, NULL);
    self->close_btn_animation =
      adw_animation_new (GTK_WIDGET (self),
                         opacity,
                         self->show_close ? 1 : 0,
                         CLOSE_BTN_ANIMATION_DURATION,
                         target);

    adw_animation_set_interpolator (self->close_btn_animation,
                                    ADW_ANIMATION_INTERPOLATOR_EASE_IN_OUT);

    g_signal_connect_swapped (self->close_btn_animation, "done", G_CALLBACK (close_btn_animation_done_cb), self);

    adw_animation_start (self->close_btn_animation);
  }
}

static void
update_tooltip (AdwTab *self)
{
  const char *tooltip = adw_tab_page_get_tooltip (self->page);

  if (tooltip && g_strcmp0 (tooltip, "") != 0)
    gtk_widget_set_tooltip_markup (GTK_WIDGET (self), tooltip);
  else
    gtk_widget_set_tooltip_text (GTK_WIDGET (self),
                                 adw_tab_page_get_title (self->page));
}

static void
update_title (AdwTab *self)
{
  const char *title = adw_tab_page_get_title (self->page);
  PangoDirection title_direction = PANGO_DIRECTION_NEUTRAL;
  GtkTextDirection direction = gtk_widget_get_direction (GTK_WIDGET (self));
  gboolean title_inverted;

  if (title)
    title_direction = adw_find_base_dir (title, -1);

  title_inverted =
    (title_direction == PANGO_DIRECTION_LTR && direction == GTK_TEXT_DIR_RTL) ||
    (title_direction == PANGO_DIRECTION_RTL && direction == GTK_TEXT_DIR_LTR);

  if (self->title_inverted != title_inverted) {
    self->title_inverted = title_inverted;
    gtk_widget_queue_allocate (GTK_WIDGET (self));
  }

  update_tooltip (self);
}

static void
update_spinner (AdwTab *self)
{
  gboolean loading = self->page && adw_tab_page_get_loading (self->page);
  gboolean mapped = gtk_widget_get_mapped (GTK_WIDGET (self));

  /* Don't use CPU when not needed */
  gtk_spinner_set_spinning (self->spinner, loading && mapped);
}

static void
update_icons (AdwTab *self)
{
  GIcon *gicon = adw_tab_page_get_icon (self->page);
  gboolean loading = adw_tab_page_get_loading (self->page);
  GIcon *indicator = adw_tab_page_get_indicator_icon (self->page);
  const char *name = loading ? "spinner" : "icon";

  if (self->pinned && !gicon)
    gicon = adw_tab_view_get_default_icon (self->view);

  gtk_image_set_from_gicon (self->icon, gicon);
  gtk_widget_set_visible (self->icon_stack,
                          (gicon != NULL || loading) &&
                          (!self->pinned || indicator == NULL));
  gtk_stack_set_visible_child_name (GTK_STACK (self->icon_stack), name);

  gtk_widget_set_visible (self->indicator_btn, indicator != NULL);
}

static void
update_indicator (AdwTab *self)
{
  gboolean activatable = self->page && adw_tab_page_get_indicator_activatable (self->page);
  gboolean clickable = activatable && (self->selected || (!self->pinned && self->fully_visible));

  gtk_widget_set_can_target (self->indicator_btn, clickable);
}

static void
update_needs_attention (AdwTab *self)
{
  set_style_class (GTK_WIDGET (self), "needs-attention",
                   adw_tab_page_get_needs_attention (self->page));
}

static void
update_loading (AdwTab *self)
{
  update_icons (self);
  update_spinner (self);
  set_style_class (GTK_WIDGET (self), "loading",
                   adw_tab_page_get_loading (self->page));
}

static void
update_selected (AdwTab *self)
{
  self->selected = self->dragging;

  if (self->page)
    self->selected |= adw_tab_page_get_selected (self->page);

  update_state (self);
  update_indicator (self);
}

static gboolean
close_idle_cb (AdwTab *self)
{
  adw_tab_view_close_page (self->view, self->page);

  return G_SOURCE_REMOVE;
}

static void
close_clicked_cb (AdwTab *self)
{
  if (!self->page)
    return;

  /* When animations are disabled, we don't want to immediately remove the
   * whole tab mid-click. Instead, defer it until the click has happened.
   */
  g_idle_add ((GSourceFunc) close_idle_cb, self);
}

static void
indicator_clicked_cb (AdwTab *self)
{
  if (!self->page)
    return;

  g_signal_emit_by_name (self->view, "indicator-activated", self->page);
}

static void
enter_cb (AdwTab             *self,
          double              x,
          double              y,
          GtkEventController *controller)
{
  self->hovering = TRUE;

  update_state (self);
}

static void
motion_cb (AdwTab             *self,
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

  update_state (self);
}

static void
leave_cb (AdwTab             *self,
          GtkEventController *controller)
{
  self->hovering = FALSE;

  update_state (self);
}

static gboolean
drop_cb (AdwTab *self,
         GValue *value)
{
  gboolean ret = GDK_EVENT_PROPAGATE;

  g_signal_emit (self, signals[SIGNAL_EXTRA_DRAG_DROP], 0, value, &ret);

  return ret;
}

static void
ensure_shader (AdwTab *self)
{
  GtkNative *native;
  GskRenderer *renderer;
  g_autoptr (GError) error = NULL;

  if (self->shader)
    return;

  self->shader = gsk_gl_shader_new_from_resource ("/org/gnome/Adwaita/glsl/fade.glsl");

  native = gtk_widget_get_native (GTK_WIDGET (self));
  renderer = gtk_native_get_renderer (native);

  self->shader_compiled = gsk_gl_shader_compile (self->shader, renderer, &error);

  if (error) {
    /* If shaders aren't supported, the error doesn't matter and we just
     * silently fall back */
    if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED))
      g_critical ("Couldn't compile shader: %s\n", error->message);
  }
}

static gboolean
activate_cb (AdwTab   *self,
             GVariant *args)
{
  GtkWidget *child;

  if (!self->page || !self->view)
    return GDK_EVENT_PROPAGATE;

  child = adw_tab_page_get_child (self->page);

  gtk_widget_grab_focus (child);

  return GDK_EVENT_STOP;
}

static void
adw_tab_measure (GtkWidget      *widget,
                 GtkOrientation  orientation,
                 int             for_size,
                 int            *minimum,
                 int            *natural,
                 int            *minimum_baseline,
                 int            *natural_baseline)
{
  AdwTab *self = ADW_TAB (widget);
  int min = 0, nat = 0;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    nat = self->pinned ? BASE_WIDTH_PINNED : BASE_WIDTH;
  } else {
    int child_min, child_nat;

    gtk_widget_measure (self->icon_stack, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->title, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->close_btn, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);

    gtk_widget_measure (self->indicator_btn, orientation, for_size,
                        &child_min, &child_nat, NULL, NULL);
    min = MAX (min, child_min);
    nat = MAX (nat, child_nat);
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

static inline void
measure_child (GtkWidget *child,
               int        height,
               int       *width)
{
  if (gtk_widget_get_visible (child))
    gtk_widget_measure (child, GTK_ORIENTATION_HORIZONTAL, height, NULL, width, NULL, NULL);
  else
    *width = 0;
}

static inline void
allocate_child (GtkWidget     *child,
                GtkAllocation *alloc,
                int            x,
                int            width,
                int            baseline)
{
  GtkAllocation child_alloc = *alloc;

  if (gtk_widget_get_direction (child) == GTK_TEXT_DIR_RTL)
    child_alloc.x += alloc->width - width - x;
  else
    child_alloc.x += x;

  child_alloc.width = width;

  gtk_widget_size_allocate (child, &child_alloc, baseline);
}

static void
allocate_contents (AdwTab        *self,
                   GtkAllocation *alloc,
                   int            baseline)
{
  int indicator_width, close_width, icon_width, title_width;
  int center_x, center_width = 0;
  int start_width = 0, end_width = 0;

  measure_child (self->icon_stack, alloc->height, &icon_width);
  measure_child (self->title, alloc->height, &title_width);
  measure_child (self->indicator_btn, alloc->height, &indicator_width);
  measure_child (self->close_btn, alloc->height, &close_width);

  if (gtk_widget_get_visible (self->indicator_btn)) {
    if (self->pinned) {
      /* Center it in a pinned tab */
      allocate_child (self->indicator_btn, alloc,
                      (alloc->width - indicator_width) / 2, indicator_width,
                      baseline);
    } else if (self->inverted) {
      allocate_child (self->indicator_btn, alloc,
                      alloc->width - indicator_width, indicator_width,
                      baseline);

      end_width = indicator_width;
    } else {
      allocate_child (self->indicator_btn, alloc, 0, indicator_width, baseline);

      start_width = indicator_width;
    }
  }

  if (gtk_widget_get_visible (self->close_btn)) {
    if (self->inverted) {
      allocate_child (self->close_btn, alloc, 0, close_width, baseline);

      start_width = close_width;
    } else {
      allocate_child (self->close_btn, alloc,
                      alloc->width - close_width, close_width, baseline);

      if (self->title_inverted)
        end_width = close_width;
    }
  }

  center_width = MIN (alloc->width - start_width - end_width,
                      icon_width + title_width);
  center_x = CLAMP ((alloc->width - center_width) / 2,
                    start_width,
                    alloc->width - center_width - end_width);

  self->close_overlap = !self->inverted &&
                        !self->title_inverted &&
                        gtk_widget_get_visible (self->title) &&
                        gtk_widget_get_visible (self->close_btn) &&
                        center_x + center_width > alloc->width - close_width;

  if (gtk_widget_get_visible (self->icon_stack)) {
    allocate_child (self->icon_stack, alloc, center_x, icon_width, baseline);

    center_x += icon_width;
    center_width -= icon_width;
  }

  if (gtk_widget_get_visible (self->title))
    allocate_child (self->title, alloc, center_x, center_width, baseline);
}

static void
adw_tab_size_allocate (GtkWidget *widget,
                       int        width,
                       int        height,
                       int        baseline)
{
  AdwTab *self = ADW_TAB (widget);
  GtkAllocation child_alloc;
  int allocated_width, width_diff;

  if (!self->icon_stack ||
      !self->indicator_btn ||
      !self->title ||
      !self->close_btn)
    return;

  allocated_width = gtk_widget_get_allocated_width (widget);
  width_diff = MAX (0, self->display_width - allocated_width);

  child_alloc.x = -width_diff / 2;
  child_alloc.y = 0;
  child_alloc.height = height;
  child_alloc.width = width + width_diff;

  allocate_contents (self, &child_alloc, baseline);
}

static void
adw_tab_map (GtkWidget *widget)
{
  AdwTab *self = ADW_TAB (widget);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->map (widget);

  update_spinner (self);
}

static void
adw_tab_unmap (GtkWidget *widget)
{
  AdwTab *self = ADW_TAB (widget);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->unmap (widget);

  update_spinner (self);
}

static void
adw_tab_snapshot (GtkWidget   *widget,
                  GtkSnapshot *snapshot)
{
  AdwTab *self = ADW_TAB (widget);
  float opacity = gtk_widget_get_opacity (self->close_btn);
  gboolean draw_fade = self->close_overlap && opacity > 0;

  gtk_widget_snapshot_child (widget, self->indicator_btn, snapshot);
  gtk_widget_snapshot_child (widget, self->icon_stack, snapshot);

  if (draw_fade) {
    gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
    int width = gtk_widget_get_width (widget);
    int height = gtk_widget_get_height (widget);
    float offset =
      gtk_widget_get_allocated_width (self->close_btn) +
      gtk_widget_get_margin_end (self->title);
    graphene_rect_t bounds;

    ensure_shader (self);

    graphene_rect_init (&bounds, 0, 0, width, height);

    if (self->shader_compiled) {
      gtk_snapshot_push_gl_shader (snapshot, self->shader, &bounds,
                                   gsk_gl_shader_format_args (self->shader,
                                                              "offsetLeft", is_rtl ? offset : 0.0f,
                                                              "offsetRight", is_rtl ? 0.0f : offset,
                                                              "strengthLeft", is_rtl ? opacity : 0.0f,
                                                              "strengthRight", is_rtl ? 0.0f : opacity,
                                                              NULL));
    } else {
      bounds.size.width -= offset;

      if (is_rtl)
        bounds.origin.x += offset;

      gtk_snapshot_push_clip (snapshot, &bounds);
    }
  }

  gtk_widget_snapshot_child (widget, self->title, snapshot);

  if (draw_fade) {
    if (self->shader_compiled)
      gtk_snapshot_gl_shader_pop_texture (snapshot);

    gtk_snapshot_pop (snapshot);
  }

  gtk_widget_snapshot_child (widget, self->close_btn, snapshot);
}

static void
adw_tab_direction_changed (GtkWidget        *widget,
                           GtkTextDirection  previous_direction)
{
  AdwTab *self = ADW_TAB (widget);

  update_title (self);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->direction_changed (widget,
                                                              previous_direction);
}

static void
adw_tab_unrealize (GtkWidget *widget)
{
  AdwTab *self = ADW_TAB (widget);

  GTK_WIDGET_CLASS (adw_tab_parent_class)->unrealize (widget);

  g_clear_object (&self->shader);
}

static void
adw_tab_constructed (GObject *object)
{
  AdwTab *self = ADW_TAB (object);

  G_OBJECT_CLASS (adw_tab_parent_class)->constructed (object);

  if (self->pinned) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "pinned");
    gtk_widget_hide (self->title);
    gtk_widget_hide (self->close_btn);
    gtk_widget_set_margin_start (self->icon_stack, 0);
    gtk_widget_set_margin_end (self->icon_stack, 0);
  }

  g_signal_connect_object (self->view, "notify::default-icon",
                           G_CALLBACK (update_icons), self,
                           G_CONNECT_SWAPPED);
}

static void
adw_tab_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  AdwTab *self = ADW_TAB (object);

  switch (prop_id) {
  case PROP_VIEW:
    g_value_set_object (value, self->view);
    break;

  case PROP_PAGE:
    g_value_set_object (value, adw_tab_get_page (self));
    break;

  case PROP_PINNED:
    g_value_set_boolean (value, self->pinned);
    break;

  case PROP_DRAGGING:
    g_value_set_boolean (value, adw_tab_get_dragging (self));
    break;

  case PROP_DISPLAY_WIDTH:
    g_value_set_int (value, adw_tab_get_display_width (self));
    break;

  case PROP_INVERTED:
    g_value_set_boolean (value, adw_tab_get_inverted (self));
    break;

    default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  AdwTab *self = ADW_TAB (object);

  switch (prop_id) {
  case PROP_VIEW:
    self->view = g_value_get_object (value);
    break;

  case PROP_PAGE:
    adw_tab_set_page (self, g_value_get_object (value));
    break;

  case PROP_PINNED:
    self->pinned = g_value_get_boolean (value);
    break;

  case PROP_DRAGGING:
    adw_tab_set_dragging (self, g_value_get_boolean (value));
    break;

  case PROP_DISPLAY_WIDTH:
    adw_tab_set_display_width (self, g_value_get_int (value));
    break;

  case PROP_INVERTED:
    adw_tab_set_inverted (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_tab_dispose (GObject *object)
{
  AdwTab *self = ADW_TAB (object);

  adw_tab_set_page (self, NULL);

  g_clear_object (&self->shader);
  gtk_widget_unparent (self->indicator_btn);
  gtk_widget_unparent (self->icon_stack);
  gtk_widget_unparent (self->title);
  gtk_widget_unparent (self->close_btn);

  G_OBJECT_CLASS (adw_tab_parent_class)->dispose (object);
}

static void
adw_tab_class_init (AdwTabClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_tab_dispose;
  object_class->constructed = adw_tab_constructed;
  object_class->get_property = adw_tab_get_property;
  object_class->set_property = adw_tab_set_property;

  widget_class->measure = adw_tab_measure;
  widget_class->size_allocate = adw_tab_size_allocate;
  widget_class->map = adw_tab_map;
  widget_class->unmap = adw_tab_unmap;
  widget_class->snapshot = adw_tab_snapshot;
  widget_class->direction_changed = adw_tab_direction_changed;
  widget_class->unrealize = adw_tab_unrealize;

  props[PROP_VIEW] =
    g_param_spec_object ("view",
                         "View",
                         "View",
                         ADW_TYPE_TAB_VIEW,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  props[PROP_PINNED] =
    g_param_spec_boolean ("pinned",
                          "Pinned",
                          "Pinned",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  props[PROP_DRAGGING] =
    g_param_spec_boolean ("dragging",
                          "Dragging",
                          "Dragging",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_PAGE] =
    g_param_spec_object ("page",
                         "Page",
                         "Page",
                         ADW_TYPE_TAB_PAGE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_DISPLAY_WIDTH] =
    g_param_spec_int ("display-width",
                      "Display Width",
                      "Display Width",
                      0, G_MAXINT, 0,
                      G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_INVERTED] =
    g_param_spec_boolean ("inverted",
                          "Inverted",
                          "Inverted",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  signals[SIGNAL_EXTRA_DRAG_DROP] =
    g_signal_new ("extra-drag-drop",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_first_wins, NULL, NULL,
                  G_TYPE_BOOLEAN,
                  1,
                  G_TYPE_VALUE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-tab.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwTab, title);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, icon_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, icon);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, spinner);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, indicator_icon);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, indicator_btn);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, close_btn);
  gtk_widget_class_bind_template_child (widget_class, AdwTab, drop_target);
  gtk_widget_class_bind_template_callback (widget_class, close_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, indicator_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, motion_cb);
  gtk_widget_class_bind_template_callback (widget_class, leave_cb);
  gtk_widget_class_bind_template_callback (widget_class, drop_cb);

  gtk_widget_class_add_binding (widget_class, GDK_KEY_space,     0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Space,  0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_Return,    0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_ISO_Enter, 0, (GtkShortcutFunc) activate_cb, NULL);
  gtk_widget_class_add_binding (widget_class, GDK_KEY_KP_Enter,  0, (GtkShortcutFunc) activate_cb, NULL);

  gtk_widget_class_set_css_name (widget_class, "tab");
}

static void
adw_tab_init (AdwTab *self)
{
  g_type_ensure (ADW_TYPE_FADING_LABEL);

  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwTab *
adw_tab_new (AdwTabView *view,
             gboolean    pinned)
{
  g_return_val_if_fail (ADW_IS_TAB_VIEW (view), NULL);

  return g_object_new (ADW_TYPE_TAB,
                       "view", view,
                       "pinned", pinned,
                       NULL);
}

AdwTabPage *
adw_tab_get_page (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), NULL);

  return self->page;
}

void
adw_tab_set_page (AdwTab     *self,
                  AdwTabPage *page)
{
  g_return_if_fail (ADW_IS_TAB (self));
  g_return_if_fail (page == NULL || ADW_IS_TAB_PAGE (page));

  if (self->page == page)
    return;

  if (self->page) {
    g_signal_handlers_disconnect_by_func (self->page, update_selected, self);
    g_signal_handlers_disconnect_by_func (self->page, update_title, self);
    g_signal_handlers_disconnect_by_func (self->page, update_tooltip, self);
    g_signal_handlers_disconnect_by_func (self->page, update_icons, self);
    g_signal_handlers_disconnect_by_func (self->page, update_indicator, self);
    g_signal_handlers_disconnect_by_func (self->page, update_needs_attention, self);
    g_signal_handlers_disconnect_by_func (self->page, update_loading, self);
  }

  g_set_object (&self->page, page);

  if (self->page) {
    update_selected (self);
    update_state (self);
    update_title (self);
    update_tooltip (self);
    update_spinner (self);
    update_icons (self);
    update_indicator (self);
    update_needs_attention (self);
    update_loading (self);

    g_signal_connect_object (self->page, "notify::selected",
                             G_CALLBACK (update_selected), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::title",
                             G_CALLBACK (update_title), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::tooltip",
                             G_CALLBACK (update_tooltip), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::icon",
                             G_CALLBACK (update_icons), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-icon",
                             G_CALLBACK (update_icons), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::indicator-activatable",
                             G_CALLBACK (update_indicator), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::needs-attention",
                             G_CALLBACK (update_needs_attention), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->page, "notify::loading",
                             G_CALLBACK (update_loading), self,
                             G_CONNECT_SWAPPED);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PAGE]);
}

int
adw_tab_get_display_width (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), 0);

  return self->display_width;
}

void
adw_tab_set_display_width (AdwTab *self,
                           int     width)
{
  g_return_if_fail (ADW_IS_TAB (self));
  g_return_if_fail (width >= 0);

  if (self->display_width == width)
    return;

  self->display_width = width;

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DISPLAY_WIDTH]);
}

gboolean
adw_tab_get_dragging (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), FALSE);

  return self->dragging;
}

void
adw_tab_set_dragging (AdwTab   *self,
                      gboolean  dragging)
{
  g_return_if_fail (ADW_IS_TAB (self));

  dragging = !!dragging;

  if (self->dragging == dragging)
    return;

  self->dragging = dragging;

  update_state (self);
  update_selected (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DRAGGING]);
}

gboolean
adw_tab_get_inverted (AdwTab *self)
{
  g_return_val_if_fail (ADW_IS_TAB (self), FALSE);

  return self->inverted;
}

void
adw_tab_set_inverted (AdwTab   *self,
                      gboolean  inverted)
{
  g_return_if_fail (ADW_IS_TAB (self));

  inverted = !!inverted;

  if (self->inverted == inverted)
    return;

  self->inverted = inverted;

  gtk_widget_queue_allocate (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INVERTED]);
}

void
adw_tab_set_fully_visible (AdwTab   *self,
                           gboolean  fully_visible)
{
  g_return_if_fail (ADW_IS_TAB (self));

  fully_visible = !!fully_visible;

  if (self->fully_visible == fully_visible)
    return;

  self->fully_visible = fully_visible;

  update_state (self);
  update_indicator (self);
}

void
adw_tab_setup_extra_drop_target (AdwTab        *self,
                                 GdkDragAction  actions,
                                 GType         *types,
                                 gsize          n_types)
{
  g_return_if_fail (ADW_IS_TAB (self));
  g_return_if_fail (n_types == 0 || types != NULL);

  gtk_drop_target_set_actions (self->drop_target, actions);
  gtk_drop_target_set_gtypes (self->drop_target, types, n_types);
}
