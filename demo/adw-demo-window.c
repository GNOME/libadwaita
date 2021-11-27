#include "adw-demo-window.h"

#include <glib/gi18n.h>
#include "adw-flap-demo-window.h"
#include "adw-style-demo-window.h"
#include "adw-tab-view-demo-window.h"
#include "adw-view-switcher-demo-window.h"

struct _AdwDemoWindow
{
  AdwApplicationWindow parent_instance;

  AdwLeaflet *content_box;
  AdwToastOverlay *toast_overlay;
  GtkBox *right_box;
  GtkWidget *color_scheme_button;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  AdwComboRow *leaflet_transition_row;
  AdwLeaflet *subpage_leaflet;
  AdwCarousel *carousel;
  GtkBox *carousel_box;
  GtkStack *carousel_indicators_stack;
  AdwAvatar *avatar;
  GtkEntry *avatar_text;
  GtkLabel *avatar_file_chooser_label;
  GtkButton *avatar_remove_button;
  GtkFileChooserNative *avatar_file_chooser;
  GtkListBox *avatar_contacts;
  int toast_undo_items;
  AdwToast *undo_toast;
  AdwAnimation *timed_animation;
  GtkWidget *timed_animation_sample;
  GtkWidget *timed_animation_button_box;
  GtkSpinButton *timed_animation_repeat_count;
  GtkSwitch *timed_animation_reverse;
  GtkSwitch *timed_animation_alternate;
  GtkSpinButton *timed_animation_duration;
  AdwComboRow *timed_animation_easing;
};

G_DEFINE_TYPE (AdwDemoWindow, adw_demo_window, ADW_TYPE_APPLICATION_WINDOW)

enum {
  PROP_0,
  PROP_TIMED_ANIMATION,
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

static char *
leaflet_transition_name (AdwEnumListItem *item,
                         gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case ADW_LEAFLET_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case ADW_LEAFLET_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  case ADW_LEAFLET_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
  default:
    return NULL;
  }
}

static void
notify_leaflet_transition_cb (GObject       *sender,
                              GParamSpec    *pspec,
                              AdwDemoWindow *self)
{
  AdwComboRow *row = ADW_COMBO_ROW (sender);

  g_assert (ADW_IS_COMBO_ROW (row));
  g_assert (ADW_IS_DEMO_WINDOW (self));

  adw_leaflet_set_transition_type (ADW_LEAFLET (self->content_box), adw_combo_row_get_selected (row));
}

static void
leaflet_go_next_row_activated_cb (AdwDemoWindow *self)
{
  g_assert (ADW_IS_DEMO_WINDOW (self));

  adw_leaflet_navigate (self->subpage_leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
}

static void
view_switcher_demo_clicked_cb (GtkButton     *btn,
                               AdwDemoWindow *self)
{
  AdwViewSwitcherDemoWindow *window = adw_view_switcher_demo_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (self));
  gtk_window_present (GTK_WINDOW (window));
}

static char *
carousel_orientation_name (AdwEnumListItem *item,
                           gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case GTK_ORIENTATION_HORIZONTAL:
    return g_strdup (_("Horizontal"));
  case GTK_ORIENTATION_VERTICAL:
    return g_strdup (_("Vertical"));
  default:
    return NULL;
  }
}

static void
notify_carousel_orientation_cb (GObject       *sender,
                                GParamSpec    *pspec,
                                AdwDemoWindow *self)
{
  AdwComboRow *row = ADW_COMBO_ROW (sender);

  g_assert (ADW_IS_COMBO_ROW (row));
  g_assert (ADW_IS_DEMO_WINDOW (self));

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->carousel_box),
                                  1 - adw_combo_row_get_selected (row));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->carousel),
                                  adw_combo_row_get_selected (row));
}

static char *
carousel_indicators_name (GtkStringObject *value)
{
  const char *style;

  g_assert (GTK_IS_STRING_OBJECT (value));

  style = gtk_string_object_get_string (value);

  if (!g_strcmp0 (style, "dots"))
    return g_strdup (_("Dots"));

  if (!g_strcmp0 (style, "lines"))
    return g_strdup (_("Lines"));

  return NULL;
}

static void
notify_carousel_indicators_cb (GObject       *sender,
                               GParamSpec    *pspec,
                               AdwDemoWindow *self)
{
  AdwComboRow *row = ADW_COMBO_ROW (sender);
  GtkStringObject *obj;

  g_assert (ADW_IS_COMBO_ROW (row));
  g_assert (ADW_IS_DEMO_WINDOW (self));

  obj = adw_combo_row_get_selected_item (row);

  gtk_stack_set_visible_child_name (self->carousel_indicators_stack,
                                    gtk_string_object_get_string (obj));
}

static void
carousel_return_clicked_cb (GtkButton     *btn,
                            AdwDemoWindow *self)
{
  adw_carousel_scroll_to (self->carousel,
                          adw_carousel_get_nth_page (self->carousel, 0));
}

AdwDemoWindow *
adw_demo_window_new (GtkApplication *application)
{
  return g_object_new (ADW_TYPE_DEMO_WINDOW, "application", application, NULL);
}

static void
avatar_file_remove_cb (AdwDemoWindow *self)
{
  g_assert (ADW_IS_DEMO_WINDOW (self));

  g_signal_handlers_disconnect_by_data (self->avatar, self);

  gtk_label_set_label (self->avatar_file_chooser_label, _("(None)"));
  gtk_widget_set_sensitive (GTK_WIDGET (self->avatar_remove_button), FALSE);
  adw_avatar_set_custom_image (self->avatar, NULL);
}

static void
avatar_file_chooser_response_cb (AdwDemoWindow *self,
                                 int            response)
{
  g_autoptr (GFile) file = NULL;
  g_autoptr (GFileInfo) info = NULL;
  g_autoptr (GdkTexture) texture = NULL;
  g_autoptr (GError) error = NULL;

  g_assert (ADW_IS_DEMO_WINDOW (self));

  if (response != GTK_RESPONSE_ACCEPT)
    return;

  file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (self->avatar_file_chooser));
  info = g_file_query_info (file,
                            G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
                            G_FILE_QUERY_INFO_NONE,
                            NULL,
                            NULL);

  if (info)
    gtk_label_set_label (self->avatar_file_chooser_label,
                         g_file_info_get_display_name (info));

  gtk_widget_set_sensitive (GTK_WIDGET (self->avatar_remove_button), TRUE);

  texture = gdk_texture_new_from_file (file, &error);
  if (error)
    g_critical ("Failed to create texture from file: %s", error->message);

  adw_avatar_set_custom_image (self->avatar, texture ? GDK_PAINTABLE (texture) : NULL);
}

static void
avatar_file_chooser_clicked_cb (AdwDemoWindow *self)
{
  gtk_native_dialog_show (GTK_NATIVE_DIALOG (self->avatar_file_chooser));
}

static void
file_chooser_response_cb (AdwDemoWindow  *self,
                          int             response_id,
                          GtkFileChooser *chooser)
{
  if (response_id == GTK_RESPONSE_ACCEPT) {
    g_autoptr (GFile) file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (chooser));
    g_autoptr (GdkTexture) texture =
      adw_avatar_draw_to_texture (self->avatar,
                                  gtk_widget_get_scale_factor (GTK_WIDGET (self)));

    gdk_texture_save_to_png (texture, g_file_peek_path (file));
  }

  g_object_unref (chooser);
}

static void
avatar_save_to_file_cb (AdwDemoWindow *self)
{
  GtkFileChooserNative *chooser = NULL;

  g_assert (ADW_IS_DEMO_WINDOW (self));

  chooser = gtk_file_chooser_native_new (_("Save Avatar"),
                                         GTK_WINDOW (self),
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         _("_Save"),
                                         _("_Cancel"));

  g_signal_connect_swapped (chooser, "response", G_CALLBACK (file_chooser_response_cb), self);

  gtk_native_dialog_show (GTK_NATIVE_DIALOG (chooser));
}

static char *
avatar_new_random_name (void)
{
  static const char *first_names[] = {
    "Adam",
    "Adrian",
    "Anna",
    "Charlotte",
    "Frédérique",
    "Ilaria",
    "Jakub",
    "Jennyfer",
    "Julia",
    "Justin",
    "Mario",
    "Miriam",
    "Mohamed",
    "Nourimane",
    "Owen",
    "Peter",
    "Petra",
    "Rachid",
    "Rebecca",
    "Sarah",
    "Thibault",
    "Wolfgang",
  };
  static const char *last_names[] = {
    "Bailey",
    "Berat",
    "Chen",
    "Farquharson",
    "Ferber",
    "Franco",
    "Galinier",
    "Han",
    "Lawrence",
    "Lepied",
    "Lopez",
    "Mariotti",
    "Rossi",
    "Urasawa",
    "Zwickelman",
  };

  return g_strdup_printf ("%s %s",
                          first_names[g_random_int_range (0, G_N_ELEMENTS (first_names))],
                          last_names[g_random_int_range (0, G_N_ELEMENTS (last_names))]);
}

static void
avatar_update_contacts (AdwDemoWindow *self)
{
  GtkWidget *row;

  while ((row = gtk_widget_get_first_child (GTK_WIDGET (self->avatar_contacts))))
    gtk_list_box_remove (self->avatar_contacts, row);

  for (int i = 0; i < 30; i++) {
    g_autofree char *name = avatar_new_random_name ();
    GtkWidget *contact = adw_action_row_new ();
    GtkWidget *avatar = adw_avatar_new (40, name, TRUE);

    gtk_widget_set_margin_top (avatar, 12);
    gtk_widget_set_margin_bottom (avatar, 12);

    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (contact), name);
    adw_action_row_add_prefix (ADW_ACTION_ROW (contact), avatar);
    gtk_list_box_append (self->avatar_contacts, contact);
  }
}

static void
flap_demo_clicked_cb (GtkButton     *btn,
                      AdwDemoWindow *self)
{
  AdwFlapDemoWindow *window = adw_flap_demo_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (self));
  gtk_window_present (GTK_WINDOW (window));
}

static void
tab_view_demo_clicked_cb (GtkButton     *btn,
                          AdwDemoWindow *self)
{
  AdwTabViewDemoWindow *window = adw_tab_view_demo_window_new ();

  adw_tab_view_demo_window_prepopulate (window);

  gtk_window_present (GTK_WINDOW (window));
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
  double progress;
  int child_width, offset;

  if (!child)
    return;

  progress = adw_animation_get_value (self->timed_animation);

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
}

static void
timed_animation_play_pause (AdwDemoWindow *self)
{
  switch (adw_animation_get_state (self->timed_animation)) {
  case ADW_ANIMATION_IDLE:
  case ADW_ANIMATION_FINISHED:
    adw_animation_play (self->timed_animation);
    break;
  case ADW_ANIMATION_PAUSED:
    adw_animation_resume (self->timed_animation);
    break;
  case ADW_ANIMATION_PLAYING:
    adw_animation_pause (self->timed_animation);
    break;
  default:
    g_assert_not_reached ();
  }
}

static void
timed_animation_skip (AdwDemoWindow *self)
{
  adw_animation_skip (self->timed_animation);
}

static char *
get_play_pause_icon_name (gpointer          user_data,
                          AdwAnimationState state)
{
  return g_strdup (state == ADW_ANIMATION_PLAYING ? "media-playback-pause-symbolic" : "media-playback-start-symbolic");
}

static gboolean
timed_animation_can_reset (gpointer          user_data,
                           AdwAnimationState state)
{
  return state != ADW_ANIMATION_IDLE;
}

static gboolean
timed_animation_can_skip (gpointer          user_data,
                          AdwAnimationState state)
{
  return state != ADW_ANIMATION_FINISHED;
}

static void
timed_animation_cb (double     value,
                    GtkWidget *self)
{
  gtk_widget_queue_allocate (self);
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
                               adw_toast_new (_("Simple toast")));
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
    title = g_strdup_printf (_("‘%s’ deleted"), "Lorem ipsum");

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
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, leaflet_transition_row);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, subpage_leaflet);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel_indicators_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_text);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_file_chooser_label);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_remove_button);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_contacts);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, toast_overlay);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_sample);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_button_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_repeat_count);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_reverse);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_alternate);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_duration);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, timed_animation_easing);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, back_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, leaflet_back_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, leaflet_transition_name);
  gtk_widget_class_bind_template_callback (widget_class, notify_leaflet_transition_cb);
  gtk_widget_class_bind_template_callback (widget_class, leaflet_go_next_row_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, get_color_scheme_icon_name);
  gtk_widget_class_bind_template_callback (widget_class, color_scheme_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, view_switcher_demo_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_carousel_orientation_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_carousel_indicators_cb);
  gtk_widget_class_bind_template_callback (widget_class, carousel_indicators_name);
  gtk_widget_class_bind_template_callback (widget_class, carousel_orientation_name);
  gtk_widget_class_bind_template_callback (widget_class, carousel_return_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, avatar_file_remove_cb);
  gtk_widget_class_bind_template_callback (widget_class, avatar_file_chooser_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, avatar_save_to_file_cb);
  gtk_widget_class_bind_template_callback (widget_class, flap_demo_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, tab_view_demo_clicked_cb);
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

  props[PROP_TIMED_ANIMATION] =
    g_param_spec_object ("timed_animation",
                         "Timed animation",
                         "Timed animation",
                         ADW_TYPE_ANIMATION,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
avatar_page_init (AdwDemoWindow *self)
{
  g_autofree char *name = avatar_new_random_name ();

  gtk_editable_set_text (GTK_EDITABLE (self->avatar_text), name);

  avatar_update_contacts (self);

  self->avatar_file_chooser =
    gtk_file_chooser_native_new (_("Select an Avatar"),
                                 GTK_WINDOW (self),
                                 GTK_FILE_CHOOSER_ACTION_OPEN,
                                 _("_Select"),
                                 _("_Cancel"));

  gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (self->avatar_file_chooser), TRUE);
  g_signal_connect_object (self->avatar_file_chooser, "response",
                           G_CALLBACK (avatar_file_chooser_response_cb), self,
                           G_CONNECT_SWAPPED);

  avatar_file_remove_cb (self);
}

static void
animation_page_init (AdwDemoWindow *self)
{
  GtkLayoutManager *manager;
  AdwAnimationTarget *target =
    adw_callback_animation_target_new ((AdwAnimationTargetFunc)
                                       timed_animation_cb,
                                       self->timed_animation_sample, NULL);

  self->timed_animation =
    adw_timed_animation_new (GTK_WIDGET (self->timed_animation_sample),
                             0, 1, 100, target);

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

  adw_timed_animation_set_easing (ADW_TIMED_ANIMATION (self->timed_animation),
                                  ADW_EASE_IN_OUT_CUBIC);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIMED_ANIMATION]);

  manager = gtk_custom_layout_new (NULL, timed_animation_measure,
                                   timed_animation_allocate);

  gtk_widget_set_layout_manager (self->timed_animation_sample, manager);

  gtk_widget_set_direction (self->timed_animation_button_box, GTK_TEXT_DIR_LTR);
}

static void
adw_demo_window_init (AdwDemoWindow *self)
{
  AdwStyleManager *manager = adw_style_manager_get_default ();

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (manager,
                           "notify::system-supports-color-schemes",
                           G_CALLBACK (notify_system_supports_color_schemes_cb),
                           self,
                           G_CONNECT_SWAPPED);

  notify_system_supports_color_schemes_cb (self);

  avatar_page_init (self);

  adw_leaflet_set_visible_child (self->content_box, GTK_WIDGET (self->right_box));

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "toast.dismiss", FALSE);

  animation_page_init (self);
}
