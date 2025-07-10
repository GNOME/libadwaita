#include <adwaita.h>

static const char *shortcuts[] = {
  "<Control>A <primary>B",
  "<Alt>c <Meta>d",
  "<Super>E <Hyper>F",

  "<Control>C Home",
  "<Alt>1...9",
  "Control_L&Control_R",
  "<Control>C+<Control>X",

  "Left&Right&Up&Down&space&Return",
  "Page_Up&Page_Down&Home&End",
  "KP_0&KP_Left&KP_Enter"
};

static GtkWidget *
create_content (void)
{
  GtkWidget *box;
  int i;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);

  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top (box, 18);
  gtk_widget_set_margin_bottom (box, 18);
  gtk_widget_set_margin_start (box, 18);
  gtk_widget_set_margin_end (box, 18);

  for (i = 0; i < G_N_ELEMENTS (shortcuts); i++) {
    GtkWidget *label = adw_shortcut_label_new (shortcuts[i]);

    gtk_widget_set_halign (label, GTK_ALIGN_START);

    gtk_box_append (GTK_BOX (box), label);
  }

  return box;
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
  gtk_window_set_title (GTK_WINDOW (window), "Shortcut Labels");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
