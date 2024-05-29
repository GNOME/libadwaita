#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_VIEW_SWITCHER_DEMO_DIALOG (adw_view_switcher_demo_dialog_get_type())

G_DECLARE_FINAL_TYPE (AdwViewSwitcherDemoDialog, adw_view_switcher_demo_dialog, ADW, VIEW_SWITCHER_DEMO_DIALOG, AdwDialog)

AdwDialog *adw_view_switcher_demo_dialog_new (void);

G_END_DECLS
