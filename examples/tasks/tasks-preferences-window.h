#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define TASKS_TYPE_PREFERENCES_WINDOW (tasks_preferences_window_get_type())

G_DECLARE_FINAL_TYPE (TasksPreferencesWindow, tasks_preferences_window, TASKS, PREFERENCES_WINDOW, AdwPreferencesWindow)

GtkWindow *tasks_preferences_window_new (void);

G_END_DECLS
