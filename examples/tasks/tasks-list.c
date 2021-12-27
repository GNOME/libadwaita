#include "tasks-list.h"

#include <gio/gio.h>

struct _TasksList
{
  GObject parent_instance;

  char *title;
  GListStore *tasks;
};

enum {
  PROP_0,
  PROP_TITLE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static void tasks_list_list_model_init (GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE (TasksList, tasks_list, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, tasks_list_list_model_init))

static void
tasks_list_finalize (GObject *object)
{
  TasksList *self = TASKS_LIST (object);

  g_clear_object (&self->tasks);
  g_clear_pointer (&self->title, g_free);

  G_OBJECT_CLASS (tasks_list_parent_class)->finalize (object);
}

static void
tasks_list_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  TasksList *self = TASKS_LIST (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, tasks_list_get_title (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
tasks_list_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  TasksList *self = TASKS_LIST (object);

  switch (prop_id) {
  case PROP_TITLE:
    tasks_list_set_title (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
tasks_list_class_init (TasksListClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = tasks_list_finalize;
  object_class->get_property = tasks_list_get_property;
  object_class->set_property = tasks_list_set_property;

  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         G_PARAM_CONSTRUCT |
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
tasks_list_init (TasksList *self)
{
  self->tasks = g_list_store_new (TASKS_TYPE_TASK);

  g_signal_connect_swapped (self->tasks, "items-changed",
                            G_CALLBACK (g_list_model_items_changed), self);
}

static gpointer
tasks_list_get_item (GListModel *model,
                     guint       position)
{
  TasksList *self = TASKS_LIST (model);

  return g_list_model_get_item (G_LIST_MODEL (self->tasks), position);
}

static guint
tasks_list_get_n_items (GListModel *model)
{
  TasksList *self = TASKS_LIST (model);

  return g_list_model_get_n_items (G_LIST_MODEL (self->tasks));
}

static GType
tasks_list_get_item_type (GListModel *model)
{
  return TASKS_TYPE_TASK;
}

static void
tasks_list_list_model_init (GListModelInterface *iface)
{
  iface->get_item = tasks_list_get_item;
  iface->get_n_items = tasks_list_get_n_items;
  iface->get_item_type = tasks_list_get_item_type;
}

TasksList *
tasks_list_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (TASKS_TYPE_LIST, "title", title, NULL);
}

const char *
tasks_list_get_title (TasksList *self)
{
  g_return_val_if_fail (TASKS_IS_LIST (self), NULL);

  return self->title;
}

void
tasks_list_set_title (TasksList  *self,
                      const char *title)
{
  g_return_if_fail (TASKS_IS_LIST (self));
  g_return_if_fail (title != NULL);

  if (!g_strcmp0 (title, self->title))
    return;

  g_clear_pointer (&self->title, g_free);
  self->title = g_strdup (title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

void
tasks_list_add (TasksList *self,
                TasksTask *task)
{
  g_return_if_fail (TASKS_IS_LIST (self));
  g_return_if_fail (TASKS_IS_TASK (task));

  g_list_store_append (self->tasks, task);
}

guint
tasks_list_remove (TasksList *self,
                   TasksTask *task)
{
  guint position;

  g_return_val_if_fail (TASKS_IS_LIST (self), 0);
  g_return_val_if_fail (TASKS_IS_TASK (task), 0);

  if (!g_list_store_find (self->tasks, task, &position))
    return G_MAXUINT;

  g_list_store_remove (self->tasks, position);

  return position;
}
