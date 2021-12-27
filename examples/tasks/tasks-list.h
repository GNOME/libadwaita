#pragma once

#include <glib-object.h>

#include "tasks-task.h"

G_BEGIN_DECLS

#define TASKS_TYPE_LIST (tasks_list_get_type())

G_DECLARE_FINAL_TYPE (TasksList, tasks_list, TASKS, LIST, GObject)

TasksList  *tasks_list_new       (const char *title);

const char *tasks_list_get_title (TasksList  *self);
void        tasks_list_set_title (TasksList  *self,
                                  const char *title);

void        tasks_list_add       (TasksList  *self,
                                  TasksTask  *task);
guint       tasks_list_remove    (TasksList  *self,
                                  TasksTask  *task);

G_END_DECLS
