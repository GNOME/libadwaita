#include "adw-demo-window.h"

#include "adw-demo-sidebar-item.h"

#include "pages/about/adw-demo-page-about.h"
#include "pages/alerts/adw-demo-page-alerts.h"
#include "pages/animations/adw-demo-page-animations.h"
#include "pages/avatar/adw-demo-page-avatar.h"
#include "pages/banners/adw-demo-page-banners.h"
#include "pages/bottom-sheets/adw-demo-page-bottom-sheets.h"
#include "pages/buttons/adw-demo-page-buttons.h"
#include "pages/carousel/adw-demo-page-carousel.h"
#include "pages/clamp/adw-demo-page-clamp.h"
#include "pages/lists/adw-demo-page-lists.h"
#include "pages/multi-layout/adw-demo-page-multi-layout.h"
#include "pages/navigation-view/adw-demo-page-navigation-view.h"
#include "pages/spinner/adw-demo-page-spinner.h"
#include "pages/split-views/adw-demo-page-split-views.h"
#include "pages/styles/adw-demo-page-styles.h"
#include "pages/tab-view/adw-demo-page-tab-view.h"
#include "pages/toasts/adw-demo-page-toasts.h"
#include "pages/toggles/adw-demo-page-toggles.h"
#include "pages/view-switcher/adw-demo-page-view-switcher.h"
#include "pages/welcome/adw-demo-page-welcome.h"
#include "pages/wrap-box/adw-demo-page-wrap-box.h"

struct _AdwDemoWindow
{
  AdwApplicationWindow parent_instance;

  AdwNavigationSplitView *split_view;
  AdwSidebar *sidebar;
  AdwSidebarItem *active_item;
};

G_DEFINE_FINAL_TYPE (AdwDemoWindow, adw_demo_window, ADW_TYPE_APPLICATION_WINDOW)

static void
update_content (AdwDemoWindow *self)
{
  AdwSidebarItem *item = adw_sidebar_get_selected_item (self->sidebar);
  GType type;
  GtkWidget *child;
  const char *title;

  if (self->active_item == item)
    return;

  self->active_item = item;

  if (!item) {
    adw_navigation_split_view_set_content (self->split_view,
                                           adw_navigation_page_new (NULL, ""));

    return;
  }

  g_assert (ADW_IS_DEMO_SIDEBAR_ITEM (item));

  type = adw_demo_sidebar_item_get_page_type (ADW_DEMO_SIDEBAR_ITEM (item));
  child = g_object_new (type, NULL);
  title = adw_sidebar_item_get_title (item);

  adw_navigation_split_view_set_content (self->split_view,
                                         adw_navigation_page_new (child, title));
}

static void
sidebar_activated_cb (AdwDemoWindow *self,
                      guint          index)
{
  update_content (self);
  adw_navigation_split_view_set_show_content (self->split_view, TRUE);
}

static void
adaptive_preview_cb (AdwDemoWindow *self)
{
  gboolean open = adw_application_window_get_adaptive_preview (ADW_APPLICATION_WINDOW (self));

  adw_application_window_set_adaptive_preview (ADW_APPLICATION_WINDOW (self), !open);
}

static void
adw_demo_window_class_init (AdwDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

#ifdef __APPLE__
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_w, GDK_META_MASK, "window.close", NULL);
#else
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_w, GDK_CONTROL_MASK, "window.close", NULL);
#endif

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/adw-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoWindow, sidebar);
  gtk_widget_class_bind_template_callback (widget_class, sidebar_activated_cb);

  gtk_widget_class_install_action (widget_class, "window.adaptive-preview", NULL, (GtkWidgetActionActivateFunc) adaptive_preview_cb);
}

static void
adw_demo_window_init (AdwDemoWindow *self)
{
  g_type_ensure (ADW_TYPE_DEMO_SIDEBAR_ITEM);

  g_type_ensure (ADW_TYPE_DEMO_PAGE_ABOUT);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_ALERTS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_ANIMATIONS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_AVATAR);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_BANNERS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_BOTTOM_SHEETS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_BUTTONS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_CAROUSEL);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_CLAMP);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_LISTS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_MULTI_LAYOUT);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_NAVIGATION_VIEW);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_SPINNER);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_SPLIT_VIEWS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_STYLES);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_TAB_VIEW);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_TOASTS);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_TOGGLES);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_VIEW_SWITCHER);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_WELCOME);
  g_type_ensure (ADW_TYPE_DEMO_PAGE_WRAP_BOX);

  gtk_widget_init_template (GTK_WIDGET (self));

  update_content (self);
}

GtkWindow *
adw_demo_window_new (GtkApplication *application)
{
  return g_object_new (ADW_TYPE_DEMO_WINDOW, "application", application, NULL);
}
