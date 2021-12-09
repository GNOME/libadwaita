#include "adw-demo-page-flap.h"

#include <glib/gi18n.h>

#include "adw-flap-demo-window.h"

struct _AdwDemoPageFlap
{
  AdwBin parent_instance;
};

G_DEFINE_TYPE (AdwDemoPageFlap, adw_demo_page_flap, ADW_TYPE_BIN)

static void
demo_run_cb (AdwDemoPageFlap *self)
{
  AdwFlapDemoWindow *window = adw_flap_demo_window_new ();
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (root));
  gtk_window_present (GTK_WINDOW (window));
}

static void
adw_demo_page_flap_class_init (AdwDemoPageFlapClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/flap/adw-demo-page-flap.ui");

  gtk_widget_class_install_action (widget_class, "demo.run", NULL, (GtkWidgetActionActivateFunc) demo_run_cb);
}

static void
adw_demo_page_flap_init (AdwDemoPageFlap *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
