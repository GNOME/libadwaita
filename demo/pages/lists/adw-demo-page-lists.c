#include "adw-demo-page-lists.h"

#include <glib/gi18n.h>

struct _AdwDemoPageLists
{
  AdwDemoPage parent_instance;

  GtkWindow *sub_view;
};

G_DEFINE_TYPE (AdwDemoPageLists, adw_demo_page_lists, ADW_TYPE_DEMO_PAGE)

static void
sub_view_activate_cb (AdwDemoPageLists *self,
                      const char       *action_name,
                      GVariant         *parameter)
{
  GtkWindow *window = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (self)));

  gtk_window_set_transient_for (self->sub_view, window);
  gtk_window_present (self->sub_view);
}

static void
external_link_activate_cb (AdwDemoPageLists *self,
                           const char       *action_name,
                           GVariant         *parameter)
{
  GtkWindow *window = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (self)));

  gtk_show_uri (window, _("https://os.gnome.org"), GDK_CURRENT_TIME);
}

static void
adw_demo_page_lists_class_init (AdwDemoPageListsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_install_action (widget_class, "page.sub-view", NULL,
                                   (GtkWidgetActionActivateFunc) sub_view_activate_cb);
  gtk_widget_class_install_action (widget_class, "page.external-link", NULL,
                                   (GtkWidgetActionActivateFunc) external_link_activate_cb);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/lists/adw-demo-page-lists.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPageLists, sub_view);
}

static void
adw_demo_page_lists_init (AdwDemoPageLists *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
