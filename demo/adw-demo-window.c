#include "adw-demo-window.h"

#include <glib/gi18n.h>

#include "adw-demo-page-info.h"

#include "pages/controls/adw-demo-page-controls.h"
#include "pages/lists/adw-demo-page-lists.h"
#include "pages/spring/adw-demo-page-spring.h"
#include "pages/stub/adw-demo-page-stub.h"

struct _AdwDemoWindow
{
  AdwApplicationWindow parent_instance;

  AdwLeaflet *leaflet;
  GtkSingleSelection *selection;
};

G_DEFINE_TYPE (AdwDemoWindow, adw_demo_window, ADW_TYPE_APPLICATION_WINDOW)

static GObject *
new_page_cb (AdwDemoWindow *self,
             GType          type)
{
  return g_object_ref (g_object_new (type, NULL));
}

static void
list_activate_cb (AdwDemoWindow *self)
{
  adw_leaflet_navigate (self->leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
}

static void
folded_notify_cb (AdwDemoWindow *self)
{
  gtk_widget_action_set_enabled (GTK_WIDGET (self), "leaflet.back",
                                 adw_leaflet_get_folded (self->leaflet));
}

static void
leaflet_back_activate_cb (AdwDemoWindow *self,
                          const char    *action_name,
                          GVariant      *parameter)
{
  adw_leaflet_navigate (self->leaflet, ADW_NAVIGATION_DIRECTION_BACK);
}

static void
adw_demo_window_class_init (AdwDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  // TODO: these should be on the leaflet instead
  gtk_widget_class_install_action (widget_class, "leaflet.back", NULL,
                                   (GtkWidgetActionActivateFunc) leaflet_back_activate_cb);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/adw-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, leaflet);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, selection);
  gtk_widget_class_bind_template_callback (widget_class, new_page_cb);
  gtk_widget_class_bind_template_callback (widget_class, list_activate_cb);
  gtk_widget_class_bind_template_callback (widget_class, folded_notify_cb);
}

#define ADD_PAGE(list, title, icon, type) g_list_store_append ((list), adw_demo_page_info_new ((title), (icon), (type)));
#define ADD_STUB(list, title, icon) ADD_PAGE ((list), (title), (icon), ADW_TYPE_DEMO_PAGE_STUB);

static void
adw_demo_window_init (AdwDemoWindow *self)
{
  GListStore *list;

  g_type_ensure (ADW_TYPE_DEMO_PAGE_INFO);

  gtk_widget_init_template (GTK_WIDGET (self));

  list = g_list_store_new (ADW_TYPE_DEMO_PAGE_INFO);

  ADD_STUB (list, _("Scrolling Window"), "page-scrolling-window-symbolic");
  ADD_STUB (list, _("Flat Window"), "page-flat-window-symbolic");
  ADD_STUB (list, _("Dialogs"), "page-dialogs-symbolic");
  ADD_STUB (list, _("Deck"), "page-deck-symbolic");
  ADD_STUB (list, _("View Switcher"), "page-view-switcher-symbolic");
  ADD_STUB (list, _("Leaflet"), "page-leaflet-symbolic");
  ADD_STUB (list, _("Sidebar"), "page-sidebar-symbolic");
  ADD_STUB (list, _("Flap"), "page-flap-symbolic");
  ADD_STUB (list, _("Carousel"), "page-carousel-symbolic");
  ADD_STUB (list, _("Tabs"), "page-tabs-symbolic");
  ADD_PAGE (list, _("Lists"), "page-lists-symbolic", ADW_TYPE_DEMO_PAGE_LISTS);
  ADD_STUB (list, _("Grids"), "page-grids-symbolic");
  ADD_PAGE (list, _("Controls"), "page-controls-symbolic", ADW_TYPE_DEMO_PAGE_CONTROLS);
  ADD_STUB (list, _("Toolbars"), "page-toolbars-symbolic");
  ADD_STUB (list, _("Search"), "page-search-symbolic");
  ADD_STUB (list, _("Feedback"), "page-feedback-symbolic");
  ADD_STUB (list, _("Status Page"), "page-status-page-symbolic");
  ADD_STUB (list, _("Avatars"), "page-avatars-symbolic");
  ADD_PAGE (list, _("Spring Animations"), "page-spring-symbolic", ADW_TYPE_DEMO_PAGE_SPRING);
  ADD_STUB (list, _("Menus"), "page-menus-symbolic");
  ADD_STUB (list, _("Preferences"), "page-preferences-symbolic");
  ADD_STUB (list, _("Keyboard Shortcuts"), "page-keyboard-shortcuts-symbolic");
  ADD_STUB (list, _("About"), "page-about-symbolic");

  gtk_single_selection_set_model (self->selection, G_LIST_MODEL (list));

  folded_notify_cb (self);
}

GtkWidget *
adw_demo_window_new (GtkApplication *application)
{
  return g_object_new (ADW_TYPE_DEMO_WINDOW,
                       "application", application,
                       NULL);
}
