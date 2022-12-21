#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <adwaita.h>

#include "adw-demo-debug-info.h"
#include "adw-demo-preferences-window.h"
#include "adw-demo-window.h"

static void
show_inspector (GSimpleAction *action,
                GVariant      *state,
                gpointer       user_data)
{
  gtk_window_set_interactive_debugging (TRUE);
}

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
show_about (GSimpleAction *action,
            GVariant      *state,
            gpointer       user_data)
{
  const char *developers[] = {
    "Adrien Plazas",
    "Alexander Mikhaylenko",
    "Andrei Lișiță",
    "Guido Günther",
    "Jamie Murphy",
    "Julian Sparber",
    "Manuel Genovés",
    "Zander Brown",
    NULL
  };

  const char *designers[] = {
    "GNOME Design Team",
    NULL
 };

  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *window = gtk_application_get_active_window (app);
  char *debug_info;
  GtkWidget *about;

  debug_info = adw_demo_generate_debug_info ();

  about =
    g_object_new (ADW_TYPE_ABOUT_WINDOW,
                  "transient-for", window,
                  "application-icon", "org.gnome.Adwaita1.Demo",
                  "application-name", _("Adwaita Demo"),
                  "developer-name", _("The GNOME Project"),
                  "version", ADW_VERSION_S,
                  "website", "https://gitlab.gnome.org/GNOME/libadwaita",
                  "issue-url", "https://gitlab.gnome.org/GNOME/libadwaita/-/issues/new",
                  "debug-info", debug_info,
                  "debug-info-filename", "adwaita-1-demo-debug-info.txt",
                  "copyright", "© 2017–2022 Purism SPC",
                  "license-type", GTK_LICENSE_LGPL_2_1,
                  "developers", developers,
                  "designers", designers,
                  "artists", designers,
                  "translator-credits", _("translator-credits"),
                  NULL);

  adw_about_window_add_link (ADW_ABOUT_WINDOW (about),
                             _("_Documentation"),
                             "https://gnome.pages.gitlab.gnome.org/libadwaita/doc/main/");
  adw_about_window_add_link (ADW_ABOUT_WINDOW (about),
                             _("_Chat"),
                             "https://matrix.to/#/#libadwaita:gnome.org");

  gtk_window_present (GTK_WINDOW (about));

  g_free (debug_info);
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
  AdwApplication *app;
  int status;
  static GActionEntry app_entries[] = {
    { "inspector", show_inspector, NULL, NULL, NULL },
    { "preferences", show_preferences, NULL, NULL, NULL },
    { "about", show_about, NULL, NULL, NULL },
  };

  app = adw_application_new ("org.gnome.Adwaita1.Demo", G_APPLICATION_NON_UNIQUE);
  g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);
  g_signal_connect (app, "activate", G_CALLBACK (show_window), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
