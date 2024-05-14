#include <adwaita.h>

static void
activate_cb (GtkApplication *app)
{
  GtkWidget *window = adw_application_window_new (app);
  GtkWidget *toolbar_view = adw_toolbar_view_new ();
  GtkWidget *header_bar = adw_header_bar_new ();
  GtkWidget *label = gtk_label_new ("Hello World");

  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), header_bar);
  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (toolbar_view), label);

  gtk_window_set_title (GTK_WINDOW (window), "Hello");
  adw_application_window_set_content (ADW_APPLICATION_WINDOW (window), toolbar_view);
  gtk_window_present (GTK_WINDOW (window));
}

int
main (int   argc,
      char *argv[])
{
  g_autoptr (AdwApplication) app = NULL;

  app = adw_application_new ("org.example.Hello", 0);

  g_signal_connect (app, "activate", G_CALLBACK (activate_cb), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
