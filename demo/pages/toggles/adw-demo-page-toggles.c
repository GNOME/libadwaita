#include "adw-demo-page-toggles.h"

#include <glib/gi18n.h>

struct _AdwDemoPageToggles
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageToggles, adw_demo_page_toggles, ADW_TYPE_BIN)

static void
adw_demo_page_toggles_class_init (AdwDemoPageTogglesClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/toggles/adw-demo-page-toggles.ui");
}

static void
adw_demo_page_toggles_init (AdwDemoPageToggles *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
