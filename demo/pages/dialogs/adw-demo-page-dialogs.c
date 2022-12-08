#include "adw-demo-page-dialogs.h"

#include <glib/gi18n.h>

struct _AdwDemoPageDialogs
{
  AdwBin parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageDialogs, adw_demo_page_dialogs, ADW_TYPE_BIN)

enum {
  SIGNAL_ADD_TOAST,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
message_cb (AdwMessageDialog   *dialog,
            GAsyncResult       *result,
            AdwDemoPageDialogs *self)
{
  const char *response = adw_message_dialog_choose_finish (dialog, result);
  AdwToast *toast = adw_toast_new_format (_("Dialog response: %s"), response);

  g_signal_emit (self, signals[SIGNAL_ADD_TOAST], 0, toast);
}

static void
demo_message_dialog_cb (AdwDemoPageDialogs *self)
{
  GtkWindow *parent = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (self)));
  GtkWidget *dialog;

  dialog = adw_message_dialog_new (parent,
                                   _("Save Changes?"),
                                   _("Open document contains unsaved changes. Changes which are not saved will be permanently lost."));

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
                                    "cancel",  _("_Cancel"),
                                    "discard", _("_Discard"),
                                    "save",    _("_Save"),
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "discard", ADW_RESPONSE_DESTRUCTIVE);
  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "save", ADW_RESPONSE_SUGGESTED);

  adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG (dialog), "save");
  adw_message_dialog_set_close_response (ADW_MESSAGE_DIALOG (dialog), "cancel");

  adw_message_dialog_choose (ADW_MESSAGE_DIALOG (dialog), NULL,
                             (GAsyncReadyCallback) message_cb, self);
}

static void
adw_demo_page_dialogs_class_init (AdwDemoPageDialogsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  signals[SIGNAL_ADD_TOAST] =
    g_signal_new ("add-toast",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 1,
                  ADW_TYPE_TOAST);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/dialogs/adw-demo-page-dialogs.ui");

  gtk_widget_class_install_action (widget_class, "demo.message-dialog", NULL, (GtkWidgetActionActivateFunc) demo_message_dialog_cb);
}

static void
adw_demo_page_dialogs_init (AdwDemoPageDialogs *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
