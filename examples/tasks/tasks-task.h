#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define TASKS_TYPE_TASK (tasks_task_get_type())

G_DECLARE_FINAL_TYPE (TasksTask, tasks_task, TASKS, TASK, GObject)

TasksTask  *tasks_task_new       (const char *title);

const char *tasks_task_get_title (TasksTask  *self);
void        tasks_task_set_title (TasksTask  *self,
                                  const char *title);

gboolean    tasks_task_get_done  (TasksTask  *self);
void        tasks_task_set_done  (TasksTask  *self,
                                  gboolean    done);

G_END_DECLS
