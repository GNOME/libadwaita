/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

/* Most of the file is based on bits of code from GtkWindow */

#include "config.h"

#include "gtk-window-private.h"
#include "hdy-window-handle-controller-private.h"

#include <glib/gi18n-lib.h>

/**
 * PRIVATE:hdy-window-handle-controller
 * @short_description: An oblect that makes widgets behave like titlebars.
 * @Title: HdyWindowHandleController
 * @See_also: #HdyHeaderBar, #HdyWindowHandle
 * @stability: Private
 *
 * When HdyWindowHandleController is added to the widget, dragging that widget
 * will move the window, and right click, double click and middle click will be
 * handled as if that widget was a titlebar. Currently it's used to implement
 * these properties in #HdyWindowHandle and #HdyHeaderBar
 *
 * Since: 1.0
 */

struct _HdyWindowHandleController
{
  GObject parent;

  GtkWidget *widget;
  GtkGesture *multipress_gesture;
  GtkWidget *fallback_menu;
  gboolean keep_above;
};

G_DEFINE_TYPE (HdyWindowHandleController, hdy_window_handle_controller, G_TYPE_OBJECT);

static GtkWindow *
get_window (HdyWindowHandleController *self)
{
  GtkWidget *toplevel = gtk_widget_get_toplevel (self->widget);

  if (GTK_IS_WINDOW (toplevel))
    return GTK_WINDOW (toplevel);

  return NULL;
}

static void
popup_menu_detach (GtkWidget *widget,
                   GtkMenu   *menu)
{
  HdyWindowHandleController *self;

  self = g_object_steal_data (G_OBJECT (menu), "hdywindowhandlecontroller");

  self->fallback_menu = NULL;
}

static void
restore_window_cb (GtkMenuItem               *menuitem,
                   HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);
  GdkWindowState state;

  if (!window)
    return;

  if (gtk_window_is_maximized (window)) {
    gtk_window_unmaximize (window);
    return;
  }

  state = hdy_gtk_window_get_state (window);

  if (state & GDK_WINDOW_STATE_ICONIFIED)
    gtk_window_deiconify (window);
}

static void
move_window_cb (GtkMenuItem               *menuitem,
                HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);

  if (!window)
    return;

  gtk_window_begin_move_drag (window,
                              0, /* 0 means "use keyboard" */
                              0, 0,
                              GDK_CURRENT_TIME);
}

static void
resize_window_cb (GtkMenuItem               *menuitem,
                  HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);

  if (!window)
    return;

  gtk_window_begin_resize_drag  (window,
                                 0,
                                 0, /* 0 means "use keyboard" */
                                 0, 0,
                                 GDK_CURRENT_TIME);
}

static void
minimize_window_cb (GtkMenuItem               *menuitem,
                    HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);

  if (!window)
    return;

  /* Turns out, we can't iconify a maximized window */
  if (gtk_window_is_maximized (window))
    gtk_window_unmaximize (window);

  gtk_window_iconify (window);
}

static void
maximize_window_cb (GtkMenuItem               *menuitem,
                    HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);
  GdkWindowState state;

  if (!window)
    return;

  state = hdy_gtk_window_get_state (window);

  if (state & GDK_WINDOW_STATE_ICONIFIED)
    gtk_window_deiconify (window);

  gtk_window_maximize (window);
}

static void
ontop_window_cb (GtkMenuItem               *menuitem,
                 HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);

  if (!window)
    return;

  /*
   * FIXME: It will go out of sync if something else calls
   * gtk_window_set_keep_above(), so we need to actually track it.
   * For some reason this doesn't seem to be reflected in the
   * window state.
   */
  self->keep_above = !self->keep_above;
  gtk_window_set_keep_above (window, self->keep_above);
}

static void
close_window_cb (GtkMenuItem               *menuitem,
                 HdyWindowHandleController *self)
{
  GtkWindow *window = get_window (self);

  if (!window)
    return;

  gtk_window_close (window);
}

static void
do_popup (HdyWindowHandleController *self,
          GdkEventButton            *event)
{
  GtkWindow *window = get_window (self);
  GtkWidget *menuitem;
  GdkWindowState state;
  gboolean maximized, iconified, resizable;
  GdkWindowTypeHint type_hint;

  if (!window)
    return;

  if (gdk_window_show_window_menu (gtk_widget_get_window (GTK_WIDGET (window)),
                                   (GdkEvent *) event))
    return;

  if (self->fallback_menu)
      gtk_widget_destroy (self->fallback_menu);

  state = hdy_gtk_window_get_state (window);

  iconified = (state & GDK_WINDOW_STATE_ICONIFIED) == GDK_WINDOW_STATE_ICONIFIED;
  maximized = gtk_window_is_maximized (window) && !iconified;
  resizable = gtk_window_get_resizable (window);
  type_hint = gtk_window_get_type_hint (window);

  self->fallback_menu = gtk_menu_new ();
  gtk_style_context_add_class (gtk_widget_get_style_context (self->fallback_menu),
                               GTK_STYLE_CLASS_CONTEXT_MENU);

  /* We can't pass self to popup_menu_detach, so will have to use custom data */
  g_object_set_data (G_OBJECT (self->fallback_menu),
                     "hdywindowhandlecontroller", self);

  gtk_menu_attach_to_widget (GTK_MENU (self->fallback_menu),
                             self->widget,
                             popup_menu_detach);

  menuitem = gtk_menu_item_new_with_label (_("Restore"));
  gtk_widget_show (menuitem);
  /* "Restore" means "Unmaximize" or "Unminimize"
   * (yes, some WMs allow window menu to be shown for minimized windows).
   * Not restorable:
   *   - visible windows that are not maximized or minimized
   *   - non-resizable windows that are not minimized
   *   - non-normal windows
   */
  if ((gtk_widget_is_visible (GTK_WIDGET (window)) &&
       !(maximized || iconified)) ||
      (!iconified && !resizable) ||
      type_hint != GDK_WINDOW_TYPE_HINT_NORMAL)
    gtk_widget_set_sensitive (menuitem, FALSE);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (restore_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Move"));
  gtk_widget_show (menuitem);
  if (maximized || iconified)
    gtk_widget_set_sensitive (menuitem, FALSE);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (move_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Resize"));
  gtk_widget_show (menuitem);
  if (!resizable || maximized || iconified)
    gtk_widget_set_sensitive (menuitem, FALSE);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (resize_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Minimize"));
  gtk_widget_show (menuitem);
  if (iconified ||
      type_hint != GDK_WINDOW_TYPE_HINT_NORMAL)
    gtk_widget_set_sensitive (menuitem, FALSE);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (minimize_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Maximize"));
  gtk_widget_show (menuitem);
  if (maximized ||
      !resizable ||
      type_hint != GDK_WINDOW_TYPE_HINT_NORMAL)
    gtk_widget_set_sensitive (menuitem, FALSE);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (maximize_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_separator_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_check_menu_item_new_with_label (_("Always on Top"));
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), self->keep_above);
  if (maximized)
    gtk_widget_set_sensitive (menuitem, FALSE);
  gtk_widget_show (menuitem);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (ontop_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_separator_menu_item_new ();
  gtk_widget_show (menuitem);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);

  menuitem = gtk_menu_item_new_with_label (_("Close"));
  gtk_widget_show (menuitem);
  if (!gtk_window_get_deletable (window))
    gtk_widget_set_sensitive (menuitem, FALSE);
  g_signal_connect (G_OBJECT (menuitem), "activate",
                    G_CALLBACK (close_window_cb), self);
  gtk_menu_shell_append (GTK_MENU_SHELL (self->fallback_menu), menuitem);
  gtk_menu_popup_at_pointer (GTK_MENU (self->fallback_menu), (GdkEvent *) event);
}

static gboolean
titlebar_action (HdyWindowHandleController *self,
                 const GdkEvent            *event,
                 guint                      button)
{
  GtkSettings *settings;
  g_autofree gchar *action = NULL;
  GtkWindow *window = get_window (self);

  if (!window)
    return FALSE;

  settings = gtk_widget_get_settings (GTK_WIDGET (window));

  switch (button) {
  case GDK_BUTTON_PRIMARY:
    g_object_get (settings, "gtk-titlebar-double-click", &action, NULL);
    break;

  case GDK_BUTTON_MIDDLE:
    g_object_get (settings, "gtk-titlebar-middle-click", &action, NULL);
    break;

  case GDK_BUTTON_SECONDARY:
    g_object_get (settings, "gtk-titlebar-right-click", &action, NULL);
    break;

  default:
    g_assert_not_reached ();
  }

  if (action == NULL)
    return FALSE;

  if (g_str_equal (action, "none"))
    return FALSE;

  if (g_str_has_prefix (action, "toggle-maximize")) {
    /*
     * gtk header bar won't show the maximize button if the following
     * properties are not met, apply the same to title bar actions for
     * consistency.
     */
    if (gtk_window_get_resizable (window) &&
        gtk_window_get_type_hint (window) == GDK_WINDOW_TYPE_HINT_NORMAL)
          hdy_gtk_window_toggle_maximized (window);

    return TRUE;
  }

  if (g_str_equal (action, "lower")) {
    gdk_window_lower (gtk_widget_get_window (GTK_WIDGET (window)));

    return TRUE;
  }

  if (g_str_equal (action, "minimize")) {
    gdk_window_iconify (gtk_widget_get_window (GTK_WIDGET (window)));

    return TRUE;
  }

  if (g_str_equal (action, "menu")) {
    do_popup (self, (GdkEventButton*) event);

    return TRUE;
  }

  g_warning ("Unsupported titlebar action %s", action);

  return FALSE;
}

static void
pressed_cb (GtkGestureMultiPress      *gesture,
            gint                       n_press,
            gdouble                    x,
            gdouble                    y,
            HdyWindowHandleController *self)
{
  GtkWidget *window = gtk_widget_get_toplevel (self->widget);
  GdkEventSequence *sequence =
    gtk_gesture_single_get_current_sequence (GTK_GESTURE_SINGLE (gesture));
  const GdkEvent *event =
    gtk_gesture_get_last_event (GTK_GESTURE (gesture), sequence);
  guint button =
    gtk_gesture_single_get_current_button (GTK_GESTURE_SINGLE (gesture));

  if (!event)
    return;

  if (gdk_display_device_is_grabbed (gtk_widget_get_display (window),
                                     gtk_gesture_get_device (GTK_GESTURE (gesture))))
    return;

  switch (button) {
  case GDK_BUTTON_PRIMARY:
    gdk_window_raise (gtk_widget_get_window (window));

    if (n_press == 2)
        titlebar_action (self, event, button);

    if (gtk_widget_has_grab (window))
      gtk_gesture_set_sequence_state (GTK_GESTURE (gesture),
                                      sequence, GTK_EVENT_SEQUENCE_CLAIMED);

    break;

  case GDK_BUTTON_SECONDARY:
    if (titlebar_action (self, event, button))
      gtk_gesture_set_sequence_state (GTK_GESTURE (gesture),
                                      sequence, GTK_EVENT_SEQUENCE_CLAIMED);

    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (gesture));
    break;

    case GDK_BUTTON_MIDDLE:
    if (titlebar_action (self, event, button))
      gtk_gesture_set_sequence_state (GTK_GESTURE (gesture),
                                      sequence, GTK_EVENT_SEQUENCE_CLAIMED);
    break;

  default:
    break;
  }
}

static void
hdy_window_handle_controller_finalize (GObject *object)
{
  HdyWindowHandleController *self = (HdyWindowHandleController *)object;

  self->widget = NULL;
  g_clear_object (&self->multipress_gesture);
  g_clear_object (&self->fallback_menu);

  G_OBJECT_CLASS (hdy_window_handle_controller_parent_class)->finalize (object);
}

static void
hdy_window_handle_controller_class_init (HdyWindowHandleControllerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = hdy_window_handle_controller_finalize;
}

static void
hdy_window_handle_controller_init (HdyWindowHandleController *self)
{
}

/**
 * hdy_window_handle_controller_new:
 * @widget: The widget to create a controller for
 *
 * Creates a new #HdyWindowHandleController for @widget.
 *
 * Returns: (transfer full): a newly created #HdyWindowHandleController
 *
 * Since: 1.0
 */
HdyWindowHandleController *
hdy_window_handle_controller_new (GtkWidget *widget)
{
  HdyWindowHandleController *self;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  self = g_object_new (HDY_TYPE_WINDOW_HANDLE_CONTROLLER, NULL);

  /* The object is intended to have the same life cycle as the widget,
   * so we don't ref it. */
  self->widget = widget;
  self->multipress_gesture = g_object_new (GTK_TYPE_GESTURE_MULTI_PRESS,
                                           "widget", widget,
                                           "button", 0,
                                           NULL);
  g_signal_connect_object (self->multipress_gesture,
                           "pressed",
                           G_CALLBACK (pressed_cb),
                           self,
                           0);

  gtk_widget_add_events (widget,
                         GDK_BUTTON_PRESS_MASK |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_BUTTON_MOTION_MASK |
                         GDK_TOUCH_MASK);

  gtk_style_context_add_class (gtk_widget_get_style_context (widget),
                               "windowhandle");

  return self;
}
