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
  GtkButton *back;
  GtkToggleButton *search_button;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  GtkWidget *box_dialer;
  HdyDialer *dialer;
  GtkLabel *display;
  GtkWidget *arrows;
  HdySearchBar *search_bar;
  GtkEntry *search_entry;
  GtkListBox *arrows_listbox;
  HdyComboRow *arrows_direction_row;
  GtkListBox *column_listbox;
  GtkListBox *lists_listbox;
  HdyComboRow *combo_row;
  HdyComboRow *enum_combo_row;
  HdyHeaderGroup *header_group;
  GtkAdjustment *adj_arrows_count;
  GtkAdjustment *adj_arrows_duration;
};

G_DEFINE_TYPE (HdyDemoWindow, hdy_demo_window, GTK_TYPE_APPLICATION_WINDOW)

static gboolean
hdy_demo_window_key_pressed_cb (GtkWidget     *sender,
                                GdkEventKey   *event,
                                HdyDemoWindow *self)
{
  GdkModifierType default_modifiers = gtk_accelerator_get_default_mod_mask ();
  guint keyval;
  GdkModifierType state;

  gdk_event_get_keyval ((GdkEvent *) event, &keyval);
  gdk_event_get_state ((GdkEvent *) event, &state);

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
  GtkWidget *header_child = hdy_leaflet_get_visible_child (self->header_box);
  HdyFold fold = hdy_leaflet_get_fold (self->header_box);

  g_assert (header_child == NULL || GTK_IS_HEADER_BAR (header_child));

  hdy_header_group_set_focus (self->header_group, fold == HDY_FOLD_FOLDED ? GTK_HEADER_BAR (header_child) : NULL);
}

static void
update_header_bar (HdyDemoWindow *self)
{
  const gchar *visible_child_name;

  visible_child_name = gtk_stack_get_visible_child_name (GTK_STACK (self->stack));
  gtk_widget_set_visible (GTK_WIDGET (self->search_button),
                          g_str_equal (visible_child_name, "search-bar"));
}

static void
hdy_demo_window_notify_header_visible_child_cb (GObject       *sender,
                                                GParamSpec    *pspec,
                                                HdyDemoWindow *self)
{
  update (self);
}

static void
hdy_demo_window_notify_fold_cb (GObject       *sender,
                                GParamSpec    *pspec,
                                HdyDemoWindow *self)
{
  update (self);
}

static void
hdy_demo_window_notify_visible_child_cb (GObject       *sender,
                                         GParamSpec    *pspec,
                                         HdyDemoWindow *self)
{
  hdy_leaflet_set_visible_child_name (self->content_box, "content");
  update_header_bar (self);
}

static void
hdy_demo_window_back_clicked_cb (GtkWidget     *sender,
                                 HdyDemoWindow *self)
{
  hdy_leaflet_set_visible_child_name (self->content_box, "sidebar");
}

static void
hdy_demo_window_submitted_cb (GtkWidget *widget,
                              gchar     *number)
{
  g_print ("Submit %s\n", number);
}


static void
deleted_cb (HdyDialer     *dialer,
            HdyDemoWindow *self)
{
  g_assert (HDY_IS_DIALER (dialer));
  g_assert (HDY_IS_DEMO_WINDOW (self));
  g_print ("Delete btn\n");
}


static void
number_notify_cb (HdyDemoWindow *self,
                  gpointer       unused)
{
  gtk_label_set_label (self->display, hdy_dialer_get_number (self->dialer));
  g_print ("wuff: %s\n", hdy_dialer_get_number (self->dialer));
}


static void
symbol_clicked_cb (HdyDialer     *dialer,
                   gchar          symbol,
                   HdyDemoWindow *self)
{
  g_assert (HDY_IS_DIALER (dialer));
  g_assert (HDY_IS_DEMO_WINDOW (self));
  g_print ("clicked: %c\n", symbol);
}


static void
stack_visible_child_notify_cb (HdyDemoWindow *self,
                               gpointer       unused)
{
  if (gtk_stack_get_visible_child (GTK_STACK (self->stack)) == GTK_WIDGET (self->box_dialer)) {
    gtk_widget_grab_focus (GTK_WIDGET (self->dialer));
  }
}


static gchar *
arrows_direction_name (HdyEnumValueObject *value,
                       gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_ARROWS_DIRECTION_UP:
    return g_strdup (_("Up"));
  case HDY_ARROWS_DIRECTION_DOWN:
    return g_strdup (_("Down"));
  case HDY_ARROWS_DIRECTION_LEFT:
    return g_strdup (_("Left"));
  case HDY_ARROWS_DIRECTION_RIGHT:
    return g_strdup (_("Right"));
  default:
    return NULL;
  }
}


static void
notify_arrows_direction_cb (GObject       *sender,
                            GParamSpec    *pspec,
                            HdyDemoWindow *self)
{
  HdyComboRow *row = HDY_COMBO_ROW (sender);

  g_assert (HDY_IS_COMBO_ROW (row));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  hdy_arrows_set_direction (HDY_ARROWS (self->arrows), hdy_combo_row_get_selected_index (row));
}


static void
adj_arrows_count_value_changed_cb (GtkAdjustment *adj,
                                   HdyDemoWindow *self)
{
  gdouble count;

  g_assert (GTK_IS_ADJUSTMENT (adj));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  count = gtk_adjustment_get_value (adj);
  hdy_arrows_set_count (HDY_ARROWS (self->arrows), count);
}


static void
adj_arrows_duration_value_changed_cb (GtkAdjustment *adj,
                                      HdyDemoWindow *self)
{
  gdouble duration;

  g_assert (GTK_IS_ADJUSTMENT (adj));
  g_assert (HDY_IS_DEMO_WINDOW (self));

  duration = gtk_adjustment_get_value (adj);
  hdy_arrows_set_duration (HDY_ARROWS (self->arrows), duration);
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
view_switcher_demo_clicked_cb (GtkButton     *btn,
                               HdyDemoWindow *self)
{
  HdyViewSwitcherDemoWindow *window = hdy_view_switcher_demo_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (self));
  gtk_widget_show (GTK_WIDGET (window));
}

HdyDemoWindow *
hdy_demo_window_new (GtkApplication *application)
{
  return g_object_new (HDY_TYPE_DEMO_WINDOW, "application", application, NULL);
}


static void
hdy_demo_window_constructed (GObject *object)
{
  HdyDemoWindow *self = HDY_DEMO_WINDOW (object);

  G_OBJECT_CLASS (hdy_demo_window_parent_class)->constructed (object);

  g_signal_connect_swapped (self->dialer,
                            "notify::number",
                            G_CALLBACK (number_notify_cb),
                            self);

  g_signal_connect_swapped (self->stack,
                            "notify::visible-child",
                            G_CALLBACK (stack_visible_child_notify_cb),
                            self);

  gtk_adjustment_set_value (self->adj_arrows_count,
                            hdy_arrows_get_count (HDY_ARROWS (self->arrows)));
  gtk_adjustment_set_value (self->adj_arrows_duration,
                            hdy_arrows_get_duration (HDY_ARROWS (self->arrows)));
  hdy_search_bar_connect_entry (self->search_bar, self->search_entry);
}


static void
hdy_demo_window_class_init (HdyDemoWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = hdy_demo_window_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/handy/demo/ui/hdy-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, header_box);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, back);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_button);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, box_dialer);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, dialer);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, display);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, arrows);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_bar);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, search_entry);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, arrows_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, arrows_direction_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, column_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, lists_listbox);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, combo_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, enum_combo_row);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, header_group);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, adj_arrows_count);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoWindow, adj_arrows_duration);
  gtk_widget_class_bind_template_callback_full (widget_class, "key_pressed_cb", G_CALLBACK(hdy_demo_window_key_pressed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_header_visible_child_cb", G_CALLBACK(hdy_demo_window_notify_header_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_fold_cb", G_CALLBACK(hdy_demo_window_notify_fold_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_visible_child_cb", G_CALLBACK(hdy_demo_window_notify_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "back_clicked_cb", G_CALLBACK(hdy_demo_window_back_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "submitted_cb", G_CALLBACK(hdy_demo_window_submitted_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "symbol_clicked_cb", G_CALLBACK(symbol_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "deleted_cb", G_CALLBACK(deleted_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_arrows_direction_cb", G_CALLBACK(notify_arrows_direction_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "adj_arrows_count_value_changed_cb", G_CALLBACK(adj_arrows_count_value_changed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "adj_arrows_duration_value_changed_cb", G_CALLBACK(adj_arrows_duration_value_changed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "dialog_clicked_cb", G_CALLBACK(dialog_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "dialog_action_clicked_cb", G_CALLBACK(dialog_action_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "view_switcher_demo_clicked_cb", G_CALLBACK(view_switcher_demo_clicked_cb));
}

static void
lists_page_init (HdyDemoWindow *self)
{
  GListStore *list_store;
  HdyValueObject *obj;

  gtk_list_box_set_header_func (self->lists_listbox, hdy_list_box_separator_header, NULL, NULL);

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
}

static void
hdy_demo_window_init (HdyDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_list_box_set_header_func (self->column_listbox, hdy_list_box_separator_header, NULL, NULL);

  gtk_list_box_set_header_func (self->arrows_listbox, hdy_list_box_separator_header, NULL, NULL);
  hdy_combo_row_set_for_enum (self->arrows_direction_row, HDY_TYPE_ARROWS_DIRECTION, arrows_direction_name, NULL, NULL);

  lists_page_init (self);

  hdy_leaflet_set_visible_child_name (self->content_box, "content");
  update_header_bar (self);
}
