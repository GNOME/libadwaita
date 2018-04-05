#include "example-window.h"
#define HANDY_USE_UNSTABLE_API
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
  GtkLabel *display;
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
  HdyFold fold = hdy_leaflet_get_fold (self->content_box);

  gtk_header_bar_set_show_close_button (self->sub_header_bar, fold == HDY_FOLD_UNFOLDED);
  gtk_widget_set_visible (GTK_WIDGET (self->back), fold == HDY_FOLD_FOLDED);
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
}


static void
example_window_class_init (ExampleWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = example_window_constructed;

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/handy/example/ui/example-window.ui");
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, content_box);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, sub_header_bar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, back);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, sidebar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, stack);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, dialer);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, display);
  gtk_widget_class_bind_template_callback_full (widget_class, "key_pressed_cb", G_CALLBACK(example_window_key_pressed_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_fold_cb", G_CALLBACK(example_window_notify_fold_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "notify_visible_child_cb", G_CALLBACK(example_window_notify_visible_child_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "back_clicked_cb", G_CALLBACK(example_window_back_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "submitted_cb", G_CALLBACK(example_window_submitted_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "symbol_clicked_cb", G_CALLBACK(symbol_clicked_cb));
  gtk_widget_class_bind_template_callback_full (widget_class, "deleted_cb", G_CALLBACK(deleted_cb));
}

static void
example_window_init (ExampleWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  hdy_leaflet_set_visible_child (self->content_box, GTK_WIDGET (self->stack));

  update (self);
}
