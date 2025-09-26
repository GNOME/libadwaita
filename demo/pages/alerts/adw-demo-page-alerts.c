#include "adw-demo-page-alerts.h"

#include <glib/gi18n.h>

struct _AdwDemoPageAlerts
{
  AdwBin parent_instance;

  AdwToast *last_toast;
};

G_DEFINE_FINAL_TYPE (AdwDemoPageAlerts, adw_demo_page_alerts, ADW_TYPE_BIN)

static void
toast_dismissed_cb (AdwToast          *toast,
                    AdwDemoPageAlerts *self)
{
  if (toast == self->last_toast)
    self->last_toast = NULL;
}

static void
alert_cb (AdwAlertDialog    *dialog,
          GAsyncResult      *result,
          AdwDemoPageAlerts *self)
{
  GtkWidget *toast_overlay = gtk_widget_get_ancestor (GTK_WIDGET (self), ADW_TYPE_TOAST_OVERLAY);
  const char *response = adw_alert_dialog_choose_finish (dialog, result);
  AdwToast *toast = adw_toast_new_format (_("Dialog response: %s"), response);
  g_signal_connect_object (toast, "dismissed", G_CALLBACK (toast_dismissed_cb), self, 0);

  g_clear_pointer (&self->last_toast, adw_toast_dismiss);
  self->last_toast = toast;

  adw_toast_overlay_add_toast (ADW_TOAST_OVERLAY (toast_overlay), toast);
}

static void
demo_alert_dialog_cb (AdwDemoPageAlerts *self)
{
  AdwDialog *dialog;

  dialog = adw_alert_dialog_new (_("Save Changes?"),
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

  adw_alert_dialog_choose (ADW_ALERT_DIALOG (dialog), GTK_WIDGET (self), NULL,
                           (GAsyncReadyCallback) alert_cb, self);
}

static void
adw_demo_page_alerts_class_init (AdwDemoPageAlertsClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/alerts/adw-demo-page-alerts.ui");

  gtk_widget_class_install_action (widget_class, "demo.alert-dialog", NULL, (GtkWidgetActionActivateFunc) demo_alert_dialog_cb);
}

static void
adw_demo_page_alerts_init (AdwDemoPageAlerts *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
