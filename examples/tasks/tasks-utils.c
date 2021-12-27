#include "tasks-utils.h"

#include <glib/gi18n.h>

static void
dialog_response_cb (GtkDialog       *dialog,
                    GtkResponseType  response,
                    gpointer         user_data)
{
  TasksDialogFunc callback = g_object_get_data (G_OBJECT (dialog), "callback");
  GtkEditable *entry = g_object_get_data (G_OBJECT (dialog), "entry");

  gtk_window_destroy (GTK_WINDOW (dialog));

  if (response != GTK_RESPONSE_ACCEPT)
    return;

  callback (gtk_editable_get_text (entry), user_data);
}

static void
entry_changed_cb (GtkDialog *dialog,
                  GtkWidget *entry)
{
  GtkWidget *button;
  const char *text;
  gboolean empty;

  text = gtk_editable_get_text (GTK_EDITABLE (entry));
  button = gtk_dialog_get_widget_for_response (dialog, GTK_RESPONSE_ACCEPT);
  empty = !(text && text[0]);

  gtk_widget_set_sensitive (button, !empty);

  if (empty)
    gtk_widget_add_css_class (entry, "error");
  else
    gtk_widget_remove_css_class (entry, "error");
}

void
tasks_show_dialog (GtkWindow       *parent,
                   const char      *title,
                   const char      *accept_label,
                   const char      *placeholder,
                   const char      *value,
                   TasksDialogFunc  callback,
                   gpointer         user_data)
{
  GtkWidget *dialog, *content_area, *entry;

  dialog = gtk_dialog_new_with_buttons (title,
                                        parent,
                                        GTK_DIALOG_MODAL |
                                        GTK_DIALOG_DESTROY_WITH_PARENT |
                                        GTK_DIALOG_USE_HEADER_BAR,
                                        _("Cancel"),
                                        GTK_RESPONSE_CANCEL,
                                        accept_label,
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  entry = gtk_entry_new ();
  gtk_widget_set_margin_top (entry, 12);
  gtk_widget_set_margin_bottom (entry, 12);
  gtk_widget_set_margin_start (entry, 12);
  gtk_widget_set_margin_end (entry, 12);
  gtk_editable_set_text (GTK_EDITABLE (entry), value);
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry), placeholder);
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);

  g_signal_connect_swapped (entry, "changed",
                            G_CALLBACK (entry_changed_cb), dialog);

  entry_changed_cb (GTK_DIALOG (dialog), entry);
  gtk_widget_remove_css_class (entry, "error");

  gtk_box_append (GTK_BOX (content_area), entry);
  g_object_set_data (G_OBJECT (dialog), "entry", entry);
  g_object_set_data (G_OBJECT (dialog), "callback", callback);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (dialog_response_cb), user_data);

  gtk_window_present (GTK_WINDOW (dialog));
}
