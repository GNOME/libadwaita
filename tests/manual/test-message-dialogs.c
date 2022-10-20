#include <adwaita.h>
#include <glib/gi18n.h>

static void
response_cb (AdwMessageDialog *dialog,
             const char       *response)
{
  g_message ("Response: %s", response);
}

static void
response_text_cb (AdwMessageDialog *dialog,
                   const char       *response)
{
  GtkWidget *entry = adw_message_dialog_get_extra_child (dialog);
  const char *text;

  g_assert (GTK_IS_EDITABLE (entry));

  text = gtk_editable_get_text (GTK_EDITABLE (entry));

  g_message ("Response: %s, text: %s", response, text);
}

/* This dialog will always have horizontal buttons */
static void
simple_cb (GtkWindow *parent)
{
  GtkWidget *dialog =
    adw_message_dialog_new (parent,
                            _("Replace File?"),
                            _("A file named “example.png” already exists. Do you want to replace it?"));

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
                                    "cancel",  _("_Cancel"),
                                    "replace", _("_Replace"),
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "replace", ADW_RESPONSE_DESTRUCTIVE);

  adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG (dialog), "cancel");
  adw_message_dialog_set_close_response (ADW_MESSAGE_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  gtk_window_present (GTK_WINDOW (dialog));
}

/* This dialog will have horizontal or vertical buttons, depending on the available room */
static void
adaptive_cb (GtkWindow *parent)
{
  GtkWidget *dialog =
    adw_message_dialog_new (parent,
                            _("Save Changes?"),
                            _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
                                    "cancel",  _("_Cancel"),
                                    "discard", _("_Discard Changes"),
                                    "save",    _("_Save"),
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "discard", ADW_RESPONSE_DESTRUCTIVE);
  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "save", ADW_RESPONSE_SUGGESTED);

  adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG (dialog), "save");
  adw_message_dialog_set_close_response (ADW_MESSAGE_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  gtk_window_present (GTK_WINDOW (dialog));
}

/* This dialog will always have vertical buttons */
static void
wide_cb (GtkWindow *parent)
{
  GtkWidget *dialog =
    adw_message_dialog_new (parent,
                            _("Do you want to empty the wastebasket before you unmount?"),
                            _("In order to regain the free space on the volume the wastebasket must be emptied. All deleted items on the volume will be permanently lost."));

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
                                    "ignore", _("Do _not Empty Wastebasket"),
                                    "cancel", _("_Cancel"),
                                    "empty",  _("_Empty Wastebasket"),
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "empty", ADW_RESPONSE_DESTRUCTIVE);

  adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG (dialog), "cancel");
  adw_message_dialog_set_close_response (ADW_MESSAGE_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  gtk_window_present (GTK_WINDOW (dialog));
}

static void
entry_changed_cb (GtkEditable      *editable,
                  AdwMessageDialog *dialog)
{
  const char *text = gtk_editable_get_text (editable);

  if (text && *text) {
    adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG (dialog), "add", TRUE);
    gtk_widget_remove_css_class (GTK_WIDGET (editable), "error");
  } else {
    adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG (dialog), "add", FALSE);
    gtk_widget_add_css_class (GTK_WIDGET (editable), "error");
  }
}

static void
child_cb (GtkWindow *parent)
{
  GtkWidget *dialog, *entry;

  dialog =
    adw_message_dialog_new (parent,
                            _("Add New Profile"),
                            _("Enter name of the new profile"));

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
                                    "cancel",  _("_Cancel"),
                                    "add", _("_Add"),
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "add", ADW_RESPONSE_SUGGESTED);

  adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG (dialog), "add");
  adw_message_dialog_set_close_response (ADW_MESSAGE_DIALOG (dialog), "cancel");

  adw_message_dialog_set_response_enabled (ADW_MESSAGE_DIALOG (dialog), "add", FALSE);

  entry = gtk_entry_new ();
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Name"));
  gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
  g_signal_connect (entry, "changed", G_CALLBACK (entry_changed_cb), dialog);
  adw_message_dialog_set_extra_child (ADW_MESSAGE_DIALOG (dialog), entry);

  g_signal_connect (dialog, "response::add", G_CALLBACK (response_text_cb), NULL);
  g_signal_connect (dialog, "response::cancel", G_CALLBACK (response_cb), NULL);

  gtk_window_present (GTK_WINDOW (dialog));
}

static GtkWidget *
create_content (GtkWindow *parent)
{
  GtkWidget *box, *button;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
  gtk_widget_set_margin_top (box, 48);
  gtk_widget_set_margin_bottom (box, 48);
  gtk_widget_set_margin_start (box, 48);
  gtk_widget_set_margin_end (box, 48);
  gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (box, GTK_ALIGN_CENTER);

  button = gtk_button_new_with_label ("Simple Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (simple_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Adaptive Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (adaptive_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Wide Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (wide_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Extra Child");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (child_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  return box;
}

static void
close_cb (gboolean *done)
{
  *done = TRUE;
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *window;
  gboolean done = FALSE;

  adw_init ();

  window = gtk_window_new ();
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Message Dialogs");
  gtk_window_set_child (GTK_WINDOW (window), create_content (GTK_WINDOW (window)));
  gtk_widget_set_size_request (window, 360, -1);
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
