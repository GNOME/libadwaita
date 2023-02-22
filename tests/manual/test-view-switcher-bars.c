#include <adwaita.h>

static GtkWidget *
create_content (void)
{
  GtkBox *box;
  int i;

  box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 0));

  gtk_widget_set_valign (GTK_WIDGET (box), GTK_ALIGN_END);

  for (i = 1; i < 6; i++) {
    GtkWidget *stack, *switcher;
    int page;

    stack = adw_view_stack_new ();

    gtk_widget_set_visible (stack, FALSE);
    gtk_box_prepend (box, stack);

    for (page = 0; page < i; page++) {
      char *title = g_strdup_printf ("Page %d", page + 1);

      adw_view_stack_add_titled_with_icon (ADW_VIEW_STACK (stack),
                                           gtk_button_new (),
                                           NULL,
                                           title,
                                           "emblem-system-symbolic");

      g_free (title);
    }

    switcher = adw_view_switcher_bar_new ();
    adw_view_switcher_bar_set_reveal (ADW_VIEW_SWITCHER_BAR (switcher), TRUE);
    adw_view_switcher_bar_set_stack (ADW_VIEW_SWITCHER_BAR (switcher),
                                     ADW_VIEW_STACK (stack));

    gtk_box_append (box, switcher);
  }

  return GTK_WIDGET (box);
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

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "View Switcher Bars");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_set_default_size (GTK_WINDOW (window), 360, -1);
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
