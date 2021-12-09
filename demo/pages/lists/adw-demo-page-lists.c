#include "adw-demo-page-lists.h"

#include <glib/gi18n.h>

struct _AdwDemoPageLists
{
  AdwBin parent_instance;
};

G_DEFINE_TYPE (AdwDemoPageLists, adw_demo_page_lists, ADW_TYPE_BIN)

static void
adw_demo_page_lists_class_init (AdwDemoPageListsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/lists/adw-demo-page-lists.ui");
}

static void
adw_demo_page_lists_init (AdwDemoPageLists *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
