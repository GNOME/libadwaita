#include <adwaita.h>
#include <glib/gi18n.h>

static void
present_shortcuts_dialog (GtkWidget  *parent,
                          const char *name)
{
  char *path = g_strdup_printf ("/org/gnome/Adwaita1/Test/resources/shortcuts-dialog-%s.ui", name);
  GtkBuilder *builder = gtk_builder_new_from_resource (path);
  GObject *dialog = gtk_builder_get_object (builder, "shortcuts_dialog");

  g_assert (dialog);

  adw_dialog_present (ADW_DIALOG (dialog), parent);

  g_object_unref (builder);
  g_free (path);
}

static void
calculator_cb (GtkWidget *parent)
{
  present_shortcuts_dialog (parent, "calculator");
}

static void
clocks_cb (GtkWidget *parent)
{
  present_shortcuts_dialog (parent, "clocks");
}

static GtkWidget *
create_content (GtkWidget *parent)
{
  GtkWidget *view, *swindow, *box, *button;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
  gtk_widget_set_margin_top (box, 48);
  gtk_widget_set_margin_bottom (box, 48);
  gtk_widget_set_margin_start (box, 48);
  gtk_widget_set_margin_end (box, 48);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label ("Calculator");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (calculator_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Clocks");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (clocks_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  swindow = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_propagate_natural_height (GTK_SCROLLED_WINDOW (swindow),
                                                    TRUE);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (swindow), box);

  view = adw_toolbar_view_new ();
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (view), adw_header_bar_new ());
  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (view), swindow);

  return view;
}

static void
close_cb (gboolean *done)
{
  *done = TRUE;
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *window;
  gboolean done = FALSE;

  adw_init ();

  window = adw_window_new ();
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  gtk_widget_set_size_request (window, 360, 294);
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Shortcuts Dialogs");
  adw_window_set_content (ADW_WINDOW (window), create_content (window));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
