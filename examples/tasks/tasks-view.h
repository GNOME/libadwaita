#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define TASKS_TYPE_VIEW (tasks_view_get_type())

G_DECLARE_FINAL_TYPE (TasksView, tasks_view, TASKS, VIEW, AdwBin)

GtkWidget *tasks_view_new (void);

G_END_DECLS
