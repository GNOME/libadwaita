#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_STYLE_DEMO_DIALOG (adw_style_demo_dialog_get_type())

G_DECLARE_FINAL_TYPE (AdwStyleDemoDialog, adw_style_demo_dialog, ADW, STYLE_DEMO_DIALOG, AdwDialog)

AdwDialog *adw_style_demo_dialog_new (void);

G_END_DECLS
