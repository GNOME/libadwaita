#include "adw-demo-page-recoloring.h"

#include "adw-demo-color-row.h"

struct _AdwDemoPageRecoloring
{
  AdwBin parent_instance;

  GtkListBox *colors_list;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageRecoloring, adw_demo_page_recoloring, ADW_TYPE_BIN)

static GtkWidget *
color_row_create_cb (AdwEnumListItem *item)
{
  AdwColor color = adw_enum_list_item_get_value (item);

  return GTK_WIDGET (adw_demo_color_row_new (color));
}

AdwDemoPageRecoloring *
adw_demo_page_recoloring_new (void)
{
  return g_object_new (ADW_TYPE_DEMO_PAGE_RECOLORING, NULL);
}

static void
adw_demo_page_recoloring_class_init (AdwDemoPageRecoloringClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/recoloring/adw-demo-page-recoloring.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageRecoloring, colors_list);
}

static void
adw_demo_page_recoloring_init (AdwDemoPageRecoloring *self)
{
  AdwEnumListModel *model;

  g_type_ensure (ADW_TYPE_DEMO_PAGE_RECOLORING);

  gtk_widget_init_template (GTK_WIDGET (self));

  model = adw_enum_list_model_new (ADW_TYPE_COLOR);
  gtk_list_box_bind_model (self->colors_list,
                           G_LIST_MODEL (model),
                           (GtkListBoxCreateWidgetFunc) color_row_create_cb,
                           NULL, NULL);
}
