#include "hdy-demo-window.h"

#include <glib/gi18n.h>
#define HANDY_USE_UNSTABLE_API
#include <handy.h>
#include "hdy-view-switcher-demo-window.h"

struct _HdyDemoWindow
{
  GtkApplicationWindow parent_instance;

  HdyLeaflet *header_box;
  HdyLeaflet *content_box;
  GtkStack *header_stack;
  GtkImage *theme_variant_image;
  GtkButton *back;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  HdyComboRow *leaflet_transition_row;
  GtkWidget *box_keypad;
  GtkListBox *keypad_listbox;
  HdyKeypad *keypad;
  HdySearchBar *search_bar;
  GtkEntry *search_entry;
  GtkListBox *column_listbox;
  GtkListBox *lists_listbox;
  HdyComboRow *combo_row;
  HdyComboRow *enum_combo_row;
  HdyCarousel *carousel;
  GtkListBox *carousel_listbox;
  HdyComboRow *carousel_orientation_row;
  HdyComboRow *carousel_indicator_style_row;
  HdyAvatar *avatar;
  GtkFileChooserButton *avatar_filechooser;
};

G_DEFINE_TYPE (HdyDemoWindow, hdy_demo_window, GTK_TYPE_APPLICATION_WINDOW)

static void
theme_variant_button_clicked_cb (HdyDemoWindow *self)
{
  GtkSettings *settings = gtk_settings_get_default ();
  gboolean prefer_dark_theme;

  g_object_get (settings, "gtk-application-prefer-dark-theme", &prefer_dark_theme, NULL);
  g_object_set (settings, "gtk-application-prefer-dark-theme", !prefer_dark_theme, NULL);
}

static gboolean
prefer_dark_theme_to_icon_name_cb (GBinding     *binding,
                                   const GValue *from_value,
                                   GValue       *to_value,
                                   gpointer      user_data)
{
  g_value_set_string (to_value,
                      g_value_get_boolean (from_value) ? "light-mode-symbolic" :
                                                         "dark-mode-symbolic");

  return TRUE;
}

static gboolean
hdy_demo_window_key_pressed_cb (GtkWidget     *sender,
                                GdkEvent      *event,
                                HdyDemoWindow *self)
{
  GdkModifierType default_modifiers = gtk_accelerator_get_default_mod_mask ();
  guint keyval;
  GdkModifierType state;

  gdk_event_get_keyval (event, &keyval);
  gdk_event_get_state (event, &state);

  if ((keyval == GDK_KEY_q || keyval == GDK_KEY_Q) &&
      (state & default_modifiers) == GDK_CONTROL_MASK) {
    gtk_widget_destroy (GTK_WIDGET (self));

    return TRUE;
  }

  return FALSE;
}

static void
update_leaflet_swipe (HdyDemoWindow *self)
{
  gboolean first_page = (hdy_carousel_get_position (self->carousel) <= 0);
  gboolean carousel_visible =
    (gtk_stack_get_visible_child (self->stack) == GTK_WIDGET (self->carousel));

  hdy_leaflet_set_can_swipe_back (self->content_box,
                                  !carousel_visible || first_page);
}

static void
update (HdyDemoWindow *self)
{
  const gchar *header_bar_name = "default";

  if (g_strcmp0 (gtk_stack_get_visible_child_name (self->stack), "search-bar") == 0) {
    header_bar_name = "search-bar";
  }

  gtk_stack_set_visible_child_name (self->header_stack, header_bar_name);
}

static void
hdy_demo_window_notify_visible_child_cb (GObject       *sender,
                                         GParamSpec    *pspec,
                                         HdyDemoWindow *self)
{
  update (self);

  hdy_leaflet_navigate (self->content_box, HDY_NAVIGATION_DIRECTION_FORWARD);
  update_leaflet_swipe (self);
}

static void
hdy_demo_window_back_clicked_cb (GtkWidget     *sender,
                                 HdyDemoWindow *self)
{
  hdy_leaflet_navigate (self->content_box, HDY_NAVIGATION_DIRECTION_BACK);
}

static gchar *
leaflet_transition_name (HdyEnumValueObject *value,
                         gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_LEAFLET_TRANSITION_TYPE_NONE:
    return g_strdup (_("None"));
  case HDY_LEAFLET_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
  case HDY_LEAFLET_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case HDY_LEAFLET_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  default:
    return NULL;
  }
}

static void
notify_leaflet_transition_cb (GObject       *sender,
                              GParamSpec    *pspec,
                              HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  hdy_leaflet_set_transition_type (HDY_LEAFLET (self->content_box), hdy_combo_row_get_selected_index (row));
}

static void
dialog_close_cb (GtkDialog *self)
{
  gtk_widget_destroy (GTK_WIDGET (self));
}

static void
dialog_clicked_cb (GtkButton     *btn,
                   HdyDemoWindow *self)
{
  GtkWidget *dlg;
  GtkWidget *lbl;

  dlg = hdy_dialog_new (GTK_WINDOW (self));
  gtk_window_set_title (GTK_WINDOW (dlg), "HdyDialog");
  lbl = gtk_label_new ("Hello, World!");
  g_object_set (lbl, "margin", 12, NULL);
  gtk_widget_set_vexpand (lbl, TRUE);
  gtk_widget_set_valign (lbl, GTK_ALIGN_CENTER);
  gtk_widget_set_halign (lbl, GTK_ALIGN_CENTER);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
                     lbl);

  gtk_widget_show (lbl);
  gtk_widget_show (dlg);
}

static void
dialog_action_clicked_cb (GtkButton     *btn,
                          HdyDemoWindow *self)
{
  GtkWidget *dlg;
  GtkWidget *lbl;

  dlg = hdy_dialog_new (GTK_WINDOW (self));
  gtk_window_set_title (GTK_WINDOW (dlg), "HdyDialog");
  gtk_dialog_add_buttons (GTK_DIALOG (dlg),
                          "Done", GTK_RESPONSE_ACCEPT,
                          "Cancel", GTK_RESPONSE_CANCEL,
                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_ACCEPT);
  g_signal_connect (G_OBJECT (dlg), "response", G_CALLBACK (dialog_close_cb), NULL);
  lbl = gtk_label_new ("Hello, World!");
  g_object_set (lbl, "margin", 12, NULL);
  gtk_widget_set_vexpand (lbl, TRUE);
  gtk_widget_set_valign (lbl, GTK_ALIGN_CENTER);
  gtk_widget_set_halign (lbl, GTK_ALIGN_CENTER);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dlg))),
                     lbl);

  gtk_widget_show (lbl);
  gtk_widget_show (dlg);
}

static void
dialog_complex_deeper_clicked_cb (HdyDeck *deck)
{
  hdy_deck_set_visible_child_name (deck, "sub");
}

static void
dialog_complex_back_clicked_cb (HdyDeck *deck)
{
  hdy_deck_navigate (deck, HDY_NAVIGATION_DIRECTION_BACK);
}

static void
dialog_complex_clicked_cb (GtkButton     *btn,
                           HdyDemoWindow *self)
{
  g_autoptr (GtkBuilder) builder = gtk_builder_new_from_resource ("/sm/puri/Handy/Demo/ui/hdy-dialog-complex-example.ui");
  GtkWidget *dlg, *back, *deeper, *deck;

  dlg = GTK_WIDGET (gtk_builder_get_object (builder, "dialog"));
  back = GTK_WIDGET (gtk_builder_get_object (builder, "back"));
  deeper = GTK_WIDGET (gtk_builder_get_object (builder, "deeper"));
  deck = GTK_WIDGET (gtk_builder_get_object (builder, "content_deck"));
  g_signal_connect_swapped (deeper, "clicked", G_CALLBACK (dialog_complex_deeper_clicked_cb), deck);
  g_signal_connect_swapped (back, "clicked", G_CALLBACK (dialog_complex_back_clicked_cb), deck);
  gtk_window_set_transient_for (GTK_WINDOW (dlg), GTK_WINDOW (self));

  gtk_widget_show (dlg);
}

static void
view_switcher_demo_clicked_cb (GtkButton     *btn,
                               HdyDemoWindow *self)
{
  HdyViewSwitcherDemoWindow *window = hdy_view_switcher_demo_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (self));
  gtk_widget_show (GTK_WIDGET (window));
}

static gchar *
carousel_orientation_name (HdyEnumValueObject *value,
                           gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case GTK_ORIENTATION_HORIZONTAL:
    return g_strdup (_("Horizontal"));
  case GTK_ORIENTATION_VERTICAL:
    return g_strdup (_("Vertical"));
  default:
    return NULL;
  }
}

static void
notify_carousel_position_cb (GObject       *sender,
                             GParamSpec    *pspec,
                             HdyDemoWindow *self)
{
  update_leaflet_swipe (self);
}

static void
notify_carousel_orientation_cb (GObject       *sender,
                                GParamSpec    *pspec,
                                HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);
  gboolean horizontal;

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  horizontal = (hdy_combo_row_get_selected_index (row) == GTK_ORIENTATION_HORIZONTAL);
  g_object_set (self->carousel,
                "orientation", hdy_combo_row_get_selected_index (row),
                "margin-top", horizontal ? 6 : 0,
                "margin-bottom", horizontal ? 6 : 0,
                "margin-left", horizontal ? 0 : 6,
                "margin-right", horizontal ? 0 : 6,
                NULL);
}

static gchar *
carousel_indicator_style_name (HdyEnumValueObject *value,
                               gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_CAROUSEL_INDICATOR_STYLE_NONE:
    return g_strdup (_("None"));
  case HDY_CAROUSEL_INDICATOR_STYLE_DOTS:
    return g_strdup (_("Dots"));
  case HDY_CAROUSEL_INDICATOR_STYLE_LINES:
    return g_strdup (_("Lines"));
  default:
    return NULL;
  }
}

static void
notify_carousel_indicator_style_cb (GObject       *sender,
                                    GParamSpec    *pspec,
                                    HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  hdy_carousel_set_indicator_style (self->carousel, hdy_combo_row_get_selected_index (row));
}

static void
carousel_return_clicked_cb (GtkButton     *btn,
                            HdyDemoWindow *self)
{
  g_autoptr (GList) children;

  children = gtk_container_get_children (GTK_CONTAINER (self->carousel));
  hdy_carousel_scroll_to (self->carousel, GTK_WIDGET (children->data));
}

HdyDemoWindow *
hdy_demo_window_new (GtkApplication *application)
{
  return g_object_new (HDY_TYPE_DEMO_WINDOW, "application", application, NULL);
}

static void
avatar_file_remove_cb (HdyDemoWindow *self)
{
  g_autofree gchar *file = NULL;

  g_assert (HDY_IS_DEMO_WINDOW (self));

  g_signal_handlers_disconnect_by_data (self->avatar, self);
  file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (self->avatar_filechooser));
  if (file)
    gtk_file_chooser_unselect_filename (GTK_FILE_CHOOSER (self->avatar_filechooser), file);
  hdy_avatar_set_image_load_func (self->avatar, NULL, NULL, NULL);
}

static GdkPixbuf *
avatar_load_file (gint size, HdyDemoWindow *self)
{
  g_autoptr (GError) error = NULL;
  g_autoptr (GdkPixbuf) pixbuf = NULL;
  g_autofree gchar *file = NULL;
  gint width, height;

  g_assert (HDY_IS_DEMO_WINDOW (self));

  file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (self->avatar_filechooser));

  gdk_pixbuf_get_file_info (file, &width, &height);

  pixbuf = gdk_pixbuf_new_from_file_at_scale (file,
                                            (width <= height) ? size : -1,
                                            (width >= height) ? size : -1,
                                            TRUE,
                                            &error);
  if (error != NULL) {
    g_critical ("Failed to create pixbuf from file: %s", error->message);

    return NULL;
  }

  return g_steal_pointer (&pixbuf);
}

static void
avatar_file_set_cb (HdyDemoWindow *self)
{
  g_assert (HDY_IS_DEMO_WINDOW (self));

  hdy_avatar_set_image_load_func (self->avatar, (HdyAvatarImageLoadFunc) avatar_load_file, self, NULL);
}

static void
hdy_demo_window_constructed (GObject *object)
{
  HdyDemoWindow *self = HDY_DEMO_WINDOW (object);

  G_OBJECT_CLASS (hdy_demo_window_parent_class)->constructed (object);

  hdy_search_bar_connect_entry (self->search_bar, self->search_entry);
}


static void
hdy_demo_window_class_init (HdyDemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = hdy_demo_window_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/Handy/Demo/ui/hdy-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, header_box);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, header_stack);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, theme_variant_image);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, back);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, leaflet_transition_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, box_keypad);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, keypad_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, keypad);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_bar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_entry);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, column_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, lists_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, combo_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, enum_combo_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_orientation_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_indicator_style_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, avatar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, avatar_filechooser);
  gtk_widget_class_bind_template_callback_full (widget_class, "key_pressed_cb", G_CALLBACK(hdy_demo_window_key_pressed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_visible_child_cb", G_CALLBACK(hdy_demo_window_notify_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "back_clicked_cb", G_CALLBACK(hdy_demo_window_back_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_leaflet_transition_cb", G_CALLBACK(notify_leaflet_transition_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "theme_variant_button_clicked_cb", G_CALLBACK(theme_variant_button_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "dialog_clicked_cb", G_CALLBACK(dialog_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "dialog_action_clicked_cb", G_CALLBACK(dialog_action_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "dialog_complex_clicked_cb", G_CALLBACK(dialog_complex_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "view_switcher_demo_clicked_cb", G_CALLBACK(view_switcher_demo_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_carousel_position_cb", G_CALLBACK(notify_carousel_position_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_carousel_orientation_cb", G_CALLBACK(notify_carousel_orientation_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_carousel_indicator_style_cb", G_CALLBACK(notify_carousel_indicator_style_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "carousel_return_clicked_cb", G_CALLBACK(carousel_return_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "avatar_file_remove_cb", G_CALLBACK(avatar_file_remove_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "avatar_file_set_cb", G_CALLBACK(avatar_file_set_cb));
}

static void
lists_page_init (HdyDemoWindow *self)
{
  GListStore *list_store;
  HdyValueObject *obj;

  list_store = g_list_store_new (HDY_TYPE_VALUE_OBJECT);

  obj = hdy_value_object_new_string ("Foo");
  g_list_store_insert (list_store, 0, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("Bar");
  g_list_store_insert (list_store, 1, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("Baz");
  g_list_store_insert (list_store, 2, obj);
  g_clear_object (&obj);

  hdy_combo_row_bind_name_model (self->combo_row, G_LIST_MODEL (list_store), (HdyComboRowGetNameFunc) hdy_value_object_dup_string, NULL, NULL);

  hdy_combo_row_set_for_enum (self->enum_combo_row, GTK_TYPE_LICENSE, hdy_enum_value_row_name, NULL, NULL);
  update (self);
}

static void
hdy_demo_window_init (HdyDemoWindow *self)
{
  GtkSettings *settings = gtk_settings_get_default ();

  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_bind_property_full (settings, "gtk-application-prefer-dark-theme",
                               self->theme_variant_image, "icon-name",
                               G_BINDING_SYNC_CREATE,
                               prefer_dark_theme_to_icon_name_cb,
                               NULL,
                               NULL,
                               NULL);

  hdy_combo_row_set_for_enum (self->leaflet_transition_row, HDY_TYPE_LEAFLET_TRANSITION_TYPE, leaflet_transition_name, NULL, NULL);
  hdy_combo_row_set_selected_index (self->leaflet_transition_row, HDY_LEAFLET_TRANSITION_TYPE_OVER);

  lists_page_init (self);

  hdy_combo_row_set_for_enum (self->carousel_orientation_row, GTK_TYPE_ORIENTATION, carousel_orientation_name, NULL, NULL);
  hdy_combo_row_set_for_enum (self->carousel_indicator_style_row, HDY_TYPE_CAROUSEL_INDICATOR_STYLE, carousel_indicator_style_name, NULL, NULL);
  hdy_combo_row_set_selected_index (self->carousel_indicator_style_row, HDY_CAROUSEL_INDICATOR_STYLE_DOTS);

  hdy_leaflet_set_visible_child_name (self->content_box, "content");
}
