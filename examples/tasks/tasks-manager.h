#pragma once

#include <glib-object.h>

#include "tasks-list.h"

G_BEGIN_DECLS

#define TASKS_TYPE_MANAGER (tasks_manager_get_type())

G_DECLARE_FINAL_TYPE (TasksManager, tasks_manager, TASKS, MANAGER, GObject)

TasksManager *tasks_manager_get_default (void);

void          tasks_manager_add_list     (TasksManager *self,
                                          TasksList    *list);
guint         tasks_manager_remove_list  (TasksManager *self,
                                          TasksList    *list);

guint         tasks_manager_get_position (TasksManager *self,
                                          TasksList    *list);

void          tasks_manager_save         (TasksManager *self);

G_END_DECLS
