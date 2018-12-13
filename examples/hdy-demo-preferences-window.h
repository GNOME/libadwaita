#pragma once

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

G_BEGIN_DECLS

#define HDY_TYPE_DEMO_PREFERENCES_WINDOW (hdy_demo_preferences_window_get_type())

G_DECLARE_FINAL_TYPE (HdyDemoPreferencesWindow, hdy_demo_preferences_window, HDY, DEMO_PREFERENCES_WINDOW, HdyPreferencesWindow)

HdyDemoPreferencesWindow *hdy_demo_preferences_window_new (void);

G_END_DECLS
