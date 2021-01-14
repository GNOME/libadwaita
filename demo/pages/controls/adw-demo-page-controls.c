#include "adw-demo-page-controls.h"

struct _AdwDemoPageControls
{
  AdwDemoPage parent_instance;
};

G_DEFINE_TYPE (AdwDemoPageControls, adw_demo_page_controls, ADW_TYPE_DEMO_PAGE)

static double
progress_to_level (AdwDemoPageControls *self,
                   double               progress)
{
  return progress * 5;
}

static void
adw_demo_page_controls_class_init (AdwDemoPageControlsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/controls/adw-demo-page-controls.ui");
  gtk_widget_class_bind_template_callback (widget_class, progress_to_level);
}

static void
adw_demo_page_controls_init (AdwDemoPageControls *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
