#include "adw-demo-page-split-views.h"

#include "adw-navigation-split-view-demo-dialog.h"
#include "adw-overlay-split-view-demo-dialog.h"

struct _AdwDemoPageSplitViews
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageSplitViews, adw_demo_page_split_views, ADW_TYPE_BIN)

static void
demo_run_navigation_cb (AdwDemoPageSplitViews *self)
{
  adw_dialog_present (adw_navigation_split_view_demo_dialog_new (), GTK_WIDGET (self));
}

static void
demo_run_overlay_cb (AdwDemoPageSplitViews *self)
{
  adw_dialog_present (adw_overlay_split_view_demo_dialog_new (), GTK_WIDGET (self));
}

static void
adw_demo_page_split_views_class_init (AdwDemoPageSplitViewsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/split-views/adw-demo-page-split-views.ui");

  gtk_widget_class_install_action (widget_class, "demo.run-navigation", NULL, (GtkWidgetActionActivateFunc) demo_run_navigation_cb);
  gtk_widget_class_install_action (widget_class, "demo.run-overlay", NULL, (GtkWidgetActionActivateFunc) demo_run_overlay_cb);
}

static void
adw_demo_page_split_views_init (AdwDemoPageSplitViews *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
