#include <gtk/gtk.h>
#define HANDY_USE_UNSTABLE_API
#include <handy.h>

#include "example-window.h"

static void
show_window (GtkApplication *app)
{
  ExampleWindow *window;

  window = example_window_new (app);

  gtk_window_set_title (GTK_WINDOW (window), "Handy Widget Factory");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  gtk_widget_show_all (GTK_WIDGET (window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("sm.puri.Handy.Example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (show_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
