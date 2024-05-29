#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_DEMO_PREFERENCES_DIALOG (adw_demo_preferences_dialog_get_type())

G_DECLARE_FINAL_TYPE (AdwDemoPreferencesDialog, adw_demo_preferences_dialog, ADW, DEMO_PREFERENCES_DIALOG, AdwPreferencesDialog)

AdwDialog *adw_demo_preferences_dialog_new (void);

G_END_DECLS
