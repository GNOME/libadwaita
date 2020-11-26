#include "hdy-demo-window.h"

#include <glib/gi18n.h>
#include "hdy-view-switcher-demo-window.h"

struct _HdyDemoWindow
{
  HdyApplicationWindow parent_instance;

  HdyLeaflet *content_box;
  GtkStack *header_revealer;
  GtkStack *header_stack;
  GtkImage *theme_variant_image;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  HdyComboRow *leaflet_transition_row;
  HdyDeck *content_deck;
  HdyComboRow *deck_transition_row;
  GtkWidget *box_keypad;
  GtkListBox *keypad_listbox;
  HdyKeypad *keypad;
  HdySearchBar *search_bar;
  GtkEntry *search_entry;
  GtkListBox *lists_listbox;
  HdyComboRow *combo_row;
  HdyComboRow *enum_combo_row;
  HdyCarousel *carousel;
  GtkBox *carousel_box;
  GtkListBox *carousel_listbox;
  GtkStack *carousel_indicators_stack;
  HdyComboRow *carousel_orientation_row;
  HdyComboRow *carousel_indicators_row;
  GListStore *carousel_indicators_model;
  HdyAvatar *avatar;
  GtkEntry *avatar_text;
  GtkFileChooserButton *avatar_filechooser;
  GtkListBox *avatar_contacts;
};

G_DEFINE_TYPE (HdyDemoWindow, hdy_demo_window, HDY_TYPE_APPLICATION_WINDOW)

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
key_pressed_cb (GtkWidget     *sender,
                GdkEvent      *event,
                HdyDemoWindow *self)
{
  GdkModifierType default_modifiers = gtk_accelerator_get_default_mod_mask ();
  guint keyval;
  GdkModifierType state;
  GdkKeymap *keymap;
  GdkEventKey *key_event = (GdkEventKey *) event;

  gdk_event_get_state (event, &state);

  keymap = gdk_keymap_get_for_display (gtk_widget_get_display (sender));

  gdk_keymap_translate_keyboard_state (keymap,
                                       key_event->hardware_keycode,
                                       state,
                                       key_event->group,
                                       &keyval, NULL, NULL, NULL);

  if ((keyval == GDK_KEY_q || keyval == GDK_KEY_Q) &&
      (state & default_modifiers) == GDK_CONTROL_MASK) {
    gtk_widget_destroy (GTK_WIDGET (self));

    return TRUE;
  }

  return FALSE;
}

static void
update (HdyDemoWindow *self)
{
  const gchar *header_bar_name = "default";

  if (g_strcmp0 (gtk_stack_get_visible_child_name (self->stack), "deck") == 0)
    header_bar_name = "deck";
  else if (g_strcmp0 (gtk_stack_get_visible_child_name (self->stack), "search-bar") == 0)
    header_bar_name = "search-bar";

  gtk_stack_set_visible_child_name (self->header_stack, header_bar_name);
}

static void
notify_deck_visible_child_cb (HdyDemoWindow *self)
{
  update (self);
}

static void
notify_visible_child_cb (GObject       *sender,
                         GParamSpec    *pspec,
                         HdyDemoWindow *self)
{
  update (self);

  hdy_leaflet_navigate (self->content_box, HDY_NAVIGATION_DIRECTION_FORWARD);
}

static void
back_clicked_cb (GtkWidget     *sender,
                 HdyDemoWindow *self)
{
  hdy_leaflet_navigate (self->content_box, HDY_NAVIGATION_DIRECTION_BACK);
}

static void
deck_back_clicked_cb (GtkWidget     *sender,
                      HdyDemoWindow *self)
{
  hdy_deck_navigate (self->content_deck, HDY_NAVIGATION_DIRECTION_BACK);
}

static gchar *
leaflet_transition_name (HdyEnumValueObject *value,
                         gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_LEAFLET_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case HDY_LEAFLET_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  case HDY_LEAFLET_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
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

static gchar *
deck_transition_name (HdyEnumValueObject *value,
                      gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_DECK_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case HDY_DECK_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  case HDY_DECK_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
  default:
    return NULL;
  }
}

static void
notify_deck_transition_cb (GObject       *sender,
                           GParamSpec    *pspec,
                           HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  hdy_deck_set_transition_type (HDY_DECK (self->content_deck), hdy_combo_row_get_selected_index (row));
}

static void
deck_go_next_row_activated_cb (HdyDemoWindow *self)
{
  g_assert (HDY_IS_DEMO_WINDOW (self));

  hdy_deck_navigate (self->content_deck, HDY_NAVIGATION_DIRECTION_FORWARD);
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
notify_carousel_orientation_cb (GObject       *sender,
                                GParamSpec    *pspec,
                                HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->carousel_box),
                                  1 - hdy_combo_row_get_selected_index (row));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->carousel),
                                  hdy_combo_row_get_selected_index (row));
}

static gchar *
carousel_indicators_name (HdyValueObject *value)
{
  const gchar *style;

  g_assert (HDY_IS_VALUE_OBJECT (value));

  style = hdy_value_object_get_string (value);

  if (!g_strcmp0 (style, "dots"))
    return g_strdup (_("Dots"));

  if (!g_strcmp0 (style, "lines"))
    return g_strdup (_("Lines"));

  return NULL;
}

static void
notify_carousel_indicators_cb (GObject       *sender,
                               GParamSpec    *pspec,
                               HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);
  HdyValueObject *obj;

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  obj = g_list_model_get_item (G_LIST_MODEL (self->carousel_indicators_model),
                               hdy_combo_row_get_selected_index (row));

  gtk_stack_set_visible_child_name (self->carousel_indicators_stack,
                                    hdy_value_object_get_string (obj));
}

static void
carousel_return_clicked_cb (GtkButton     *btn,
                            HdyDemoWindow *self)
{
  g_autoptr (GList) children =
    gtk_container_get_children (GTK_CONTAINER (self->carousel));

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

static gchar *
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
avatar_update_contacts (HdyDemoWindow *self)
{
  g_autoptr (GList) children = gtk_container_get_children (GTK_CONTAINER (self->avatar_contacts));

  for (GList *child = children; child; child = child->next)
    gtk_container_remove (GTK_CONTAINER (self->avatar_contacts), child->data);

  for (int i = 0; i < 30; i++) {
    g_autofree gchar *name = avatar_new_random_name ();
    GtkWidget *contact = hdy_action_row_new ();
    GtkWidget *avatar = hdy_avatar_new (40, name, TRUE);

    gtk_widget_show (contact);
    gtk_widget_show (avatar);

    gtk_widget_set_margin_top (avatar, 12);
    gtk_widget_set_margin_bottom (avatar, 12);

    hdy_preferences_row_set_title (HDY_PREFERENCES_ROW (contact), name);
    hdy_action_row_add_prefix (HDY_ACTION_ROW (contact), avatar);
    gtk_container_add (GTK_CONTAINER (self->avatar_contacts), contact);
  }
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
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, header_revealer);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, header_stack);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, theme_variant_image);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, leaflet_transition_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, content_deck);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, deck_transition_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, box_keypad);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, keypad_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, keypad);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_bar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_entry);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, lists_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, combo_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, enum_combo_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_box);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_indicators_stack);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_orientation_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, carousel_indicators_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, avatar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, avatar_text);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, avatar_filechooser);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, avatar_contacts);
  gtk_widget_class_bind_template_callback (widget_class, key_pressed_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_deck_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, back_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, deck_back_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_leaflet_transition_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_deck_transition_cb);
  gtk_widget_class_bind_template_callback (widget_class, deck_go_next_row_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, theme_variant_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, view_switcher_demo_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_carousel_orientation_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_carousel_indicators_cb);
  gtk_widget_class_bind_template_callback (widget_class, carousel_return_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, avatar_file_remove_cb);
  gtk_widget_class_bind_template_callback (widget_class, avatar_file_set_cb);
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
carousel_page_init (HdyDemoWindow *self)
{
  HdyValueObject *obj;

  hdy_combo_row_set_for_enum (self->carousel_orientation_row,
                              GTK_TYPE_ORIENTATION,
                              carousel_orientation_name,
                              NULL,
                              NULL);

  self->carousel_indicators_model = g_list_store_new (HDY_TYPE_VALUE_OBJECT);

  obj = hdy_value_object_new_string ("dots");
  g_list_store_insert (self->carousel_indicators_model, 0, obj);
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("lines");
  g_list_store_insert (self->carousel_indicators_model, 1, obj);
  g_clear_object (&obj);

  hdy_combo_row_bind_name_model (self->carousel_indicators_row,
                                 G_LIST_MODEL (self->carousel_indicators_model),
                                 (HdyComboRowGetNameFunc) carousel_indicators_name,
                                 NULL,
                                 NULL);
}

static void
avatar_page_init (HdyDemoWindow *self)
{
  g_autofree gchar *name = avatar_new_random_name ();

  gtk_entry_set_text (self->avatar_text, name);

  avatar_update_contacts (self);
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

  hdy_combo_row_set_for_enum (self->deck_transition_row, HDY_TYPE_DECK_TRANSITION_TYPE, deck_transition_name, NULL, NULL);
  hdy_combo_row_set_selected_index (self->deck_transition_row, HDY_DECK_TRANSITION_TYPE_OVER);

  lists_page_init (self);
  carousel_page_init (self);
  avatar_page_init (self);

  hdy_leaflet_set_visible_child_name (self->content_box, "content");
}
