#pragma once

#include <handy.h>

G_BEGIN_DECLS

#define HDY_TYPE_FLAP_DEMO_WINDOW (hdy_flap_demo_window_get_type())

G_DECLARE_FINAL_TYPE (HdyFlapDemoWindow, hdy_flap_demo_window, HDY, FLAP_DEMO_WINDOW, HdyWindow)

HdyFlapDemoWindow *hdy_flap_demo_window_new (void);

G_END_DECLS
