#include <gtk/gtk.h>
#include <handy.h>

static void
print_number (GtkWidget *widget,
	      gchar *number,
	      gpointer   data)
{
  g_print ("Dial %s\n", number);
}


static void
barf (GtkWidget *widget,
      gpointer   data)
{
  g_print ("wuff: %s\n", hdy_dialer_get_number (HDY_DIALER (widget)));
}


static void
show_dialer (GtkApplication *app,
             gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *dialer;

  window = gtk_application_window_new (app);
  dialer = hdy_dialer_new();

  gtk_window_set_title (GTK_WINDOW (window), "Dialer Example");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  gtk_container_add (GTK_CONTAINER (window), dialer);
  g_signal_connect (dialer, "notify::number", G_CALLBACK (barf), NULL);
  g_signal_connect (dialer, "dialed", G_CALLBACK (print_number), NULL);
  g_signal_connect_swapped (dialer, "dialed", G_CALLBACK (gtk_widget_destroy), window);

  gtk_widget_show_all (window);
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("sm.puri.Handy.Example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (show_dialer), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
