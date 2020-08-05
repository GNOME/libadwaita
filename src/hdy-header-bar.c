/*
 * Copyright (c) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-header-bar.h"

#include "hdy-animation-private.h"
#include "hdy-cairo-private.h"
#include "hdy-css-private.h"
#include "hdy-enums.h"
#include "hdy-window-handle-controller-private.h"
#include "gtkprogresstrackerprivate.h"
#include "gtk-window-private.h"

/**
 * SECTION:hdy-header-bar
 * @short_description: A box with a centered child.
 * @Title: HdyHeaderBar
 * @See_also: #GtkHeaderBar, #HdyApplicationWindow, #HdyTitleBar, #HdyViewSwitcher, #HdyWindow
 *
 * HdyHeaderBar is similar to #GtkHeaderBar but is designed to fix some of its
 * shortcomings for adaptive applications.
 *
 * HdyHeaderBar doesn't force the custom title widget to be vertically centered,
 * hence allowing it to fill up the whole height, which is e.g. needed for
 * #HdyViewSwitcher.
 *
 * When used in a mobile dialog, HdyHeaderBar will replace its window
 * decorations by a back button allowing to close it. It doesn't have to be its
 * direct child and you can use any complex contraption you like as the dialog's
 * titlebar.
 *
 * #HdyHeaderBar can be used in window's content area rather than titlebar, and
 * will still be draggable and will handle right click, middle click and double
 * click as expected from a titlebar. This is particularly useful with
 * #HdyWindow or #HdyApplicationWindow.
 *
 * # CSS nodes
 *
 * #HdyHeaderBar has a single CSS node with name headerbar.
 */

/**
 * HdyCenteringPolicy:
 * @HDY_CENTERING_POLICY_LOOSE: Keep the title centered when possible
 * @HDY_CENTERING_POLICY_STRICT: Keep the title centered at all cost
 */

#define DEFAULT_SPACING 6
#define MIN_TITLE_CHARS 5

#define MOBILE_WINDOW_WIDTH  480
#define MOBILE_WINDOW_HEIGHT 800

typedef struct {
  gchar *title;
  gchar *subtitle;
  GtkWidget *title_label;
  GtkWidget *subtitle_label;
  GtkWidget *label_box;
  GtkWidget *label_sizing_box;
  GtkWidget *subtitle_sizing_label;
  GtkWidget *custom_title;
  gint spacing;
  gboolean has_subtitle;

  GList *children;

  gboolean shows_wm_decorations;
  gchar *decoration_layout;
  gboolean decoration_layout_set;

  GtkWidget *titlebar_start_box;
  GtkWidget *titlebar_end_box;

  GtkWidget *titlebar_start_separator;
  GtkWidget *titlebar_end_separator;

  GtkWidget *titlebar_icon;

  guint tick_id;
  GtkProgressTracker tracker;
  gboolean first_frame_skipped;

  HdyCenteringPolicy centering_policy;
  guint transition_duration;
  gboolean interpolate_size;

  gboolean is_mobile_window;

  gulong window_size_allocated_id;

  HdyWindowHandleController *controller;
} HdyHeaderBarPrivate;

typedef struct _Child Child;
struct _Child
{
  GtkWidget *widget;
  GtkPackType pack_type;
};

enum {
  PROP_0,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_HAS_SUBTITLE,
  PROP_CUSTOM_TITLE,
  PROP_SPACING,
  PROP_SHOW_CLOSE_BUTTON,
  PROP_DECORATION_LAYOUT,
  PROP_DECORATION_LAYOUT_SET,
  PROP_CENTERING_POLICY,
  PROP_TRANSITION_DURATION,
  PROP_TRANSITION_RUNNING,
  PROP_INTERPOLATE_SIZE,
  LAST_PROP
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_PACK_TYPE,
  CHILD_PROP_POSITION
};

static GParamSpec *props[LAST_PROP] = { NULL, };

static void hdy_header_bar_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyHeaderBar, hdy_header_bar, GTK_TYPE_CONTAINER,
                         G_ADD_PRIVATE (HdyHeaderBar)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
                                                hdy_header_bar_buildable_init));

static gboolean
hdy_header_bar_transition_cb (GtkWidget     *widget,
                              GdkFrameClock *frame_clock,
                              gpointer       user_data)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (widget);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  if (priv->first_frame_skipped)
    gtk_progress_tracker_advance_frame (&priv->tracker,
                                        gdk_frame_clock_get_frame_time (frame_clock));
  else
    priv->first_frame_skipped = TRUE;

  /* Finish the animation early if the widget isn't mapped anymore. */
  if (!gtk_widget_get_mapped (widget))
    gtk_progress_tracker_finish (&priv->tracker);

  gtk_widget_queue_resize (widget);

  if (gtk_progress_tracker_get_state (&priv->tracker) == GTK_PROGRESS_STATE_AFTER) {
    priv->tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);

    return FALSE;
  }

  return TRUE;
}

static void
hdy_header_bar_schedule_ticks (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  if (priv->tick_id == 0) {
    priv->tick_id =
      gtk_widget_add_tick_callback (GTK_WIDGET (self), hdy_header_bar_transition_cb, self, NULL);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
  }
}

static void
hdy_header_bar_unschedule_ticks (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  if (priv->tick_id != 0) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), priv->tick_id);
    priv->tick_id = 0;
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_RUNNING]);
  }
}

static void
hdy_header_bar_start_transition (HdyHeaderBar *self,
                                 guint         transition_duration)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (self);

  if (gtk_widget_get_mapped (widget) &&
      priv->interpolate_size &&
      transition_duration != 0) {
    priv->first_frame_skipped = FALSE;
    hdy_header_bar_schedule_ticks (self);
    gtk_progress_tracker_start (&priv->tracker,
                                priv->transition_duration * 1000,
                                0,
                                1.0);
  } else {
    hdy_header_bar_unschedule_ticks (self);
    gtk_progress_tracker_finish (&priv->tracker);
  }

  gtk_widget_queue_resize (widget);
}

static void
init_sizing_box (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkWidget *w;
  GtkStyleContext *context;

  /* We use this box to always request size for the two labels (title
   * and subtitle) as if they were always visible, but then allocate
   * the real label box with its actual size, to keep it center-aligned
   * in case we have only the title.
   */
  w = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_show (w);
  priv->label_sizing_box = g_object_ref_sink (w);

  w = gtk_label_new (NULL);
  gtk_widget_show (w);
  context = gtk_widget_get_style_context (w);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_TITLE);
  gtk_box_pack_start (GTK_BOX (priv->label_sizing_box), w, FALSE, FALSE, 0);
  gtk_label_set_line_wrap (GTK_LABEL (w), FALSE);
  gtk_label_set_single_line_mode (GTK_LABEL (w), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (w), PANGO_ELLIPSIZE_END);
  gtk_label_set_width_chars (GTK_LABEL (w), MIN_TITLE_CHARS);

  w = gtk_label_new (NULL);
  context = gtk_widget_get_style_context (w);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_SUBTITLE);
  gtk_box_pack_start (GTK_BOX (priv->label_sizing_box), w, FALSE, FALSE, 0);
  gtk_label_set_line_wrap (GTK_LABEL (w), FALSE);
  gtk_label_set_single_line_mode (GTK_LABEL (w), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (w), PANGO_ELLIPSIZE_END);
  gtk_widget_set_visible (w, priv->has_subtitle || (priv->subtitle && priv->subtitle[0]));
  priv->subtitle_sizing_label = w;
}

static GtkWidget *
create_title_box (const char *title,
                  const char *subtitle,
                  GtkWidget **ret_title_label,
                  GtkWidget **ret_subtitle_label)
{
  GtkWidget *label_box;
  GtkWidget *title_label;
  GtkWidget *subtitle_label;
  GtkStyleContext *context;

  label_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_valign (label_box, GTK_ALIGN_CENTER);
  gtk_widget_show (label_box);

  title_label = gtk_label_new (title);
  context = gtk_widget_get_style_context (title_label);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_TITLE);
  gtk_label_set_line_wrap (GTK_LABEL (title_label), FALSE);
  gtk_label_set_single_line_mode (GTK_LABEL (title_label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (title_label), PANGO_ELLIPSIZE_END);
  gtk_box_pack_start (GTK_BOX (label_box), title_label, FALSE, FALSE, 0);
  gtk_widget_show (title_label);
  gtk_label_set_width_chars (GTK_LABEL (title_label), MIN_TITLE_CHARS);

  subtitle_label = gtk_label_new (subtitle);
  context = gtk_widget_get_style_context (subtitle_label);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_SUBTITLE);
  gtk_label_set_line_wrap (GTK_LABEL (subtitle_label), FALSE);
  gtk_label_set_single_line_mode (GTK_LABEL (subtitle_label), TRUE);
  gtk_label_set_ellipsize (GTK_LABEL (subtitle_label), PANGO_ELLIPSIZE_END);
  gtk_box_pack_start (GTK_BOX (label_box), subtitle_label, FALSE, FALSE, 0);
  gtk_widget_set_no_show_all (subtitle_label, TRUE);
  gtk_widget_set_visible (subtitle_label, subtitle && subtitle[0]);

  if (ret_title_label)
    *ret_title_label = title_label;
  if (ret_subtitle_label)
    *ret_subtitle_label = subtitle_label;

  return label_box;
}

static gboolean
hdy_header_bar_update_window_icon (HdyHeaderBar *self,
                                   GtkWindow    *window)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GdkPixbuf *pixbuf;
  gint scale;

  if (priv->titlebar_icon == NULL)
    return FALSE;

  scale = gtk_widget_get_scale_factor (priv->titlebar_icon);
  if (GTK_IS_BUTTON (gtk_widget_get_parent (priv->titlebar_icon)))
    pixbuf = hdy_gtk_window_get_icon_for_size (window, scale * 16);
  else
    pixbuf = hdy_gtk_window_get_icon_for_size (window, scale * 20);

  if (pixbuf) {
    g_autoptr (cairo_surface_t) surface =
      gdk_cairo_surface_create_from_pixbuf (pixbuf, scale, gtk_widget_get_window (priv->titlebar_icon));

    gtk_image_set_from_surface (GTK_IMAGE (priv->titlebar_icon), surface);
    g_object_unref (pixbuf);
    gtk_widget_show (priv->titlebar_icon);

    return TRUE;
  }

  return FALSE;
}

static void
_hdy_header_bar_update_separator_visibility (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  gboolean have_visible_at_start = FALSE;
  gboolean have_visible_at_end = FALSE;
  GList *l;

  for (l = priv->children; l != NULL; l = l->next) {
    Child *child = l->data;

    if (gtk_widget_get_visible (child->widget)) {
      if (child->pack_type == GTK_PACK_START)
        have_visible_at_start = TRUE;
      else
        have_visible_at_end = TRUE;
    }
  }

  if (priv->titlebar_start_separator != NULL)
    gtk_widget_set_visible (priv->titlebar_start_separator, have_visible_at_start);

  if (priv->titlebar_end_separator != NULL)
    gtk_widget_set_visible (priv->titlebar_end_separator, have_visible_at_end);
}

static void
hdy_header_bar_update_window_buttons (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkWidget *widget = GTK_WIDGET (self), *toplevel;
  GtkWindow *window;
  GtkTextDirection direction;
  gchar *layout_desc;
  gchar **tokens, **t;
  gint i, j;
  GMenuModel *menu;
  gboolean shown_by_shell;
  gboolean is_sovereign_window;
  gboolean is_mobile_dialog;

  toplevel = gtk_widget_get_toplevel (widget);
  if (!gtk_widget_is_toplevel (toplevel))
    return;

  if (priv->titlebar_start_box) {
    gtk_widget_unparent (priv->titlebar_start_box);
    priv->titlebar_start_box = NULL;
    priv->titlebar_start_separator = NULL;
  }
  if (priv->titlebar_end_box) {
    gtk_widget_unparent (priv->titlebar_end_box);
    priv->titlebar_end_box = NULL;
    priv->titlebar_end_separator = NULL;
  }

  priv->titlebar_icon = NULL;

  if (!priv->shows_wm_decorations)
    return;

  direction = gtk_widget_get_direction (widget);

  g_object_get (gtk_widget_get_settings (widget),
                "gtk-shell-shows-app-menu", &shown_by_shell,
                "gtk-decoration-layout", &layout_desc,
                NULL);

  if (priv->decoration_layout_set) {
    g_free (layout_desc);
    layout_desc = g_strdup (priv->decoration_layout);
  }

  window = GTK_WINDOW (toplevel);

  if (!shown_by_shell && gtk_window_get_application (window))
    menu = gtk_application_get_app_menu (gtk_window_get_application (window));
  else
    menu = NULL;

  is_sovereign_window = (!gtk_window_get_modal (window) &&
                          gtk_window_get_transient_for (window) == NULL &&
                          gtk_window_get_type_hint (window) == GDK_WINDOW_TYPE_HINT_NORMAL);

  is_mobile_dialog= (priv->is_mobile_window && !is_sovereign_window);

  tokens = g_strsplit (layout_desc, ":", 2);
  if (tokens)   {
    for (i = 0; i < 2; i++) {
      GtkWidget *box;
      GtkWidget *separator;
      int n_children = 0;

      if (tokens[i] == NULL)
        break;

      t = g_strsplit (tokens[i], ",", -1);

      separator = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
      gtk_widget_set_no_show_all (separator, TRUE);
      gtk_style_context_add_class (gtk_widget_get_style_context (separator), "titlebutton");

      box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, priv->spacing);

      for (j = 0; t[j]; j++) {
        GtkWidget *button = NULL;
        GtkWidget *image = NULL;
        AtkObject *accessible;

        if (strcmp (t[j], "icon") == 0 &&
            is_sovereign_window) {
            button = gtk_image_new ();
            gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
            priv->titlebar_icon = button;
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "titlebutton");
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "icon");
            gtk_widget_set_size_request (button, 20, 20);
            gtk_widget_show (button);

            if (!hdy_header_bar_update_window_icon (self, window))
              {
                gtk_widget_destroy (button);
                priv->titlebar_icon = NULL;
                button = NULL;
              }
        } else if (strcmp (t[j], "menu") == 0 &&
                 menu != NULL &&
                 is_sovereign_window) {
            button = gtk_menu_button_new ();
            gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
            gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button), menu);
            gtk_menu_button_set_use_popover (GTK_MENU_BUTTON (button), TRUE);
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "titlebutton");
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "appmenu");
            image = gtk_image_new ();
            gtk_container_add (GTK_CONTAINER (button), image);
            gtk_widget_set_can_focus (button, FALSE);
            gtk_widget_show_all (button);

            accessible = gtk_widget_get_accessible (button);
            if (GTK_IS_ACCESSIBLE (accessible))
              atk_object_set_name (accessible, _("Application menu"));

            priv->titlebar_icon = image;
            if (!hdy_header_bar_update_window_icon (self, window))
              gtk_image_set_from_icon_name (GTK_IMAGE (priv->titlebar_icon),
                                            "application-x-executable-symbolic", GTK_ICON_SIZE_MENU);
        } else if (strcmp (t[j], "minimize") == 0 &&
                 is_sovereign_window) {
            button = gtk_button_new ();
            gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "titlebutton");
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "minimize");
            image = gtk_image_new_from_icon_name ("window-minimize-symbolic", GTK_ICON_SIZE_MENU);
            g_object_set (image, "use-fallback", TRUE, NULL);
            gtk_container_add (GTK_CONTAINER (button), image);
            gtk_widget_set_can_focus (button, FALSE);
            gtk_widget_show_all (button);
            g_signal_connect_swapped (button, "clicked",
                                      G_CALLBACK (gtk_window_iconify), window);

            accessible = gtk_widget_get_accessible (button);
            if (GTK_IS_ACCESSIBLE (accessible))
              atk_object_set_name (accessible, _("Minimize"));
        } else if (strcmp (t[j], "maximize") == 0 &&
                 gtk_window_get_resizable (window) &&
                 is_sovereign_window) {
            const gchar *icon_name;
            gboolean maximized = gtk_window_is_maximized (window);

            icon_name = maximized ? "window-restore-symbolic" : "window-maximize-symbolic";
            button = gtk_button_new ();
            gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "titlebutton");
            gtk_style_context_add_class (gtk_widget_get_style_context (button), "maximize");
            image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);
            g_object_set (image, "use-fallback", TRUE, NULL);
            gtk_container_add (GTK_CONTAINER (button), image);
            gtk_widget_set_can_focus (button, FALSE);
            gtk_widget_show_all (button);
            g_signal_connect_swapped (button, "clicked",
                                      G_CALLBACK (hdy_gtk_window_toggle_maximized), window);

            accessible = gtk_widget_get_accessible (button);
            if (GTK_IS_ACCESSIBLE (accessible))
              atk_object_set_name (accessible, maximized ? _("Restore") : _("Maximize"));
        } else if (strcmp (t[j], "close") == 0 &&
                   gtk_window_get_deletable (window) &&
                   !is_mobile_dialog) {
          button = gtk_button_new ();
          gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
          image = gtk_image_new_from_icon_name ("window-close-symbolic", GTK_ICON_SIZE_MENU);
          gtk_style_context_add_class (gtk_widget_get_style_context (button), "titlebutton");
          gtk_style_context_add_class (gtk_widget_get_style_context (button), "close");
          g_object_set (image, "use-fallback", TRUE, NULL);
          gtk_container_add (GTK_CONTAINER (button), image);
          gtk_widget_set_can_focus (button, FALSE);
          gtk_widget_show_all (button);
          g_signal_connect_swapped (button, "clicked",
                                    G_CALLBACK (gtk_window_close), window);

          accessible = gtk_widget_get_accessible (button);
          if (GTK_IS_ACCESSIBLE (accessible))
            atk_object_set_name (accessible, _("Close"));
        } else if (i == 0 && /* Only at the start. */
                   gtk_window_get_deletable (window) &&
                   is_mobile_dialog) {
          button = gtk_button_new ();
          gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
          image = gtk_image_new_from_icon_name ("go-previous-symbolic", GTK_ICON_SIZE_BUTTON);
          g_object_set (image, "use-fallback", TRUE, NULL);
          gtk_container_add (GTK_CONTAINER (button), image);
          gtk_widget_set_can_focus (button, TRUE);
          gtk_widget_show_all (button);
          g_signal_connect_swapped (button, "clicked",
                                    G_CALLBACK (gtk_window_close), window);

          accessible = gtk_widget_get_accessible (button);
          if (GTK_IS_ACCESSIBLE (accessible))
            atk_object_set_name (accessible, _("Back"));
        }

        if (button) {
          gtk_box_pack_start (GTK_BOX (box), button, FALSE, FALSE, 0);
          n_children ++;
        }
      }
      g_strfreev (t);

      if (n_children == 0) {
        g_object_ref_sink (box);
        g_object_unref (box);
        g_object_ref_sink (separator);
        g_object_unref (separator);
        continue;
      }

      gtk_box_pack_start (GTK_BOX (box), separator, FALSE, FALSE, 0);
      if (i == 1)
        gtk_box_reorder_child (GTK_BOX (box), separator, 0);

      if ((direction == GTK_TEXT_DIR_LTR && i == 0) ||
          (direction == GTK_TEXT_DIR_RTL && i == 1))
        gtk_style_context_add_class (gtk_widget_get_style_context (box), GTK_STYLE_CLASS_LEFT);
      else
        gtk_style_context_add_class (gtk_widget_get_style_context (box), GTK_STYLE_CLASS_RIGHT);

      gtk_widget_show (box);
      gtk_widget_set_parent (box, GTK_WIDGET (self));

      if (i == 0) {
        priv->titlebar_start_box = box;
        priv->titlebar_start_separator = separator;
      } else {
        priv->titlebar_end_box = box;
        priv->titlebar_end_separator = separator;
      }
    }
    g_strfreev (tokens);
  }
  g_free (layout_desc);

  _hdy_header_bar_update_separator_visibility (self);
}

static gboolean
compute_is_mobile_window (GtkWindow *window)
{
  gint window_width, window_height;

  gtk_window_get_size (window, &window_width, &window_height);

  if (window_width <= MOBILE_WINDOW_WIDTH &&
      gtk_window_is_maximized (window))
    return TRUE;

  /* Mobile landscape mode. */
  if (window_width <= MOBILE_WINDOW_HEIGHT &&
      window_height <= MOBILE_WINDOW_WIDTH &&
      gtk_window_is_maximized (window))
    return TRUE;

  return FALSE;
}

static void
update_is_mobile_window (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));
  gboolean was_mobile_window = priv->is_mobile_window;

  if (!gtk_widget_is_toplevel (toplevel))
    return;

  priv->is_mobile_window = compute_is_mobile_window (GTK_WINDOW (toplevel));

  if (priv->is_mobile_window != was_mobile_window)
    hdy_header_bar_update_window_buttons (self);
}

static void
construct_label_box (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_assert (priv->label_box == NULL);

  priv->label_box = create_title_box (priv->title,
                                      priv->subtitle,
                                      &priv->title_label,
                                      &priv->subtitle_label);
  gtk_widget_set_parent (priv->label_box, GTK_WIDGET (self));
}

static gint
count_visible_children (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  Child *child;
  gint n;

  n = 0;
  for (l = priv->children; l; l = l->next) {
    child = l->data;
    if (gtk_widget_get_visible (child->widget))
      n++;
  }

  return n;
}

static gint
count_visible_children_for_pack_type (HdyHeaderBar *self, GtkPackType pack_type)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  Child *child;
  gint n;

  n = 0;
  for (l = priv->children; l; l = l->next) {
    child = l->data;
    if (gtk_widget_get_visible (child->widget) && child->pack_type == pack_type)
      n++;
  }

  return n;
}

static gboolean
add_child_size (GtkWidget      *child,
                GtkOrientation  orientation,
                gint           *minimum,
                gint           *natural)
{
  gint child_minimum, child_natural;

  if (!gtk_widget_get_visible (child))
    return FALSE;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_get_preferred_width (child, &child_minimum, &child_natural);
  else
    gtk_widget_get_preferred_height (child, &child_minimum, &child_natural);

  if (GTK_ORIENTATION_HORIZONTAL == orientation) {
    *minimum += child_minimum;
    *natural += child_natural;
  } else {
    *minimum = MAX (*minimum, child_minimum);
    *natural = MAX (*natural, child_natural);
  }

  return TRUE;
}

static void
hdy_header_bar_get_size (GtkWidget      *widget,
                         GtkOrientation  orientation,
                         gint           *minimum,
                         gint           *natural)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (widget);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  gint n_start_children = 0, n_end_children = 0;
  gint start_min = 0, start_nat = 0;
  gint end_min = 0, end_nat = 0;
  gint center_min = 0, center_nat = 0;

  for (l = priv->children; l; l = l->next) {
    Child *child = l->data;

    if (child->pack_type == GTK_PACK_START) {
      if (add_child_size (child->widget, orientation, &start_min, &start_nat))
        n_start_children += 1;
    } else {
      if (add_child_size (child->widget, orientation, &end_min, &end_nat))
        n_end_children += 1;
    }
  }

  if (priv->label_box != NULL) {
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      add_child_size (priv->label_box, orientation, &center_min, &center_nat);
    else
      add_child_size (priv->label_sizing_box, orientation, &center_min, &center_nat);
  }

  if (priv->custom_title != NULL)
    add_child_size (priv->custom_title, orientation, &center_min, &center_nat);

  if (priv->titlebar_start_box != NULL) {
    if (add_child_size (priv->titlebar_start_box, orientation, &start_min, &start_nat))
      n_start_children += 1;
  }

  if (priv->titlebar_end_box != NULL) {
    if (add_child_size (priv->titlebar_end_box, orientation, &end_min, &end_nat))
      n_end_children += 1;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    gdouble strict_centering_t;
    gint start_min_spaced = start_min + n_start_children * priv->spacing;
    gint end_min_spaced = end_min + n_end_children * priv->spacing;
    gint start_nat_spaced = start_nat + n_start_children * priv->spacing;
    gint end_nat_spaced = end_nat + n_end_children * priv->spacing;

    if (gtk_progress_tracker_get_state (&priv->tracker) != GTK_PROGRESS_STATE_AFTER) {
      strict_centering_t = gtk_progress_tracker_get_ease_out_cubic (&priv->tracker, FALSE);
      if (priv->centering_policy != HDY_CENTERING_POLICY_STRICT)
        strict_centering_t = 1.0 - strict_centering_t;
    } else
      strict_centering_t = priv->centering_policy == HDY_CENTERING_POLICY_STRICT ? 1.0 : 0.0;

    *minimum = center_min + n_start_children * priv->spacing +
               hdy_lerp (start_min_spaced + end_min_spaced,
                         2 * MAX (start_min_spaced, end_min_spaced),
                         strict_centering_t);
    *natural = center_nat + n_start_children * priv->spacing +
               hdy_lerp (start_nat_spaced + end_nat_spaced,
                         2 * MAX (start_nat_spaced, end_nat_spaced),
                         strict_centering_t);
  } else {
    *minimum = MAX (MAX (start_min, end_min), center_min);
    *natural = MAX (MAX (start_nat, end_nat), center_nat);
  }
}

static void
hdy_header_bar_compute_size_for_orientation (GtkWidget *widget,
                                             gint       avail_size,
                                             gint      *minimum_size,
                                             gint      *natural_size)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (widget);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *children;
  gint required_size = 0;
  gint required_natural = 0;
  gint child_size;
  gint child_natural;
  gint nvis_children = 0;

  for (children = priv->children; children != NULL; children = children->next) {
    Child *child = children->data;

    if (gtk_widget_get_visible (child->widget)) {
      gtk_widget_get_preferred_width_for_height (child->widget,
                                                 avail_size, &child_size, &child_natural);

      required_size += child_size;
      required_natural += child_natural;

      nvis_children += 1;
    }
  }

  if (priv->label_box != NULL) {
    gtk_widget_get_preferred_width (priv->label_sizing_box,
                                    &child_size, &child_natural);
    required_size += child_size;
    required_natural += child_natural;
  }

  if (priv->custom_title != NULL &&
      gtk_widget_get_visible (priv->custom_title)) {
    gtk_widget_get_preferred_width (priv->custom_title,
                                    &child_size, &child_natural);
    required_size += child_size;
    required_natural += child_natural;
  }

  if (priv->titlebar_start_box != NULL) {
    gtk_widget_get_preferred_width (priv->titlebar_start_box,
                                    &child_size, &child_natural);
    required_size += child_size;
    required_natural += child_natural;
    nvis_children += 1;
  }

  if (priv->titlebar_end_box != NULL) {
    gtk_widget_get_preferred_width (priv->titlebar_end_box,
                                    &child_size, &child_natural);
    required_size += child_size;
    required_natural += child_natural;
    nvis_children += 1;
  }

  required_size += nvis_children * priv->spacing;
  required_natural += nvis_children * priv->spacing;

  *minimum_size = required_size;
  *natural_size = required_natural;
}

static void
hdy_header_bar_compute_size_for_opposing_orientation (GtkWidget *widget,
                                                      gint       avail_size,
                                                      gint      *minimum_size,
                                                      gint      *natural_size)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (widget);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  Child *child;
  GList *children;
  gint nvis_children;
  gint computed_minimum = 0;
  gint computed_natural = 0;
  GtkRequestedSize *sizes;
  GtkPackType packing;
  gint i;
  gint child_size;
  gint child_minimum;
  gint child_natural;
  gint center_min, center_nat;

  nvis_children = count_visible_children (self);

  if (nvis_children <= 0)
    return;

  sizes = g_newa (GtkRequestedSize, nvis_children);

  /* Retrieve desired size for visible children */
  for (i = 0, children = priv->children; children; children = children->next) {
    child = children->data;

    if (gtk_widget_get_visible (child->widget)) {
      gtk_widget_get_preferred_width (child->widget,
                                      &sizes[i].minimum_size,
                                      &sizes[i].natural_size);

      sizes[i].data = child;
      i += 1;
    }
  }

  /* Bring children up to size first */
  gtk_distribute_natural_allocation (MAX (0, avail_size), nvis_children, sizes);

  /* Allocate child positions. */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; ++packing) {
    for (i = 0, children = priv->children; children; children = children->next) {
      child = children->data;

      /* If widget is not visible, skip it. */
      if (!gtk_widget_get_visible (child->widget))
        continue;

      /* If widget is packed differently skip it, but still increment i,
       * since widget is visible and will be handled in next loop
       * iteration.
       */
      if (child->pack_type != packing) {
        i++;
        continue;
      }

      child_size = sizes[i].minimum_size;

      gtk_widget_get_preferred_height_for_width (child->widget,
                                                 child_size, &child_minimum, &child_natural);

      computed_minimum = MAX (computed_minimum, child_minimum);
      computed_natural = MAX (computed_natural, child_natural);
    }
  }

  center_min = center_nat = 0;
  if (priv->label_box != NULL) {
    gtk_widget_get_preferred_height (priv->label_sizing_box,
                                     &center_min, &center_nat);
  }

  if (priv->custom_title != NULL &&
      gtk_widget_get_visible (priv->custom_title)) {
    gtk_widget_get_preferred_height (priv->custom_title,
                                     &center_min, &center_nat);
  }

  if (priv->titlebar_start_box != NULL) {
    gtk_widget_get_preferred_height (priv->titlebar_start_box,
                                     &child_minimum, &child_natural);
    computed_minimum = MAX (computed_minimum, child_minimum);
    computed_natural = MAX (computed_natural, child_natural);
  }

  if (priv->titlebar_end_box != NULL) {
    gtk_widget_get_preferred_height (priv->titlebar_end_box,
                                     &child_minimum, &child_natural);
    computed_minimum = MAX (computed_minimum, child_minimum);
    computed_natural = MAX (computed_natural, child_natural);
  }

  *minimum_size = computed_minimum;
  *natural_size = computed_natural;
}

static void
hdy_header_bar_measure (GtkWidget      *widget,
                        GtkOrientation  orientation,
                        gint            for_size,
                        gint           *minimum,
                        gint           *natural,
                        gint           *minimum_baseline,
                        gint           *natural_baseline)
{
  gint css_width, css_height;

  gtk_style_context_get (gtk_widget_get_style_context (widget),
                         gtk_widget_get_state_flags (widget),
                         "min-width", &css_width,
                         "min-height", &css_height,
                         NULL);

  if (for_size < 0)
    hdy_header_bar_get_size (widget, orientation, minimum, natural);
  else if (orientation == GTK_ORIENTATION_HORIZONTAL)
    hdy_header_bar_compute_size_for_orientation (widget, MAX (for_size, css_height), minimum, natural);
  else
    hdy_header_bar_compute_size_for_opposing_orientation (widget, MAX (for_size, css_width), minimum, natural);

  hdy_css_measure (widget, orientation, minimum, natural);
}

static void
hdy_header_bar_get_preferred_width (GtkWidget *widget,
                                    gint      *minimum,
                                    gint      *natural)
{
  hdy_header_bar_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                          minimum, natural,
                          NULL, NULL);
}

static void
hdy_header_bar_get_preferred_height (GtkWidget *widget,
                                     gint      *minimum,
                                     gint      *natural)
{
  hdy_header_bar_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                          minimum, natural,
                          NULL, NULL);
}

static void
hdy_header_bar_get_preferred_width_for_height (GtkWidget *widget,
                                               gint       height,
                                               gint      *minimum,
                                               gint      *natural)
{
  hdy_header_bar_measure (widget, GTK_ORIENTATION_HORIZONTAL, height,
                          minimum, natural,
                          NULL, NULL);
}

static void
hdy_header_bar_get_preferred_height_for_width (GtkWidget *widget,
                                               gint       width,
                                               gint      *minimum,
                                               gint      *natural)
{
  hdy_header_bar_measure (widget, GTK_ORIENTATION_VERTICAL, width,
                          minimum, natural,
                          NULL, NULL);
}

static GtkWidget *
get_title_size (HdyHeaderBar     *self,
                gint              for_height,
                GtkRequestedSize *size,
                gint             *expanded)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkWidget *title_widget;

  if (priv->custom_title != NULL &&
      gtk_widget_get_visible (priv->custom_title))
    title_widget = priv->custom_title;
  else if (priv->label_box != NULL)
    title_widget = priv->label_box;
  else
    return NULL;

  gtk_widget_get_preferred_width_for_height (title_widget,
                                             for_height,
                                             &(size->minimum_size),
                                             &(size->natural_size));

  *expanded = gtk_widget_compute_expand (title_widget, GTK_ORIENTATION_HORIZONTAL);

  return title_widget;
}

static void
children_allocate (HdyHeaderBar      *self,
                   GtkAllocation     *allocation,
                   GtkAllocation    **allocations,
                   GtkRequestedSize  *sizes,
                   gint               decoration_width[2],
                   gint               uniform_expand_bonus[2],
                   gint               leftover_expand_bonus[2])
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkPackType packing;
  GtkAllocation child_allocation;
  gint x;
  gint i;
  GList *l;
  Child *child;
  gint child_size;
  /* GtkTextDirection direction; */

  /* Allocate the children on both sides of the title. */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    child_allocation.y = allocation->y;
    child_allocation.height = allocation->height;
    if (packing == GTK_PACK_START)
      x = allocation->x + decoration_width[0];
    else
      x = allocation->x + allocation->width - decoration_width[1];

    i = 0;
    for (l = priv->children; l != NULL; l = l->next) {
      child = l->data;
      if (!gtk_widget_get_visible (child->widget))
        continue;

      if (child->pack_type != packing)
        goto next;

      child_size = sizes[i].minimum_size;

      /* If this child is expanded, give it extra space from the reserves. */
      if (gtk_widget_compute_expand (child->widget, GTK_ORIENTATION_HORIZONTAL)) {
        gint expand_bonus;

        expand_bonus = uniform_expand_bonus[packing];

        if (leftover_expand_bonus[packing] > 0) {
          expand_bonus++;
          leftover_expand_bonus[packing]--;
        }

        child_size += expand_bonus;
      }

      child_allocation.width = child_size;

      if (packing == GTK_PACK_START) {
        child_allocation.x = x;
        x += child_size;
        x += priv->spacing;
      } else {
        x -= child_size;
        child_allocation.x = x;
        x -= priv->spacing;
      }

      if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
        child_allocation.x = allocation->x + allocation->width - (child_allocation.x - allocation->x) - child_allocation.width;

      (*allocations)[i] = child_allocation;

      next:
        i++;
    }
  }
}

static void
get_loose_centering_allocations (HdyHeaderBar   *self,
                                 GtkAllocation  *allocation,
                                 GtkAllocation **allocations,
                                 GtkAllocation  *title_allocation,
                                 gint            decoration_width[2])
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkRequestedSize *sizes;
  gint width;
  gint nvis_children;
  GtkRequestedSize title_size = { 0 };
  gboolean title_expands = FALSE;
  gint side[2] = { 0 };
  gint uniform_expand_bonus[2] = { 0 };
  gint leftover_expand_bonus[2] = { 0 };
  gint side_free_space[2] = { 0 };
  gint center_free_space[2] = { 0 };
  gint nexpand_children[2] = { 0 };
  gint center_free_space_min;
  GList *l;
  gint i;
  Child *child;
  GtkPackType packing;

  nvis_children = count_visible_children (self);
  sizes = g_newa (GtkRequestedSize, nvis_children);

  width = allocation->width - nvis_children * priv->spacing;

  i = 0;
  for (l = priv->children; l; l = l->next) {
    child = l->data;
    if (!gtk_widget_get_visible (child->widget))
      continue;

    if (gtk_widget_compute_expand (child->widget, GTK_ORIENTATION_HORIZONTAL))
      nexpand_children[child->pack_type]++;

    gtk_widget_get_preferred_width_for_height (child->widget,
                                               allocation->height,
                                               &sizes[i].minimum_size,
                                               &sizes[i].natural_size);
    width -= sizes[i].minimum_size;
    i++;
  }

  get_title_size (self, allocation->height, &title_size, &title_expands);
  width -= title_size.minimum_size;

  /* Compute the nominal size of the children filling up each side of the title
   * in titlebar.
   */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    i = 0;
    for (l = priv->children; l != NULL; l = l->next) {
      child = l->data;
      if (!gtk_widget_get_visible (child->widget))
        continue;

      if (child->pack_type == packing)
        side[packing] += sizes[i].minimum_size + priv->spacing;

      i++;
    }
  }

  /* Distribute the available space for natural expansion of the children. */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++)
    width -= decoration_width[packing];
  width = gtk_distribute_natural_allocation (MAX (0, width), 1, &title_size);
  width = gtk_distribute_natural_allocation (MAX (0, width), nvis_children, sizes);

  /* Compute the nominal size of the children filling up each side of the title
   * in titlebar.
   */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    i = 0;
    side[packing] = 0;
    for (l = priv->children; l != NULL; l = l->next) {
      child = l->data;
      if (!gtk_widget_get_visible (child->widget))
        continue;

      if (child->pack_type == packing)
        side[packing] += sizes[i].minimum_size + priv->spacing;

      i++;
    }
  }

  /* Figure out how much space is left on each side of the title, and earkmark
   * that space for the expanded children.
   *
   * If the title itself is expanded, then it gets half the spoils from each
   * side.
   */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    side_free_space[packing] = MIN (MAX (allocation->width / 2 - title_size.natural_size / 2 - decoration_width[packing] - side[packing], 0), width);
    if (title_expands)
      center_free_space[packing] = nexpand_children[packing] > 0 ?
        side_free_space[packing] / 2 :
        side_free_space[packing];
  }
  center_free_space_min = MIN (center_free_space[0], center_free_space[1]);
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    center_free_space[packing] = center_free_space_min;
    side_free_space[packing] -= center_free_space[packing];
    width -= side_free_space[packing];

    if (nexpand_children[packing] == 0)
      continue;

    uniform_expand_bonus[packing] = (side_free_space[packing]) / nexpand_children[packing];
    leftover_expand_bonus[packing] = (side_free_space[packing]) % nexpand_children[packing];
  }

  children_allocate (self, allocation, allocations, sizes, decoration_width, uniform_expand_bonus, leftover_expand_bonus);

  /* We don't enforce css borders on the center widget, to make title/subtitle
   * combinations fit without growing the header.
   */
  title_allocation->y = allocation->y;
  title_allocation->height = allocation->height;

  title_allocation->width = MIN (allocation->width - decoration_width[0] - side[0] - decoration_width[1] - side[1],
                                 title_size.natural_size);
  title_allocation->x = allocation->x + (allocation->width - title_allocation->width) / 2;

  /* If the title widget is expanded, then grow it by all the available free
   * space, and recenter it.
   */
  if (title_expands && width > 0) {
    title_allocation->width += width;
    title_allocation->x -= width / 2;
  }

  if (allocation->x + decoration_width[0] + side[0] > title_allocation->x)
    title_allocation->x = allocation->x + decoration_width[0] + side[0];
  else if (allocation->x + allocation->width - decoration_width[1] - side[1] < title_allocation->x + title_allocation->width)
    title_allocation->x = allocation->x + allocation->width - decoration_width[1] - side[1] - title_allocation->width;

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    title_allocation->x = allocation->x + allocation->width - (title_allocation->x - allocation->x) - title_allocation->width;
}

static void
get_strict_centering_allocations (HdyHeaderBar   *self,
                                  GtkAllocation  *allocation,
                                  GtkAllocation **allocations,
                                  GtkAllocation  *title_allocation,
                                  gint            decoration_width[2])
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  GtkRequestedSize *children_sizes = { 0 };
  GtkRequestedSize *children_sizes_for_side[2] = { 0 };
  GtkRequestedSize side_size[2] = { 0 }; /* The size requested by each side. */
  GtkRequestedSize title_size = { 0 }; /* The size requested by the title. */
  GtkRequestedSize side_request = { 0 }; /* The maximum size requested by each side, decoration included. */
  gint side_max; /* The maximum space allocatable to each side, decoration included. */
  gint title_leftover; /* The or 0px or 1px leftover from ensuring each side is allocated the same size. */
  /* The space available for expansion on each side, including for the title. */
  gint free_space[2] = { 0 };
  /* The space the title will take from the free space of each side for its expansion. */
  gint title_expand_bonus = 0;
  gint uniform_expand_bonus[2] = { 0 };
  gint leftover_expand_bonus[2] = { 0 };

  gint nvis_children, n_side_vis_children[2] = { 0 };
  gint nexpand_children[2] = { 0 };
  gboolean title_expands = FALSE;
  GList *l;
  gint i;
  Child *child;
  GtkPackType packing;

  get_title_size (self, allocation->height, &title_size, &title_expands);

  nvis_children = count_visible_children (self);
  children_sizes = g_newa (GtkRequestedSize, nvis_children);
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    n_side_vis_children[packing] = count_visible_children_for_pack_type (self, packing);
    children_sizes_for_side[packing] = packing == 0 ? children_sizes : children_sizes + n_side_vis_children[packing - 1];
    free_space[packing] = (allocation->width - title_size.minimum_size) / 2 - decoration_width[packing];
  }

  /* Compute the nominal size of the children filling up each side of the title
   * in titlebar.
   */
  i = 0;
  for (l = priv->children; l; l = l->next) {
    child = l->data;
    if (!gtk_widget_get_visible (child->widget))
      continue;

    if (gtk_widget_compute_expand (child->widget, GTK_ORIENTATION_HORIZONTAL))
      nexpand_children[child->pack_type]++;

    gtk_widget_get_preferred_width_for_height (child->widget,
                                               allocation->height,
                                               &children_sizes[i].minimum_size,
                                               &children_sizes[i].natural_size);
    side_size[child->pack_type].minimum_size += children_sizes[i].minimum_size + priv->spacing;
    side_size[child->pack_type].natural_size += children_sizes[i].natural_size + priv->spacing;
    free_space[child->pack_type] -= children_sizes[i].minimum_size + priv->spacing;

    i++;
  }

  /* Figure out the space maximum size requests from each side to help centering
   * the title.
   */
  side_request.minimum_size = MAX (side_size[GTK_PACK_START].minimum_size + decoration_width[GTK_PACK_START],
                                   side_size[GTK_PACK_END].minimum_size + decoration_width[GTK_PACK_END]);
  side_request.natural_size = MAX (side_size[GTK_PACK_START].natural_size + decoration_width[GTK_PACK_START],
                                   side_size[GTK_PACK_END].natural_size + decoration_width[GTK_PACK_END]);
  title_leftover = (allocation->width - title_size.natural_size) % 2;
  side_max = MAX ((allocation->width - title_size.natural_size) / 2, side_request.minimum_size);

  /* Distribute the available space for natural expansion of the children and
   * figure out how much space is left on each side of the title, free to be
   * used for expansion.
   */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    gint leftovers = side_max - side_size[packing].minimum_size - decoration_width[packing];
    free_space[packing] = gtk_distribute_natural_allocation (leftovers, n_side_vis_children[packing], children_sizes_for_side[packing]);
  }

  /* Compute how much of each side's free space should be distributed to the
   * title for its expansion.
   */
  title_expand_bonus = !title_expands ? 0 :
    MIN (nexpand_children[GTK_PACK_START] > 0 ? free_space[GTK_PACK_START] / 2 :
                                                free_space[GTK_PACK_START],
         nexpand_children[GTK_PACK_END] > 0 ? free_space[GTK_PACK_END] / 2 :
                                              free_space[GTK_PACK_END]);

  /* Remove the space the title takes from each side for its expansion. */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++)
    free_space[packing] -= title_expand_bonus;

  /* Distribute the free space for expansion of the children. */
  for (packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    if (nexpand_children[packing] == 0)
      continue;

    uniform_expand_bonus[packing] = free_space[packing] / nexpand_children[packing];
    leftover_expand_bonus[packing] = free_space[packing] % nexpand_children[packing];
  }

  children_allocate (self, allocation, allocations, children_sizes, decoration_width, uniform_expand_bonus, leftover_expand_bonus);

  /* We don't enforce css borders on the center widget, to make title/subtitle
   * combinations fit without growing the header.
   */
  title_allocation->y = allocation->y;
  title_allocation->height = allocation->height;

  title_allocation->width = MIN (allocation->width - 2 * side_max + title_leftover,
                                title_size.natural_size);
  title_allocation->x = allocation->x + (allocation->width - title_allocation->width) / 2;

  /* If the title widget is expanded, then grow it by the free space available
   * for it.
   */
  if (title_expands) {
    title_allocation->width += 2 * title_expand_bonus;
    title_allocation->x -= title_expand_bonus;
  }

  if (gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL)
    title_allocation->x = allocation->x + allocation->width - (title_allocation->x - allocation->x) - title_allocation->width;
}

static void
hdy_header_bar_size_allocate (GtkWidget     *widget,
                              GtkAllocation *allocation)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (widget);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GtkAllocation *allocations;
  GtkAllocation title_allocation;
  GtkAllocation clip;
  gint nvis_children;
  GList *l;
  gint i;
  Child *child;
  GtkAllocation child_allocation;
  GtkTextDirection direction;
  GtkWidget *decoration_box[2] = { priv->titlebar_start_box, priv->titlebar_end_box };
  gint decoration_width[2] = { 0 };

  gtk_render_background_get_clip (gtk_widget_get_style_context (widget),
                                  allocation->x,
                                  allocation->y,
                                  allocation->width,
                                  allocation->height,
                                  &clip);

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
    gdk_window_move_resize (gtk_widget_get_window (widget),
                            allocation->x,
                            allocation->y,
                            allocation->width,
                            allocation->height);

  hdy_css_size_allocate (widget, allocation);

  direction = gtk_widget_get_direction (widget);
  nvis_children = count_visible_children (self);
  allocations = g_newa (GtkAllocation, nvis_children);

  /* Get the decoration width. */
  for (GtkPackType packing = GTK_PACK_START; packing <= GTK_PACK_END; packing++) {
    gint min, nat;

    if (decoration_box[packing] == NULL)
      continue;

    gtk_widget_get_preferred_width_for_height (decoration_box[packing],
                                               allocation->height,
                                               &min, &nat);
    decoration_width[packing] = nat + priv->spacing;
  }

  /* Allocate the decoration widgets. */
  child_allocation.y = allocation->y;
  child_allocation.height = allocation->height;

  if (priv->titlebar_start_box) {
    if (direction == GTK_TEXT_DIR_LTR)
      child_allocation.x = allocation->x;
    else
      child_allocation.x = allocation->x + allocation->width - decoration_width[GTK_PACK_START] + priv->spacing;
    child_allocation.width = decoration_width[GTK_PACK_START] - priv->spacing;
    gtk_widget_size_allocate (priv->titlebar_start_box, &child_allocation);
  }

  if (priv->titlebar_end_box) {
    if (direction != GTK_TEXT_DIR_LTR)
      child_allocation.x = allocation->x;
    else
      child_allocation.x = allocation->x + allocation->width - decoration_width[GTK_PACK_END] + priv->spacing;
    child_allocation.width = decoration_width[GTK_PACK_END] - priv->spacing;
    gtk_widget_size_allocate (priv->titlebar_end_box, &child_allocation);
  }

  /* Get the allocation for widgets on both side of the title. */
  if (gtk_progress_tracker_get_state (&priv->tracker) == GTK_PROGRESS_STATE_AFTER) {
    if (priv->centering_policy == HDY_CENTERING_POLICY_STRICT)
      get_strict_centering_allocations (self, allocation, &allocations, &title_allocation, decoration_width);
    else
      get_loose_centering_allocations (self, allocation, &allocations, &title_allocation, decoration_width);
  } else {
    /* For memory usage optimisation's sake, we will use the allocations
     * variable to store the loose centering allocations and the
     * title_allocation variable to store the loose title allocation.
     */
    GtkAllocation *strict_allocations = g_newa (GtkAllocation, nvis_children);
    GtkAllocation strict_title_allocation;
    gdouble strict_centering_t = gtk_progress_tracker_get_ease_out_cubic (&priv->tracker, FALSE);

    if (priv->centering_policy != HDY_CENTERING_POLICY_STRICT)
      strict_centering_t = 1.0 - strict_centering_t;

    get_loose_centering_allocations (self, allocation, &allocations, &title_allocation, decoration_width);
    get_strict_centering_allocations (self, allocation, &strict_allocations, &strict_title_allocation, decoration_width);

    for (i = 0; i < nvis_children; i++) {
      allocations[i].x = hdy_lerp (allocations[i].x, strict_allocations[i].x, strict_centering_t);
      allocations[i].y = hdy_lerp (allocations[i].y, strict_allocations[i].y, strict_centering_t);
      allocations[i].width = hdy_lerp (allocations[i].width, strict_allocations[i].width, strict_centering_t);
      allocations[i].height = hdy_lerp (allocations[i].height, strict_allocations[i].height, strict_centering_t);
    }
    title_allocation.x = hdy_lerp (title_allocation.x, strict_title_allocation.x, strict_centering_t);
    title_allocation.y = hdy_lerp (title_allocation.y, strict_title_allocation.y, strict_centering_t);
    title_allocation.width = hdy_lerp (title_allocation.width, strict_title_allocation.width, strict_centering_t);
    title_allocation.height = hdy_lerp (title_allocation.height, strict_title_allocation.height, strict_centering_t);
  }

  /* Allocate the children on both sides of the title. */
  i = 0;
  for (l = priv->children; l != NULL; l = l->next) {
    child = l->data;
    if (!gtk_widget_get_visible (child->widget))
      continue;

    gtk_widget_size_allocate (child->widget, &allocations[i]);
    i++;
  }

  /* Allocate the title widget. */
  if (priv->custom_title != NULL && gtk_widget_get_visible (priv->custom_title))
    gtk_widget_size_allocate (priv->custom_title, &title_allocation);
  else if (priv->label_box != NULL)
    gtk_widget_size_allocate (priv->label_box, &title_allocation);

  gtk_widget_set_clip (widget, &clip);
}

static void
hdy_header_bar_destroy (GtkWidget *widget)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (HDY_HEADER_BAR (widget));

  if (priv->label_sizing_box) {
    gtk_widget_destroy (priv->label_sizing_box);
    g_clear_object (&priv->label_sizing_box);
  }

  if (priv->custom_title) {
    gtk_widget_unparent (priv->custom_title);
    priv->custom_title = NULL;
  }

  if (priv->label_box) {
    gtk_widget_unparent (priv->label_box);
    priv->label_box = NULL;
  }

  if (priv->titlebar_start_box) {
    gtk_widget_unparent (priv->titlebar_start_box);
    priv->titlebar_start_box = NULL;
    priv->titlebar_start_separator = NULL;
  }

  if (priv->titlebar_end_box) {
    gtk_widget_unparent (priv->titlebar_end_box);
    priv->titlebar_end_box = NULL;
    priv->titlebar_end_separator = NULL;
  }

  GTK_WIDGET_CLASS (hdy_header_bar_parent_class)->destroy (widget);
}

static void
hdy_header_bar_finalize (GObject *object)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (HDY_HEADER_BAR (object));

  g_clear_pointer (&priv->title, g_free);
  g_clear_pointer (&priv->subtitle, g_free);
  g_clear_pointer (&priv->decoration_layout, g_free);
  g_clear_object (&priv->controller);

  G_OBJECT_CLASS (hdy_header_bar_parent_class)->finalize (object);
}

static void
hdy_header_bar_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (object);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, priv->title);
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, priv->subtitle);
    break;
  case PROP_CUSTOM_TITLE:
    g_value_set_object (value, priv->custom_title);
    break;
  case PROP_SPACING:
    g_value_set_int (value, priv->spacing);
    break;
  case PROP_SHOW_CLOSE_BUTTON:
    g_value_set_boolean (value, hdy_header_bar_get_show_close_button (self));
    break;
  case PROP_HAS_SUBTITLE:
    g_value_set_boolean (value, hdy_header_bar_get_has_subtitle (self));
    break;
  case PROP_DECORATION_LAYOUT:
    g_value_set_string (value, hdy_header_bar_get_decoration_layout (self));
    break;
  case PROP_DECORATION_LAYOUT_SET:
    g_value_set_boolean (value, priv->decoration_layout_set);
    break;
  case PROP_CENTERING_POLICY:
    g_value_set_enum (value, hdy_header_bar_get_centering_policy (self));
    break;
  case PROP_TRANSITION_DURATION:
    g_value_set_uint (value, hdy_header_bar_get_transition_duration (self));
    break;
  case PROP_TRANSITION_RUNNING:
    g_value_set_boolean (value, hdy_header_bar_get_transition_running (self));
    break;
  case PROP_INTERPOLATE_SIZE:
    g_value_set_boolean (value, hdy_header_bar_get_interpolate_size (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_header_bar_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (object);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  switch (prop_id) {
  case PROP_TITLE:
    hdy_header_bar_set_title (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE:
    hdy_header_bar_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_CUSTOM_TITLE:
    hdy_header_bar_set_custom_title (self, g_value_get_object (value));
    break;
  case PROP_SPACING:
    if (priv->spacing != g_value_get_int (value)) {
      priv->spacing = g_value_get_int (value);
      gtk_widget_queue_resize (GTK_WIDGET (self));
      g_object_notify_by_pspec (object, pspec);
    }
    break;
  case PROP_SHOW_CLOSE_BUTTON:
    hdy_header_bar_set_show_close_button (self, g_value_get_boolean (value));
    break;
  case PROP_HAS_SUBTITLE:
    hdy_header_bar_set_has_subtitle (self, g_value_get_boolean (value));
    break;
  case PROP_DECORATION_LAYOUT:
    hdy_header_bar_set_decoration_layout (self, g_value_get_string (value));
    break;
  case PROP_DECORATION_LAYOUT_SET:
    priv->decoration_layout_set = g_value_get_boolean (value);
    break;
  case PROP_CENTERING_POLICY:
    hdy_header_bar_set_centering_policy (self, g_value_get_enum (value));
    break;
  case PROP_TRANSITION_DURATION:
    hdy_header_bar_set_transition_duration (self, g_value_get_uint (value));
    break;
  case PROP_INTERPOLATE_SIZE:
    hdy_header_bar_set_interpolate_size (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
notify_child_cb (GObject      *child,
                 GParamSpec   *pspec,
                 HdyHeaderBar *self)
{
  _hdy_header_bar_update_separator_visibility (self);
}

static void
hdy_header_bar_pack (HdyHeaderBar *self,
                     GtkWidget    *widget,
                     GtkPackType   pack_type)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  Child *child;

  g_return_if_fail (gtk_widget_get_parent (widget) == NULL);

  child = g_new (Child, 1);
  child->widget = widget;
  child->pack_type = pack_type;

  priv->children = g_list_append (priv->children, child);

  gtk_widget_freeze_child_notify (widget);
  gtk_widget_set_parent (widget, GTK_WIDGET (self));
  g_signal_connect (widget, "notify::visible", G_CALLBACK (notify_child_cb), self);
  gtk_widget_child_notify (widget, "pack-type");
  gtk_widget_child_notify (widget, "position");
  gtk_widget_thaw_child_notify (widget);

  _hdy_header_bar_update_separator_visibility (self);
}

static void
hdy_header_bar_add (GtkContainer *container,
                    GtkWidget    *child)
{
  hdy_header_bar_pack (HDY_HEADER_BAR (container), child, GTK_PACK_START);
}

static GList *
find_child_link (HdyHeaderBar *self,
                 GtkWidget    *widget,
                 gint         *position)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  Child *child;
  gint i;

  for (l = priv->children, i = 0; l; l = l->next, i++) {
    child = l->data;
    if (child->widget == widget) {
      if (position)
        *position = i;

      return l;
    }
  }

  return NULL;
}

static void
hdy_header_bar_remove (GtkContainer *container,
                       GtkWidget    *widget)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (container);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  Child *child;

  l = find_child_link (self, widget, NULL);
  if (l) {
    child = l->data;
    g_signal_handlers_disconnect_by_func (widget, notify_child_cb, self);
    gtk_widget_unparent (child->widget);
    priv->children = g_list_delete_link (priv->children, l);
    g_free (child);
    gtk_widget_queue_resize (GTK_WIDGET (container));
    _hdy_header_bar_update_separator_visibility (self);
  }
}

static void
hdy_header_bar_forall (GtkContainer *container,
                       gboolean      include_internals,
                       GtkCallback   callback,
                       gpointer      callback_data)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (container);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  Child *child;
  GList *children;

  if (include_internals && priv->titlebar_start_box != NULL)
    (* callback) (priv->titlebar_start_box, callback_data);

  children = priv->children;
  while (children) {
    child = children->data;
    children = children->next;
    if (child->pack_type == GTK_PACK_START)
      (* callback) (child->widget, callback_data);
  }

  if (priv->custom_title != NULL)
    (* callback) (priv->custom_title, callback_data);

  if (include_internals && priv->label_box != NULL)
    (* callback) (priv->label_box, callback_data);

  children = priv->children;
  while (children) {
    child = children->data;
    children = children->next;
    if (child->pack_type == GTK_PACK_END)
      (* callback) (child->widget, callback_data);
  }

  if (include_internals && priv->titlebar_end_box != NULL)
    (* callback) (priv->titlebar_end_box, callback_data);
}

static void
hdy_header_bar_reorder_child (HdyHeaderBar *self,
                              GtkWidget    *widget,
                              gint          position)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  gint old_position;
  Child *child;

  l = find_child_link (self, widget, &old_position);

  if (l == NULL)
    return;

  if (old_position == position)
    return;

  child = l->data;
  priv->children = g_list_delete_link (priv->children, l);

  if (position < 0)
    l = NULL;
  else
    l = g_list_nth (priv->children, position);

  priv->children = g_list_insert_before (priv->children, l, child);
  gtk_widget_child_notify (widget, "position");
  gtk_widget_queue_resize (widget);
}

static GType
hdy_header_bar_child_type (GtkContainer *container)
{
  return GTK_TYPE_WIDGET;
}

static void
hdy_header_bar_get_child_property (GtkContainer *container,
                                   GtkWidget    *widget,
                                   guint         property_id,
                                   GValue       *value,
                                   GParamSpec   *pspec)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (container);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  GList *l;
  Child *child;

  l = find_child_link (self, widget, NULL);
  if (l == NULL) {
    g_param_value_set_default (pspec, value);

    return;
  }

  child = l->data;

  switch (property_id) {
  case CHILD_PROP_PACK_TYPE:
    g_value_set_enum (value, child->pack_type);
    break;

  case CHILD_PROP_POSITION:
    g_value_set_int (value, g_list_position (priv->children, l));
    break;

  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static void
hdy_header_bar_set_child_property (GtkContainer *container,
                                   GtkWidget    *widget,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (container);
  GList *l;
  Child *child;

  l = find_child_link (self, widget, NULL);
  if (l == NULL)
    return;

  child = l->data;

  switch (property_id) {
  case CHILD_PROP_PACK_TYPE:
    child->pack_type = g_value_get_enum (value);
    _hdy_header_bar_update_separator_visibility (self);
    gtk_widget_queue_resize (widget);
    break;

  case CHILD_PROP_POSITION:
    hdy_header_bar_reorder_child (self, widget, g_value_get_int (value));
    break;

  default:
    GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, property_id, pspec);
    break;
  }
}

static gboolean
hdy_header_bar_draw (GtkWidget *widget,
                     cairo_t   *cr)
{
  GtkStyleContext *context;

  context = gtk_widget_get_style_context (widget);
  /* GtkWidget draws nothing by default so we have to render the background
   * explicitly for HdyHederBar to render the typical titlebar background.
   */
  gtk_render_background (context,
                         cr,
                         0, 0,
                         gtk_widget_get_allocated_width (widget),
                         gtk_widget_get_allocated_height (widget));
  /* Ditto for the borders. */
  gtk_render_frame (context,
                    cr,
                    0, 0,
                    gtk_widget_get_allocated_width (widget),
                    gtk_widget_get_allocated_height (widget));

  return GTK_WIDGET_CLASS (hdy_header_bar_parent_class)->draw (widget, cr);
}

static void
hdy_header_bar_realize (GtkWidget *widget)
{
  GtkSettings *settings;
  GtkAllocation allocation;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GdkWindow *window;

  settings = gtk_widget_get_settings (widget);
  g_signal_connect_swapped (settings, "notify::gtk-shell-shows-app-menu",
                            G_CALLBACK (hdy_header_bar_update_window_buttons), widget);
  g_signal_connect_swapped (settings, "notify::gtk-decoration-layout",
                            G_CALLBACK (hdy_header_bar_update_window_buttons), widget);
  update_is_mobile_window (HDY_HEADER_BAR (widget));
  hdy_header_bar_update_window_buttons (HDY_HEADER_BAR (widget));

  gtk_widget_get_allocation (widget, &allocation);
  gtk_widget_set_realized (widget, TRUE);

  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget);
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

  window = gdk_window_new (gtk_widget_get_parent_window (widget),
                           &attributes,
                           attributes_mask);
  gtk_widget_set_window (widget, window);
  gtk_widget_register_window (widget, window);
}

static void
hdy_header_bar_unrealize (GtkWidget *widget)
{
  GtkSettings *settings;

  settings = gtk_widget_get_settings (widget);

  g_signal_handlers_disconnect_by_func (settings, hdy_header_bar_update_window_buttons, widget);

  GTK_WIDGET_CLASS (hdy_header_bar_parent_class)->unrealize (widget);
}

static gboolean
window_state_changed (GtkWidget           *window,
                      GdkEventWindowState *event,
                      gpointer             data)
{
  HdyHeaderBar *self = HDY_HEADER_BAR (data);

  if (event->changed_mask & (GDK_WINDOW_STATE_FULLSCREEN |
                             GDK_WINDOW_STATE_MAXIMIZED |
                             GDK_WINDOW_STATE_TILED |
                             GDK_WINDOW_STATE_TOP_TILED |
                             GDK_WINDOW_STATE_RIGHT_TILED |
                             GDK_WINDOW_STATE_BOTTOM_TILED |
                             GDK_WINDOW_STATE_LEFT_TILED))
    hdy_header_bar_update_window_buttons (self);

  return FALSE;
}

static void
hdy_header_bar_hierarchy_changed (GtkWidget *widget,
                                  GtkWidget *previous_toplevel)
{
  GtkWidget *toplevel;
  HdyHeaderBar *self = HDY_HEADER_BAR (widget);
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  toplevel = gtk_widget_get_toplevel (widget);

  if (previous_toplevel)
    g_signal_handlers_disconnect_by_func (previous_toplevel,
                                          window_state_changed, widget);

  if (toplevel)
    g_signal_connect_after (toplevel, "window-state-event",
                            G_CALLBACK (window_state_changed), widget);

  if (priv->window_size_allocated_id > 0) {
    g_signal_handler_disconnect (previous_toplevel, priv->window_size_allocated_id);
    priv->window_size_allocated_id = 0;
  }

  if (GTK_IS_WINDOW (toplevel))
    priv->window_size_allocated_id =
      g_signal_connect_swapped (toplevel, "size-allocate",
                                G_CALLBACK (update_is_mobile_window), self);

  update_is_mobile_window (self);
  hdy_header_bar_update_window_buttons (self);
}

static void
hdy_header_bar_class_init (HdyHeaderBarClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (class);

  object_class->finalize = hdy_header_bar_finalize;
  object_class->get_property = hdy_header_bar_get_property;
  object_class->set_property = hdy_header_bar_set_property;

  widget_class->destroy = hdy_header_bar_destroy;
  widget_class->size_allocate = hdy_header_bar_size_allocate;
  widget_class->get_preferred_width = hdy_header_bar_get_preferred_width;
  widget_class->get_preferred_height = hdy_header_bar_get_preferred_height;
  widget_class->get_preferred_height_for_width = hdy_header_bar_get_preferred_height_for_width;
  widget_class->get_preferred_width_for_height = hdy_header_bar_get_preferred_width_for_height;
  widget_class->draw = hdy_header_bar_draw;
  widget_class->realize = hdy_header_bar_realize;
  widget_class->unrealize = hdy_header_bar_unrealize;
  widget_class->hierarchy_changed = hdy_header_bar_hierarchy_changed;

  container_class->add = hdy_header_bar_add;
  container_class->remove = hdy_header_bar_remove;
  container_class->forall = hdy_header_bar_forall;
  container_class->child_type = hdy_header_bar_child_type;
  container_class->set_child_property = hdy_header_bar_set_child_property;
  container_class->get_child_property = hdy_header_bar_get_child_property;
  gtk_container_class_handle_border_width (container_class);

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_PACK_TYPE,
                                              g_param_spec_enum ("pack-type",
                                                                 _("Pack type"),
                                                                 _("A GtkPackType indicating whether the child is packed with reference to the start or end of the parent"),
                                                                 GTK_TYPE_PACK_TYPE, GTK_PACK_START,
                                                                 G_PARAM_READWRITE));
  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_POSITION,
                                              g_param_spec_int ("position",
                                                                _("Position"),
                                                                _("The index of the child in the parent"),
                                                                -1, G_MAXINT, 0,
                                                                G_PARAM_READWRITE));

  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("The title to display"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         _("Subtitle"),
                         _("The subtitle to display"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_CUSTOM_TITLE] =
    g_param_spec_object ("custom-title",
                         _("Custom Title"),
                         _("Custom title widget to display"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS);

  props[PROP_SPACING] =
    g_param_spec_int ("spacing",
                      _("Spacing"),
                      _("The amount of space between children"),
                      0, G_MAXINT,
                      DEFAULT_SPACING,
                      G_PARAM_READWRITE|G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyHeaderBar:show-close-button:
   *
   * Whether to show window decorations.
   *
   * Which buttons are actually shown and where is determined
   * by the #HdyHeaderBar:decoration-layout property, and by
   * the state of the window (e.g. a close button will not be
   * shown if the window can't be closed).
   *
   * Since: 0.0.10
   */
  props[PROP_SHOW_CLOSE_BUTTON] =
    g_param_spec_boolean ("show-close-button",
                          _("Show decorations"),
                          _("Whether to show window decorations"),
                          FALSE,
                          G_PARAM_READWRITE|G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyHeaderBar:decoration-layout:
   *
   * The decoration layout for buttons. If this property is
   * not set, the #GtkSettings:gtk-decoration-layout setting
   * is used.
   *
   * See hdy_header_bar_set_decoration_layout() for information
   * about the format of this string.
   *
   * Since: 0.0.10
   */
  props[PROP_DECORATION_LAYOUT] =
    g_param_spec_string ("decoration-layout",
                         _("Decoration Layout"),
                         _("The layout for window decorations"),
                         NULL,
                         G_PARAM_READWRITE);

  /**
   * HdyHeaderBar:decoration-layout-set:
   *
   * Set to %TRUE if #HdyHeaderBar:decoration-layout is set.
   *
   * Since: 0.0.10
   */
  props[PROP_DECORATION_LAYOUT_SET] =
    g_param_spec_boolean ("decoration-layout-set",
                          _("Decoration Layout Set"),
                          _("Whether the decoration-layout property has been set"),
                          FALSE,
                          G_PARAM_READWRITE);

  /**
   * HdyHeaderBar:has-subtitle:
   *
   * If %TRUE, reserve space for a subtitle, even if none
   * is currently set.
   *
   * Since: 0.0.10
   */
  props[PROP_HAS_SUBTITLE] =
    g_param_spec_boolean ("has-subtitle",
                          _("Has Subtitle"),
                          _("Whether to reserve space for a subtitle"),
                          TRUE,
                          G_PARAM_READWRITE|G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_CENTERING_POLICY] =
    g_param_spec_enum ("centering-policy",
                       _("Centering policy"),
                       _("The policy to horizontally align the center widget"),
                       HDY_TYPE_CENTERING_POLICY, HDY_CENTERING_POLICY_LOOSE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration",
                       _("Transition duration"),
                       _("The animation duration, in milliseconds"),
                       0, G_MAXUINT, 200,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_TRANSITION_RUNNING] =
    g_param_spec_boolean ("transition-running",
                          _("Transition running"),
                          _("Whether or not the transition is currently running"),
                          FALSE,
                          G_PARAM_READABLE);

  props[PROP_INTERPOLATE_SIZE] =
    g_param_spec_boolean ("interpolate-size",
                          _("Interpolate size"),
                          _("Whether or not the size should smoothly change when changing between differently sized children"),
                            FALSE,
                            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_PANEL);
  gtk_widget_class_set_css_name (widget_class, "headerbar");
}

static void
hdy_header_bar_init (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv;
  GtkStyleContext *context;

  priv = hdy_header_bar_get_instance_private (self);

  priv->title = NULL;
  priv->subtitle = NULL;
  priv->custom_title = NULL;
  priv->children = NULL;
  priv->spacing = DEFAULT_SPACING;
  priv->has_subtitle = TRUE;
  priv->decoration_layout = NULL;
  priv->decoration_layout_set = FALSE;
  priv->transition_duration = 200;

  init_sizing_box (self);
  construct_label_box (self);

  priv->controller = hdy_window_handle_controller_new (GTK_WIDGET (self));

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  /* Ensure the widget has the titlebar style class. */
  gtk_style_context_add_class (context, "titlebar");
}

static void
hdy_header_bar_buildable_add_child (GtkBuildable *buildable,
                                    GtkBuilder   *builder,
                                    GObject      *child,
                                    const gchar  *type)
{
  if (type && strcmp (type, "title") == 0)
    hdy_header_bar_set_custom_title (HDY_HEADER_BAR (buildable), GTK_WIDGET (child));
  else if (!type)
    gtk_container_add (GTK_CONTAINER (buildable), GTK_WIDGET (child));
  else
    GTK_BUILDER_WARN_INVALID_CHILD_TYPE (HDY_HEADER_BAR (buildable), type);
}

static void
hdy_header_bar_buildable_init (GtkBuildableIface *iface)
{
  iface->add_child = hdy_header_bar_buildable_add_child;
}

/**
 * hdy_header_bar_new:
 *
 * Creates a new #HdyHeaderBar widget.
 *
 * Returns: a new #HdyHeaderBar
 *
 * Since: 0.0.10
 */
GtkWidget *
hdy_header_bar_new (void)
{
  return GTK_WIDGET (g_object_new (HDY_TYPE_HEADER_BAR, NULL));
}

/**
 * hdy_header_bar_pack_start:
 * @self: A #HdyHeaderBar
 * @child: the #GtkWidget to be added to @self:
 *
 * Adds @child to @self:, packed with reference to the
 * start of the @self:.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_pack_start (HdyHeaderBar *self,
                           GtkWidget    *child)
{
  hdy_header_bar_pack (self, child, GTK_PACK_START);
}

/**
 * hdy_header_bar_pack_end:
 * @self: A #HdyHeaderBar
 * @child: the #GtkWidget to be added to @self:
 *
 * Adds @child to @self:, packed with reference to the
 * end of the @self:.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_pack_end (HdyHeaderBar *self,
                         GtkWidget    *child)
{
  hdy_header_bar_pack (self, child, GTK_PACK_END);
}

/**
 * hdy_header_bar_set_title:
 * @self: a #HdyHeaderBar
 * @title: (nullable): a title, or %NULL
 *
 * Sets the title of the #HdyHeaderBar. The title should help a user
 * identify the current view. A good title should not include the
 * application name.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_title (HdyHeaderBar *self,
                          const gchar  *title)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  gchar *new_title;

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  new_title = g_strdup (title);
  g_free (priv->title);
  priv->title = new_title;

  if (priv->title_label != NULL) {
    gtk_label_set_label (GTK_LABEL (priv->title_label), priv->title);
    gtk_widget_queue_resize (GTK_WIDGET (self));
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * hdy_header_bar_get_title:
 * @self: a #HdyHeaderBar
 *
 * Retrieves the title of the header. See hdy_header_bar_set_title().
 *
 * Returns: (nullable): the title of the header, or %NULL if none has
 *    been set explicitly. The returned string is owned by the widget
 *    and must not be modified or freed.
 *
 * Since: 0.0.10
 */
const gchar *
hdy_header_bar_get_title (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), NULL);

  return priv->title;
}

/**
 * hdy_header_bar_set_subtitle:
 * @self: a #HdyHeaderBar
 * @subtitle: (nullable): a subtitle, or %NULL
 *
 * Sets the subtitle of the #HdyHeaderBar. The title should give a user
 * an additional detail to help them identify the current view.
 *
 * Note that HdyHeaderBar by default reserves room for the subtitle,
 * even if none is currently set. If this is not desired, set the
 * #HdyHeaderBar:has-subtitle property to %FALSE.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_subtitle (HdyHeaderBar *self,
                             const gchar  *subtitle)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);
  gchar *new_subtitle;

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  new_subtitle = g_strdup (subtitle);
  g_free (priv->subtitle);
  priv->subtitle = new_subtitle;

  if (priv->subtitle_label != NULL) {
    gtk_label_set_label (GTK_LABEL (priv->subtitle_label), priv->subtitle);
    gtk_widget_set_visible (priv->subtitle_label, priv->subtitle && priv->subtitle[0]);
    gtk_widget_queue_resize (GTK_WIDGET (self));
  }

  gtk_widget_set_visible (priv->subtitle_sizing_label, priv->has_subtitle || (priv->subtitle && priv->subtitle[0]));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * hdy_header_bar_get_subtitle:
 * @self: a #HdyHeaderBar
 *
 * Retrieves the subtitle of the header. See hdy_header_bar_set_subtitle().
 *
 * Returns: (nullable): the subtitle of the header, or %NULL if none has
 *    been set explicitly. The returned string is owned by the widget
 *    and must not be modified or freed.
 *
 * Since: 0.0.10
 */
const gchar *
hdy_header_bar_get_subtitle (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), NULL);

  return priv->subtitle;
}

/**
 * hdy_header_bar_set_custom_title:
 * @self: a #HdyHeaderBar
 * @title_widget: (nullable): a custom widget to use for a title
 *
 * Sets a custom title for the #HdyHeaderBar.
 *
 * The title should help a user identify the current view. This
 * supersedes any title set by hdy_header_bar_set_title() or
 * hdy_header_bar_set_subtitle(). To achieve the same style as
 * the builtin title and subtitle, use the “title” and “subtitle”
 * style classes.
 *
 * You should set the custom title to %NULL, for the header title
 * label to be visible again.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_custom_title (HdyHeaderBar *self,
                                 GtkWidget    *title_widget)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_if_fail (HDY_IS_HEADER_BAR (self));
  if (title_widget)
    g_return_if_fail (GTK_IS_WIDGET (title_widget));

  /* No need to do anything if the custom widget stays the same */
  if (priv->custom_title == title_widget)
    return;

  if (priv->custom_title) {
    GtkWidget *custom = priv->custom_title;

    priv->custom_title = NULL;
    gtk_widget_unparent (custom);
  }

  if (title_widget != NULL) {
    priv->custom_title = title_widget;

    gtk_widget_set_parent (priv->custom_title, GTK_WIDGET (self));

    if (priv->label_box != NULL) {
      GtkWidget *label_box = priv->label_box;

      priv->label_box = NULL;
      priv->title_label = NULL;
      priv->subtitle_label = NULL;
      gtk_widget_unparent (label_box);
    }
  } else {
    if (priv->label_box == NULL)
      construct_label_box (self);
  }

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CUSTOM_TITLE]);
}

/**
 * hdy_header_bar_get_custom_title:
 * @self: a #HdyHeaderBar
 *
 * Retrieves the custom title widget of the header. See
 * hdy_header_bar_set_custom_title().
 *
 * Returns: (nullable) (transfer none): the custom title widget
 *    of the header, or %NULL if none has been set explicitly.
 *
 * Since: 0.0.10
 */
GtkWidget *
hdy_header_bar_get_custom_title (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), NULL);

  return priv->custom_title;
}

/**
 * hdy_header_bar_get_show_close_button:
 * @self: a #HdyHeaderBar
 *
 * Returns whether this header bar shows the standard window
 * decorations.
 *
 * Returns: %TRUE if the decorations are shown
 *
 * Since: 0.0.10
 */
gboolean
hdy_header_bar_get_show_close_button (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), FALSE);

  priv = hdy_header_bar_get_instance_private (self);

  return priv->shows_wm_decorations;
}

/**
 * hdy_header_bar_set_show_close_button:
 * @self: a #HdyHeaderBar
 * @setting: %TRUE to show standard window decorations
 *
 * Sets whether this header bar shows the standard window decorations,
 * including close, maximize, and minimize.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_show_close_button (HdyHeaderBar *self,
                                      gboolean      setting)
{
  HdyHeaderBarPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  priv = hdy_header_bar_get_instance_private (self);

  setting = setting != FALSE;

  if (priv->shows_wm_decorations == setting)
    return;

  priv->shows_wm_decorations = setting;
  hdy_header_bar_update_window_buttons (self);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_CLOSE_BUTTON]);
}

/**
 * hdy_header_bar_set_has_subtitle:
 * @self: a #HdyHeaderBar
 * @setting: %TRUE to reserve space for a subtitle
 *
 * Sets whether the header bar should reserve space
 * for a subtitle, even if none is currently set.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_has_subtitle (HdyHeaderBar *self,
                                 gboolean      setting)
{
  HdyHeaderBarPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  priv = hdy_header_bar_get_instance_private (self);

  setting = setting != FALSE;

  if (priv->has_subtitle == setting)
    return;

  priv->has_subtitle = setting;
  gtk_widget_set_visible (priv->subtitle_sizing_label, setting || (priv->subtitle && priv->subtitle[0]));

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HAS_SUBTITLE]);
}

/**
 * hdy_header_bar_get_has_subtitle:
 * @self: a #HdyHeaderBar
 *
 * Retrieves whether the header bar reserves space for
 * a subtitle, regardless if one is currently set or not.
 *
 * Returns: %TRUE if the header bar reserves space
 *     for a subtitle
 *
 * Since: 0.0.10
 */
gboolean
hdy_header_bar_get_has_subtitle (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), FALSE);

  priv = hdy_header_bar_get_instance_private (self);

  return priv->has_subtitle;
}

/**
 * hdy_header_bar_set_decoration_layout:
 * @self: a #HdyHeaderBar
 * @layout: (nullable): a decoration layout, or %NULL to unset the layout
 *
 * Sets the decoration layout for this header bar, overriding
 * the #GtkSettings:gtk-decoration-layout setting.
 *
 * There can be valid reasons for overriding the setting, such
 * as a header bar design that does not allow for buttons to take
 * room on the right, or only offers room for a single close button.
 * Split header bars are another example for overriding the
 * setting.
 *
 * The format of the string is button names, separated by commas.
 * A colon separates the buttons that should appear on the left
 * from those on the right. Recognized button names are minimize,
 * maximize, close, icon (the window icon) and menu (a menu button
 * for the fallback app menu).
 *
 * For example, “menu:minimize,maximize,close” specifies a menu
 * on the left, and minimize, maximize and close buttons on the right.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_decoration_layout (HdyHeaderBar *self,
                                      const gchar  *layout)
{
  HdyHeaderBarPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  priv = hdy_header_bar_get_instance_private (self);

  g_clear_pointer (&priv->decoration_layout, g_free);
  priv->decoration_layout = g_strdup (layout);
  priv->decoration_layout_set = (layout != NULL);

  hdy_header_bar_update_window_buttons (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DECORATION_LAYOUT]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DECORATION_LAYOUT_SET]);
}

/**
 * hdy_header_bar_get_decoration_layout:
 * @self: a #HdyHeaderBar
 *
 * Gets the decoration layout set with
 * hdy_header_bar_set_decoration_layout().
 *
 * Returns: the decoration layout
 *
 * Since: 0.0.10
 */
const gchar *
hdy_header_bar_get_decoration_layout (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), NULL);

  priv = hdy_header_bar_get_instance_private (self);

  return priv->decoration_layout;
}

/**
 * hdy_header_bar_get_centering_policy:
 * @self: a #HdyHeaderBar
 *
 * Gets the policy @self follows to horizontally align its center widget.
 *
 * Returns: the centering policy
 *
 * Since: 0.0.10
 */
HdyCenteringPolicy
hdy_header_bar_get_centering_policy (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), HDY_CENTERING_POLICY_LOOSE);

  return priv->centering_policy;
}

/**
 * hdy_header_bar_set_centering_policy:
 * @self: a #HdyHeaderBar
 * @centering_policy: the centering policy
 *
 * Sets the policy @self must follow to horizontally align its center widget.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_centering_policy (HdyHeaderBar       *self,
                                     HdyCenteringPolicy  centering_policy)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  if (priv->centering_policy == centering_policy)
    return;

  priv->centering_policy = centering_policy;
  if (priv->interpolate_size)
    hdy_header_bar_start_transition (self, priv->transition_duration);
  else
    gtk_widget_queue_resize (GTK_WIDGET (self));
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CENTERING_POLICY]);
}

/**
 * hdy_header_bar_get_transition_duration:
 * @self: a #HdyHeaderBar
 *
 * Returns the amount of time (in milliseconds) that
 * transitions between pages in @self will take.
 *
 * Returns: the transition duration
 *
 * Since: 0.0.10
 */
guint
hdy_header_bar_get_transition_duration (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), 0);

  return priv->transition_duration;
}

/**
 * hdy_header_bar_set_transition_duration:
 * @self: a #HdyHeaderBar
 * @duration: the new duration, in milliseconds
 *
 * Sets the duration that transitions between pages in @self
 * will take.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_transition_duration (HdyHeaderBar *self,
                                         guint          duration)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  if (priv->transition_duration == duration)
    return;

  priv->transition_duration = duration;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TRANSITION_DURATION]);
}

/**
 * hdy_header_bar_get_transition_running:
 * @self: a #HdyHeaderBar
 *
 * Returns whether the @self is currently in a transition from one page to
 * another.
 *
 * Returns: %TRUE if the transition is currently running, %FALSE otherwise.
 *
 * Since: 0.0.10
 */
gboolean
hdy_header_bar_get_transition_running (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv = hdy_header_bar_get_instance_private (self);

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), FALSE);

  return (priv->tick_id != 0);
}

/**
 * hdy_header_bar_get_interpolate_size:
 * @self: A #HdyHeaderBar
 *
 * Gets whether @self should interpolate its size on visible child change.
 *
 * See hdy_header_bar_set_interpolate_size().
 *
 * Returns: %TRUE if @self interpolates its size on visible child change, %FALSE if not
 *
 * Since: 0.0.10
 */
gboolean
hdy_header_bar_get_interpolate_size (HdyHeaderBar *self)
{
  HdyHeaderBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_HEADER_BAR (self), FALSE);

  priv = hdy_header_bar_get_instance_private (self);

  return priv->interpolate_size;
}

/**
 * hdy_header_bar_set_interpolate_size:
 * @self: A #HdyHeaderBar
 * @interpolate_size: %TRUE to interpolate the size
 *
 * Sets whether or not @self will interpolate the size of its opposing
 * orientation when changing the visible child. If %TRUE, @self will interpolate
 * its size between the one of the previous visible child and the one of the new
 * visible child, according to the set transition duration and the orientation,
 * e.g. if @self is horizontal, it will interpolate the its height.
 *
 * Since: 0.0.10
 */
void
hdy_header_bar_set_interpolate_size (HdyHeaderBar *self,
                                      gboolean       interpolate_size)
{
  HdyHeaderBarPrivate *priv;

  g_return_if_fail (HDY_IS_HEADER_BAR (self));

  priv = hdy_header_bar_get_instance_private (self);

  interpolate_size = !!interpolate_size;

  if (priv->interpolate_size == interpolate_size)
    return;

  priv->interpolate_size = interpolate_size;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERPOLATE_SIZE]);
}
