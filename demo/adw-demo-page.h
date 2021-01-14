#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_PAGE (adw_demo_page_get_type())

G_DECLARE_DERIVABLE_TYPE (AdwDemoPage, adw_demo_page, ADW, DEMO_PAGE, GtkWidget)

struct _AdwDemoPageClass
{
  GtkWidgetClass parent_class;
};

G_END_DECLS
