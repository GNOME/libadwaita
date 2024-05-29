#include "adw-demo-page-view-switcher.h"

#include "adw-view-switcher-demo-dialog.h"

struct _AdwDemoPageViewSwitcher
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageViewSwitcher, adw_demo_page_view_switcher, ADW_TYPE_BIN)

static void
demo_run_cb (AdwDemoPageViewSwitcher *self)
{
  adw_dialog_present (adw_view_switcher_demo_dialog_new (), GTK_WIDGET (self));
}

static void
adw_demo_page_view_switcher_class_init (AdwDemoPageViewSwitcherClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/view-switcher/adw-demo-page-view-switcher.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_view_switcher_init (AdwDemoPageViewSwitcher *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
