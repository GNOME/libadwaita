/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * glade-gtk-stack.c - GladeWidgetAdaptor for GtkStack
 * Copyright (C) 2014 Red Hat, Inc.
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-preferences-page.h"

#include <gladeui/glade.h>
#include "glade-hdy-utils.h"

static GtkWidget *
get_child_by_title (GtkContainer *container,
                    const gchar  *title)
{
  g_autoptr (GList) children = gtk_container_get_children (container);
  GList *l;

  for (l = children; l; l = l->next) {
    const gchar *child_title;

    g_assert (HDY_IS_PREFERENCES_GROUP (l->data));

    child_title = hdy_preferences_group_get_title (HDY_PREFERENCES_GROUP (l->data));

    if (child_title && !strcmp (child_title, title))
      return l->data;
  }

  return NULL;
}

static gchar *
get_unused_title (GtkContainer *container)
{
  gint i = 1;

  while (TRUE) {
    g_autofree gchar *title = g_strdup_printf ("Group %d", i);

    if (get_child_by_title (container, title) == NULL)
      return g_steal_pointer (&title);

    i++;
  }

  return NULL;
}

static void
add_group (GladeWidgetAdaptor *adaptor,
           GObject            *container)
{
  GladeWidget *gwidget = glade_widget_get_from_gobject (container);
  GladeWidget *gpage;
  GladeWidgetAdaptor *page_adaptor;
  g_autofree gchar *title = get_unused_title (GTK_CONTAINER (container));

  page_adaptor = glade_widget_adaptor_get_by_type (HDY_TYPE_PREFERENCES_GROUP);

  gpage = glade_widget_adaptor_create_widget (page_adaptor, FALSE,
                                              "parent", gwidget,
                                              "project", glade_widget_get_project (gwidget),
                                              NULL);

  glade_widget_property_set (gpage, "title", title);

  glade_widget_add_child (gwidget, gpage, FALSE);
}

void
glade_hdy_preferences_page_post_create (GladeWidgetAdaptor *adaptor,
                                        GObject            *container,
                                        GladeCreateReason   reason)
{
  if (reason == GLADE_CREATE_USER) {
    add_group (adaptor, container);
    add_group (adaptor, container);
    add_group (adaptor, container);
  }
}

gboolean
glade_hdy_preferences_page_add_verify (GladeWidgetAdaptor *adaptor,
                                       GtkWidget          *object,
                                       GtkWidget          *child,
                                       gboolean            user_feedback)
{
  if (!HDY_IS_PREFERENCES_GROUP (child)) {
    if (user_feedback) {
        GladeWidgetAdaptor *page_adaptor =
          glade_widget_adaptor_get_by_type (HDY_TYPE_PREFERENCES_GROUP);

        glade_util_ui_message (glade_app_get_window (),
                               GLADE_UI_INFO, NULL,
                               ONLY_THIS_GOES_IN_THAT_MSG,
                               glade_widget_adaptor_get_title (page_adaptor),
                               glade_widget_adaptor_get_title (adaptor));
    }

    return FALSE;
  }

  return TRUE;
}

void
glade_hdy_preferences_page_add_child (GladeWidgetAdaptor *adaptor,
                                      GObject            *object,
                                      GObject            *child)
{
  gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (child));
}

void
glade_hdy_preferences_page_remove_child (GladeWidgetAdaptor *adaptor,
                                         GObject            *object,
                                         GObject            *child)
{
  gtk_container_remove (GTK_CONTAINER (object), GTK_WIDGET (child));
}

void
glade_hdy_preferences_page_replace_child (GladeWidgetAdaptor *adaptor,
                                          GObject            *object,
                                          GObject            *current,
                                          GObject            *new_widget)
{
  gint index = glade_hdy_get_child_index (GTK_CONTAINER (object), GTK_WIDGET (current));
  gtk_container_remove (GTK_CONTAINER (object), GTK_WIDGET (current));
  gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (new_widget));
  glade_hdy_reorder_child (GTK_CONTAINER (object), GTK_WIDGET (new_widget), index);
}

GList *
glade_hdy_preferences_page_get_children (GladeWidgetAdaptor *adaptor,
                                         GObject            *object)
{
  return gtk_container_get_children (GTK_CONTAINER (object));
}

void
glade_hdy_preferences_page_action_activate (GladeWidgetAdaptor *adaptor,
                                            GObject            *object,
                                            const gchar        *action_path)
{
  GladeWidget *parent = glade_widget_get_from_gobject (object);

  if (!g_strcmp0 (action_path, "add_group")) {
    g_autofree gchar *title = get_unused_title (GTK_CONTAINER (object));
    GladeWidget *gchild;

    glade_command_push_group (_("Add group to %s"),
                              glade_widget_get_name (parent));

    gchild = glade_command_create (glade_widget_adaptor_get_by_type (HDY_TYPE_PREFERENCES_GROUP),
                                   parent,
                                   NULL,
                                   glade_widget_get_project (parent));

    glade_widget_property_set (gchild, "title", title);

    glade_command_pop_group ();
  } else {
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->action_activate (adaptor,
                                                         object,
                                                         action_path);
  }
}

void
glade_hdy_preferences_page_child_set_property (GladeWidgetAdaptor *adaptor,
                                               GObject            *container,
                                               GObject            *child,
                                               const gchar        *property_name,
                                               const GValue       *value)
{
  if (!g_strcmp0 (property_name, "position")) {
    GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (child));

    gtk_container_child_set_property (GTK_CONTAINER (parent),
                                      GTK_WIDGET (child), property_name, value);
  } else {
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->child_set_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
  }
}

void
glade_hdy_preferences_page_child_get_property (GladeWidgetAdaptor *adaptor,
                                               GObject            *container,
                                               GObject            *child,
                                               const gchar        *property_name,
                                               GValue             *value)
{
  if (!g_strcmp0 (property_name, "position")) {
    GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (child));

    gtk_container_child_get_property (GTK_CONTAINER (parent),
                                      GTK_WIDGET (child), property_name, value);
  } else {
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->child_get_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
  }
}
