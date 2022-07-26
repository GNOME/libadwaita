#include "adw-demo-page-buttons.h"

#include <glib/gi18n.h>

struct _AdwDemoPageButtons
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageButtons, adw_demo_page_buttons, ADW_TYPE_BIN)

static char *
get_display_mode_name (AdwEnumListItem *item,
                      gpointer         user_data)
{
  switch (adw_enum_list_item_get_value (item)) {
  case ADW_TOGGLE_DISPLAY_MODE_LABEL:
    return g_strdup (_("Label"));
  case ADW_TOGGLE_DISPLAY_MODE_ICON:
    return g_strdup (_("Image"));
  case ADW_TOGGLE_DISPLAY_MODE_BOTH:
    return g_strdup (_("Both"));
  default:
    return NULL;
  }
}

static void
adw_demo_page_buttons_class_init (AdwDemoPageButtonsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/buttons/adw-demo-page-buttons.ui");

  gtk_widget_class_bind_template_callback (widget_class, get_display_mode_name);
}

static void
adw_demo_page_buttons_init (AdwDemoPageButtons *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
