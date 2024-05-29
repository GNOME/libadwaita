#include "adw-demo-preferences-dialog.h"

struct _AdwDemoPreferencesDialog
{
  AdwPreferencesDialog parent_instance;

  AdwNavigationPage *subpage1;
  AdwNavigationPage *subpage2;
};

G_DEFINE_FINAL_TYPE (AdwDemoPreferencesDialog, adw_demo_preferences_dialog, ADW_TYPE_PREFERENCES_DIALOG)

static void
subpage1_activated_cb (AdwDemoPreferencesDialog *self)
{
  adw_preferences_dialog_push_subpage (ADW_PREFERENCES_DIALOG (self), self->subpage1);
}

static void
subpage2_activated_cb (AdwDemoPreferencesDialog *self)
{
  adw_preferences_dialog_push_subpage (ADW_PREFERENCES_DIALOG (self), self->subpage2);
}

static void
toast_show_cb (AdwPreferencesDialog *self)
{
  adw_preferences_dialog_add_toast (self, adw_toast_new ("Example Toast"));
}

static void
adw_demo_preferences_dialog_class_init (AdwDemoPreferencesDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/adw-demo-preferences-dialog.ui");

  gtk_widget_class_bind_template_child (widget_class, AdwDemoPreferencesDialog, subpage1);
  gtk_widget_class_bind_template_child (widget_class, AdwDemoPreferencesDialog, subpage2);

  gtk_widget_class_bind_template_callback (widget_class, subpage1_activated_cb);
  gtk_widget_class_bind_template_callback (widget_class, subpage2_activated_cb);

  gtk_widget_class_install_action (widget_class, "toast.show", NULL, (GtkWidgetActionActivateFunc) toast_show_cb);
}

static void
adw_demo_preferences_dialog_init (AdwDemoPreferencesDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwDialog *
adw_demo_preferences_dialog_new (void)
{
  return g_object_new (ADW_TYPE_DEMO_PREFERENCES_DIALOG, NULL);
}
