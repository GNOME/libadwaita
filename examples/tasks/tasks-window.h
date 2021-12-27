#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define TASKS_TYPE_WINDOW (tasks_window_get_type())

G_DECLARE_FINAL_TYPE (TasksWindow, tasks_window, TASKS, WINDOW, AdwApplicationWindow)

GtkWindow *tasks_window_new (GtkApplication *app);

G_END_DECLS
