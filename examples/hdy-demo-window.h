#pragma once

#include <handy.h>

G_BEGIN_DECLS

#define HDY_TYPE_DEMO_WINDOW (hdy_demo_window_get_type())

G_DECLARE_FINAL_TYPE (HdyDemoWindow, hdy_demo_window, HDY, DEMO_WINDOW, HdyApplicationWindow)

HdyDemoWindow *hdy_demo_window_new (GtkApplication *application);

G_END_DECLS
