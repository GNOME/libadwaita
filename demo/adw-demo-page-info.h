#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_PAGE_INFO (adw_demo_page_info_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoPageInfo, adw_demo_page_info, ADW, DEMO_PAGE_INFO, GObject)

AdwDemoPageInfo *adw_demo_page_info_new (const char *title,
                                         const char *icon_name,
                                         GType       type);

G_END_DECLS
