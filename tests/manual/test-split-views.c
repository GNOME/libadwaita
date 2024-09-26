#include <adwaita.h>
#include <glib/gi18n.h>

static AdwNavigationPage *
create_page_with_child (const char *tag,
                        const char *title,
                        GtkWidget  *child)
{
  GtkWidget *header, *content, *page;

  header = adw_header_bar_new ();
  adw_header_bar_set_show_title (ADW_HEADER_BAR (header), FALSE);

  content = adw_status_page_new ();
  adw_status_page_set_title (ADW_STATUS_PAGE (content), title);
  adw_status_page_set_child (ADW_STATUS_PAGE (content), child);

  page = adw_toolbar_view_new ();
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (page), header);
  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (page), content);

  return adw_navigation_page_new_with_tag (page, title, tag);
}

static inline AdwNavigationPage *
create_page (const char *tag,
             const char *title)
{
  return create_page_with_child (tag, title, NULL);
}

static AdwNavigationPage *
create_page_with_button (const char  *tag,
                         const char  *title,
                         const char  *button_title,
                         const char  *button_action,
                         GtkWidget  **button)
{
  if (button_title && button_action) {
    GtkWidget *btn = gtk_button_new_with_label (button_title);
    gtk_button_set_can_shrink (GTK_BUTTON (btn), TRUE);
    gtk_widget_set_halign (btn, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class (btn, "pill");
    gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (btn), button_action);

    if (button)
      *button = btn;

    return create_page_with_child (tag, title, btn);
  }

  return create_page (tag, title);
}

static void
navigation_cb (void)
{
  GtkWidget *window, *view, *button;
  AdwNavigationPage *sidebar, *content;
  AdwBreakpoint *breakpoint;

  sidebar = create_page_with_button ("sidebar", "Sidebar", "Open Content", "navigation.push::content", &button);
  content = create_page ("content", "Content");

  gtk_widget_set_visible (button, FALSE);

  view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (view), sidebar);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (view), content);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 400sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              G_OBJECT (button), "visible", TRUE,
                              NULL);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Navigation Sidebar");
  adw_window_set_content (ADW_WINDOW (window), view);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

  gtk_window_present (GTK_WINDOW (window));
}

static void
navigation_inverted_cb (void)
{
  GtkWidget *window, *view, *button;
  AdwNavigationPage *sidebar, *content;
  AdwBreakpoint *breakpoint;

  sidebar = create_page ("sidebar", "Sidebar");
  content = create_page_with_button ("content", "Content", "Open Sidebar", "navigation.push::sidebar", &button);

  gtk_widget_set_visible (button, FALSE);

  view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_sidebar_position (ADW_NAVIGATION_SPLIT_VIEW (view), GTK_PACK_END);
  adw_navigation_split_view_set_show_content (ADW_NAVIGATION_SPLIT_VIEW (view), TRUE);
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (view), sidebar);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (view), content);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 400sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              G_OBJECT (button), "visible", TRUE,
                              NULL);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Navigation Sidebar");
  adw_window_set_content (ADW_WINDOW (window), view);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

  gtk_window_present (GTK_WINDOW (window));
}

static void
move_sidebar_cb (GObject             *button,
                 AdwOverlaySplitView *view)
{
  gboolean start = adw_overlay_split_view_get_sidebar_position (view) == GTK_PACK_START;

  adw_overlay_split_view_set_sidebar_position (view, start ? GTK_PACK_END : GTK_PACK_START);
}

static void
overlay_cb (void)
{
  GtkWidget *window, *view, *button, *toggle, *box;
  AdwNavigationPage *sidebar, *content;
  AdwBreakpoint *breakpoint;

  button = gtk_button_new_with_label ("Move Sidebar");
  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);
  gtk_widget_add_css_class (button, "pill");

  toggle = gtk_toggle_button_new_with_label ("Show Sidebar");
  gtk_button_set_can_shrink (GTK_BUTTON (toggle), TRUE);
  gtk_widget_add_css_class (toggle, "pill");

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), button);
  gtk_box_append (GTK_BOX (box), toggle);

  sidebar = create_page ("sidebar", "Sidebar");
  content = create_page_with_child ("content", "Content", box);

  view = adw_overlay_split_view_new ();
  adw_overlay_split_view_set_sidebar (ADW_OVERLAY_SPLIT_VIEW (view), GTK_WIDGET (sidebar));
  adw_overlay_split_view_set_content (ADW_OVERLAY_SPLIT_VIEW (view), GTK_WIDGET (content));

  g_object_bind_property (view, "show-sidebar", toggle, "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);
  g_signal_connect (button, "clicked", G_CALLBACK (move_sidebar_cb), view);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 400sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              NULL);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Transient Sidebar");
  adw_window_set_content (ADW_WINDOW (window), view);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

  gtk_window_present (GTK_WINDOW (window));
}

static void
triple_pane_mail_cb (void)
{
  GtkWidget *outer_view, *inner_view, *window;
  AdwNavigationPage *folders, *inbox, *message, *inner_view_page;
  GtkWidget *inbox_button, *message_button;
  AdwBreakpoint *breakpoint;

  folders = create_page_with_button ("folders", "Folders", "Open Inbox", "navigation.push::inbox", &inbox_button);
  inbox = create_page_with_button ("inbox", "Inbox", "Open Message", "navigation.push::message", &message_button);
  message = create_page ("message", "Message");

  gtk_widget_set_visible (inbox_button, FALSE);
  gtk_widget_set_visible (message_button, FALSE);

  inner_view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_max_sidebar_width (ADW_NAVIGATION_SPLIT_VIEW (inner_view), 260);
  adw_navigation_split_view_set_sidebar_width_fraction (ADW_NAVIGATION_SPLIT_VIEW (inner_view), 0.38);
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (inner_view), folders);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (inner_view), inbox);

  inner_view_page = adw_navigation_page_new (inner_view, "");

  outer_view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_min_sidebar_width (ADW_NAVIGATION_SPLIT_VIEW (outer_view), 470);
  adw_navigation_split_view_set_max_sidebar_width (ADW_NAVIGATION_SPLIT_VIEW (outer_view), 780);
  adw_navigation_split_view_set_sidebar_width_fraction (ADW_NAVIGATION_SPLIT_VIEW (outer_view), 0.47);
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (outer_view), inner_view_page);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (outer_view), message);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Mail");
  adw_window_set_content (ADW_WINDOW (window), outer_view);
  gtk_window_set_default_size (GTK_WINDOW (window), 1200, 600);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 860sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (message_button), "visible", TRUE,
                              G_OBJECT (inner_view), "sidebar-width-fraction", 0.33f,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 500sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (inner_view), "collapsed", TRUE,
                              G_OBJECT (inner_view), "sidebar-width-fraction", 0.33f,
                              G_OBJECT (inbox_button), "visible", TRUE,
                              G_OBJECT (message_button), "visible", TRUE,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static void
triple_pane_feeds_cb (void)
{
  GtkWidget *outer_view, *inner_view, *window, *button, *toggle, *box;
  AdwNavigationPage *feeds, *articles, *content;
  AdwBreakpoint *breakpoint;

  toggle = gtk_toggle_button_new_with_label ("Show Feeds");
  gtk_button_set_can_shrink (GTK_BUTTON (toggle), TRUE);
  gtk_widget_add_css_class (toggle, "pill");
  gtk_widget_set_visible (toggle, FALSE);

  button = gtk_button_new_with_label ("Open Content");
  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);
  gtk_widget_add_css_class (button, "pill");
  gtk_actionable_set_detailed_action_name (GTK_ACTIONABLE (button), "navigation.push::content");
  gtk_widget_set_visible (button, FALSE);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_box_append (GTK_BOX (box), toggle);
  gtk_box_append (GTK_BOX (box), button);
  gtk_widget_set_visible (box, FALSE);

  feeds = create_page ("feeds", "Feeds");
  articles = create_page_with_child ("articles", "Articles", box);
  content = create_page ("content", "Content");

  inner_view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_sidebar_width_fraction (ADW_NAVIGATION_SPLIT_VIEW (inner_view), 0.355);
  adw_navigation_split_view_set_min_sidebar_width (ADW_NAVIGATION_SPLIT_VIEW (inner_view), 290);
  adw_navigation_split_view_set_max_sidebar_width (ADW_NAVIGATION_SPLIT_VIEW (inner_view), 520);
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (inner_view), articles);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (inner_view), content);

  outer_view = adw_overlay_split_view_new ();
  adw_overlay_split_view_set_sidebar_width_fraction (ADW_OVERLAY_SPLIT_VIEW (outer_view), 0.179);
  adw_overlay_split_view_set_max_sidebar_width (ADW_OVERLAY_SPLIT_VIEW (outer_view), 260);
  adw_overlay_split_view_set_sidebar (ADW_OVERLAY_SPLIT_VIEW (outer_view), GTK_WIDGET (feeds));
  adw_overlay_split_view_set_content (ADW_OVERLAY_SPLIT_VIEW (outer_view), inner_view);

  g_object_bind_property (outer_view, "show-sidebar", toggle, "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Feeds");
  adw_window_set_content (ADW_WINDOW (window), outer_view);
  gtk_window_set_default_size (GTK_WINDOW (window), 1200, 600);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 860sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (box), "visible", TRUE,
                              G_OBJECT (toggle), "visible", TRUE,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 500sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (outer_view), "collapsed", TRUE,
                              G_OBJECT (inner_view), "collapsed", TRUE,
                              G_OBJECT (box), "visible", TRUE,
                              G_OBJECT (toggle), "visible", TRUE,
                              G_OBJECT (button), "visible", TRUE,
                              NULL);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);

  gtk_window_present (GTK_WINDOW (window));
}

static void
complex_navigation_cb (void)
{
  GtkWidget *window, *sidebar, *content, *content_button, *view;
  AdwNavigationPage *sidebar_1, *sidebar_2, *content_1, *content_2;
  AdwNavigationPage *sidebar_page, *content_page;
  AdwBreakpoint *breakpoint;

  /* Sidebar */
  sidebar_1 = create_page_with_button ("sidebar", "Sidebar", "Open Page 2", "navigation.push::sidebar-2", NULL);
  sidebar_2 = create_page_with_button ("sidebar-2", "Sidebar 2", "Open Content", "navigation.push::content", &content_button);

  gtk_widget_set_visible (content_button, FALSE);

  sidebar = adw_navigation_view_new ();
  adw_navigation_view_add (ADW_NAVIGATION_VIEW (sidebar), sidebar_1);
  adw_navigation_view_add (ADW_NAVIGATION_VIEW (sidebar), sidebar_2);

  sidebar_page = adw_navigation_page_new (sidebar, "");

  /* Content */
  content_1 = create_page_with_button ("content", "Content", "Open Page 2", "navigation.push::content-2", NULL);
  content_2 = create_page ("content-2", "Content 2");

  content = adw_navigation_view_new ();
  adw_navigation_view_add (ADW_NAVIGATION_VIEW (content), content_1);
  adw_navigation_view_add (ADW_NAVIGATION_VIEW (content), content_2);

  content_page = adw_navigation_page_new_with_tag (content, "", "content");

  /* Window */
  view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (view), sidebar_page);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (view), content_page);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 400sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (view), "collapsed", TRUE,
                              G_OBJECT (content_button), "visible", TRUE,
                              NULL);

  window = adw_window_new ();
  gtk_window_set_title (GTK_WINDOW (window), "Navigation Sidebar");
  adw_window_set_content (ADW_WINDOW (window), view);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

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

  button = gtk_button_new_with_label ("Navigation");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (navigation_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Navigation (Inverted)");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (navigation_inverted_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Overlay");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (overlay_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Triple Pane (Mail)");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (triple_pane_mail_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Triple Pane (Feeds)");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (triple_pane_feeds_cb), NULL);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Complex Navigation");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect (button, "clicked", G_CALLBACK (complex_navigation_cb), NULL);
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
  gboolean done = FALSE;

  adw_init ();

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Split Views");
  gtk_window_set_child (GTK_WINDOW (window), create_content (GTK_WINDOW (window)));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
