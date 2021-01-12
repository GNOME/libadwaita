#include "adw-flap-demo-window.h"

#include <glib/gi18n.h>

struct _AdwFlapDemoWindow
{
  AdwWindow parent_instance;

  AdwFlap *flap;
  GtkWidget *reveal_btn_start;
  GtkWidget *reveal_btn_end;
};

G_DEFINE_TYPE (AdwFlapDemoWindow, adw_flap_demo_window, ADW_TYPE_WINDOW)

static gchar *
fold_policy_name (AdwEnumValueObject *value,
                  gpointer            user_data)
{
  g_return_val_if_fail (ADW_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (adw_enum_value_object_get_value (value)) {
  case ADW_FLAP_FOLD_POLICY_NEVER:
    return g_strdup (_("Never"));
  case ADW_FLAP_FOLD_POLICY_ALWAYS:
    return g_strdup (_("Always"));
  case ADW_FLAP_FOLD_POLICY_AUTO:
    return g_strdup (_("Auto"));
  default:
    return NULL;
  }
}

static gchar *
transition_type_name (AdwEnumValueObject *value,
                      gpointer            user_data)
{
  g_return_val_if_fail (ADW_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (adw_enum_value_object_get_value (value)) {
  case ADW_FLAP_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case ADW_FLAP_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  case ADW_FLAP_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
  default:
    return NULL;
  }
}

static void
start_toggle_button_toggled_cb (GtkToggleButton    *button,
                                AdwFlapDemoWindow *self)
{
  if (gtk_toggle_button_get_active (button)) {
    adw_flap_set_flap_position (self->flap, GTK_PACK_START);
    gtk_widget_hide (self->reveal_btn_end);
    gtk_widget_show (self->reveal_btn_start);
  } else {
    adw_flap_set_flap_position (self->flap, GTK_PACK_END);
    gtk_widget_hide (self->reveal_btn_start);
    gtk_widget_show (self->reveal_btn_end);
  }
}

static void
stack_notify_visible_child_cb (AdwFlapDemoWindow *self)
{
  if (adw_flap_get_folded (self->flap) && !adw_flap_get_locked (self->flap))
    adw_flap_set_reveal_flap (self->flap, FALSE);
}

static void
adw_flap_demo_window_class_init (AdwFlapDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/ui/adw-flap-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwFlapDemoWindow, flap);
  gtk_widget_class_bind_template_child (widget_class, AdwFlapDemoWindow, reveal_btn_start);
  gtk_widget_class_bind_template_child (widget_class, AdwFlapDemoWindow, reveal_btn_end);
  gtk_widget_class_bind_template_callback (widget_class, start_toggle_button_toggled_cb);
  gtk_widget_class_bind_template_callback (widget_class, stack_notify_visible_child_cb);
  gtk_widget_class_bind_template_callback (widget_class, fold_policy_name);
  gtk_widget_class_bind_template_callback (widget_class, transition_type_name);
}

static void
adw_flap_demo_window_init (AdwFlapDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

AdwFlapDemoWindow *
adw_flap_demo_window_new (void)
{
  return g_object_new (ADW_TYPE_FLAP_DEMO_WINDOW, NULL);
}
