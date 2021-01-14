#include <gtk/gtk.h>
#include <adwaita.h>

#include "adw-bin.h"
#include "adw-demo-window.h"

static void
startup_cb (GtkApplication *app)
{
  g_autoptr (GtkCssProvider) css_provider = NULL;

  adw_init ();

  // FIXME
  g_type_ensure (ADW_TYPE_BIN);

  css_provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (css_provider, "/org/gnome/Adwaita/Demo/style.css");
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
activate_cb (GtkApplication *app)
{
  GtkWidget *window = adw_demo_window_new (app);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int    argc,
      char **argv)
{
  g_autoptr (GtkApplication) app = NULL;

  app = gtk_application_new ("org.gnome.Adwaita.Demo", G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "startup", G_CALLBACK (startup_cb), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
