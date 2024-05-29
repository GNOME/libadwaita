#include "adw-overlay-split-view-demo-dialog.h"

struct _AdwOverlaySplitViewDemoDialog
{
  AdwDialog parent_instance;

  AdwOverlaySplitView *split_view;
  GtkToggleButton *start_button;
};

G_DEFINE_FINAL_TYPE (AdwOverlaySplitViewDemoDialog, adw_overlay_split_view_demo_dialog, ADW_TYPE_DIALOG)

static void
start_button_notify_active_cb (AdwOverlaySplitViewDemoDialog *self)
{
  gboolean start = gtk_toggle_button_get_active (self->start_button);

  adw_overlay_split_view_set_sidebar_position (self->split_view,
                                               start ? GTK_PACK_START : GTK_PACK_END);
}

static void
adw_overlay_split_view_demo_dialog_class_init (AdwOverlaySplitViewDemoDialogClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita1/Demo/ui/pages/split-views/adw-overlay-split-view-demo-dialog.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwOverlaySplitViewDemoDialog, split_view);
  gtk_widget_class_bind_template_child (widget_class, AdwOverlaySplitViewDemoDialog, start_button);
  gtk_widget_class_bind_template_callback (widget_class, start_button_notify_active_cb);
}

static void
adw_overlay_split_view_demo_dialog_init (AdwOverlaySplitViewDemoDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwDialog *
adw_overlay_split_view_demo_dialog_new (void)
{
  return g_object_new (ADW_TYPE_OVERLAY_SPLIT_VIEW_DEMO_DIALOG, NULL);
}
