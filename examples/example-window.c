#include "example-window.h"
#define HANDY_USE_UNSTABLE_API
#include <handy.h>

struct _ExampleWindow
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
  GtkListBox *column_listbox;
  HdyHeaderGroup *header_group;
  GtkAdjustment *adj_arrows_count;
  GtkAdjustment *adj_arrows_duration;
};

G_DEFINE_TYPE (ExampleWindow, example_window, GTK_TYPE_APPLICATION_WINDOW)

static gboolean
example_window_key_pressed_cb (GtkWidget     *sender,
                               GdkEventKey   *event,
                               ExampleWindow *self)
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
update (ExampleWindow *self)
{
  GtkWidget *header_child = hdy_leaflet_get_visible_child (self->header_box);
  HdyFold fold = hdy_leaflet_get_fold (self->header_box);

  g_assert (header_child == NULL || GTK_IS_HEADER_BAR (header_child));

  hdy_header_group_set_focus (self->header_group, fold == HDY_FOLD_FOLDED ? GTK_HEADER_BAR (header_child) : NULL);
}

static void
update_header_bar (ExampleWindow *self)
{
  const gchar *visible_child_name;

  visible_child_name = gtk_stack_get_visible_child_name (GTK_STACK (self->stack));
  gtk_widget_set_visible (GTK_WIDGET (self->search_button),
                          g_str_equal (visible_child_name, "search-bar"));
}

static void
example_window_notify_header_visible_child_cb (GObject       *sender,
                                               GParamSpec    *pspec,
                                               ExampleWindow *self)
{
  update (self);
}

static void
example_window_notify_fold_cb (GObject       *sender,
                                 GParamSpec    *pspec,
                                 ExampleWindow *self)
{
  update (self);
}

static void
example_window_notify_visible_child_cb (GObject       *sender,
                                        GParamSpec    *pspec,
                                        ExampleWindow *self)
{
  hdy_leaflet_set_visible_child_name (self->content_box, "content");
  update_header_bar (self);
}

static void
example_window_back_clicked_cb (GtkWidget     *sender,
                                ExampleWindow *self)
{
  hdy_leaflet_set_visible_child_name (self->content_box, "sidebar");
}

static void
example_window_submitted_cb (GtkWidget *widget,
                          gchar     *number)
{
  g_print ("Submit %s\n", number);
}


static void
deleted_cb (HdyDialer *dialer,
            ExampleWindow *self)
{
  g_assert (HDY_IS_DIALER (dialer));
  g_assert (EXAMPLE_IS_WINDOW (self));
  g_print ("Delete btn\n");
}


static void
number_notify_cb (ExampleWindow *self,
                  gpointer unused)
{
  gtk_label_set_label (self->display, hdy_dialer_get_number (self->dialer));
  g_print ("wuff: %s\n", hdy_dialer_get_number (self->dialer));
}


static void
symbol_clicked_cb (HdyDialer *dialer,
                   gchar symbol,
                   ExampleWindow *self)
{
  g_assert (HDY_IS_DIALER (dialer));
  g_assert (EXAMPLE_IS_WINDOW (self));
  g_print ("clicked: %c\n", symbol);
}


static void
stack_visible_child_notify_cb (ExampleWindow *self,
                               gpointer       unused)
{
  if (gtk_stack_get_visible_child (GTK_STACK (self->stack)) == GTK_WIDGET (self->box_dialer)) {
    gtk_widget_grab_focus (GTK_WIDGET (self->dialer));
  }
}


static void
btn_arrows_up_toggled_cb (GtkToggleButton *btn,
                          ExampleWindow *self)
{
  g_assert (GTK_IS_TOGGLE_BUTTON (btn));
  g_assert (EXAMPLE_IS_WINDOW (self));

  hdy_arrows_set_direction (HDY_ARROWS (self->arrows), HDY_ARROWS_DIRECTION_UP);
  hdy_arrows_animate (HDY_ARROWS (self->arrows));
}


static void
btn_arrows_down_toggled_cb (GtkToggleButton *btn,
                          ExampleWindow *self)
{
  g_assert (GTK_IS_TOGGLE_BUTTON (btn));
  g_assert (EXAMPLE_IS_WINDOW (self));

  hdy_arrows_set_direction (HDY_ARROWS (self->arrows), HDY_ARROWS_DIRECTION_DOWN);
  hdy_arrows_animate (HDY_ARROWS (self->arrows));
}


static void
btn_arrows_left_toggled_cb (GtkToggleButton *btn,
                          ExampleWindow *self)
{
  g_assert (GTK_IS_TOGGLE_BUTTON (btn));
  g_assert (EXAMPLE_IS_WINDOW (self));

  hdy_arrows_set_direction (HDY_ARROWS (self->arrows), HDY_ARROWS_DIRECTION_LEFT);
  hdy_arrows_animate (HDY_ARROWS (self->arrows));
}


static void
btn_arrows_right_toggled_cb (GtkToggleButton *btn,
                          ExampleWindow *self)
{
  g_assert (GTK_IS_TOGGLE_BUTTON (btn));
  g_assert (EXAMPLE_IS_WINDOW (self));

  hdy_arrows_set_direction (HDY_ARROWS (self->arrows), HDY_ARROWS_DIRECTION_RIGHT);
  hdy_arrows_animate (HDY_ARROWS (self->arrows));
}


static void
adj_arrows_count_value_changed_cb (GtkAdjustment *adj,
                                   ExampleWindow *self)
{
  gdouble count;

  g_assert (GTK_IS_ADJUSTMENT (adj));
  g_assert (EXAMPLE_IS_WINDOW (self));

  count = gtk_adjustment_get_value (adj);
  hdy_arrows_set_count (HDY_ARROWS (self->arrows), count);
  hdy_arrows_animate (HDY_ARROWS (self->arrows));
}


static void
adj_arrows_duration_value_changed_cb (GtkAdjustment *adj,
                                      ExampleWindow *self)
{
  gdouble duration;

  g_assert (GTK_IS_ADJUSTMENT (adj));
  g_assert (EXAMPLE_IS_WINDOW (self));

  duration = gtk_adjustment_get_value (adj);
  hdy_arrows_set_duration (HDY_ARROWS (self->arrows), duration);
  hdy_arrows_animate (HDY_ARROWS (self->arrows));
}


ExampleWindow *
example_window_new (GtkApplication *application)
{
  return g_object_new (EXAMPLE_TYPE_WINDOW, "application", application, NULL);
}


static void
example_window_constructed (GObject *object)
{
  ExampleWindow *self = EXAMPLE_WINDOW (object);

  G_OBJECT_CLASS (example_window_parent_class)->constructed (object);

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
example_window_class_init (ExampleWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = example_window_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/handy/example/ui/example-window.ui");
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, header_box);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, back);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, search_button);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, box_dialer);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, dialer);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, display);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, arrows);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, search_bar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, search_entry);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, column_listbox);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, header_group);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, adj_arrows_count);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, adj_arrows_duration);
  gtk_widget_class_bind_template_callback_full (widget_class, "key_pressed_cb", G_CALLBACK(example_window_key_pressed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_header_visible_child_cb", G_CALLBACK(example_window_notify_header_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_fold_cb", G_CALLBACK(example_window_notify_fold_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_visible_child_cb", G_CALLBACK(example_window_notify_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "back_clicked_cb", G_CALLBACK(example_window_back_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "submitted_cb", G_CALLBACK(example_window_submitted_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "symbol_clicked_cb", G_CALLBACK(symbol_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "deleted_cb", G_CALLBACK(deleted_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "btn_arrows_up_toggled_cb", G_CALLBACK(btn_arrows_up_toggled_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "btn_arrows_down_toggled_cb", G_CALLBACK(btn_arrows_down_toggled_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "btn_arrows_left_toggled_cb", G_CALLBACK(btn_arrows_left_toggled_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "btn_arrows_right_toggled_cb", G_CALLBACK(btn_arrows_right_toggled_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "adj_arrows_count_value_changed_cb", G_CALLBACK(adj_arrows_count_value_changed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "adj_arrows_duration_value_changed_cb", G_CALLBACK(adj_arrows_duration_value_changed_cb));
}

static void
example_window_init (ExampleWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_list_box_set_header_func (self->column_listbox, hdy_list_box_separator_header, NULL, NULL);

  hdy_leaflet_set_visible_child_name (self->content_box, "content");
  update_header_bar (self);
}
