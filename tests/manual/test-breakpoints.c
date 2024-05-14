#include <adwaita.h>
#include <glib/gi18n.h>

#define STYLE "" \
".camera {" \
"  background: #444444;" \
"  color: white;" \
"}" \
".camera headerbar {" \
"  background: none;" \
"  box-shadow: none; " \
"  color: inherit;" \
"}"

static void
simple_cb (void)
{
  GtkWidget *window, *box, *bin;
  AdwBreakpoint *breakpoint;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);

  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top (box, 12);
  gtk_widget_set_margin_bottom (box, 12);
  gtk_widget_set_margin_start (box, 12);
  gtk_widget_set_margin_end (box, 12);

  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 1"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 2"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 3"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 4"));
  gtk_box_append (GTK_BOX (box), gtk_button_new_with_label ("Button 5"));

  bin = adw_breakpoint_bin_new ();
  gtk_widget_set_size_request (bin, 200, 300);
  adw_breakpoint_bin_set_child (ADW_BREAKPOINT_BIN (bin), box);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 420pt"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
                              NULL);
  adw_breakpoint_bin_add_breakpoint (ADW_BREAKPOINT_BIN (bin), breakpoint);

  window = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Simple");
  gtk_window_set_child (GTK_WINDOW (window), bin);
  gtk_window_set_default_size (GTK_WINDOW (window), 700, 300);

  gtk_window_present (GTK_WINDOW (window));
}

static void
browser_cb (void)
{
  GtkWidget *window, *view, *top_bar, *bottom_bar;
  GtkWidget *clamp, *box, *entry;
  GtkWidget *back, *forward, *refresh, *star;
  AdwBreakpoint *breakpoint;

  back = gtk_button_new_from_icon_name ("go-previous-symbolic");
  forward = gtk_button_new_from_icon_name ("go-next-symbolic");
  refresh = gtk_button_new_from_icon_name ("view-refresh-symbolic");
  star = gtk_button_new_from_icon_name ("starred-symbolic");

  entry = gtk_entry_new ();
  gtk_editable_set_max_width_chars (GTK_EDITABLE (entry), 200);

  clamp = adw_clamp_new ();
  adw_clamp_set_maximum_size (ADW_CLAMP (clamp), 600);
  adw_clamp_set_tightening_threshold (ADW_CLAMP (clamp), 400);
  adw_clamp_set_child (ADW_CLAMP (clamp), entry);

  top_bar = adw_header_bar_new ();

  adw_header_bar_pack_start (ADW_HEADER_BAR (top_bar), back);
  adw_header_bar_pack_start (ADW_HEADER_BAR (top_bar), forward);
  adw_header_bar_pack_start (ADW_HEADER_BAR (top_bar), refresh);
  adw_header_bar_set_title_widget (ADW_HEADER_BAR (top_bar), clamp);
  adw_header_bar_pack_end (ADW_HEADER_BAR (top_bar),
                           gtk_button_new_from_icon_name ("open-menu-symbolic"));
  adw_header_bar_pack_end (ADW_HEADER_BAR (top_bar), star);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_set_homogeneous (GTK_BOX (box), TRUE);
  gtk_widget_add_css_class (box, "toolbar");

  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("go-previous-symbolic"));
  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("go-next-symbolic"));
  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("view-refresh-symbolic"));
  gtk_box_append (GTK_BOX (box),
                  gtk_button_new_from_icon_name ("starred-symbolic"));

  bottom_bar = adw_clamp_new ();
  adw_clamp_set_maximum_size (ADW_CLAMP (bottom_bar), 400);
  adw_clamp_set_child (ADW_CLAMP (bottom_bar), box);
  gtk_widget_set_visible (bottom_bar, FALSE);

  view = adw_toolbar_view_new ();
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (view), top_bar);
  adw_toolbar_view_set_top_bar_style (ADW_TOOLBAR_VIEW (view), ADW_TOOLBAR_RAISED);
  adw_toolbar_view_add_bottom_bar (ADW_TOOLBAR_VIEW (view), bottom_bar);
  adw_toolbar_view_set_bottom_bar_style (ADW_TOOLBAR_VIEW (view), ADW_TOOLBAR_RAISED);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Browser");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  adw_window_set_content (ADW_WINDOW (window), view);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 500px"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (back), "visible", FALSE,
                              G_OBJECT (forward), "visible", FALSE,
                              G_OBJECT (refresh), "visible", FALSE,
                              G_OBJECT (star), "visible", FALSE,
                              G_OBJECT (bottom_bar), "visible", TRUE,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static void
camera_cb (void)
{
  GtkWidget *window, *overlay, *box, *bar1, *bar2, *headerbar;
  AdwBreakpoint *breakpoint;

  /* Single vertical by default */

  bar1 = gtk_window_handle_new ();
  gtk_widget_set_size_request (bar1, 60, 60);
  gtk_widget_add_css_class (bar1, "osd");
  gtk_widget_set_halign (bar1, GTK_ALIGN_END);
  gtk_widget_set_hexpand (bar1, TRUE);
  gtk_widget_set_vexpand (bar1, TRUE);

  bar2 = gtk_window_handle_new ();
  gtk_widget_set_size_request (bar2, 60, 60);
  gtk_widget_add_css_class (bar2, "osd");
  gtk_widget_set_visible (bar2, FALSE);
  gtk_widget_set_halign (bar2, GTK_ALIGN_START);
  gtk_widget_set_hexpand (bar2, TRUE);
  gtk_widget_set_vexpand (bar2, TRUE);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_append (GTK_BOX (box), bar2);
  gtk_box_append (GTK_BOX (box), bar1);

  headerbar = adw_header_bar_new ();
  gtk_widget_set_valign (headerbar, GTK_ALIGN_START);
  adw_header_bar_set_show_title (ADW_HEADER_BAR (headerbar), FALSE);

  overlay = gtk_overlay_new ();
  gtk_overlay_set_child (GTK_OVERLAY (overlay), box);
  gtk_overlay_add_overlay (GTK_OVERLAY (overlay), headerbar);

  window = adw_window_new ();
  gtk_widget_set_size_request (window, 300, 300);
  gtk_widget_add_css_class (window, "camera");
  adw_window_set_content (ADW_WINDOW (window), overlay);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 580);

  /* Single horizontal */
  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-aspect-ratio: 4/3"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
                              G_OBJECT (bar1), "halign", GTK_ALIGN_FILL,
                              G_OBJECT (bar1), "valign", GTK_ALIGN_END,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  /* Dual vertical */
  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-height: 400px"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (bar2), "visible", TRUE,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  /* Dual horizontal */
  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-aspect-ratio: 4/3 and max-width: 450px"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (box), "orientation", GTK_ORIENTATION_VERTICAL,
                              G_OBJECT (bar1), "halign", GTK_ALIGN_FILL,
                              G_OBJECT (bar1), "valign", GTK_ALIGN_END,
                              G_OBJECT (bar2), "visible", TRUE,
                              G_OBJECT (bar2), "halign", GTK_ALIGN_FILL,
                              G_OBJECT (bar2), "valign", GTK_ALIGN_START,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static GtkWidget *
create_content (GtkWindow *parent)
{
  GtkWidget *box, *button;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
  gtk_widget_set_margin_top (box, 48);
  gtk_widget_set_margin_bottom (box, 48);
  gtk_widget_set_margin_start (box, 48);
  gtk_widget_set_margin_end (box, 48);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label ("Simple");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (simple_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Browser");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (browser_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Camera");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (camera_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

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
  GtkCssProvider *provider;
  gboolean done = FALSE;

  adw_init ();

  provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (provider, STYLE);
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                              GTK_STYLE_PROVIDER (provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Breakpoints");
  gtk_window_set_child (GTK_WINDOW (window), create_content (GTK_WINDOW (window)));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  g_object_unref (provider);

  return 0;
}
