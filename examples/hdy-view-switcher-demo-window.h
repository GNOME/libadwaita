#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_VIEW_SWITCHER_DEMO_WINDOW (hdy_view_switcher_demo_window_get_type())

G_DECLARE_FINAL_TYPE (HdyViewSwitcherDemoWindow, hdy_view_switcher_demo_window, HDY, VIEW_SWITCHER_DEMO_WINDOW, GtkWindow)

HdyViewSwitcherDemoWindow *hdy_view_switcher_demo_window_new (void);

G_END_DECLS
