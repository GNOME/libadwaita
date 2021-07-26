#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <adwaita.h>

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
  const char *authors[] = {
    "Adrien Plazas",
    "Alexander Mikhaylenko",
    "Andrei Lișiță",
    "Guido Günther",
    "Julian Sparber",
    "Manuel Genovés",
    "Zander Brown",
    NULL
  };

  const char *artists[] = {
    "GNOME Design Team",
    NULL
  };

  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *window = gtk_application_get_active_window (app);
  char *version;

  version = g_strdup_printf ("%s\nRunning against libadwaita %d.%d.%d, GTK %d.%d.%d",
                             ADW_VERSION_S,
                             adw_get_major_version (),
                             adw_get_minor_version (),
                             adw_get_micro_version (),
                             gtk_get_major_version (),
                             gtk_get_minor_version (),
                             gtk_get_micro_version ());

  gtk_show_about_dialog (window,
                         "program-name", _("Adwaita Demo"),
                         "title", _("About Adwaita Demo"),
                         "logo-icon-name", "org.gnome.Adwaita1.Demo",
                         "version", version,
                         "copyright", "Copyright © 2017–2021 Purism SPC",
                         "comments", _("Tour of the features in Libadwaita"),
                         "website", "https://gitlab.gnome.org/GNOME/libadwaita",
                         "license-type", GTK_LICENSE_LGPL_2_1,
                         "authors", authors,
                         "artists", artists,
                         "translator-credits", _("translator-credits"),
                         NULL);
  g_free (version);
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
