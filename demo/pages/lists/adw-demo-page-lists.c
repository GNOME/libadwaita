#include "adw-demo-page-lists.h"

struct _AdwDemoPageLists
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageLists, adw_demo_page_lists, ADW_TYPE_BIN)

static void
entry_apply_cb (AdwDemoPageLists *self)
{
  GtkWidget *toast_overlay = gtk_widget_get_ancestor (GTK_WIDGET (self), ADW_TYPE_TOAST_OVERLAY);
  AdwToast *toast = adw_toast_new ("Changes applied");

  adw_toast_overlay_add_toast (ADW_TOAST_OVERLAY (toast_overlay), toast);
}

static void
adw_demo_page_lists_class_init (AdwDemoPageListsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/lists/adw-demo-page-lists.ui");

  gtk_widget_class_bind_template_callback (widget_class, entry_apply_cb);
}

static void
adw_demo_page_lists_init (AdwDemoPageLists *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
