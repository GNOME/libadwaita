#include <adwaita.h>

typedef enum {
  TOGGLE_MODE_LABELS,
  TOGGLE_MODE_ICONS,
  TOGGLE_MODE_BOTH,
  TOGGLE_MODE_CHILD,
} ToggleMode;

#define N_TOGGLES 3

static GtkWidget *
create_group (GtkOrientation orientation,
              ToggleMode     mode,
              gboolean       round,
              gboolean       flat)
{
  GtkWidget *group = adw_toggle_group_new ();
  int i;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (group), orientation);

  gtk_widget_set_halign (group, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (group, GTK_ALIGN_CENTER);

  if (round)
    gtk_widget_add_css_class (group, "round");

  if (flat)
    gtk_widget_add_css_class (group, "flat");

  for (i = 0; i < N_TOGGLES; i++) {
    AdwToggle *toggle = adw_toggle_new ();

    if (mode == TOGGLE_MODE_ICONS || mode == TOGGLE_MODE_BOTH)
      adw_toggle_set_icon_name (toggle, "adw-tab-icon-missing-symbolic");

    if (mode == TOGGLE_MODE_LABELS || mode == TOGGLE_MODE_BOTH || mode == TOGGLE_MODE_CHILD) {
      char *label = g_strdup_printf ("Page %d", i + 1);
      adw_toggle_set_label (toggle, label);
      g_free (label);
    }

    if (mode == TOGGLE_MODE_CHILD) {
      GtkWidget *spinner = adw_spinner_new ();

      gtk_widget_set_halign (spinner, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (spinner, GTK_ALIGN_CENTER);

      adw_toggle_set_child (toggle, spinner);
    }

    {
      char *tooltip = g_strdup_printf ("Tooltip for page %d", i + 1);
      adw_toggle_set_tooltip (toggle, tooltip);
      g_free (tooltip);
    }

    if (i == N_TOGGLES - 1)
      adw_toggle_set_enabled (toggle, FALSE);

    adw_toggle_group_add (ADW_TOGGLE_GROUP (group), toggle);
  }

  return group;
}

static GtkWidget *
create_section (GtkOrientation orientation,
                gboolean       round,
                gboolean       flat)
{
  GtkWidget *box;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  else
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  gtk_box_append (GTK_BOX (box), create_group (orientation, TOGGLE_MODE_LABELS, round, flat));
  gtk_box_append (GTK_BOX (box), create_group (orientation, TOGGLE_MODE_ICONS,  round, flat));
  gtk_box_append (GTK_BOX (box), create_group (orientation, TOGGLE_MODE_BOTH,   round, flat));
  gtk_box_append (GTK_BOX (box), create_group (orientation, TOGGLE_MODE_CHILD,  round, flat));

  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand (box, TRUE);
  gtk_widget_set_vexpand (box, TRUE);

  return box;
}

static GtkWidget *
create_content (void)
{
  GtkBox *hbox, *vbox1, *vbox2;

  hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 18));
  vbox1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 18));
  vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 18));

  gtk_widget_set_margin_top    (GTK_WIDGET (hbox), 18);
  gtk_widget_set_margin_bottom (GTK_WIDGET (hbox), 18);
  gtk_widget_set_margin_start  (GTK_WIDGET (hbox), 18);
  gtk_widget_set_margin_end    (GTK_WIDGET (hbox), 18);

  gtk_box_append (vbox1, create_section (GTK_ORIENTATION_HORIZONTAL, FALSE, FALSE));
  gtk_box_append (vbox1, create_section (GTK_ORIENTATION_HORIZONTAL, TRUE,  FALSE));
  gtk_box_append (vbox1, create_section (GTK_ORIENTATION_HORIZONTAL, FALSE, TRUE));
  gtk_box_append (vbox1, create_section (GTK_ORIENTATION_HORIZONTAL, TRUE,  TRUE));

  gtk_box_append (vbox2, create_section (GTK_ORIENTATION_VERTICAL, FALSE, FALSE));
  gtk_box_append (vbox2, create_section (GTK_ORIENTATION_VERTICAL, TRUE,  FALSE));
  gtk_box_append (vbox2, create_section (GTK_ORIENTATION_VERTICAL, FALSE, TRUE));
  gtk_box_append (vbox2, create_section (GTK_ORIENTATION_VERTICAL, TRUE,  TRUE));

  gtk_box_append (hbox, GTK_WIDGET (vbox1));
  gtk_box_append (hbox, GTK_WIDGET (vbox2));

  return GTK_WIDGET (hbox);
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
  gtk_window_set_title (GTK_WINDOW (window), "Toggle Groups");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
