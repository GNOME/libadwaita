#include "tasks-task.h"

struct _TasksTask
{
  GObject parent_instance;

  char *title;
  gboolean done;
};

enum {
  PROP_0,
  PROP_TITLE,
  PROP_DONE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (TasksTask, tasks_task, G_TYPE_OBJECT)

static void
tasks_task_finalize (GObject *object)
{
  TasksTask *self = TASKS_TASK (object);

  g_clear_pointer (&self->title, g_free);

  G_OBJECT_CLASS (tasks_task_parent_class)->finalize (object);
}

static void
tasks_task_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  TasksTask *self = TASKS_TASK (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, tasks_task_get_title (self));
    break;
  case PROP_DONE:
    g_value_set_boolean (value, tasks_task_get_done (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
tasks_task_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  TasksTask *self = TASKS_TASK (object);

  switch (prop_id) {
  case PROP_TITLE:
    tasks_task_set_title (self, g_value_get_string (value));
    break;
  case PROP_DONE:
    tasks_task_set_done (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
tasks_task_class_init (TasksTaskClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = tasks_task_finalize;
  object_class->get_property = tasks_task_get_property;
  object_class->set_property = tasks_task_set_property;

  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         G_PARAM_CONSTRUCT |
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_DONE] =
    g_param_spec_boolean ("done",
                          "Done",
                          "Done",
                          FALSE,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
tasks_task_init (TasksTask *self)
{
}

TasksTask *
tasks_task_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (TASKS_TYPE_TASK, "title", title, NULL);
}

const char *
tasks_task_get_title (TasksTask *self)
{
  g_return_val_if_fail (TASKS_IS_TASK (self), NULL);

  return self->title;
}

void
tasks_task_set_title (TasksTask  *self,
                      const char *title)
{
  g_return_if_fail (TASKS_IS_TASK (self));
  g_return_if_fail (title != NULL);

  if (!g_strcmp0 (title, self->title))
    return;

  g_clear_pointer (&self->title, g_free);
  self->title = g_strdup (title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

gboolean
tasks_task_get_done (TasksTask *self)
{
  g_return_val_if_fail (TASKS_IS_TASK (self), FALSE);

  return self->done;
}

void
tasks_task_set_done (TasksTask *self,
                     gboolean   done)
{
  g_return_if_fail (TASKS_IS_TASK (self));

  if (done == self->done)
    return;

  self->done = done;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DONE]);
}
