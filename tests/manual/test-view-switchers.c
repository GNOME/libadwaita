#include <adwaita.h>

static AdwViewStack *
create_stack (void)
{
  AdwViewStack *stack = ADW_VIEW_STACK (adw_view_stack_new ());
  AdwViewStackPage *page;

  page = adw_view_stack_add_titled_with_icon (stack,
                                              adw_bin_new (),
                                              NULL,
                                              "Page 1",
                                              "adw-tab-icon-missing-symbolic");
  adw_view_stack_page_set_section_title (page, "Section 1");

  page = adw_view_stack_add_titled_with_icon (stack,
                                              adw_bin_new (),
                                              NULL,
                                              "Page 2",
                                              "adw-tab-icon-missing-symbolic");
  adw_view_stack_page_set_needs_attention (page, TRUE);

  page = adw_view_stack_add_titled_with_icon (stack,
                                              adw_bin_new (),
                                              NULL,
                                              "Page 3",
                                              "adw-tab-icon-missing-symbolic");
  adw_view_stack_page_set_badge_number (page, 3);
  adw_view_stack_page_set_starts_section (page, TRUE);
  adw_view_stack_page_set_section_title (page, "Section 2");

  page = adw_view_stack_add_titled_with_icon (stack,
                                              adw_bin_new (),
                                              NULL,
                                              "Page 4",
                                              "adw-tab-icon-missing-symbolic");
  adw_view_stack_page_set_needs_attention (page, TRUE);
  adw_view_stack_page_set_badge_number (page, 3);

  return stack;
}

static GtkWidget *
create_switcher_section (AdwViewStack          *stack,
                         AdwViewSwitcherPolicy  policy)
{
  GtkWidget *switcher = adw_view_switcher_new ();

  adw_view_switcher_set_stack (ADW_VIEW_SWITCHER (switcher), stack);
  adw_view_switcher_set_policy (ADW_VIEW_SWITCHER (switcher), policy);

  gtk_widget_set_halign (switcher, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (switcher, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand (switcher, TRUE);
  gtk_widget_set_vexpand (switcher, TRUE);

  return switcher;
}

static GtkWidget *
create_inline_section (AdwViewStack   *stack,
                       GtkOrientation  orientation,
                       gboolean        round,
                       gboolean        flat)
{
  GtkWidget *box;
  AdwInlineViewSwitcherDisplayMode mode;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  else
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);

  for (mode = ADW_INLINE_VIEW_SWITCHER_LABELS;
       mode <= ADW_INLINE_VIEW_SWITCHER_BOTH;
       mode++) {
    GtkWidget *switcher = adw_inline_view_switcher_new ();

    gtk_orientable_set_orientation (GTK_ORIENTABLE (switcher), orientation);

    adw_inline_view_switcher_set_stack (ADW_INLINE_VIEW_SWITCHER (switcher), stack);
    adw_inline_view_switcher_set_display_mode (ADW_INLINE_VIEW_SWITCHER (switcher), mode);

    gtk_widget_set_halign (switcher, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (switcher, GTK_ALIGN_CENTER);

    if (round)
      gtk_widget_add_css_class (switcher, "round");

    if (flat)
      gtk_widget_add_css_class (switcher, "flat");

    gtk_box_append (GTK_BOX (box), switcher);
  }

  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand (box, TRUE);
  gtk_widget_set_vexpand (box, TRUE);

  return box;
}

static GtkWidget *
create_sidebar (AdwViewStack *stack, AdwSidebarMode mode)
{
  GtkWidget *sidebar = adw_view_switcher_sidebar_new ();

  adw_view_switcher_sidebar_set_stack (ADW_VIEW_SWITCHER_SIDEBAR (sidebar), stack);
  adw_view_switcher_sidebar_set_mode (ADW_VIEW_SWITCHER_SIDEBAR (sidebar), mode);

  return sidebar;
}

static GtkWidget *
create_content (void)
{
  GtkBox *hbox, *vbox1, *vbox2;
  AdwViewStack *stack;
  AdwOverlaySplitView *split_view1, *split_view2;

  stack = create_stack ();

  split_view1 = ADW_OVERLAY_SPLIT_VIEW (adw_overlay_split_view_new ());
  split_view2 = ADW_OVERLAY_SPLIT_VIEW (adw_overlay_split_view_new ());

  adw_overlay_split_view_set_sidebar_position (split_view2, GTK_PACK_END);

  hbox = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 18));
  vbox1 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 18));
  vbox2 = GTK_BOX (gtk_box_new (GTK_ORIENTATION_VERTICAL, 18));

  gtk_widget_set_margin_top    (GTK_WIDGET (hbox), 18);
  gtk_widget_set_margin_bottom (GTK_WIDGET (hbox), 18);
  gtk_widget_set_margin_start  (GTK_WIDGET (hbox), 18);
  gtk_widget_set_margin_end    (GTK_WIDGET (hbox), 18);

  gtk_widget_set_visible (GTK_WIDGET (stack), FALSE);

  gtk_box_append (vbox1, create_switcher_section (stack, ADW_VIEW_SWITCHER_POLICY_WIDE));
  gtk_box_append (vbox1, create_inline_section (stack, GTK_ORIENTATION_HORIZONTAL, FALSE, FALSE));
  gtk_box_append (vbox1, create_inline_section (stack, GTK_ORIENTATION_HORIZONTAL, TRUE,  FALSE));
  gtk_box_append (vbox1, create_inline_section (stack, GTK_ORIENTATION_HORIZONTAL, FALSE, TRUE));
  gtk_box_append (vbox1, create_inline_section (stack, GTK_ORIENTATION_HORIZONTAL, TRUE,  TRUE));

  gtk_box_append (vbox2, create_switcher_section (stack, ADW_VIEW_SWITCHER_POLICY_NARROW));
  gtk_box_append (vbox2, create_inline_section (stack, GTK_ORIENTATION_VERTICAL, FALSE, FALSE));
  gtk_box_append (vbox2, create_inline_section (stack, GTK_ORIENTATION_VERTICAL, TRUE,  FALSE));
  gtk_box_append (vbox2, create_inline_section (stack, GTK_ORIENTATION_VERTICAL, FALSE, TRUE));
  gtk_box_append (vbox2, create_inline_section (stack, GTK_ORIENTATION_VERTICAL, TRUE,  TRUE));

  gtk_box_append (hbox, GTK_WIDGET (stack));
  gtk_box_append (hbox, GTK_WIDGET (vbox1));
  gtk_box_append (hbox, GTK_WIDGET (vbox2));

  adw_overlay_split_view_set_sidebar (split_view1, create_sidebar (stack, ADW_SIDEBAR_MODE_SIDEBAR));
  adw_overlay_split_view_set_content (split_view1, GTK_WIDGET (split_view2));

  adw_overlay_split_view_set_sidebar (split_view2, create_sidebar (stack, ADW_SIDEBAR_MODE_PAGE));
  adw_overlay_split_view_set_content (split_view2, GTK_WIDGET (hbox));

  return GTK_WIDGET (split_view1);
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
  gtk_window_set_title (GTK_WINDOW (window), "View Switchers");
  gtk_window_set_child (GTK_WINDOW (window), create_content ());
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
