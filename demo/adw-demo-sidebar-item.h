#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_SIDEBAR_ITEM (adw_demo_sidebar_item_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoSidebarItem, adw_demo_sidebar_item, ADW, DEMO_SIDEBAR_ITEM, AdwSidebarItem)

GType adw_demo_sidebar_item_get_page_type (AdwDemoSidebarItem *self);

G_END_DECLS
