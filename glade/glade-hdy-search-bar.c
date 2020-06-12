/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * glade-gtk-searchbar.c - GladeWidgetAdaptor for GtkSearchBar
 * Copyright (C) 2014 Red Hat, Inc.
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-search-bar.h"

#include <gladeui/glade.h>

void
glade_hdy_search_bar_post_create (GladeWidgetAdaptor *adaptor,
                                  GObject            *widget,
                                  GladeCreateReason   reason)
{
  if (reason == GLADE_CREATE_USER) {
    GtkWidget *child = glade_placeholder_new ();
    gtk_container_add (GTK_CONTAINER (widget), child);
    g_object_set_data (G_OBJECT (widget), "child", child);
  }

  hdy_search_bar_set_search_mode (HDY_SEARCH_BAR (widget), TRUE);
  hdy_search_bar_set_show_close_button (HDY_SEARCH_BAR (widget), FALSE);
}

void
glade_hdy_search_bar_add_child (GladeWidgetAdaptor *adaptor,
                                GObject            *object,
                                GObject            *child)
{
  GObject *current = g_object_get_data (G_OBJECT (object), "child");

  if (current)
    gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (GTK_WIDGET (current))),
                          GTK_WIDGET (current));

  gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (child));
  g_object_set_data (G_OBJECT (object), "child", child);
}

void
glade_hdy_search_bar_remove_child (GladeWidgetAdaptor *adaptor,
                                   GObject            *object,
                                   GObject            *child)
{
  GObject *current = g_object_get_data (G_OBJECT (object), "child");
  GtkWidget *new_child;

  if (current != child)
    return;

  gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (GTK_WIDGET (child))), GTK_WIDGET (child));
  new_child = glade_placeholder_new ();
  gtk_container_add (GTK_CONTAINER (object), new_child);
  g_object_set_data (G_OBJECT (object), "child", new_child);
}

void
glade_hdy_search_bar_replace_child (GladeWidgetAdaptor *adaptor,
                                    GtkWidget          *container,
                                    GtkWidget          *current,
                                    GtkWidget          *new_widget)
{
  if (current != (GtkWidget *) g_object_get_data (G_OBJECT (container), "child"))
    return;

  gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (GTK_WIDGET (current))),
                        GTK_WIDGET (current));
  gtk_container_add (GTK_CONTAINER (container), new_widget);
  g_object_set_data (G_OBJECT (container), "child", new_widget);
}

GList *
glade_hdy_search_bar_get_children (GladeWidgetAdaptor *adaptor,
                                   GObject            *widget)
{
  GObject *current = g_object_get_data (G_OBJECT (widget), "child");

  return g_list_append (NULL, current);
}

gboolean
glade_hdy_search_bar_add_verify (GladeWidgetAdaptor *adaptor,
                                 GtkWidget          *container,
                                 GtkWidget          *child,
                                 gboolean            user_feedback)
{
  GObject *current = g_object_get_data (G_OBJECT (container), "child");

  if (!GLADE_IS_PLACEHOLDER (current)) {
    if (user_feedback)
      glade_util_ui_message (glade_app_get_window (),
                             GLADE_UI_INFO, NULL,
                             _("Search bar is already full"));

    return FALSE;
  }

  return TRUE;
}
