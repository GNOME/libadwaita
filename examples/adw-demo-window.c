#include "adw-demo-window.h"

#include <glib/gi18n.h>
#include "adw-flap-demo-window.h"
#include "adw-tab-view-demo-window.h"
#include "adw-view-switcher-demo-window.h"

struct _AdwDemoWindow
{
  AdwApplicationWindow parent_instance;

  AdwLeaflet *content_box;
  GtkBox *right_box;
  GtkWidget *color_scheme_button;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  AdwComboRow *leaflet_transition_row;
  AdwLeaflet *subpage_leaflet;
  GtkListBox *lists_listbox;
  AdwCarousel *carousel;
  GtkBox *carousel_box;
  GtkListBox *carousel_listbox;
  GtkStack *carousel_indicators_stack;
  AdwAvatar *avatar;
  GtkEntry *avatar_text;
  GtkLabel *avatar_file_chooser_label;
  GtkButton *avatar_remove_button;
  GtkFileChooserNative *avatar_file_chooser;
  GtkListBox *avatar_contacts;
};

G_DEFINE_TYPE (AdwDemoWindow, adw_demo_window, ADW_TYPE_APPLICATION_WINDOW)

static char *
get_color_scheme_icon_name (gpointer user_data,
                            gboolean dark)
{
  return g_strdup (dark ? "light-mode-symbolic" : "dark-mode-symbolic");
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

    g_autofree char *path = g_file_get_path (file);

    gdk_texture_save_to_png (texture, path);
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

static void
adw_demo_window_class_init (AdwDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_q, GDK_CONTROL_MASK, "window.close", NULL);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/ui/adw-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, right_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, color_scheme_button);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, leaflet_transition_row);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, subpage_leaflet);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, lists_listbox);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel_box);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel_listbox);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, carousel_indicators_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_text);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_file_chooser_label);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_remove_button);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, avatar_contacts);
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
}
