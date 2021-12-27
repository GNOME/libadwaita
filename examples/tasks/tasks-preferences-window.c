#include "tasks-preferences-window.h"

struct _TasksPreferencesWindow
{
  AdwPreferencesWindow parent_instance;

  GtkSwitch *show_completed_switch;
};

G_DEFINE_TYPE (TasksPreferencesWindow, tasks_preferences_window, ADW_TYPE_PREFERENCES_WINDOW)

static void
tasks_preferences_window_class_init (TasksPreferencesWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/example/Tasks/tasks-preferences-window.ui");
  gtk_widget_class_bind_template_child (widget_class, TasksPreferencesWindow, show_completed_switch);
}

static void
tasks_preferences_window_init (TasksPreferencesWindow *self)
{
  GSettings *settings;

  gtk_widget_init_template (GTK_WIDGET (self));

  settings = g_settings_new ("org.example.Tasks");

  g_settings_bind (settings, "show-completed",
                   self->show_completed_switch, "active",
                   G_SETTINGS_BIND_DEFAULT);
}

GtkWindow *
tasks_preferences_window_new (void)
{
  return g_object_new (TASKS_TYPE_PREFERENCES_WINDOW, NULL);
}
