#include "hdy-demo-preferences-window.h"

struct _HdyDemoPreferencesWindow
{
  HdyPreferencesWindow parent_instance;

  GtkWidget *subpage1;
  GtkWidget *subpage2;
};

G_DEFINE_TYPE (HdyDemoPreferencesWindow, hdy_demo_preferences_window, HDY_TYPE_PREFERENCES_WINDOW)

HdyDemoPreferencesWindow *
hdy_demo_preferences_window_new (void)
{
  return g_object_new (HDY_TYPE_DEMO_PREFERENCES_WINDOW, NULL);
}

static void
return_to_preferences_cb (HdyDemoPreferencesWindow *self)
{
  hdy_preferences_window_close_subpage (HDY_PREFERENCES_WINDOW (self));
}

static void
subpage1_activated_cb (HdyDemoPreferencesWindow *self)
{
  hdy_preferences_window_present_subpage (HDY_PREFERENCES_WINDOW (self), self->subpage1);
}

static void
subpage2_activated_cb (HdyDemoPreferencesWindow *self)
{
  hdy_preferences_window_present_subpage (HDY_PREFERENCES_WINDOW (self), self->subpage2);
}

static void
hdy_demo_preferences_window_class_init (HdyDemoPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/Handy/Demo/ui/hdy-demo-preferences-window.ui");

  gtk_widget_class_bind_template_child (widget_class, HdyDemoPreferencesWindow, subpage1);
  gtk_widget_class_bind_template_child (widget_class, HdyDemoPreferencesWindow, subpage2);

  gtk_widget_class_bind_template_callback (widget_class, return_to_preferences_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpage1_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpage2_activated_cb);
}

static void
hdy_demo_preferences_window_init (HdyDemoPreferencesWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
