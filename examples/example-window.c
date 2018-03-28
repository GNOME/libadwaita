#include "example-window.h"
#include <handy.h>

struct _ExampleWindow
{
  GtkApplicationWindow parent_instance;

  HdyLeaflet *content_box;
  GtkHeaderBar *sub_header_bar;
  GtkButton *back;
  GtkStackSidebar *sidebar;
  GtkStack *stack;
  HdyDialer *dialer;
};

G_DEFINE_TYPE (ExampleWindow, example_window, GTK_TYPE_APPLICATION_WINDOW)

static gboolean
example_window_key_pressed_cb (GtkWidget     *sender,
                               GdkEventKey   *event,
                               ExampleWindow *self)
{
  GdkModifierType default_modifiers = gtk_accelerator_get_default_mod_mask ();

  if ((event->keyval == GDK_KEY_q || event->keyval == GDK_KEY_Q) &&
      (event->state & default_modifiers) == GDK_CONTROL_MASK) {
    gtk_widget_destroy (GTK_WIDGET (self));

    return TRUE;
  }

  return FALSE;
}

static void
update (ExampleWindow *self)
{
  gboolean folded = hdy_leaflet_get_folded (self->content_box);

  gtk_header_bar_set_show_close_button (self->sub_header_bar, !folded);
  gtk_widget_set_visible (GTK_WIDGET (self->back), folded);
}

static void
example_window_notify_folded_cb (GObject       *sender,
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
  hdy_leaflet_set_visible_child (self->content_box, GTK_WIDGET (self->stack));
}

static void
example_window_back_clicked_cb (GtkWidget     *sender,
                                ExampleWindow *self)
{
  hdy_leaflet_set_visible_child (self->content_box, GTK_WIDGET (self->sidebar));
}

static void
example_window_submitted_cb (GtkWidget *widget,
                          gchar     *number)
{
  g_print ("Dial %s\n", number);
}


static void
example_window_notify_number_cb (GtkWidget *widget)
{
  g_print ("wuff: %s\n", hdy_dialer_get_number (HDY_DIALER (widget)));
}

ExampleWindow *
example_window_new (GtkApplication *application)
{
  return g_object_new (EXAMPLE_TYPE_WINDOW, "application", application, NULL);
}

static void
example_window_class_init (ExampleWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/handy/example/ui/example-window.ui");
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, sub_header_bar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, back);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, dialer);
  gtk_widget_class_bind_template_callback_full (widget_class, "key_pressed_cb", G_CALLBACK(example_window_key_pressed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_folded_cb", G_CALLBACK(example_window_notify_folded_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_visible_child_cb", G_CALLBACK(example_window_notify_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "back_clicked_cb", G_CALLBACK(example_window_back_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "submitted_cb", G_CALLBACK(example_window_submitted_cb));
}

static void
example_window_init (ExampleWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  hdy_leaflet_set_visible_child (self->content_box, GTK_WIDGET (self->stack));

  update (self);
}
