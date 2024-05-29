#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_OVERLAY_SPLIT_VIEW_DEMO_DIALOG (adw_overlay_split_view_demo_dialog_get_type())

G_DECLARE_FINAL_TYPE (AdwOverlaySplitViewDemoDialog, adw_overlay_split_view_demo_dialog, ADW, OVERLAY_SPLIT_VIEW_DEMO_DIALOG, AdwDialog)

AdwDialog *adw_overlay_split_view_demo_dialog_new (void);

G_END_DECLS
