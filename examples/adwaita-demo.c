#include <gtk/gtk.h>
#include <adwaita.h>

#include "adw-demo-preferences-window.h"
#include "adw-demo-window.h"

static void
show_preferences (GSimpleAction *action,
                  GVariant      *state,
                  gpointer       user_data)
{
  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *window = gtk_application_get_active_window (app);
  AdwDemoPreferencesWindow *preferences = adw_demo_preferences_window_new ();

  gtk_window_set_transient_for (GTK_WINDOW (preferences), window);
  gtk_window_present (GTK_WINDOW (preferences));
}

static void
startup (GtkApplication *app)
{
  GtkCssProvider *css_provider = gtk_css_provider_new ();

  adw_init ();

  gtk_css_provider_load_from_resource (css_provider, "/org/gnome/Adwaita/Demo/ui/style.css");
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  g_object_unref (css_provider);
}

static void
show_window (GtkApplication *app)
{
  AdwDemoWindow *window;

  window = adw_demo_window_new (app);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;
  static GActionEntry app_entries[] = {
    { "preferences", show_preferences, NULL, NULL, NULL },
  };

  app = gtk_application_new ("org.gnome.Adwaita.Demo", G_APPLICATION_FLAGS_NONE);
  g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);
  g_signal_connect (app, "startup", G_CALLBACK (startup), NULL);
  g_signal_connect (app, "activate", G_CALLBACK (show_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
