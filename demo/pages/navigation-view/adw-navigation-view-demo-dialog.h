#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_NAVIGATION_VIEW_DEMO_DIALOG (adw_navigation_view_demo_dialog_get_type())

G_DECLARE_FINAL_TYPE (AdwNavigationViewDemoDialog, adw_navigation_view_demo_dialog, ADW, NAVIGATION_VIEW_DEMO_DIALOG, AdwDialog)

AdwDialog *adw_navigation_view_demo_dialog_new (void);

G_END_DECLS
