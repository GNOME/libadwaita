#include "adw-view-switcher-demo-dialog.h"

struct _AdwViewSwitcherDemoDialog
{
  AdwDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwViewSwitcherDemoDialog, adw_view_switcher_demo_dialog, ADW_TYPE_DIALOG)

static void
adw_view_switcher_demo_dialog_class_init (AdwViewSwitcherDemoDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/view-switcher/adw-view-switcher-demo-dialog.ui");
}

static void
adw_view_switcher_demo_dialog_init (AdwViewSwitcherDemoDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwDialog *
adw_view_switcher_demo_dialog_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_DEMO_DIALOG, NULL);
}
