#include "adw-demo-page-stub.h"

struct _AdwDemoPageStub
{
  AdwDemoPage parent_instance;
};

G_DEFINE_TYPE (AdwDemoPageStub, adw_demo_page_stub, ADW_TYPE_DEMO_PAGE)

static void
adw_demo_page_stub_class_init (AdwDemoPageStubClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/stub/adw-demo-page-stub.ui");
}

static void
adw_demo_page_stub_init (AdwDemoPageStub *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
