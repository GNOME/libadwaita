#include <adwaita.h>

static GtkWidget *
create_scales (GtkOrientation orientation)
{
  GtkWidget *box, *scale;
  GtkAdjustment *adjustment;
  int i;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_size_request (box, 300, -1);
  } else {
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_size_request (box, -1, 300);
  }

  adjustment = gtk_adjustment_new (0, -1, 1, 0.01, 0.1, 0);

  scale = gtk_scale_new (orientation, adjustment);
  gtk_box_append (GTK_BOX (box), scale);

  scale = gtk_scale_new (orientation, adjustment);
  gtk_scale_set_has_origin (GTK_SCALE (scale), FALSE);
  gtk_box_append (GTK_BOX (box), scale);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    GtkWidget *label = gtk_label_new ("Values");
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_widget_add_css_class (label, "heading");
    gtk_box_append (GTK_BOX (box), label);
  }

  for (i = 0; i < 4; i++) {
    scale = gtk_scale_new (orientation, adjustment);
    gtk_scale_set_draw_value (GTK_SCALE (scale), TRUE);
    gtk_scale_set_value_pos (GTK_SCALE (scale), i);
    gtk_box_append (GTK_BOX (box), scale);
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    GtkWidget *label = gtk_label_new ("Marks");
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_widget_add_css_class (label, "heading");
    gtk_box_append (GTK_BOX (box), label);
  }

  scale = gtk_scale_new (orientation, adjustment);
  gtk_scale_add_mark (GTK_SCALE (scale), -1,   GTK_POS_TOP, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale), -0.5, GTK_POS_TOP, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale),  0,   GTK_POS_TOP, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale),  0.5, GTK_POS_TOP, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale),  1,   GTK_POS_TOP, NULL);
  gtk_box_append (GTK_BOX (box), scale);

  scale = gtk_scale_new (orientation, adjustment);
  gtk_scale_add_mark (GTK_SCALE (scale), -1,   GTK_POS_BOTTOM, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale), -0.5, GTK_POS_BOTTOM, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale),  0,   GTK_POS_BOTTOM, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale),  0.5, GTK_POS_BOTTOM, NULL);
  gtk_scale_add_mark (GTK_SCALE (scale),  1,   GTK_POS_BOTTOM, NULL);
  gtk_box_append (GTK_BOX (box), scale);

  scale = gtk_scale_new (orientation, adjustment);
  gtk_scale_add_mark (GTK_SCALE (scale), -1, GTK_POS_TOP, "Top");
  gtk_scale_add_mark (GTK_SCALE (scale),  1, GTK_POS_BOTTOM, "Bottom");
  gtk_box_append (GTK_BOX (box), scale);

  return box;
}

static GtkWidget *
create_content (void)
{
  GtkWidget *box;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 24);
  gtk_widget_set_margin_top (box, 24);
  gtk_widget_set_margin_bottom (box, 24);
  gtk_widget_set_margin_start (box, 24);
  gtk_widget_set_margin_end (box, 24);

  gtk_box_append (GTK_BOX (box), create_scales (GTK_ORIENTATION_HORIZONTAL));
  gtk_box_append (GTK_BOX (box), gtk_separator_new (GTK_ORIENTATION_VERTICAL));
  gtk_box_append (GTK_BOX (box), create_scales (GTK_ORIENTATION_VERTICAL));

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
  gtk_window_set_title (GTK_WINDOW (window), "Scales");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
