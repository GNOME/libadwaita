/* GTK - The GIMP Toolkit
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (C) 2018 Purism SPC
 *
 * Authors:
 * - Bastien Nocera <bnocera@redhat.com>
 * - Adrien Plazas <adrien.plazas@puri.sm>
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
 * Modified by the GTK+ Team and others 2013.  See the AUTHORS
 * file for a list of people on the GTK Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK at ftp://ftp.gtk.org/pub/gtk/.
 */

/*
 * Forked from the GTK+ 3.94.0 GtkSearchBar widget and modified for libhandy by
 * Adrien Plazas on behalf of Purism SPC 2018.
 *
 * The AUTHORS file referenced above is part of GTK and not present in
 * libhandy. At the time of the fork it was available here:
 * https://gitlab.gnome.org/GNOME/gtk/blob/faba0f0145b1281facba20fb90699e3db594fbb0/AUTHORS
 *
 * The ChangeLog file referenced above was not present in GTK+ at the time of
 * the fork.
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-search-bar.h"

/**
 * SECTION:hdy-search-bar
 * @short_description: A toolbar to integrate a search entry with.
 * @Title: HdySearchBar
 *
 * #HdySearchBar is a container made to have a search entry (possibly
 * with additional connex widgets, such as drop-down menus, or buttons)
 * built-in. The search bar would appear when a search is started through
 * typing on the keyboard, or the application’s search mode is toggled on.
 *
 * For keyboard presses to start a search, events will need to be
 * forwarded from the top-level window that contains the search bar.
 * See hdy_search_bar_handle_event() for example code. Common shortcuts
 * such as Ctrl+F should be handled as an application action, or through
 * the menu items.
 *
 * You will also need to tell the search bar about which entry you
 * are using as your search entry using hdy_search_bar_connect_entry().
 * The following example shows you how to create a more complex search
 * entry.
 *
 * HdySearchBar is very similar to #GtkSearchBar, the main difference being that
 * it allows the search entry to fill all the available space. This allows you
 * to control your search entry's width with a #HdyClamp.
 *
 * # CSS nodes
 *
 * #HdySearchBar has a single CSS node with name searchbar.
 *
 * Since: 0.0.6
 */

typedef struct {
  /* Template widgets */
  GtkWidget *revealer;
  GtkWidget *tool_box;
  GtkWidget *start;
  GtkWidget *end;
  GtkWidget *close_button;

  GtkWidget *entry;
  gboolean reveal_child;
  gboolean show_close_button;
} HdySearchBarPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdySearchBar, hdy_search_bar, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_SEARCH_MODE_ENABLED,
  PROP_SHOW_CLOSE_BUTTON,
  LAST_PROPERTY
};

static GParamSpec *props[LAST_PROPERTY] = { NULL, };

/* This comes from gtksearchentry.c in GTK. */
static gboolean
gtk_search_entry_is_keynav_event (GdkEvent *event)
{
  GdkModifierType state = 0;
  guint keyval;

  if (!gdk_event_get_keyval (event, &keyval))
    return FALSE;

  gdk_event_get_state (event, &state);

  if (keyval == GDK_KEY_Tab       || keyval == GDK_KEY_KP_Tab ||
      keyval == GDK_KEY_Up        || keyval == GDK_KEY_KP_Up ||
      keyval == GDK_KEY_Down      || keyval == GDK_KEY_KP_Down ||
      keyval == GDK_KEY_Left      || keyval == GDK_KEY_KP_Left ||
      keyval == GDK_KEY_Right     || keyval == GDK_KEY_KP_Right ||
      keyval == GDK_KEY_Home      || keyval == GDK_KEY_KP_Home ||
      keyval == GDK_KEY_End       || keyval == GDK_KEY_KP_End ||
      keyval == GDK_KEY_Page_Up   || keyval == GDK_KEY_KP_Page_Up ||
      keyval == GDK_KEY_Page_Down || keyval == GDK_KEY_KP_Page_Down ||
      ((state & (GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0))
    return TRUE;

  /* Other navigation events should get automatically
   * ignored as they will not change the content of the entry
   */
  return FALSE;
}

static void
stop_search_cb (GtkWidget    *entry,
                HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->revealer), FALSE);
}

static gboolean
entry_key_pressed_event_cb (GtkWidget    *widget,
                            GdkEvent     *event,
                            HdySearchBar *self)
{
  if (event->key.keyval == GDK_KEY_Escape) {
    stop_search_cb (widget, self);

    return GDK_EVENT_STOP;
  } else {
    return GDK_EVENT_PROPAGATE;
  }
}

static void
preedit_changed_cb (GtkEntry  *entry,
                    GtkWidget *popup,
                    gboolean  *preedit_changed)
{
  *preedit_changed = TRUE;
}

static gboolean
hdy_search_bar_handle_event_for_entry (HdySearchBar *self,
                                       GdkEvent     *event)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);
  gboolean handled;
  gboolean preedit_changed;
  guint preedit_change_id;
  gboolean res;
  char *old_text, *new_text;

  if (gtk_search_entry_is_keynav_event (event) ||
      event->key.keyval == GDK_KEY_space ||
      event->key.keyval == GDK_KEY_Menu)
    return GDK_EVENT_PROPAGATE;

  if (!gtk_widget_get_realized (priv->entry))
    gtk_widget_realize (priv->entry);

  handled = GDK_EVENT_PROPAGATE;
  preedit_changed = FALSE;
  preedit_change_id = g_signal_connect (priv->entry, "preedit-changed",
                                        G_CALLBACK (preedit_changed_cb), &preedit_changed);

  old_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry)));
  res = gtk_widget_event (priv->entry, event);
  new_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry)));

  g_signal_handler_disconnect (priv->entry, preedit_change_id);

  if ((res && g_strcmp0 (new_text, old_text) != 0) || preedit_changed)
    handled = GDK_EVENT_STOP;

  g_free (old_text);
  g_free (new_text);

  return handled;
}

/**
 * hdy_search_bar_handle_event:
 * @self: a #HdySearchBar
 * @event: a #GdkEvent containing key press events
 *
 * This function should be called when the top-level
 * window which contains the search bar received a key event.
 *
 * If the key event is handled by the search bar, the bar will
 * be shown, the entry populated with the entered text and %GDK_EVENT_STOP
 * will be returned. The caller should ensure that events are
 * not propagated further.
 *
 * If no entry has been connected to the search bar, using
 * hdy_search_bar_connect_entry(), this function will return
 * immediately with a warning.
 *
 * ## Showing the search bar on key presses
 *
 * |[<!-- language="C" -->
 * static gboolean
 * on_key_press_event (GtkWidget *widget,
 *                     GdkEvent  *event,
 *                     gpointer   user_data)
 * {
 *   HdySearchBar *bar = HDY_SEARCH_BAR (user_data);
 *   return hdy_search_bar_handle_event (self, event);
 * }
 *
 * static void
 * create_toplevel (void)
 * {
 *   GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
 *   GtkWindow *search_bar = hdy_search_bar_new ();
 *
 *  // Add more widgets to the window...
 *
 *   g_signal_connect (window,
 *                    "key-press-event",
 *                     G_CALLBACK (on_key_press_event),
 *                     search_bar);
 * }
 * ]|
 *
 * Returns: %GDK_EVENT_STOP if the key press event resulted
 *     in text being entered in the search entry (and revealing
 *     the search bar if necessary), %GDK_EVENT_PROPAGATE otherwise.
 *
 * Since: 0.0.6
 */
gboolean
hdy_search_bar_handle_event (HdySearchBar *self,
                             GdkEvent     *event)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);
  gboolean handled;

  if (priv->reveal_child)
    return GDK_EVENT_PROPAGATE;

  if (priv->entry == NULL) {
    g_warning ("The search bar does not have an entry connected to it. Call hdy_search_bar_connect_entry() to connect one.");

    return GDK_EVENT_PROPAGATE;
  }

  if (GTK_IS_SEARCH_ENTRY (priv->entry))
    handled = gtk_search_entry_handle_event (GTK_SEARCH_ENTRY (priv->entry), event);
  else
    handled = hdy_search_bar_handle_event_for_entry (self, event);

  if (handled == GDK_EVENT_STOP)
    gtk_revealer_set_reveal_child (GTK_REVEALER (priv->revealer), TRUE);

  return handled;
}

static void
reveal_child_changed_cb (GObject      *object,
                         GParamSpec   *pspec,
                         HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);
  gboolean reveal_child;

  g_object_get (object, "reveal-child", &reveal_child, NULL);
  if (reveal_child)
    gtk_widget_set_child_visible (priv->revealer, TRUE);

  if (reveal_child == priv->reveal_child)
    return;

  priv->reveal_child = reveal_child;

  if (priv->entry) {
    if (reveal_child)
      gtk_entry_grab_focus_without_selecting (GTK_ENTRY (priv->entry));
    else
      gtk_entry_set_text (GTK_ENTRY (priv->entry), "");
  }

  g_object_notify (G_OBJECT (self), "search-mode-enabled");
}

static void
child_revealed_changed_cb (GObject      *object,
                           GParamSpec   *pspec,
                           HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);
  gboolean val;

  g_object_get (object, "child-revealed", &val, NULL);
  if (!val)
    gtk_widget_set_child_visible (priv->revealer, FALSE);
}

static void
close_button_clicked_cb (GtkWidget    *button,
                         HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->revealer), FALSE);
}

static void
hdy_search_bar_add (GtkContainer *container,
                    GtkWidget    *child)
{
  HdySearchBar *self = HDY_SEARCH_BAR (container);
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  if (priv->revealer == NULL) {
    GTK_CONTAINER_CLASS (hdy_search_bar_parent_class)->add (container, child);
  } else {
    gtk_box_set_center_widget (GTK_BOX (priv->tool_box), child);
    gtk_container_child_set (GTK_CONTAINER (priv->tool_box), child,
                             "expand", TRUE,
                             NULL);
    /* If an entry is the only child, save the developer a couple of
     * lines of code
     */
    if (GTK_IS_ENTRY (child))
      hdy_search_bar_connect_entry (self, GTK_ENTRY (child));
  }
}

static void
hdy_search_bar_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  HdySearchBar *self = HDY_SEARCH_BAR (object);

  switch (prop_id) {
  case PROP_SEARCH_MODE_ENABLED:
    hdy_search_bar_set_search_mode (self, g_value_get_boolean (value));
    break;
  case PROP_SHOW_CLOSE_BUTTON:
    hdy_search_bar_set_show_close_button (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_search_bar_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  HdySearchBar *self = HDY_SEARCH_BAR (object);
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  switch (prop_id) {
  case PROP_SEARCH_MODE_ENABLED:
    g_value_set_boolean (value, priv->reveal_child);
    break;
  case PROP_SHOW_CLOSE_BUTTON:
    g_value_set_boolean (value, hdy_search_bar_get_show_close_button (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void hdy_search_bar_set_entry (HdySearchBar *self,
                                      GtkEntry     *entry);

static void
hdy_search_bar_dispose (GObject *object)
{
  HdySearchBar *self = HDY_SEARCH_BAR (object);

  hdy_search_bar_set_entry (self, NULL);

  G_OBJECT_CLASS (hdy_search_bar_parent_class)->dispose (object);
}

static gboolean
hdy_search_bar_draw (GtkWidget *widget,
                     cairo_t *cr)
{
  gint width, height;
  GtkStyleContext *context;

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);
  context = gtk_widget_get_style_context (widget);

  gtk_render_background (context, cr, 0, 0, width, height);
  gtk_render_frame (context, cr, 0, 0, width, height);

  GTK_WIDGET_CLASS (hdy_search_bar_parent_class)->draw (widget, cr);

  return FALSE;
}

static void
hdy_search_bar_class_init (HdySearchBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->dispose = hdy_search_bar_dispose;
  object_class->set_property = hdy_search_bar_set_property;
  object_class->get_property = hdy_search_bar_get_property;
  widget_class->draw = hdy_search_bar_draw;

  container_class->add = hdy_search_bar_add;

  /**
   * HdySearchBar:search-mode-enabled:
   *
   * Whether the search mode is on and the search bar shown.
   *
   * See hdy_search_bar_set_search_mode() for details.
   */
  props[PROP_SEARCH_MODE_ENABLED] =
    g_param_spec_boolean ("search-mode-enabled",
                          _("Search Mode Enabled"),
                          _("Whether the search mode is on and the search bar shown"),
                          FALSE,
                          G_PARAM_READWRITE|G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdySearchBar:show-close-button:
   *
   * Whether to show the close button in the toolbar.
   */
  props[PROP_SHOW_CLOSE_BUTTON] =
    g_param_spec_boolean ("show-close-button",
                          _("Show Close Button"),
                          _("Whether to show the close button in the toolbar"),
                          FALSE,
                          G_PARAM_READWRITE|G_PARAM_CONSTRUCT|G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROPERTY, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-search-bar.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdySearchBar, tool_box);
  gtk_widget_class_bind_template_child_private (widget_class, HdySearchBar, revealer);
  gtk_widget_class_bind_template_child_private (widget_class, HdySearchBar, start);
  gtk_widget_class_bind_template_child_private (widget_class, HdySearchBar, end);
  gtk_widget_class_bind_template_child_private (widget_class, HdySearchBar, close_button);

  gtk_widget_class_set_css_name (widget_class, "searchbar");
}

static void
hdy_search_bar_init (HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  /* We use child-visible to avoid the unexpanded revealer
   * peaking out by 1 pixel
   */
  gtk_widget_set_child_visible (priv->revealer, FALSE);

  g_signal_connect (priv->revealer, "notify::reveal-child",
                    G_CALLBACK (reveal_child_changed_cb), self);
  g_signal_connect (priv->revealer, "notify::child-revealed",
                    G_CALLBACK (child_revealed_changed_cb), self);

  gtk_widget_set_no_show_all (priv->start, TRUE);
  gtk_widget_set_no_show_all (priv->end, TRUE);
  g_signal_connect (priv->close_button, "clicked",
                    G_CALLBACK (close_button_clicked_cb), self);
};

/**
 * hdy_search_bar_new:
 *
 * Creates a #HdySearchBar. You will need to tell it about
 * which widget is going to be your text entry using
 * hdy_search_bar_connect_entry().
 *
 * Returns: a new #HdySearchBar
 *
 * Since: 0.0.6
 */
GtkWidget *
hdy_search_bar_new (void)
{
  return g_object_new (HDY_TYPE_SEARCH_BAR, NULL);
}

static void
hdy_search_bar_set_entry (HdySearchBar *self,
                          GtkEntry     *entry)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  if (priv->entry != NULL) {
    if (GTK_IS_SEARCH_ENTRY (priv->entry))
      g_signal_handlers_disconnect_by_func (priv->entry, stop_search_cb, self);
    else
      g_signal_handlers_disconnect_by_func (priv->entry, entry_key_pressed_event_cb, self);
    g_object_remove_weak_pointer (G_OBJECT (priv->entry), (gpointer *) &priv->entry);
  }

  priv->entry = GTK_WIDGET (entry);

  if (priv->entry != NULL) {
    g_object_add_weak_pointer (G_OBJECT (priv->entry), (gpointer *) &priv->entry);
    if (GTK_IS_SEARCH_ENTRY (priv->entry))
      g_signal_connect (priv->entry, "stop-search",
                        G_CALLBACK (stop_search_cb), self);
    else
      g_signal_connect (priv->entry, "key-press-event",
                        G_CALLBACK (entry_key_pressed_event_cb), self);
  }
}

/**
 * hdy_search_bar_connect_entry:
 * @self: a #HdySearchBar
 * @entry: a #GtkEntry
 *
 * Connects the #GtkEntry widget passed as the one to be used in
 * this search bar. The entry should be a descendant of the search bar.
 * This is only required if the entry isn’t the direct child of the
 * search bar (as in our main example).
 *
 * Since: 0.0.6
 */
void
hdy_search_bar_connect_entry (HdySearchBar *self,
                              GtkEntry     *entry)
{
  g_return_if_fail (HDY_IS_SEARCH_BAR (self));
  g_return_if_fail (entry == NULL || GTK_IS_ENTRY (entry));

  hdy_search_bar_set_entry (self, entry);
}

/**
 * hdy_search_bar_get_search_mode:
 * @self: a #HdySearchBar
 *
 * Returns whether the search mode is on or off.
 *
 * Returns: whether search mode is toggled on
 *
 * Since: 0.0.6
 */
gboolean
hdy_search_bar_get_search_mode (HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_SEARCH_BAR (self), FALSE);

  return priv->reveal_child;
}

/**
 * hdy_search_bar_set_search_mode:
 * @self: a #HdySearchBar
 * @search_mode: the new state of the search mode
 *
 * Switches the search mode on or off.
 *
 * Since: 0.0.6
 */
void
hdy_search_bar_set_search_mode (HdySearchBar *self,
                                gboolean      search_mode)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  g_return_if_fail (HDY_IS_SEARCH_BAR (self));

  gtk_revealer_set_reveal_child (GTK_REVEALER (priv->revealer), search_mode);
}

/**
 * hdy_search_bar_get_show_close_button:
 * @self: a #HdySearchBar
 *
 * Returns whether the close button is shown.
 *
 * Returns: whether the close button is shown
 *
 * Since: 0.0.6
 */
gboolean
hdy_search_bar_get_show_close_button (HdySearchBar *self)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_SEARCH_BAR (self), FALSE);

  return priv->show_close_button;
}

/**
 * hdy_search_bar_set_show_close_button:
 * @self: a #HdySearchBar
 * @visible: whether the close button will be shown or not
 *
 * Shows or hides the close button. Applications that
 * already have a “search” toggle button should not show a close
 * button in their search bar, as it duplicates the role of the
 * toggle button.
 *
 * Since: 0.0.6
 */
void
hdy_search_bar_set_show_close_button (HdySearchBar *self,
                                      gboolean      visible)
{
  HdySearchBarPrivate *priv = hdy_search_bar_get_instance_private (self);

  g_return_if_fail (HDY_IS_SEARCH_BAR (self));

  visible = visible != FALSE;

  if (priv->show_close_button == visible)
    return;

  priv->show_close_button = visible;
  gtk_widget_set_visible (priv->start, visible);
  gtk_widget_set_visible (priv->end, visible);
  g_object_notify (G_OBJECT (self), "show-close-button");
}
