#include "adw-demo-page-spinner.h"

struct _AdwDemoPageSpinner
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageSpinner, adw_demo_page_spinner, ADW_TYPE_BIN)

static void
adw_demo_page_spinner_class_init (AdwDemoPageSpinnerClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/spinner/adw-demo-page-spinner.ui");
}

static void
adw_demo_page_spinner_init (AdwDemoPageSpinner *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
