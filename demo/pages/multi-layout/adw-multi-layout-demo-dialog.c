#include "adw-multi-layout-demo-dialog.h"

struct _AdwMultiLayoutDemoDialog
{
  AdwDialog parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwMultiLayoutDemoDialog, adw_multi_layout_demo_dialog, ADW_TYPE_DIALOG)

static void
adw_multi_layout_demo_dialog_class_init (AdwMultiLayoutDemoDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/multi-layout/adw-multi-layout-demo-dialog.ui");
}

static void
adw_multi_layout_demo_dialog_init (AdwMultiLayoutDemoDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwDialog *
adw_multi_layout_demo_dialog_new (void)
{
  return g_object_new (ADW_TYPE_MULTI_LAYOUT_DEMO_DIALOG, NULL);
}
