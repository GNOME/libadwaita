#include "hdy-view-switcher-demo-window.h"

#include <glib/gi18n.h>
#define HANDY_USE_UNSTABLE_API
#include <handy.h>

struct _HdyViewSwitcherDemoWindow
{
  GtkWindow parent_instance;

  HdySqueezer *squeezer;
  HdyViewSwitcherBar *switcher_bar;
  GtkLabel *title_label;
  HdyViewSwitcher *title_narrow_switcher;
  HdyViewSwitcher *title_wide_switcher;
};

G_DEFINE_TYPE (HdyViewSwitcherDemoWindow, hdy_view_switcher_demo_window, GTK_TYPE_WINDOW)

static gboolean
is_title_label_visible (GBinding     *binding,
                        const GValue *from_value,
                        GValue       *to_value,
                        gpointer      user_data)
{
  g_value_set_boolean (to_value, g_value_get_object (from_value) == user_data);

  return TRUE;
}

static void
hdy_view_switcher_demo_window_size_allocate (GtkWidget     *widget,
                                             GtkAllocation *allocation)
{
  HdyViewSwitcherDemoWindow *self = HDY_VIEW_SWITCHER_DEMO_WINDOW (widget);
  hdy_squeezer_set_child_enabled (self->squeezer,
                                  GTK_WIDGET (self->title_wide_switcher),
                                  allocation->width > 600);
  hdy_squeezer_set_child_enabled (self->squeezer,
                                  GTK_WIDGET (self->title_narrow_switcher),
                                  allocation->width > 400);

  GTK_WIDGET_CLASS (hdy_view_switcher_demo_window_parent_class)->size_allocate (widget, allocation);
}

static void
hdy_view_switcher_demo_window_class_init (HdyViewSwitcherDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->size_allocate = hdy_view_switcher_demo_window_size_allocate;

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/handy/demo/ui/hdy-view-switcher-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherDemoWindow, squeezer);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherDemoWindow, switcher_bar);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherDemoWindow, title_label);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherDemoWindow, title_narrow_switcher);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherDemoWindow, title_wide_switcher);
}

static void
hdy_view_switcher_demo_window_init (HdyViewSwitcherDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_bind_property_full (self->squeezer,
                               "visible-child",
                               self->switcher_bar,
                               "reveal",
                               G_BINDING_SYNC_CREATE,
                               is_title_label_visible,
                               NULL,
                               self->title_label,
                               NULL);

}

HdyViewSwitcherDemoWindow *
hdy_view_switcher_demo_window_new (void)
{
  return g_object_new (HDY_TYPE_VIEW_SWITCHER_DEMO_WINDOW, NULL);
}
