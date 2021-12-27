#include "tasks-window.h"

#include <adwaita.h>

static void
activate_cb (GtkApplication *app)
{
  GtkWindow *window;

  window = gtk_application_get_active_window (app);
  if (window == NULL)
    window = tasks_window_new (app);

  gtk_window_present (window);
}

int
main (int   argc,
      char *argv[])
{
  g_autoptr(AdwApplication) app =
    adw_application_new ("org.example.Tasks", G_APPLICATION_NON_UNIQUE); // FIXME _NONE

  g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
