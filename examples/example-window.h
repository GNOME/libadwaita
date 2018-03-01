#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EXAMPLE_TYPE_WINDOW (example_window_get_type())

G_DECLARE_FINAL_TYPE (ExampleWindow, example_window, EXAMPLE, WINDOW, GtkApplicationWindow)

ExampleWindow *example_window_new (GtkApplication *application);

G_END_DECLS
