#include "adw-demo-page-bottom-sheets.h"

struct _AdwDemoPageBottomSheets
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageBottomSheets, adw_demo_page_bottom_sheets, ADW_TYPE_BIN)

static void
adw_demo_page_bottom_sheets_class_init (AdwDemoPageBottomSheetsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/bottom-sheets/adw-demo-page-bottom-sheets.ui");
}

static void
adw_demo_page_bottom_sheets_init (AdwDemoPageBottomSheets *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
