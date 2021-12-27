#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef void (* TasksDialogFunc) (const char *value,
                                  gpointer    user_data);

void tasks_show_dialog (GtkWindow       *parent,
                        const char      *title,
                        const char      *accept_label,
                        const char      *placeholder,
                        const char      *value,
                        TasksDialogFunc  callback,
                        gpointer         user_data);

G_END_DECLS
