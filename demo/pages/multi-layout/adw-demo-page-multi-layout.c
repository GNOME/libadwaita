#include "adw-demo-page-multi-layout.h"

#include <glib/gi18n.h>

#include "adw-multi-layout-demo-dialog.h"

struct _AdwDemoPageMultiLayout
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageMultiLayout, adw_demo_page_multi_layout, ADW_TYPE_BIN)

static void
demo_run_cb (GtkWidget *sender)
{
  adw_dialog_present (adw_multi_layout_demo_dialog_new (), sender);
}

static void
adw_demo_page_multi_layout_class_init (AdwDemoPageMultiLayoutClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/multi-layout/adw-demo-page-multi-layout.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_multi_layout_init (AdwDemoPageMultiLayout *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
