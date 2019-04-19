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

/* Bits taken from GTK 3.24 and tweaked to be used by libhandy. */

#include "gtk-window-private.h"

typedef struct
{
  GList *icon_list;
  gchar *icon_name;
  guint realized : 1;
  guint using_default_icon : 1;
  guint using_parent_icon : 1;
  guint using_themed_icon : 1;
} GtkWindowIconInfo;

static GQuark quark_gtk_window_icon_info = 0;

static void
ensure_quarks (void)
{
  if (!quark_gtk_window_icon_info)
    quark_gtk_window_icon_info = g_quark_from_static_string ("gtk-window-icon-info");
}

void
hdy_gtk_window_toggle_maximized (GtkWindow *window)
{
  if (gtk_window_is_maximized (window))
    gtk_window_unmaximize (window);
  else
    gtk_window_maximize (window);
}

static GtkWindowIconInfo*
get_icon_info (GtkWindow *window)
{
  ensure_quarks ();

  return g_object_get_qdata (G_OBJECT (window), quark_gtk_window_icon_info);
}

static void
free_icon_info (GtkWindowIconInfo *info)
{
  g_free (info->icon_name);
  g_slice_free (GtkWindowIconInfo, info);
}

static GtkWindowIconInfo*
ensure_icon_info (GtkWindow *window)
{
  GtkWindowIconInfo *info;

  ensure_quarks ();

  info = get_icon_info (window);

  if (info == NULL)
    {
      info = g_slice_new0 (GtkWindowIconInfo);
      g_object_set_qdata_full (G_OBJECT (window),
                              quark_gtk_window_icon_info,
                              info,
                              (GDestroyNotify)free_icon_info);
    }

  return info;
}

static GdkPixbuf *
icon_from_list (GList *list,
                gint   size)
{
  GdkPixbuf *best;
  GdkPixbuf *pixbuf;
  GList *l;

  best = NULL;
  for (l = list; l; l = l->next)
    {
      pixbuf = list->data;
      if (gdk_pixbuf_get_width (pixbuf) <= size &&
          gdk_pixbuf_get_height (pixbuf) <= size)
        {
          best = g_object_ref (pixbuf);
          break;
        }
    }

  if (best == NULL)
    best = gdk_pixbuf_scale_simple (GDK_PIXBUF (list->data), size, size, GDK_INTERP_BILINEAR);

  return best;
}

static GdkPixbuf *
icon_from_name (const gchar *name,
                gint         size)
{
  return gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                   name, size,
                                   GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
}

GdkPixbuf *
hdy_gtk_window_get_icon_for_size (GtkWindow *window,
                                  gint       size)
{
  GtkWindowIconInfo *info;
  const gchar *name;
  g_autoptr (GList) default_icon_list = gtk_window_get_default_icon_list ();

  info = ensure_icon_info (window);

  if (info->icon_list != NULL)
    return icon_from_list (info->icon_list, size);

  name = gtk_window_get_icon_name (window);
  if (name != NULL)
    return icon_from_name (name, size);

  if (gtk_window_get_transient_for (window) != NULL)
    {
      info = ensure_icon_info (gtk_window_get_transient_for (window));
      if (info->icon_list)
        return icon_from_list (info->icon_list, size);
    }

  if (default_icon_list != NULL)
    return icon_from_list (default_icon_list, size);

  if (gtk_window_get_default_icon_name () != NULL)
    return icon_from_name (gtk_window_get_default_icon_name (), size);

  return NULL;
}

