#include <adwaita.h>
#include <glib/gi18n.h>

static void
add_item_cb (GtkStringList *list)
{
  guint n = g_list_model_get_n_items (G_LIST_MODEL (list));

  gtk_string_list_take (list, g_strdup_printf ("Item %u", n + 1));
}

static void
remove_item_cb (AdwSidebarItem *item)
{
  AdwSidebarSection *section = adw_sidebar_item_get_section (item);
  GtkStringList *list = g_object_get_data (G_OBJECT (section), "list");
  guint index = adw_sidebar_item_get_section_index (item);

  gtk_string_list_remove (list, index);
}

static AdwSidebarItem *
create_item_cb (gpointer object,
                gpointer user_data)
{
  GtkStringObject *obj = GTK_STRING_OBJECT (object);
  AdwSidebarItem *item = adw_sidebar_item_new (gtk_string_object_get_string (obj));
  GtkWidget *button = gtk_button_new_from_icon_name ("user-trash-symbolic");

  gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
  gtk_widget_add_css_class (button, "flat");
  gtk_widget_add_css_class (button, "sidebar-button");

  g_signal_connect_swapped (button, "clicked", G_CALLBACK (remove_item_cb), item);

  adw_sidebar_item_set_suffix (item, button);

  return item;
}

static GtkWidget *
create_sidebar (void)
{
  AdwSidebar *sidebar = ADW_SIDEBAR (adw_sidebar_new ());

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    AdwSidebarItem *item;

    item = adw_sidebar_item_new ("Text Only");
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Lorem ipsum dolor sit amet, consectetur adipiscing elit");
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Tooltip");
    adw_sidebar_item_set_tooltip (item, "Lorem ipsum dolor sit amet, consectetur adipiscing elit");
    adw_sidebar_section_append (section, item);

    adw_sidebar_append (sidebar, section);
  }

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    AdwSidebarItem *item;

    item = adw_sidebar_item_new ("Item 1");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Item 2");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_section_append (section, item);

    adw_sidebar_append (sidebar, section);
  }

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    AdwSidebarItem *item;

    adw_sidebar_section_set_title (section, "Subtitles");

    item = adw_sidebar_item_new ("Short Subtitle");
    adw_sidebar_item_set_subtitle (item, "Subtitle 1");
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Long Subtitle");
    adw_sidebar_item_set_subtitle (item, "Lorem ipsum dolor sit amet, consectetur adipiscing elit");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_section_append (section, item);

    adw_sidebar_append (sidebar, section);
  }

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    AdwSidebarItem *item;

    adw_sidebar_section_set_title (section, "Icons and Widgets");

    item = adw_sidebar_item_new ("Icon Name");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Paintable");
    adw_sidebar_item_set_icon_paintable (item, GDK_PAINTABLE (adw_spinner_paintable_new (GTK_WIDGET (sidebar))));
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Prefix");
    adw_sidebar_item_set_prefix (item, adw_spinner_new ());
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Prefix and Icon");
    adw_sidebar_item_set_prefix (item, adw_spinner_new ());
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Suffix");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_item_set_suffix (item, adw_spinner_new ());
    adw_sidebar_section_append (section, item);

    adw_sidebar_append (sidebar, section);
  }

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    AdwSidebarItem *item;

    adw_sidebar_section_set_title (section, "State");

    item = adw_sidebar_item_new ("Disabled");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_item_set_enabled (item, FALSE);
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Hidden");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_item_set_visible (item, FALSE);
    adw_sidebar_section_append (section, item);

    adw_sidebar_append (sidebar, section);
  }

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    AdwSidebarItem *item;

    adw_sidebar_section_set_title (section, "Hidden");

    item = adw_sidebar_item_new ("Item 1");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_item_set_visible (item, FALSE);
    adw_sidebar_section_append (section, item);

    item = adw_sidebar_item_new ("Item 2");
    adw_sidebar_item_set_icon_name (item, "list-add-symbolic");
    adw_sidebar_item_set_visible (item, FALSE);
    adw_sidebar_section_append (section, item);

    adw_sidebar_append (sidebar, section);
  }

  {
    AdwSidebarSection *section = adw_sidebar_section_new ();
    GtkWidget *button = gtk_button_new_from_icon_name ("list-add-symbolic");
    const char *items[4] = { "Item 1", "Item 2", "Item 3", NULL };
    GtkStringList *list = gtk_string_list_new (items);

    gtk_widget_add_css_class (button, "flat");
    g_signal_connect_swapped (button, "clicked", G_CALLBACK (add_item_cb), list);
    adw_sidebar_section_set_suffix (section, button);

    adw_sidebar_section_set_title (section, "Model");
    adw_sidebar_section_bind_model (section, G_LIST_MODEL (list), create_item_cb, NULL, NULL);

    g_object_set_data (G_OBJECT (section), "list", list);

    adw_sidebar_append (sidebar, section);
  }

  return GTK_WIDGET (sidebar);
}

static void
sidebar_activated_cb (AdwSidebar        *sidebar,
                      guint              index,
                      AdwNavigationPage *page)
{
  AdwSidebarItem *item = adw_sidebar_get_item (sidebar, index);

  adw_navigation_page_set_title (page, adw_sidebar_item_get_title (item));
  gtk_widget_activate_action (GTK_WIDGET (sidebar), "navigation.push", "s", "content");
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
  GtkWidget *window, *split_view, *sidebar;
  GtkWidget *sidebar_toolbar_view, *content_toolbar_view;
  AdwNavigationPage *sidebar_page, *content_page;
  AdwSidebarItem *first_item;
  AdwBreakpoint *breakpoint;
  gboolean done = FALSE;

  adw_init ();

  sidebar = create_sidebar ();

  sidebar_toolbar_view = adw_toolbar_view_new ();
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (sidebar_toolbar_view),
                                adw_header_bar_new ());
  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (sidebar_toolbar_view), sidebar);
  sidebar_page = adw_navigation_page_new (sidebar_toolbar_view, "Sidebar");

  content_toolbar_view = adw_toolbar_view_new ();
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (content_toolbar_view),
                                adw_header_bar_new ());
  content_page = adw_navigation_page_new_with_tag (content_toolbar_view, "", "content");

  g_signal_connect (sidebar, "activated", G_CALLBACK (sidebar_activated_cb), content_page);

  first_item = adw_sidebar_get_item (ADW_SIDEBAR (sidebar), 0);
  adw_navigation_page_set_title (content_page, adw_sidebar_item_get_title (first_item));

  split_view = adw_navigation_split_view_new ();
  adw_navigation_split_view_set_sidebar (ADW_NAVIGATION_SPLIT_VIEW (split_view), sidebar_page);
  adw_navigation_split_view_set_content (ADW_NAVIGATION_SPLIT_VIEW (split_view), content_page);

  breakpoint = adw_breakpoint_new (adw_breakpoint_condition_parse ("max-width: 400sp"));
  adw_breakpoint_add_setters (breakpoint,
                              G_OBJECT (split_view), "collapsed", TRUE,
                              G_OBJECT (sidebar), "mode", ADW_SIDEBAR_MODE_PAGE,
                              NULL);

  window = adw_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Navigation Sidebar");
  adw_window_set_content (ADW_WINDOW (window), split_view);
  adw_window_add_breakpoint (ADW_WINDOW (window), breakpoint);
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
