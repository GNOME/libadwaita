#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ADW_TYPE_FLAP_DEMO_WINDOW (adw_flap_demo_window_get_type())

G_DECLARE_FINAL_TYPE (AdwFlapDemoWindow, adw_flap_demo_window, ADW, FLAP_DEMO_WINDOW, AdwWindow)

AdwFlapDemoWindow *adw_flap_demo_window_new (void);

G_END_DECLS
