#include <adwaita.h>
#include <glib/gi18n.h>

static void
response_cb (AdwAlertDialog *dialog,
             const char     *response)
{
  g_message ("Response: %s", response);
}

static void
response_text_cb (AdwAlertDialog *dialog,
                  const char     *response)
{
  GtkWidget *entry = g_object_get_data (G_OBJECT (dialog), "entry");
  const char *text;

  g_assert (GTK_IS_EDITABLE (entry));

  text = gtk_editable_get_text (GTK_EDITABLE (entry));

  g_message ("Response: %s, text: %s", response, text);
}

static void
dialog_cb (AdwAlertDialog *dialog,
           GAsyncResult   *result,
           gpointer        user_data)
{
  const char *response = adw_alert_dialog_choose_finish (dialog, result);

  g_message ("Response: %s", response);
}

/* This dialog will always have horizontal buttons */
static void
simple_cb (GtkWidget *parent)
{
  AdwDialog *dialog =
    adw_alert_dialog_new (_("Replace File?"),
                          _("A file named “example.png” already exists. Do you want to replace it?"));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "replace", _("_Replace"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "replace",
                                            ADW_RESPONSE_DESTRUCTIVE);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adw_dialog_present (dialog, parent);
}

static void
narrow_cb (GtkWidget *parent)
{
  AdwDialog *dialog =
    adw_alert_dialog_new (_("Save Changes?"),
                          _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "discard", _("_Discard"),
                                  "save",    _("_Save"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "discard",
                                            ADW_RESPONSE_DESTRUCTIVE);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "save",
                                            ADW_RESPONSE_SUGGESTED);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "save");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adw_dialog_present (dialog, parent);
}

/* This dialog will always have vertical buttons */
static void
wide_cb (GtkWidget *parent)
{
  AdwDialog *dialog =
    adw_alert_dialog_new (_("Do you want to empty the wastebasket before you unmount?"),
                          _("In order to regain the free space on the volume the wastebasket must be emptied. All deleted items on the volume will be permanently lost."));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "ignore", _("Do _not Empty Wastebasket"),
                                  "cancel", _("_Cancel"),
                                  "empty",  _("_Empty Wastebasket"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "empty",
                                            ADW_RESPONSE_DESTRUCTIVE);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adw_dialog_present (dialog, parent);
}

static void
entry_changed_cb (GtkEditable    *editable,
                  AdwAlertDialog *dialog)
{
  const char *text = gtk_editable_get_text (editable);

  if (text && *text) {
    adw_alert_dialog_set_response_enabled (dialog, "add", TRUE);
    gtk_widget_remove_css_class (GTK_WIDGET (editable), "error");
  } else {
    adw_alert_dialog_set_response_enabled (dialog, "add", FALSE);
    gtk_widget_add_css_class (GTK_WIDGET (editable), "error");
  }
}

static void
adaptive_cb (GtkWidget *parent)
{
  AdwDialog *dialog =
    adw_alert_dialog_new (_("Save Changes?"),
                          _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));
  GtkWidget *group;
  int i;

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "discard", _("_Discard All"),
                                  "save",    _("_Save"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "discard",
                                            ADW_RESPONSE_DESTRUCTIVE);
  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "save",
                                            ADW_RESPONSE_SUGGESTED);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "save");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  group = adw_preferences_group_new ();

  for (i = 0; i < 3; i++) {
    GtkWidget *row = adw_action_row_new ();
    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (row), _("Row"));
    adw_preferences_group_add (ADW_PREFERENCES_GROUP (group), row);
  }

  adw_alert_dialog_set_extra_child (ADW_ALERT_DIALOG (dialog), group);
  adw_alert_dialog_set_prefer_wide_layout (ADW_ALERT_DIALOG (dialog), TRUE);

  g_signal_connect (dialog, "response", G_CALLBACK (response_cb), NULL);

  adw_dialog_present (dialog, parent);
}

static void
entry_cb (GtkWidget *parent)
{
  AdwDialog *dialog;
  GtkWidget *group, *entry;

  dialog =
    adw_alert_dialog_new (_("Add New Profile"),
                          _("Enter name of the new profile"));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "add", _("_Add"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "add",
                                            ADW_RESPONSE_SUGGESTED);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "add");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  adw_alert_dialog_set_response_enabled (ADW_ALERT_DIALOG (dialog), "add", FALSE);

  group = adw_preferences_group_new ();
  adw_alert_dialog_set_extra_child (ADW_ALERT_DIALOG (dialog), group);

  entry = adw_entry_row_new ();
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (entry), _("Name"));
  adw_entry_row_set_activates_default (ADW_ENTRY_ROW (entry), TRUE);
  g_signal_connect (entry, "changed", G_CALLBACK (entry_changed_cb), dialog);
  adw_preferences_group_add (ADW_PREFERENCES_GROUP (group), entry);

  g_object_set_data (G_OBJECT (dialog), "entry", entry);

  g_signal_connect (dialog, "response::add", G_CALLBACK (response_text_cb), NULL);
  g_signal_connect (dialog, "response::cancel", G_CALLBACK (response_cb), NULL);

  adw_dialog_present (dialog, parent);
}

static void
async_cb (GtkWidget *parent)
{
  AdwDialog *dialog =
    adw_alert_dialog_new (_("Replace File?"),
                          _("A file named “example.png” already exists. Do you want to replace it?"));

  adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
                                  "cancel",  _("_Cancel"),
                                  "replace", _("_Replace"),
                                  NULL);

  adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
                                            "replace",
                                            ADW_RESPONSE_DESTRUCTIVE);

  adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
  adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");

  adw_alert_dialog_choose (ADW_ALERT_DIALOG (dialog), GTK_WIDGET (parent),
                            NULL, (GAsyncReadyCallback) dialog_cb, NULL);
}

static GtkWidget *
create_content (GtkWidget *parent)
{
  GtkWidget *view, *swindow, *box, *button;

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

  button = gtk_button_new_with_label ("Narrow Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (narrow_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Wide Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (wide_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Adaptive Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (adaptive_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Entry Dialog");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (entry_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  button = gtk_button_new_with_label ("Async Call");
  gtk_widget_add_css_class (button, "pill");
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (async_cb), parent);
  gtk_box_append (GTK_BOX (box), button);

  swindow = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_propagate_natural_height (GTK_SCROLLED_WINDOW (swindow),
                                                    TRUE);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (swindow), box);

  view = adw_toolbar_view_new ();
  adw_toolbar_view_add_top_bar (ADW_TOOLBAR_VIEW (view), adw_header_bar_new ());
  adw_toolbar_view_set_content (ADW_TOOLBAR_VIEW (view), swindow);

  return view;
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

  window = adw_window_new ();
  gtk_widget_set_size_request (window, 360, 294);
  g_signal_connect_swapped (window, "destroy", G_CALLBACK (close_cb), &done);
  gtk_window_set_title (GTK_WINDOW (window), "Alert Dialogs");
  adw_window_set_content (ADW_WINDOW (window), create_content (window));
  gtk_window_present (GTK_WINDOW (window));

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  return 0;
}
