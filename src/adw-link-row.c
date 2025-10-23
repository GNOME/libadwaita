/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-link-row-private.h"
#include "adw-marshalers.h"

struct _AdwLinkRow
{
  AdwActionRow parent_instance;

  GtkWidget *icon;

  char *uri;
  gboolean visited;

  GtkWidget *context_menu;
};

G_DEFINE_FINAL_TYPE (AdwLinkRow, adw_link_row, ADW_TYPE_ACTION_ROW)

enum
{
  PROP_0,
  PROP_URI,
  PROP_VISITED,
  PROP_LAST_PROP,
};

static GParamSpec *props[PROP_LAST_PROP];

enum {
  SIGNAL_ACTIVATE_LINK,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
context_menu_closed_cb (AdwLinkRow *self)
{
  gtk_widget_remove_css_class (GTK_WIDGET (self), "has-open-popup");
}

static void
open_context_menu (AdwLinkRow *self,
                   double      x,
                   double      y)
{
  GdkRectangle rect;

  if (x > -0.5 && y > -0.5) {
    rect.x = x;
    rect.y = y;
  } else {
    if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
      rect.x = gtk_widget_get_width (GTK_WIDGET (self));
    else
      rect.x = 0;

    rect.y = gtk_widget_get_height (GTK_WIDGET (self));
  }

  rect.width = 0;
  rect.height = 0;

  gtk_popover_set_pointing_to (GTK_POPOVER (self->context_menu), &rect);

  gtk_popover_popup (GTK_POPOVER (self->context_menu));

  gtk_widget_add_css_class (GTK_WIDGET (self), "has-open-popup");
}

static inline gboolean
is_touchscreen (GtkGesture *gesture)
{
  GtkEventController *controller = GTK_EVENT_CONTROLLER (gesture);
  GdkDevice *device = gtk_event_controller_get_current_event_device (controller);
  GdkInputSource input_source = gdk_device_get_source (device);

  return input_source == GDK_SOURCE_TOUCHSCREEN;
}

static void
pressed_cb (AdwLinkRow *self,
            int         n_press,
            double      x,
            double      y,
            GtkGesture *gesture)
{
  GdkEventSequence *current = gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  GdkEvent *event = gtk_gesture_get_last_event (gesture, current);

   if (gdk_event_triggers_context_menu (event)) {
    open_context_menu (self, x, y);
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));

    return;
  }

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
}

static void
long_pressed_cb (AdwLinkRow *self,
                 double      x,
                 double      y,
                 GtkGesture *gesture)
{
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
  open_context_menu (self, x, y);
}

static void
popup_menu_cb (GtkWidget  *widget,
               const char *action_name,
               GVariant   *parameter)
{
  AdwLinkRow *self = ADW_LINK_ROW (widget);

  open_context_menu (self, -1, -1);
}

static void
launch_done (GObject      *source,
             GAsyncResult *result,
             gpointer      data)
{
  GError *error = NULL;
  gboolean success;

  if (GTK_IS_FILE_LAUNCHER (source))
    success = gtk_file_launcher_launch_finish (GTK_FILE_LAUNCHER (source), result, &error);
  else if (GTK_IS_URI_LAUNCHER (source))
    success = gtk_uri_launcher_launch_finish (GTK_URI_LAUNCHER (source), result, &error);
  else
    g_assert_not_reached ();

  if (!success) {
    g_warning ("Failed to launch handler: %s", error->message);
    g_error_free (error);
  }
}

static gboolean
activate_link_default_cb (AdwLinkRow *self)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (self));
  const char *uri_scheme = g_uri_peek_scheme (self->uri);

  if (g_strcmp0 (uri_scheme, "file") == 0) {
    GFile *file = g_file_new_for_uri (self->uri);
    GtkFileLauncher *launcher = gtk_file_launcher_new (file);

    gtk_file_launcher_launch (launcher, GTK_WINDOW (root), NULL, launch_done, NULL);

    g_object_unref (launcher);
    g_object_unref (file);
  } else {
    GtkUriLauncher *launcher = gtk_uri_launcher_new (self->uri);

    gtk_uri_launcher_launch (launcher, GTK_WINDOW (root), NULL, launch_done, NULL);

    g_object_unref (launcher);
  }

  adw_link_row_set_visited (self, TRUE);

  return GDK_EVENT_STOP;
}

static void
adw_link_row_activate (AdwActionRow *row)
{
  gboolean retval = FALSE;

  g_signal_emit (row, signals[SIGNAL_ACTIVATE_LINK], 0, &retval);
}

static void
link_open_cb (GtkWidget  *widget,
              const char *name,
              GVariant   *parameter)
{
  adw_link_row_activate (ADW_ACTION_ROW (widget));
}

static void
link_copy_cb (GtkWidget  *widget,
              const char *name,
              GVariant   *parameter)
{
  AdwLinkRow *self = ADW_LINK_ROW (widget);
  GdkClipboard *clipboard = gtk_widget_get_clipboard (widget);

  gdk_clipboard_set_text (clipboard, self->uri);
}

static void
adw_link_row_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwLinkRow *self = ADW_LINK_ROW (object);

  switch (prop_id) {
  case PROP_URI:
    g_value_set_string (value, adw_link_row_get_uri (self));
    break;
  case PROP_VISITED:
    g_value_set_boolean (value, adw_link_row_get_visited (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_link_row_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwLinkRow *self = ADW_LINK_ROW (object);

  switch (prop_id) {
  case PROP_URI:
    adw_link_row_set_uri (self, g_value_get_string (value));
    break;
  case PROP_VISITED:
    adw_link_row_set_visited (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_link_row_dispose (GObject *object)
{
  AdwLinkRow *self = ADW_LINK_ROW (object);

  g_clear_pointer (&self->context_menu, gtk_widget_unparent);
  self->icon = NULL;

  G_OBJECT_CLASS (adw_link_row_parent_class)->dispose (object);
}

static void
adw_link_row_finalize (GObject *object)
{
  AdwLinkRow *self = ADW_LINK_ROW (object);

  g_free (self->uri);

  G_OBJECT_CLASS (adw_link_row_parent_class)->finalize (object);
}

static void
adw_link_row_class_init (AdwLinkRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  AdwActionRowClass *row_class = ADW_ACTION_ROW_CLASS (klass);

  object_class->get_property = adw_link_row_get_property;
  object_class->set_property = adw_link_row_set_property;
  object_class->dispose = adw_link_row_dispose;
  object_class->finalize = adw_link_row_finalize;

  row_class->activate = adw_link_row_activate;

  props[PROP_URI] =
    g_param_spec_string ("uri", NULL, NULL,
                          "",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_VISITED] =
    g_param_spec_boolean ("visited", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  signals[SIGNAL_ACTIVATE_LINK] =
    g_signal_new ("activate-link",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  g_signal_accumulator_true_handled,
                  NULL,
                  adw_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN,
                  0);
  g_signal_set_va_marshaller (signals[SIGNAL_ACTIVATE_LINK],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_BOOLEAN__VOIDv);

  g_signal_override_class_handler ("activate-link",
                                   G_TYPE_FROM_CLASS (klass),
                                   G_CALLBACK (activate_link_default_cb));

  gtk_widget_class_install_action (widget_class, "menu.popup", NULL, popup_menu_cb);
  gtk_widget_class_install_action (widget_class, "link.open", NULL, link_open_cb);
  gtk_widget_class_install_action (widget_class, "link.copy", NULL, link_copy_cb);


  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_F10, GDK_SHIFT_MASK, "menu.popup", NULL);
  gtk_widget_class_add_binding_action (widget_class, GDK_KEY_Menu, 0, "menu.popup", NULL);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_LINK);
}

static void
setup_context_menu (AdwLinkRow *self)
{
  GtkEventController *controller;
  GMenu *menu = g_menu_new ();

  g_menu_append (menu, _("_Open Link"), "link.open");
  g_menu_append (menu, _("Copy _Link Address"), "link.copy");

  self->context_menu = gtk_popover_menu_new_from_model (G_MENU_MODEL (menu));
  gtk_widget_set_parent (self->context_menu, GTK_WIDGET (self));
  gtk_popover_set_position (GTK_POPOVER (self->context_menu), GTK_POS_BOTTOM);
  gtk_popover_set_has_arrow (GTK_POPOVER (self->context_menu), FALSE);
  gtk_widget_set_halign (self->context_menu, GTK_ALIGN_START);

  g_signal_connect_object (self->context_menu, "closed",
                           G_CALLBACK (context_menu_closed_cb), self,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_click_new ());
  gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (controller), 0);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "pressed", G_CALLBACK (pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_long_press_new ());
  gtk_gesture_long_press_set_delay_factor (GTK_GESTURE_LONG_PRESS (controller), 2);
  gtk_gesture_single_set_exclusive (GTK_GESTURE_SINGLE (controller), TRUE);
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (controller), TRUE);
  g_signal_connect_swapped (controller, "pressed", G_CALLBACK (long_pressed_cb), self);
  gtk_widget_add_controller (GTK_WIDGET (self), controller);

  g_object_unref (menu);

  gtk_accessible_update_property (GTK_ACCESSIBLE (self),
                                  GTK_ACCESSIBLE_PROPERTY_HAS_POPUP, TRUE,
                                  -1);
}

static void
adw_link_row_init (AdwLinkRow *self)
{
  self->uri = g_strdup ("");

  self->icon = g_object_new (GTK_TYPE_IMAGE,
                             "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                             "icon-name", "adw-external-link-symbolic",
                             NULL);
  adw_action_row_add_suffix (ADW_ACTION_ROW (self), self->icon);

  gtk_list_box_row_set_activatable (GTK_LIST_BOX_ROW (self), TRUE);

  setup_context_menu (self);

  gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_LINK, FALSE);

  gtk_widget_add_css_class (GTK_WIDGET (self), "link");
}

GtkWidget *
adw_link_row_new (void)
{
  return g_object_new (ADW_TYPE_LINK_ROW, NULL);
}

const char *
adw_link_row_get_uri (AdwLinkRow *self)
{
  g_return_val_if_fail (ADW_IS_LINK_ROW (self), NULL);

  return self->uri;
}

void
adw_link_row_set_uri (AdwLinkRow *self,
                      const char *uri)
{
  const char *uri_scheme;

  g_return_if_fail (ADW_IS_LINK_ROW (self));
  g_return_if_fail (uri != NULL);

  if (!g_set_str (&self->uri, uri))
    return;

  uri_scheme = g_uri_peek_scheme (self->uri);

  if (g_strcmp0 (uri_scheme, "mailto") == 0)
    gtk_image_set_from_icon_name (GTK_IMAGE (self->icon), "adw-mail-send-symbolic");
  else
    gtk_image_set_from_icon_name (GTK_IMAGE (self->icon), "adw-external-link-symbolic");

  gtk_widget_set_tooltip_text (GTK_WIDGET (self), uri);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_URI]);
}

gboolean
adw_link_row_get_visited (AdwLinkRow *self)
{
  g_return_val_if_fail (ADW_IS_LINK_ROW (self), FALSE);

  return self->visited;
}

void
adw_link_row_set_visited (AdwLinkRow *self,
                          gboolean    visited)
{
  g_return_if_fail (ADW_IS_LINK_ROW (self));

  visited = !!visited;

  if (visited == self->visited)
    return;

  self->visited = visited;

  gtk_accessible_update_state (GTK_ACCESSIBLE (self),
                               GTK_ACCESSIBLE_STATE_VISITED, visited,
                               -1);

  if (visited) {
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_LINK);
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_VISITED, FALSE);
  } else {
    gtk_widget_unset_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_VISITED);
    gtk_widget_set_state_flags (GTK_WIDGET (self), GTK_STATE_FLAG_LINK, FALSE);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VISITED]);
}
