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

#include "glade-hdy-window.h"

#include <gladeui/glade.h>

#define ALREADY_HAS_A_CHILD_MSG _("%s cannot have more than one child.")

static GtkWidget *
get_child (GtkContainer *window)
{
  g_autoptr (GList) children = gtk_container_get_children (window);

  if (!children)
    return NULL;

  return children->data;
}

void
glade_hdy_window_post_create (GladeWidgetAdaptor *adaptor,
                              GObject            *object,
                              GladeCreateReason   reason)
{
  if (reason != GLADE_CREATE_USER)
    return;

  gtk_container_add (GTK_CONTAINER (object), glade_placeholder_new ());
}

void
glade_hdy_window_add_child (GladeWidgetAdaptor *adaptor,
                            GObject            *object,
                            GObject            *child)
{
  GtkWidget *window_child = get_child (GTK_CONTAINER (object));

  if (window_child) {
    if (GLADE_IS_PLACEHOLDER (window_child)) {
      gtk_container_remove (GTK_CONTAINER (object), window_child);
    } else {
      g_critical ("Can't add more than one widget to a HdyWindow");

      return;
    }
  }

  gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (child));
}

void
glade_hdy_window_remove_child (GladeWidgetAdaptor *adaptor,
                               GObject            *object,
                               GObject            *child)
{
  gtk_container_remove (GTK_CONTAINER (object), GTK_WIDGET (child));
  gtk_container_add (GTK_CONTAINER (object), glade_placeholder_new ());
}

void
glade_hdy_window_replace_child (GladeWidgetAdaptor *adaptor,
                                GtkWidget          *object,
                                GtkWidget          *current,
                                GtkWidget          *new_widget)
{
  gtk_container_remove (GTK_CONTAINER (object), current);
  gtk_container_add (GTK_CONTAINER (object), new_widget);
}

GList *
glade_hdy_window_get_children (GladeWidgetAdaptor *adaptor,
                               GObject            *object)
{
  return gtk_container_get_children (GTK_CONTAINER (object));
}

gboolean
glade_hdy_window_add_verify (GladeWidgetAdaptor *adaptor,
                             GtkWidget          *object,
                             GtkWidget          *child,
                             gboolean            user_feedback)
{
  GtkWidget *window_child = get_child (GTK_CONTAINER (object));

  if (window_child && !GLADE_IS_PLACEHOLDER (window_child)) {
    if (user_feedback)
      glade_util_ui_message (glade_app_get_window (),
                             GLADE_UI_INFO, NULL,
                             ALREADY_HAS_A_CHILD_MSG,
                             glade_widget_adaptor_get_title (adaptor));

    return FALSE;
  }

  return TRUE;
}

