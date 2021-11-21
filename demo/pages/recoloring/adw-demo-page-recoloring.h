#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_PAGE_RECOLORING (adw_demo_page_recoloring_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoPageRecoloring, adw_demo_page_recoloring, ADW, DEMO_PAGE_RECOLORING, AdwBin)

AdwDemoPageRecoloring *adw_demo_page_recoloring_new (void);

G_END_DECLS
