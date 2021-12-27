#include "tasks-view.h"

#include "tasks-list.h"
#include "tasks-task.h"
#include "tasks-utils.h"

#include <glib/gi18n.h>

struct _TasksView
{
  AdwBin parent_instance;

  GtkListBox *tasks_list;
  GtkEditable *new_task_entry;
  GMenu *task_menu;

  TasksList *list;
  TasksTask *current_task;
  GtkFilterListModel *filter_model;
  GSettings *settings;
};

enum {
  PROP_0,
  PROP_LIST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (TasksView, tasks_view, ADW_TYPE_BIN)

static void
update_filter (TasksView *self)
{
  gboolean show_completed;

  if (!self->filter_model)
    return;

  show_completed = g_settings_get_boolean (self->settings, "show-completed");

  if (show_completed) {
    gtk_filter_list_model_set_filter (self->filter_model, NULL);
  } else {
    g_autoptr (GtkBoolFilter) filter = NULL;
    GtkExpression *expr = gtk_property_expression_new (TASKS_TYPE_TASK, NULL, "done");

    filter = gtk_bool_filter_new (expr);
    gtk_bool_filter_set_invert (filter, TRUE);

    gtk_filter_list_model_set_filter (self->filter_model, GTK_FILTER (filter));
  }
}

static void
notify_task_menu_visible_cb (TasksView  *self,
                             GParamSpec *pspec,
                             GtkWidget  *popover)
{
  GtkWidget *row = gtk_widget_get_ancestor (popover, ADW_TYPE_ACTION_ROW);

  if (gtk_widget_get_visible (popover)) {

    self->current_task = g_object_get_data (G_OBJECT (row), "task");

    gtk_widget_add_css_class (row, "has-open-popup");
  } else {
    gtk_widget_remove_css_class (row, "has-open-popup");
  }
}

static GtkWidget *
create_task_row (TasksTask *task,
                 TasksView *self)
{
  GtkWidget *row;
  GtkWidget *check;
  GtkWidget *menu_button;
  GtkPopover *popover;

  check = gtk_check_button_new ();
  gtk_widget_set_valign (check, GTK_ALIGN_CENTER);
  gtk_widget_set_can_focus (check, FALSE);

  menu_button = gtk_menu_button_new ();
  gtk_widget_set_valign (menu_button, GTK_ALIGN_CENTER);
  gtk_widget_add_css_class (menu_button, "flat");
  gtk_menu_button_set_icon_name (GTK_MENU_BUTTON (menu_button),
                                 "view-more-symbolic");
  gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (menu_button),
                                  G_MENU_MODEL (self->task_menu));

  popover = gtk_menu_button_get_popover (GTK_MENU_BUTTON (menu_button));
  g_signal_connect_swapped (popover, "notify::visible",
                            G_CALLBACK (notify_task_menu_visible_cb), self);

  row = adw_action_row_new ();
  adw_action_row_add_prefix (ADW_ACTION_ROW (row), check);
  adw_action_row_add_suffix (ADW_ACTION_ROW (row), menu_button);
  adw_action_row_set_activatable_widget (ADW_ACTION_ROW (row), check);

  g_object_bind_property (task, "title", row, "title",
                          G_BINDING_SYNC_CREATE);
  g_object_bind_property (task, "done", check, "active",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_set_data (G_OBJECT (row), "task", task);

  return row;
}

static void
tasks_changed_cb (TasksView *self)
{
  guint n_tasks = 0;

  if (self->list)
    n_tasks = g_list_model_get_n_items (G_LIST_MODEL (self->filter_model));

  gtk_widget_set_visible (GTK_WIDGET (self->tasks_list), n_tasks > 0);

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "task.rename", n_tasks > 0);
  gtk_widget_action_set_enabled (GTK_WIDGET (self), "task.delete", n_tasks > 0);
}

static void
set_list (TasksView *self,
          TasksList *list)
{
  if (self->list == list)
    return;

  if (self->list) {
    g_signal_handlers_disconnect_by_func (self->list,
                                          G_CALLBACK (tasks_changed_cb),
                                          self);

    gtk_list_box_bind_model (self->tasks_list, NULL, NULL, NULL, NULL);

    g_clear_object (&self->filter_model);
  }

  g_set_object (&self->list, list);

  if (self->list) {
    self->filter_model = gtk_filter_list_model_new (NULL, NULL);

    gtk_filter_list_model_set_model (self->filter_model, G_LIST_MODEL (list));

    update_filter (self);

    g_signal_connect_swapped (self->filter_model,
                              "items-changed",
                              G_CALLBACK (tasks_changed_cb),
                              self);

    gtk_list_box_bind_model (self->tasks_list,
                             G_LIST_MODEL (self->filter_model),
                             (GtkListBoxCreateWidgetFunc) create_task_row,
                             self,
                             NULL);
  }

  tasks_changed_cb (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LIST]);
}


static void
task_rename_dialog_cb (const char *value,
                       TasksView  *self)
{
  tasks_task_set_title (self->current_task, value);
}

static void
task_rename_cb (TasksView *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));

  tasks_show_dialog (GTK_WINDOW (root),
                     _("Rename Task"),
                     _("Rename"),
                     _("Name"),
                     tasks_task_get_title (self->current_task),
                     (TasksDialogFunc) task_rename_dialog_cb,
                     self);
}

static void
task_delete_cb (TasksView *self)
{
  g_assert (self->current_task);

  tasks_list_remove (self->list, self->current_task);

  self->current_task = NULL;
}

static void
new_task_activate_cb (TasksView *self)
{
  const char *title = gtk_editable_get_text (self->new_task_entry);
  g_autoptr (TasksTask) task = NULL;

  if (!title[0])
    return;

  task = tasks_task_new (title);

  tasks_list_add (self->list, task);

  gtk_editable_set_text (self->new_task_entry, "");
}

static void
tasks_view_dispose (GObject *object)
{
  TasksView *self = TASKS_VIEW (object);

  set_list (self, NULL);
  g_clear_object (&self->settings);

  G_OBJECT_CLASS (tasks_view_parent_class)->dispose (object);
}

static void
tasks_view_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  TasksView *self = TASKS_VIEW (object);

  switch (prop_id) {
  case PROP_LIST:
    g_value_set_object (value, self->list);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
tasks_view_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  TasksView *self = TASKS_VIEW (object);

  switch (prop_id) {
  case PROP_LIST:
    set_list (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
tasks_view_class_init (TasksViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = tasks_view_dispose;
  object_class->get_property = tasks_view_get_property;
  object_class->set_property = tasks_view_set_property;

  props[PROP_LIST] =
    g_param_spec_object ("list",
                         "List",
                         "List",
                         TASKS_TYPE_LIST,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/example/Tasks/tasks-view.ui");
  gtk_widget_class_bind_template_child (widget_class, TasksView, tasks_list);
  gtk_widget_class_bind_template_child (widget_class, TasksView, new_task_entry);
  gtk_widget_class_bind_template_child (widget_class, TasksView, task_menu);
  gtk_widget_class_bind_template_callback (widget_class, new_task_activate_cb);

  gtk_widget_class_install_action (widget_class, "task.rename", NULL,
                                   (GtkWidgetActionActivateFunc) task_rename_cb);
  gtk_widget_class_install_action (widget_class, "task.delete", NULL,
                                   (GtkWidgetActionActivateFunc) task_delete_cb);
}

static void
tasks_view_init (TasksView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->settings = g_settings_new ("org.example.Tasks");

  g_signal_connect_swapped (self->settings, "changed::show-completed",
                            G_CALLBACK (update_filter), self);
}

GtkWidget *
tasks_view_new (void)
{
  return g_object_new (TASKS_TYPE_VIEW, NULL);
}
