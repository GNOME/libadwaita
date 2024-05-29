#include "adw-demo-page-navigation-view.h"

#include "adw-navigation-view-demo-dialog.h"

struct _AdwDemoPageNavigationView
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageNavigationView, adw_demo_page_navigation_view, ADW_TYPE_BIN)

static void
demo_run_cb (AdwDemoPageNavigationView *self)
{
  adw_dialog_present (adw_navigation_view_demo_dialog_new (), GTK_WIDGET (self));
}

static void
adw_demo_page_navigation_view_class_init (AdwDemoPageNavigationViewClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/navigation-view/adw-demo-page-navigation-view.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_navigation_view_init (AdwDemoPageNavigationView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
