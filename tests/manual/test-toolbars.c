#include <adwaita.h>

static GtkWidget *
create_content (void)
{
  GtkWidget *box, *toolbar_view, *tab_view, *stack;

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  /* Left column */

  toolbar_view = adw_toolbar_view_new ();
  gtk_widget_set_hexpand (toolbar_view, TRUE);
  gtk_widget_set_size_request (toolbar_view, 360, -1);
  gtk_box_append (GTK_BOX (box), toolbar_view);

  adw_toolbar_view_set_top_bar_style (ADW_TOOLBAR_VIEW (toolbar_view),
                                      ADW_TOOLBAR_RAISED);

  adw_toolbar_view_set_bottom_bar_style (ADW_TOOLBAR_VIEW (toolbar_view),
                                         ADW_TOOLBAR_RAISED);

  /* Contents */
  {
    AdwTabPage *page;

    tab_view = GTK_WIDGET (adw_tab_view_new ());

    page = adw_tab_view_add_page (ADW_TAB_VIEW (tab_view),
                                  gtk_label_new ("Page 1"), NULL);
    adw_tab_page_set_title (page, "Page 1");

    page = adw_tab_view_add_page (ADW_TAB_VIEW (tab_view),
                                  gtk_label_new ("Page 2"), NULL);
    adw_tab_page_set_title (page, "Page 2");

    adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (toolbar_view), tab_view);
  }

  /* Header bar .default-decoration */
  {
    GtkWidget *headerbar = gtk_header_bar_new ();
    gtk_widget_add_css_class (headerbar, "default-decoration");

    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), headerbar);
  }

  /* Menu bar */
  {
    GtkWidget *menubar;
    GMenu *menu, *submenu;

    menu = g_menu_new ();

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "File", G_MENU_MODEL (submenu));

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "Edit", G_MENU_MODEL (submenu));
    g_object_unref (submenu);

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "View", G_MENU_MODEL (submenu));

    submenu = g_menu_new ();
    g_menu_append_submenu (menu, "Help", G_MENU_MODEL (submenu));

    menubar = gtk_popover_menu_bar_new_from_model (G_MENU_MODEL (menu));

    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), menubar);

    g_object_unref (menu);
  }

  /* .toolbar */
  {
    GtkWidget *toolbar, *button, *content, *spacer;

    toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (toolbar, "toolbar");

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("document-new-symbolic"));

    content = adw_button_content_new ();
    adw_button_content_set_icon_name (ADW_BUTTON_CONTENT (content),
                                      "document-open-symbolic");
    adw_button_content_set_label (ADW_BUTTON_CONTENT (content), "Open");

    button = gtk_button_new ();
    gtk_button_set_child (GTK_BUTTON (button), content);
    gtk_box_append (GTK_BOX (toolbar), button);

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("document-save-symbolic"));

    spacer = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class (spacer, "spacer");
    gtk_box_append (GTK_BOX (toolbar), spacer);

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("edit-undo-symbolic"));
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("edit-redo-symbolic"));

    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), toolbar);
  }

  /* Tab bar */
  {
    GtkWidget *tabbar, *button;

    tabbar = GTK_WIDGET (adw_tab_bar_new ());
    adw_tab_bar_set_view (ADW_TAB_BAR (tabbar), ADW_TAB_VIEW (tab_view));

    button = gtk_button_new_from_icon_name ("pan-down-symbolic");
    gtk_widget_add_css_class (button, "flat");
    adw_tab_bar_set_start_action_widget (ADW_TAB_BAR (tabbar), button);

    button = gtk_button_new_from_icon_name ("pan-down-symbolic");
    gtk_widget_add_css_class (button, "flat");
    adw_tab_bar_set_end_action_widget (ADW_TAB_BAR (tabbar), button);

    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), tabbar);
  }

  /* .toolbar */
  {
    GtkWidget *clamp;

    clamp = adw_clamp_new ();
    gtk_widget_add_css_class (clamp, "toolbar");
    adw_clamp_set_maximum_size (ADW_CLAMP (clamp), 400);
    adw_clamp_set_child (ADW_CLAMP (clamp), gtk_entry_new ());

    adw_toolbar_view_add_bottom_bar (ADW_TOOLBAR_VIEW (toolbar_view), clamp);
  }

  /* .toolbar */
  {
    GtkWidget *toolbar, *clamp, *button;

    toolbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_add_css_class (toolbar, "toolbar");
    gtk_box_set_homogeneous (GTK_BOX (toolbar), TRUE);

    button = adw_tab_button_new ();
    adw_tab_button_set_view (ADW_TAB_BUTTON (button), ADW_TAB_VIEW (tab_view));

    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("go-previous-symbolic"));
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("go-next-symbolic"));
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("starred-symbolic"));
    gtk_box_append (GTK_BOX (toolbar), button);
    gtk_box_append (GTK_BOX (toolbar),
                    gtk_button_new_from_icon_name ("open-menu-symbolic"));

    clamp = adw_clamp_new ();
    adw_clamp_set_maximum_size (ADW_CLAMP (clamp), 400);
    adw_clamp_set_child (ADW_CLAMP (clamp), toolbar);

    adw_toolbar_view_add_bottom_bar (ADW_TOOLBAR_VIEW (toolbar_view), clamp);
  }

  gtk_box_append (GTK_BOX (box), gtk_separator_new (GTK_ORIENTATION_VERTICAL));

  /* Right column */

  toolbar_view = adw_toolbar_view_new ();
  gtk_widget_set_hexpand (toolbar_view, TRUE);
  gtk_widget_set_size_request (toolbar_view, 360, -1);
  gtk_box_append (GTK_BOX (box), toolbar_view);

  adw_toolbar_view_set_top_bar_style (ADW_TOOLBAR_VIEW (toolbar_view),
                                      ADW_TOOLBAR_RAISED);

  adw_toolbar_view_set_bottom_bar_style (ADW_TOOLBAR_VIEW (toolbar_view),
                                         ADW_TOOLBAR_RAISED);

  /* Contents */
  {
    AdwViewStackPage *page;

    stack = adw_view_stack_new ();

    adw_view_stack_add_titled_with_icon (ADW_VIEW_STACK (stack),
                                         gtk_label_new ("Page 1"),
                                         NULL,
                                         "Page 1",
                                         "emblem-system-symbolic");

    page = adw_view_stack_add_titled_with_icon (ADW_VIEW_STACK (stack),
                                                gtk_label_new ("Page 2"),
                                                NULL,
                                                "Page 2",
                                                "emblem-system-symbolic");
    adw_view_stack_page_set_needs_attention (page, TRUE);
    adw_view_stack_page_set_badge_number (page, 3);

    adw_view_stack_add_titled_with_icon (ADW_VIEW_STACK (stack),
                                         gtk_label_new ("Page 3"),
                                         NULL,
                                         "Page 3",
                                         "emblem-system-symbolic");

    adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (toolbar_view), stack);
  }

  /* Header bar */
  {
    GtkWidget *headerbar = gtk_header_bar_new ();
    gtk_header_bar_pack_start (GTK_HEADER_BAR (headerbar),
                               gtk_button_new_from_icon_name ("edit-find-symbolic"));
    gtk_header_bar_pack_end (GTK_HEADER_BAR (headerbar),
                             gtk_button_new_from_icon_name ("open-menu-symbolic"));

    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), headerbar);
  }

  /* Search bar */
  {
    GtkWidget *entry, *clamp, *searchbar;

    entry = gtk_search_entry_new ();

    clamp = adw_clamp_new ();
    adw_clamp_set_maximum_size (ADW_CLAMP (clamp), 400);
    adw_clamp_set_child (ADW_CLAMP (clamp), entry);

    searchbar = gtk_search_bar_new ();
    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (searchbar), TRUE);
    gtk_search_bar_set_child (GTK_SEARCH_BAR (searchbar), clamp);
    adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (toolbar_view), searchbar);
  }

  /* Action bar */
  {
    GtkWidget *actionbar;

    actionbar = gtk_action_bar_new ();
    gtk_action_bar_set_revealed (GTK_ACTION_BAR (actionbar), TRUE);

    gtk_action_bar_pack_start (GTK_ACTION_BAR (actionbar),
                               gtk_button_new_with_label ("Export"));
    gtk_action_bar_pack_start (GTK_ACTION_BAR (actionbar),
                               gtk_button_new_with_label ("Link"));
    gtk_action_bar_pack_end (GTK_ACTION_BAR (actionbar),
                               gtk_button_new_from_icon_name ("view-more-symbolic"));

    adw_toolbar_view_add_bottom_bar (ADW_TOOLBAR_VIEW (toolbar_view), actionbar);
  }

  /* Switcher bar */
  {
    GtkWidget *switcher;

    switcher = adw_view_switcher_bar_new ();
    adw_view_switcher_bar_set_reveal (ADW_VIEW_SWITCHER_BAR (switcher), TRUE);
    adw_view_switcher_bar_set_stack (ADW_VIEW_SWITCHER_BAR (switcher),
                                     ADW_VIEW_STACK (stack));

    adw_toolbar_view_add_bottom_bar (ADW_TOOLBAR_VIEW (toolbar_view), switcher);
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

  window = adw_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Toolbars");
  adw_window_set_content (ADW_WINDOW (window), create_content ());
  gtk_window_set_default_size (GTK_WINDOW (window), 720, 400);
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
