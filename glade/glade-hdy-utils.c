/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-utils.h"

#include <gladeui/glade.h>

void
glade_hdy_init (const gchar *name)
{
  g_assert (strcmp (name, "libhandy") == 0);

  gtk_init (NULL, NULL);
  hdy_init ();
}

/* This function has been copied and modified from:
 * glade-gtk-list-box.c - GladeWidgetAdaptor for GtkListBox widget
 *
 * Copyright (C) 2013 Kalev Lember
 *
 * Authors:
 *      Kalev Lember <kalevlember@gmail.com>
 */
void
glade_hdy_sync_child_positions (GtkContainer *container)
{
  g_autoptr (GList) children = NULL;
  GList *l;
  gint position;
  static gboolean recursion = FALSE;

  /* Avoid feedback loop */
  if (recursion)
    return;

  children = gtk_container_get_children (container);

  position = 0;
  for (l = children; l; l = l->next) {
    gint old_position;

    glade_widget_pack_property_get (glade_widget_get_from_gobject (l->data),
                                    "position", &old_position);
    if (position != old_position) {
      /* Update glade with the new value */
      recursion = TRUE;
      glade_widget_pack_property_set (glade_widget_get_from_gobject (l->data),
                                      "position", position);
      recursion = FALSE;
    }

    position++;
  }
}

gint
glade_hdy_get_child_index (GtkContainer *container,
                           GtkWidget    *child)
{
  g_autoptr (GList) children = gtk_container_get_children (container);

  return g_list_index (children, child);
}

void
glade_hdy_reorder_child (GtkContainer *container,
                         GtkWidget    *child,
                         gint          index)
{
  gint old_index = glade_hdy_get_child_index (container, child);
  gint i = 0, n;
  g_autoptr (GList) children = NULL;
  g_autoptr (GList) removed_children = NULL;
  GList *l;

  if (old_index == index)
    return;

  gtk_container_remove (container, g_object_ref (child));

  children = gtk_container_get_children (container);
  n = g_list_length (children);

  children = g_list_reverse (children);
  l = children;

  if (index > old_index)
    n--;

  for (i = n - 1; i >= index; i--) {
    GtkWidget *last_child = l->data;

    gtk_container_remove (container, g_object_ref (last_child));
    l = l->next;

    removed_children = g_list_prepend (removed_children, last_child);
  }

  removed_children = g_list_prepend (removed_children, child);

  for (l = removed_children; l; l = l->next) {
    gtk_container_add (container, l->data);
    g_object_unref (l->data);
  }
}

GtkWidget *
glade_hdy_get_nth_child (GtkContainer *container,
                         gint          n)
{
  g_autoptr (GList) children = gtk_container_get_children (container);

  return g_list_nth_data (children, n);
}
