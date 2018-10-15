#include <gtk/gtk.h>
#define HANDY_USE_UNSTABLE_API
#include <handy.h>

#include "example-window.h"

static void
startup (GtkApplication *app)
{
  GtkCssProvider *css_provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (css_provider, "/sm/puri/handy/example/ui/style.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref (css_provider);
}

static void
show_window (GtkApplication *app)
{
  ExampleWindow *window;

  window = example_window_new (app);

  gtk_widget_show (GTK_WIDGET (window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  hdy_init (&argc, &argv);
  app = gtk_application_new ("sm.puri.Handy.Example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (show_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
