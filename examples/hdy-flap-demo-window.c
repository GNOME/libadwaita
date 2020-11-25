#include "hdy-flap-demo-window.h"

#include <glib/gi18n.h>

struct _HdyFlapDemoWindow
{
  HdyWindow parent_instance;

  HdyFlap *flap;
  GtkWidget *reveal_btn_start;
  GtkWidget *reveal_btn_end;
  HdyComboRow *fold_policy_row;
  HdyComboRow *transition_type_row;
};

G_DEFINE_TYPE (HdyFlapDemoWindow, hdy_flap_demo_window, HDY_TYPE_WINDOW)

static gchar *
fold_policy_name (HdyEnumValueObject *value,
                  gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_FLAP_FOLD_POLICY_NEVER:
    return g_strdup (_("Never"));
  case HDY_FLAP_FOLD_POLICY_ALWAYS:
    return g_strdup (_("Always"));
  case HDY_FLAP_FOLD_POLICY_AUTO:
    return g_strdup (_("Auto"));
  default:
    return NULL;
  }
}

static gchar *
transition_type_name (HdyEnumValueObject *value,
                      gpointer            user_data)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (value), NULL);

  switch (hdy_enum_value_object_get_value (value)) {
  case HDY_FLAP_TRANSITION_TYPE_OVER:
    return g_strdup (_("Over"));
  case HDY_FLAP_TRANSITION_TYPE_UNDER:
    return g_strdup (_("Under"));
  case HDY_FLAP_TRANSITION_TYPE_SLIDE:
    return g_strdup (_("Slide"));
  default:
    return NULL;
  }
}

static void
start_toggle_button_toggled_cb (GtkToggleButton    *button,
                                HdyFlapDemoWindow *self)
{
  if (gtk_toggle_button_get_active (button)) {
    hdy_flap_set_flap_position (self->flap, GTK_PACK_START);
    gtk_widget_hide (self->reveal_btn_end);
    gtk_widget_show (self->reveal_btn_start);
  } else {
    hdy_flap_set_flap_position (self->flap, GTK_PACK_END);
    gtk_widget_hide (self->reveal_btn_start);
    gtk_widget_show (self->reveal_btn_end);
  }
}

static void
stack_notify_visible_child_cb (HdyFlapDemoWindow *self)
{
  if (hdy_flap_get_folded (self->flap) && !hdy_flap_get_locked (self->flap))
    hdy_flap_set_reveal_flap (self->flap, FALSE);
}

static void
hdy_flap_demo_window_class_init (HdyFlapDemoWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/sm/puri/Handy/Demo/ui/hdy-flap-demo-window.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyFlapDemoWindow, flap);
  gtk_widget_class_bind_template_child (widget_class, HdyFlapDemoWindow, reveal_btn_start);
  gtk_widget_class_bind_template_child (widget_class, HdyFlapDemoWindow, reveal_btn_end);
  gtk_widget_class_bind_template_child (widget_class, HdyFlapDemoWindow, fold_policy_row);
  gtk_widget_class_bind_template_child (widget_class, HdyFlapDemoWindow, transition_type_row);
  gtk_widget_class_bind_template_callback (widget_class, start_toggle_button_toggled_cb);
  gtk_widget_class_bind_template_callback (widget_class, stack_notify_visible_child_cb);
}

static void
hdy_flap_demo_window_init (HdyFlapDemoWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  hdy_combo_row_set_for_enum (self->fold_policy_row,
                              HDY_TYPE_FLAP_FOLD_POLICY,
                              fold_policy_name, NULL, NULL);
  hdy_combo_row_set_selected_index (self->fold_policy_row,
                                    HDY_FLAP_FOLD_POLICY_AUTO);

  hdy_combo_row_set_for_enum (self->transition_type_row,
                              HDY_TYPE_FLAP_TRANSITION_TYPE,
                              transition_type_name, NULL, NULL);
  hdy_combo_row_set_selected_index (self->transition_type_row,
                                    HDY_FLAP_TRANSITION_TYPE_OVER);
}

HdyFlapDemoWindow *
hdy_flap_demo_window_new (void)
{
  return g_object_new (HDY_TYPE_FLAP_DEMO_WINDOW, NULL);
}
