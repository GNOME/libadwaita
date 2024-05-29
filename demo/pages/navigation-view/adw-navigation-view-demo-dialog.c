#include "adw-navigation-view-demo-dialog.h"

struct _AdwNavigationViewDemoDialog
{
  AdwDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwNavigationViewDemoDialog, adw_navigation_view_demo_dialog, ADW_TYPE_DIALOG)

static void
adw_navigation_view_demo_dialog_class_init (AdwNavigationViewDemoDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/navigation-view/adw-navigation-view-demo-dialog.ui");
}

static void
adw_navigation_view_demo_dialog_init (AdwNavigationViewDemoDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwDialog *
adw_navigation_view_demo_dialog_new (void)
{
  return g_object_new (ADW_TYPE_NAVIGATION_VIEW_DEMO_DIALOG, NULL);
}
