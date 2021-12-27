#include "tasks-window.h"

#include "tasks-list.h"
#include "tasks-manager.h"
#include "tasks-preferences-window.h"
#include "tasks-utils.h"
#include "tasks-view.h"

#include <glib/gi18n.h>

struct _TasksWindow
{
  AdwApplicationWindow parent_instance;

  GtkStack *empty_stack;
  AdwLeaflet *leaflet;
  GtkListBox *list;
  TasksView *view;

  TasksList *current_list;
};

enum {
  PROP_0,
  PROP_CURRENT_LIST,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (TasksWindow, tasks_window, ADW_TYPE_APPLICATION_WINDOW)

static GtkWidget *
create_list_row (TasksList   *list,
                 TasksWindow *self)
{
  GtkWidget *row, *label;

  label = gtk_label_new (NULL);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_xalign (GTK_LABEL (label), 0);

  g_object_bind_property (list, "title", label, "label",
                          G_BINDING_SYNC_CREATE);

  row = gtk_list_box_row_new ();
  gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), label);

  g_object_set_data (G_OBJECT (row), "list", list);

  return row;
}

static void
select_current_row (TasksWindow *self)
{
  guint position;
  GtkListBoxRow *row;

  if (!self->current_list)
    return;

  position = tasks_manager_get_position (tasks_manager_get_default (),
                                         self->current_list);
  row = gtk_list_box_get_row_at_index (self->list, position);

  gtk_list_box_select_row (self->list, row);
}

static void
view_back_cb (TasksWindow *self)
{
  adw_leaflet_navigate (self->leaflet, ADW_NAVIGATION_DIRECTION_BACK);
}

static void
set_current_list (TasksWindow *self,
                  TasksList   *list)
{
  self->current_list = list;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CURRENT_LIST]);

  select_current_row (self);
}

static void
list_new_dialog_cb (const char  *value,
                    TasksWindow *self)
{
  g_autoptr (TasksList) list = tasks_list_new (value);

  tasks_manager_add_list (tasks_manager_get_default (), list);

  set_current_list (self, list);

  adw_leaflet_navigate (self->leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
}

static void
list_new_cb (TasksWindow *self)
{
  tasks_show_dialog (GTK_WINDOW (self),
                     _("New List"),
                     _("Create"),
                     _("Name"),
                     "",
                     (TasksDialogFunc) list_new_dialog_cb,
                     self);
}

static void
list_rename_dialog_cb (const char  *value,
                       TasksWindow *self)
{
  tasks_list_set_title (self->current_list, value);
}

static void
list_rename_cb (TasksWindow *self)
{
  tasks_show_dialog (GTK_WINDOW (self),
                     _("Rename List"),
                     _("Rename"),
                     _("Name"),
                     tasks_list_get_title (self->current_list),
                     (TasksDialogFunc) list_rename_dialog_cb,
                     self);
}

static void
list_delete_cb (TasksWindow *self)
{
  TasksManager *manager = tasks_manager_get_default ();
  guint position;

  position = tasks_manager_remove_list (manager, self->current_list);

  if (position > 0) {
    g_autoptr (TasksList) list = NULL;

    list = g_list_model_get_item (G_LIST_MODEL (manager), position - 1);

    set_current_list (self, list);
  } else {
    set_current_list (self, NULL);
  }

  adw_leaflet_navigate (self->leaflet, ADW_NAVIGATION_DIRECTION_BACK);
}

static void
win_preferences_cb (TasksWindow *self)
{
  GtkWindow *window = tasks_preferences_window_new ();

  gtk_window_set_transient_for (window, GTK_WINDOW (self));
  gtk_window_present (window);
}

static void
win_about_cb (TasksWindow *self)
{
  gtk_show_about_dialog (GTK_WINDOW (self),
                         "title", _("About Tasks"),
                         "program-name", _("Tasks"),
                         "logo-icon-name", "application-x-executable",
                         "version", "1.2.3",
                         NULL);
}

static void
notify_leaflet_folded_cb (TasksWindow *self)
{
  if (adw_leaflet_get_folded (self->leaflet)) {
    gtk_list_box_set_selection_mode (self->list, GTK_SELECTION_NONE);
  } else {
    gtk_list_box_set_selection_mode (self->list, GTK_SELECTION_SINGLE);
    select_current_row (self);
  }
}

static void
row_activated_cb (TasksWindow   *self,
                  GtkListBoxRow *row)
{
  set_current_list (self, g_object_get_data (G_OBJECT (row), "list"));

  adw_leaflet_navigate (self->leaflet, ADW_NAVIGATION_DIRECTION_FORWARD);
}

static void
lists_changed_cb (TasksWindow *self)
{
  TasksManager *manager = tasks_manager_get_default ();
  guint n_lists = g_list_model_get_n_items (G_LIST_MODEL (manager));

  if (n_lists > 0)
    gtk_stack_set_visible_child_name (self->empty_stack, "main");
  else
    gtk_stack_set_visible_child_name (self->empty_stack, "empty");

  gtk_widget_action_set_enabled (GTK_WIDGET (self), "list.rename", n_lists > 0);
  gtk_widget_action_set_enabled (GTK_WIDGET (self), "list.delete", n_lists > 0);
}

static void
tasks_window_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  TasksWindow *self = TASKS_WINDOW (object);

  switch (prop_id) {
  case PROP_CURRENT_LIST:
    g_value_set_object (value, self->current_list);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static gboolean
tasks_window_close_request (GtkWindow *window)
{
  tasks_manager_save (tasks_manager_get_default ());

  return FALSE;
}

static void
tasks_window_class_init (TasksWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkWindowClass *window_class = GTK_WINDOW_CLASS (klass);

  object_class->get_property = tasks_window_get_property;
  window_class->close_request = tasks_window_close_request;

  props[PROP_CURRENT_LIST] =
    g_param_spec_object ("current-list",
                         "Current List",
                         "Current List",
                         TASKS_TYPE_LIST,
                         G_PARAM_READABLE |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/example/Tasks/tasks-window.ui");
  gtk_widget_class_bind_template_child (widget_class, TasksWindow, empty_stack);
  gtk_widget_class_bind_template_child (widget_class, TasksWindow, leaflet);
  gtk_widget_class_bind_template_child (widget_class, TasksWindow, list);
  gtk_widget_class_bind_template_child (widget_class, TasksWindow, view);
  gtk_widget_class_bind_template_callback (widget_class, notify_leaflet_folded_cb);
  gtk_widget_class_bind_template_callback (widget_class, row_activated_cb);

  gtk_widget_class_install_action (widget_class, "view.back", NULL,
                                   (GtkWidgetActionActivateFunc) view_back_cb);

  gtk_widget_class_install_action (widget_class, "list.new", NULL,
                                   (GtkWidgetActionActivateFunc) list_new_cb);
  gtk_widget_class_install_action (widget_class, "list.rename", NULL,
                                   (GtkWidgetActionActivateFunc) list_rename_cb);
  gtk_widget_class_install_action (widget_class, "list.delete", NULL,
                                   (GtkWidgetActionActivateFunc) list_delete_cb);

  gtk_widget_class_install_action (widget_class, "win.preferences", NULL,
                                   (GtkWidgetActionActivateFunc) win_preferences_cb);
  gtk_widget_class_install_action (widget_class, "win.about", NULL,
                                   (GtkWidgetActionActivateFunc) win_about_cb);

  g_type_ensure (TASKS_TYPE_LIST);
  g_type_ensure (TASKS_TYPE_VIEW);
}

static void
tasks_window_init (TasksWindow *self)
{
  TasksManager *manager = tasks_manager_get_default ();

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_list_box_bind_model (self->list,
                           G_LIST_MODEL (manager),
                           (GtkListBoxCreateWidgetFunc) create_list_row,
                           self,
                           NULL);

  g_signal_connect_swapped (manager,
                            "items-changed",
                            G_CALLBACK (lists_changed_cb),
                            self);

  if (g_list_model_get_n_items (G_LIST_MODEL (manager)) > 0) {
    g_autoptr (TasksList) list = NULL;

    list = g_list_model_get_item (G_LIST_MODEL (manager), 0);

    set_current_list (self, list);

    lists_changed_cb (self);
  }
}

GtkWindow *
tasks_window_new (GtkApplication *app)
{
  g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);

  return g_object_new (TASKS_TYPE_WINDOW,
                       "application", app,
                       NULL);
}
