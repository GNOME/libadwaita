#include "adw-demo-window.h"

#include <glib/gi18n.h>
#include "pages/avatar/adw-demo-page-avatar.h"
#include "pages/buttons/adw-demo-page-buttons.h"
#include "pages/carousel/adw-demo-page-carousel.h"
#include "pages/clamp/adw-demo-page-clamp.h"
#include "pages/flap/adw-demo-page-flap.h"
#include "pages/leaflet/adw-demo-page-leaflet.h"
#include "pages/lists/adw-demo-page-lists.h"
#include "pages/tab-view/adw-demo-page-tab-view.h"
#include "pages/view-switcher/adw-demo-page-view-switcher.h"
#include "pages/welcome/adw-demo-page-welcome.h"
#include "adw-style-demo-window.h"

struct _AdwDemoWindow
{
  AdwApplicationWindow parent_instance;

  AdwLeaflet *content_box;
  AdwToastOverlay *toast_overlay;
  GtkBox *right_box;
  GtkWidget *color_scheme_button;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  AdwLeaflet *subpage_leaflet;
  int toast_undo_items;
  AdwToast *undo_toast;
  GtkStack *animation_preferences_stack;
  AdwAnimation *timed_animation;
  GtkWidget *timed_animation_sample;
  GtkWidget *timed_animation_button_box;
  GtkSpinButton *timed_animation_repeat_count;
  GtkSwitch *timed_animation_reverse;
  GtkSwitch *timed_animation_alternate;
  GtkSpinButton *timed_animation_duration;
  AdwComboRow *timed_animation_easing;
  AdwAnimation *spring_animation;
  GtkSpinButton *spring_animation_velocity;
  GtkSpinButton *spring_animation_damping;
  GtkSpinButton *spring_animation_mass;
  GtkSpinButton *spring_animation_stiffness;
  GtkSpinButton *spring_animation_epsilon;
  GtkSwitch *spring_animation_clamp_switch;
};

G_DEFINE_TYPE (AdwDemoWindow, adw_demo_window, ADW_TYPE_APPLICATION_WINDOW)

enum {
  PROP_0,
  PROP_TIMED_ANIMATION,
  PROP_SPRING_ANIMATION,
  LAST_PROP,
};

static char *
get_color_scheme_icon_name (gpointer user_data,
                            gboolean dark)
{
  return g_strdup (dark ? "light-mode-symbolic" : "dark-mode-symbolic");
}

static GParamSpec *props[LAST_PROP];

static void
adw_demo_window_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  AdwDemoWindow *self = ADW_DEMO_WINDOW (object);

  switch (prop_id) {
  case PROP_TIMED_ANIMATION:
    g_value_set_object (value, self->timed_animation);
    break;

  case PROP_SPRING_ANIMATION:
    g_value_set_object (value, self->spring_animation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_window_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  AdwDemoWindow *self = ADW_DEMO_WINDOW (object);

  switch (prop_id) {
  case PROP_TIMED_ANIMATION:
    g_set_object (&self->timed_animation, g_value_get_object (value));
    break;

  case PROP_SPRING_ANIMATION:
    g_set_object (&self->spring_animation, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
color_scheme_button_clicked_cb (AdwDemoWindow *self)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();

  if (adw_style_manager_get_dark (manager))
    adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_FORCE_LIGHT);
  else
    adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_FORCE_DARK);
}

static void
notify_system_supports_color_schemes_cb (AdwDemoWindow *self)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();
  gboolean supports = adw_style_manager_get_system_supports_color_schemes (manager);

  gtk_widget_set_visible (self->color_scheme_button, !supports);

  if (supports)
    adw_style_manager_set_color_scheme (manager, ADW_COLOR_SCHEME_DEFAULT);
}

static void
notify_visible_child_cb (GObject       *sender,
                         GParamSpec    *pspec,
                         AdwDemoWindow *self)
{
  adw_leaflet_navigate (self->content_box, ADW_NAVIGATION_DIRECTION_FORWARD);
}

static void
back_clicked_cb (GtkWidget     *sender,
                 AdwDemoWindow *self)
{
  adw_leaflet_navigate (self->content_box, ADW_NAVIGATION_DIRECTION_BACK);
}

static void
leaflet_back_clicked_cb (GtkWidget     *sender,
                         AdwDemoWindow *self)
{
  adw_leaflet_navigate (self->subpage_leaflet, ADW_NAVIGATION_DIRECTION_BACK);
}

static void
leaflet_next_page_cb (AdwDemoWindow *self)
{
  adw_leaflet_navigate (self->subpage_leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
}

AdwDemoWindow *
adw_demo_window_new (GtkApplication *application)
{
  return g_object_new (ADW_TYPE_DEMO_WINDOW, "application", application, NULL);
}

static AdwAnimation *
get_current_animation (AdwDemoWindow *self)
{
  const char *current_animation;

  current_animation = gtk_stack_get_visible_child_name (self->animation_preferences_stack);

  if (!g_strcmp0 (current_animation, "Timed")) {
    return self->timed_animation;
  } else if (!g_strcmp0 (current_animation, "Spring")) {
    return self->spring_animation;
  } else {
    g_assert_not_reached ();
  }
}

static char *
animations_easing_name (AdwEnumListItem *value,
                        gpointer         user_data)
{
  g_return_val_if_fail (ADW_IS_ENUM_LIST_ITEM (value), NULL);

  switch (adw_enum_list_item_get_value (value)) {
  case ADW_LINEAR:
    return g_strdup (_("Linear"));
  case ADW_EASE_IN_QUAD:
    return g_strdup (_("Ease-in (Quadratic)"));
  case ADW_EASE_OUT_QUAD:
    return g_strdup (_("Ease-out (Quadratic)"));
  case ADW_EASE_IN_OUT_QUAD:
    return g_strdup (_("Ease-in-out (Quadratic)"));
  case ADW_EASE_IN_CUBIC:
    return g_strdup (_("Ease-in (Cubic)"));
  case ADW_EASE_OUT_CUBIC:
    return g_strdup (_("Ease-out (Cubic)"));
  case ADW_EASE_IN_OUT_CUBIC:
    return g_strdup (_("Ease-in-out (Cubic)"));
  case ADW_EASE_IN_QUART:
    return g_strdup (_("Ease-in (Quartic)"));
  case ADW_EASE_OUT_QUART:
    return g_strdup (_("Ease-out (Quartic)"));
  case ADW_EASE_IN_OUT_QUART:
    return g_strdup (_("Ease-in-out (Quartic)"));
  case ADW_EASE_IN_QUINT:
    return g_strdup (_("Ease-in (Quintic)"));
  case ADW_EASE_OUT_QUINT:
    return g_strdup (_("Ease-out (Quintic)"));
  case ADW_EASE_IN_OUT_QUINT:
    return g_strdup (_("Ease-in-out (Quintic)"));
  case ADW_EASE_IN_SINE:
    return g_strdup (_("Ease-in (Sine)"));
  case ADW_EASE_OUT_SINE:
    return g_strdup (_("Ease-out (Sine)"));
  case ADW_EASE_IN_OUT_SINE:
    return g_strdup (_("Ease-in-out (Sine)"));
  case ADW_EASE_IN_EXPO:
    return g_strdup (_("Ease-in (Exponential)"));
  case ADW_EASE_OUT_EXPO:
    return g_strdup (_("Ease-out (Exponential)"));
  case ADW_EASE_IN_OUT_EXPO:
    return g_strdup (_("Ease-in-out (Exponential)"));
  case ADW_EASE_IN_CIRC:
    return g_strdup (_("Ease-in (Circular)"));
  case ADW_EASE_OUT_CIRC:
    return g_strdup (_("Ease-out (Circular)"));
  case ADW_EASE_IN_OUT_CIRC:
    return g_strdup (_("Ease-in-out (Circular)"));
  case ADW_EASE_IN_ELASTIC:
    return g_strdup (_("Ease-in (Elastic)"));
  case ADW_EASE_OUT_ELASTIC:
    return g_strdup (_("Ease-out (Elastic)"));
  case ADW_EASE_IN_OUT_ELASTIC:
    return g_strdup (_("Ease-in-out (Elastic)"));
  case ADW_EASE_IN_BACK:
    return g_strdup (_("Ease-in (Back)"));
  case ADW_EASE_OUT_BACK:
    return g_strdup (_("Ease-out (Back)"));
  case ADW_EASE_IN_OUT_BACK:
    return g_strdup (_("Ease-in-out (Back)"));
  case ADW_EASE_IN_BOUNCE:
    return g_strdup (_("Ease-in (Bounce)"));
  case ADW_EASE_OUT_BOUNCE:
    return g_strdup (_("Ease-out (Bounce)"));
  case ADW_EASE_IN_OUT_BOUNCE:
    return g_strdup (_("Ease-in-out (Bounce)"));
  default:
    return NULL;
  }
}

static void
timed_animation_measure (GtkWidget      *widget,
                         GtkOrientation  orientation,
                         int             for_size,
                         int            *minimum,
                         int            *natural,
                         int            *minimum_baseline,
                         int            *natural_baseline)
{
  GtkWidget *child = gtk_widget_get_first_child (widget);

  if (!child)
    return;

  gtk_widget_measure (child, orientation, for_size, minimum, natural,
                      minimum_baseline, natural_baseline);
}

static void
timed_animation_allocate (GtkWidget *widget,
                          int        width,
                          int        height,
                          int        baseline)
{
  AdwDemoWindow *self = ADW_DEMO_WINDOW (gtk_widget_get_root (widget));
  GtkWidget *child = gtk_widget_get_first_child (widget);
  AdwAnimation *animation = get_current_animation (self);
  double progress;
  int child_width, offset;

  if (!child)
    return;

  progress = adw_animation_get_value (animation);

  gtk_widget_measure (child, GTK_ORIENTATION_HORIZONTAL, -1,
                      &child_width, NULL, NULL, NULL);

  offset = (int) ((width - child_width) * (progress - 0.5));

  gtk_widget_allocate (child, width, height, baseline,
                       gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (offset, 0)));
}

static void
timed_animation_reset (AdwDemoWindow *self)
{
  adw_animation_reset (self->timed_animation);
  adw_animation_reset (self->spring_animation);
}

static void
timed_animation_play_pause (AdwDemoWindow *self)
{
  AdwAnimation *animation = get_current_animation (self);

  switch (adw_animation_get_state (animation)) {
  case ADW_ANIMATION_IDLE:
  case ADW_ANIMATION_FINISHED:
    adw_animation_play (animation);
    break;
  case ADW_ANIMATION_PAUSED:
    adw_animation_resume (animation);
    break;
  case ADW_ANIMATION_PLAYING:
    adw_animation_pause (animation);
    break;
  default:
    g_assert_not_reached ();
  }

}

static void
timed_animation_skip (AdwDemoWindow *self)
{
  adw_animation_skip (self->timed_animation);
  adw_animation_skip (self->spring_animation);
}

static char *
get_play_pause_icon_name (gpointer          user_data,
                           AdwAnimationState timed_state,
                           AdwAnimationState spring_state)
{
  gboolean playing = timed_state  == ADW_ANIMATION_PLAYING ||
                     spring_state == ADW_ANIMATION_PLAYING;

  return g_strdup (playing ? "media-playback-pause-symbolic" : "media-playback-start-symbolic");
}

static gboolean
timed_animation_can_reset (gpointer          user_data,
                           AdwAnimationState timed_state,
                           AdwAnimationState spring_state)
{
  return timed_state  != ADW_ANIMATION_IDLE ||
         spring_state != ADW_ANIMATION_IDLE;
}

static gboolean
timed_animation_can_skip (gpointer          user_data,
                          AdwAnimationState timed_state,
                          AdwAnimationState spring_state)
{
  return timed_state  != ADW_ANIMATION_FINISHED &&
         spring_state != ADW_ANIMATION_FINISHED;
}

static void
timed_animation_cb (double     value,
                    GtkWidget *self)
{
  gtk_widget_queue_allocate (self);
}

static void
notify_spring_params_change (AdwDemoWindow *self)
{
  g_autoptr (AdwSpringParams) spring_params =
    adw_spring_params_new_full (gtk_spin_button_get_value (self->spring_animation_damping),
                                gtk_spin_button_get_value (self->spring_animation_mass),
                                gtk_spin_button_get_value (self->spring_animation_stiffness));

  adw_spring_animation_set_spring_params (ADW_SPRING_ANIMATION (self->spring_animation), spring_params);
}

static void
style_classes_demo_clicked_cb (GtkButton     *btn,
                               AdwDemoWindow *self)
{
  AdwStyleDemoWindow *window = adw_style_demo_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (self));
  gtk_window_present (GTK_WINDOW (window));
}

static void
add_toast_cb (AdwDemoWindow *self)
{
  adw_toast_overlay_add_toast (self->toast_overlay,
                               adw_toast_new (_("Simple Toast")));
}

static void
dismissed_cb (AdwDemoWindow *self)
{
  self->undo_toast = NULL;
  self->toast_undo_items = 0;

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);
}

static void
add_toast_with_button_cb (AdwDemoWindow *self)
{
  g_autofree char *title = NULL;

  self->toast_undo_items++;

  if (!self->undo_toast) {
    title = g_strdup_printf (_("‘%s’ deleted"), "Lorem Ipsum");

    self->undo_toast = adw_toast_new (title);

    adw_toast_set_priority (self->undo_toast, ADW_TOAST_PRIORITY_HIGH);
    adw_toast_set_button_label (self->undo_toast, _("_Undo"));
    adw_toast_set_action_name (self->undo_toast, "toast.undo");

    g_signal_connect_swapped (self->undo_toast, "dismissed", G_CALLBACK (dismissed_cb), self);

    adw_toast_overlay_add_toast (self->toast_overlay, self->undo_toast);

    gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", TRUE);

    return;
  }

  title =
    g_strdup_printf (ngettext ("<span font_features='tnum=1'>%d</span> item deleted",
                               "<span font_features='tnum=1'>%d</span> items deleted",
                               self->toast_undo_items), self->toast_undo_items);

  adw_toast_set_title (self->undo_toast, title);
}

static void
add_toast_with_long_title_cb (AdwDemoWindow *self)
{
  adw_toast_overlay_add_toast (self->toast_overlay,
                               adw_toast_new (_("Lorem ipsum dolor sit amet, "
                                                "consectetur adipiscing elit, "
                                                "sed do eiusmod tempor incididunt "
                                                "ut labore et dolore magnam aliquam "
                                                "quaerat voluptatem.")));
}

static void
toast_undo_cb (AdwDemoWindow *self)
{
  g_autofree char *title =
    g_strdup_printf (ngettext ("Undoing deleting <span font_features='tnum=1'>%d</span> item…",
                               "Undoing deleting <span font_features='tnum=1'>%d</span> items…",
                               self->toast_undo_items), self->toast_undo_items);
  AdwToast *toast = adw_toast_new (title);

  adw_toast_set_priority (toast, ADW_TOAST_PRIORITY_HIGH);

  adw_toast_overlay_add_toast (self->toast_overlay, toast);
}

static void
toast_dismiss_cb (AdwDemoWindow *self)
{
  if (self->undo_toast)
    adw_toast_dismiss (self->undo_toast);
}

static void
adw_demo_window_class_init (AdwDemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = adw_demo_window_set_property;
  object_class->get_property = adw_demo_window_get_property;

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_q, GDK_CONTROL_MASK, "window.close", NULL);

  gtk_widget_class_install_action (widget_class, "toast.undo", NULL, (GtkWidgetActionActivateFunc) toast_undo_cb);
  gtk_widget_class_install_action (widget_class, "toast.dismiss", NULL, (GtkWidgetActionActivateFunc) toast_dismiss_cb);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/adw-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, right_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, color_scheme_button);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, subpage_leaflet);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, toast_overlay);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, animation_preferences_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_sample);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_button_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_repeat_count);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_reverse);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_alternate);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_duration);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_easing);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, spring_animation_velocity);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, spring_animation_damping);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, spring_animation_mass);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, spring_animation_stiffness);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, spring_animation_epsilon);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, spring_animation_clamp_switch);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, back_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, leaflet_back_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, leaflet_next_page_cb);
  gtk_widget_class_bind_template_callback (widget_class, get_color_scheme_icon_name);
  gtk_widget_class_bind_template_callback (widget_class, color_scheme_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, style_classes_demo_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, add_toast_cb);
  gtk_widget_class_bind_template_callback (widget_class, add_toast_with_button_cb);
  gtk_widget_class_bind_template_callback (widget_class, add_toast_with_long_title_cb);
  gtk_widget_class_bind_template_callback (widget_class, animations_easing_name);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_reset);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_play_pause);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_skip);
  gtk_widget_class_bind_template_callback (widget_class, get_play_pause_icon_name);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_can_reset);
  gtk_widget_class_bind_template_callback (widget_class, timed_animation_can_skip);
  gtk_widget_class_bind_template_callback (widget_class, notify_spring_params_change);

  props[PROP_TIMED_ANIMATION] =
    g_param_spec_object ("timed_animation",
                         "Timed animation",
                         "Timed animation",
                         ADW_TYPE_ANIMATION,
                         G_PARAM_READWRITE);

  props[PROP_SPRING_ANIMATION] =
    g_param_spec_object ("spring_animation",
                         "Spring animation",
                         "Spring animation",
                         ADW_TYPE_ANIMATION,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
animation_page_init (AdwDemoWindow *self)
{
  GtkLayoutManager *manager;
  AdwAnimationTarget *target;

  target = adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                              timed_animation_cb,
                                              self->timed_animation_sample,
                                              NULL);

  self->timed_animation =
    adw_timed_animation_new (GTK_WIDGET (self->timed_animation_sample),
                             0, 1, 100, g_object_ref (target));

  self->spring_animation =
    adw_spring_animation_new (GTK_WIDGET (self->timed_animation_sample), 0, 1,
                              adw_spring_params_new_full (10, 1, 100),
                              target);

  notify_spring_params_change (self);

  g_object_bind_property (self->timed_animation_repeat_count, "value",
                          self->timed_animation, "repeat-count",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_reverse, "state",
                          self->timed_animation, "reverse",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_alternate, "state",
                          self->timed_animation, "alternate",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_duration, "value",
                          self->timed_animation, "duration",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->timed_animation_easing, "selected",
                          self->timed_animation, "easing",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property (self->spring_animation_velocity, "value",
                          self->spring_animation, "initial_velocity",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->spring_animation_epsilon, "value",
                          self->spring_animation, "epsilon",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_object_bind_property (self->spring_animation_clamp_switch, "active",
                          self->spring_animation, "clamp",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->timed_animation),
                                  ADW_EASE_IN_OUT_CUBIC);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIMED_ANIMATION]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPRING_ANIMATION]);

  manager = gtk_custom_layout_new (NULL, timed_animation_measure,
                                   timed_animation_allocate);

  gtk_widget_set_layout_manager (self->timed_animation_sample, manager);

  gtk_widget_set_direction (self->timed_animation_button_box, GTK_TEXT_DIR_LTR);
}

static void
adw_demo_window_init (AdwDemoWindow *self)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();

  g_type_ensure (ADW_TYPE_DEMO_PAGE_AVATAR);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_BUTTONS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_CAROUSEL);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_CLAMP);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_FLAP);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_LEAFLET);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_LISTS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_TAB_VIEW);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_VIEW_SWITCHER);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_WELCOME);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (manager,
                           "notify::system-supports-color-schemes",
                           G_CALLBACK (notify_system_supports_color_schemes_cb),
                           self,
                           G_CONNECT_SWAPPED);

  notify_system_supports_color_schemes_cb (self);

  adw_leaflet_set_visible_child (self->content_box, GTK_WIDGET (self->right_box));

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);

  animation_page_init (self);
}
