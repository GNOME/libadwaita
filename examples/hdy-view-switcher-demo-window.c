#include "hdy-view-switcher-demo-window.h"

#include <glib/gi18n.h>

struct _HdyViewSwitcherDemoWindow
{
  HdyWindow parent_instance;
};

G_DEFINE_TYPE (HdyViewSwitcherDemoWindow, hdy_view_switcher_demo_window, HDY_TYPE_WINDOW)

static void
hdy_view_switcher_demo_window_class_init (HdyViewSwitcherDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/Handy/Demo/ui/hdy-view-switcher-demo-window.ui");
}

static void
hdy_view_switcher_demo_window_init (HdyViewSwitcherDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

HdyViewSwitcherDemoWindow *
hdy_view_switcher_demo_window_new (void)
{
  return g_object_new (HDY_TYPE_VIEW_SWITCHER_DEMO_WINDOW, NULL);
}
