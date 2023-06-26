#include "config.h"

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
    "Alice Mikhaylenko",
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

  about = adw_about_window_new_from_appdata ("/org/gnome/Adwaita1/Demo/org.gnome.Adwaita1.Demo.metainfo.xml", NULL);
  gtk_window_set_transient_for (GTK_WINDOW (about), window);
  adw_about_window_set_version (ADW_ABOUT_WINDOW (about), ADW_VERSION_S);
  adw_about_window_set_debug_info (ADW_ABOUT_WINDOW (about), debug_info);
  adw_about_window_set_debug_info_filename (ADW_ABOUT_WINDOW (about), "adwaita-1-demo-debug-info.txt");
  adw_about_window_set_copyright (ADW_ABOUT_WINDOW (about), "© 2017–2022 Purism SPC");
  adw_about_window_set_developers (ADW_ABOUT_WINDOW (about), developers);
  adw_about_window_set_designers (ADW_ABOUT_WINDOW (about), designers);
  adw_about_window_set_artists (ADW_ABOUT_WINDOW (about), designers);
  adw_about_window_set_translator_credits (ADW_ABOUT_WINDOW (about), _("translator-credits"));

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
