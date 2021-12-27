#include "tasks-manager.h"

#include <gio/gio.h>

struct _TasksManager
{
  GObject parent_instance;

  GListStore *lists;
};

static void tasks_manager_list_model_init (GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE (TasksManager, tasks_manager, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, tasks_manager_list_model_init))

static TasksManager *default_instance = NULL;

static GVariant *
serialize_lists (TasksManager *self)
{
  GVariantBuilder builder;
  guint i, n_lists;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a(sa(sb))"));

  n_lists = g_list_model_get_n_items (G_LIST_MODEL (self->lists));
  for (i = 0; i < n_lists; i++) {
    g_autoptr (TasksList) list =
      g_list_model_get_item (G_LIST_MODEL (self->lists), i);
    int j, n_tasks;

    g_variant_builder_open (&builder, G_VARIANT_TYPE ("(sa(sb))"));
    g_variant_builder_add (&builder, "s", tasks_list_get_title (list));

    g_variant_builder_open (&builder, G_VARIANT_TYPE ("a(sb)"));

    n_tasks = g_list_model_get_n_items (G_LIST_MODEL (list));
    for (j = 0; j < n_tasks; j++) {
      g_autoptr (TasksTask) task =
        g_list_model_get_item (G_LIST_MODEL (list), j);

      g_variant_builder_open (&builder, G_VARIANT_TYPE ("(sb)"));
      g_variant_builder_add (&builder, "s", tasks_task_get_title (task));
      g_variant_builder_add (&builder, "b", tasks_task_get_done (task));
      g_variant_builder_close (&builder);
    }

    g_variant_builder_close (&builder);
    g_variant_builder_close (&builder);
  }

  return g_variant_ref (g_variant_builder_end (&builder));
}

static TasksList *
parse_list (GVariant *variant)
{
  TasksList *list = NULL;
  g_autoptr (GVariantIter) tasks_iter = NULL;
  g_autofree char *title = NULL;
  const char *task_title;
  gboolean done;

  g_variant_get (variant, ("(sa(sb))"), &title, &tasks_iter);

  list = tasks_list_new (title);

  while (g_variant_iter_loop (tasks_iter, "(sb)", &task_title, &done)) {
    g_autoptr (TasksTask) task = NULL;

    task = tasks_task_new (task_title);
    tasks_task_set_done (task, done);
    tasks_list_add (list, task);
  }

  return list;
}

static void
load (TasksManager *self)
{
  g_autoptr (GSettings) settings = g_settings_new ("org.example.Tasks");
  GVariant *variant = g_settings_get_value (settings, "tasks");
  GVariantIter iter;
  GVariant *child;

  g_variant_iter_init (&iter, variant);
  while ((child = g_variant_iter_next_value (&iter))) {
    g_autoptr (TasksList) list = parse_list (child);

    tasks_manager_add_list (self, list);

    g_variant_unref (child);
  }
}

static void
tasks_manager_finalize (GObject *object)
{
  TasksManager *self = TASKS_MANAGER (object);

  g_clear_object (&self->lists);

  G_OBJECT_CLASS (tasks_manager_parent_class)->finalize (object);
}

static void
tasks_manager_class_init (TasksManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = tasks_manager_finalize;
}

static void
tasks_manager_init (TasksManager *self)
{
  self->lists = g_list_store_new (TASKS_TYPE_LIST);

  g_signal_connect_swapped (self->lists, "items-changed",
                            G_CALLBACK (g_list_model_items_changed), self);

  load (self);
}

static gpointer
tasks_manager_get_item (GListModel *model,
                        guint       position)
{
  TasksManager *self = TASKS_MANAGER (model);

  return g_list_model_get_item (G_LIST_MODEL (self->lists), position);
}

static guint
tasks_manager_get_n_items (GListModel *model)
{
  TasksManager *self = TASKS_MANAGER (model);

  return g_list_model_get_n_items (G_LIST_MODEL (self->lists));
}

static GType
tasks_manager_get_item_type (GListModel *model)
{
  return TASKS_TYPE_LIST;
}

static void
tasks_manager_list_model_init (GListModelInterface *iface)
{
  iface->get_item = tasks_manager_get_item;
  iface->get_n_items = tasks_manager_get_n_items;
  iface->get_item_type = tasks_manager_get_item_type;
}

TasksManager *
tasks_manager_get_default (void)
{
  if (!default_instance)
    default_instance = g_object_new (TASKS_TYPE_MANAGER, NULL);

  return default_instance;
}

void
tasks_manager_add_list (TasksManager *self,
                        TasksList    *list)
{
  g_return_if_fail (TASKS_IS_MANAGER (self));
  g_return_if_fail (TASKS_IS_LIST (list));

  g_list_store_append (self->lists, list);
}

guint
tasks_manager_remove_list (TasksManager *self,
                           TasksList    *list)
{
  guint position;

  g_return_val_if_fail (TASKS_IS_MANAGER (self), 0);
  g_return_val_if_fail (TASKS_IS_LIST (list), 0);

  position = tasks_manager_get_position (self, list);
  g_list_store_remove (self->lists, position);

  return position;
}

guint
tasks_manager_get_position (TasksManager *self,
                            TasksList    *list)
{
  guint position;

  g_return_val_if_fail (TASKS_IS_MANAGER (self), 0);
  g_return_val_if_fail (TASKS_IS_LIST (list), 0);

  if (g_list_store_find (self->lists, list, &position))
    return position;

  return G_MAXUINT;
}

void
tasks_manager_save (TasksManager *self)
{
  g_autoptr (GVariant) variant = NULL;
  g_autoptr (GSettings) settings = NULL;

  g_return_if_fail (TASKS_IS_MANAGER (self));

  variant = serialize_lists (self);
  settings = g_settings_new ("org.example.Tasks");

  g_settings_set_value (settings, "tasks", variant);
}
