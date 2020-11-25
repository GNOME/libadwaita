/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * glade-gtk-header-bar.c - GladeWidgetAdaptor for GtkHeaderBar
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-flap.h"

#include <gladeui/glade.h>
#include "glade-hdy-utils.h"

static void
selection_changed_cb (GladeProject *project,
                      GladeWidget  *gwidget)
{
  GList *list;
  GtkWidget *sel_widget;
  GtkContainer *container = GTK_CONTAINER (glade_widget_get_object (gwidget));

  if ((list = glade_project_selection_get (project)) != NULL &&
      g_list_length (list) == 1) {
    sel_widget = list->data;

    if (GTK_IS_WIDGET (sel_widget) &&
        gtk_widget_is_ancestor (sel_widget, GTK_WIDGET (container))) {
      GtkWidget *content = hdy_flap_get_content (HDY_FLAP (container));
      GtkWidget *flap = hdy_flap_get_flap (HDY_FLAP (container));
      GtkWidget *separator = hdy_flap_get_separator (HDY_FLAP (container));
      gboolean folded = hdy_flap_get_folded (HDY_FLAP (container));

      if (folded &&
          (sel_widget == content ||
           gtk_widget_is_ancestor (sel_widget, content)))
        hdy_flap_set_reveal_flap (HDY_FLAP (container), FALSE);

      if ((sel_widget == flap ||
           gtk_widget_is_ancestor (sel_widget, flap)))
        hdy_flap_set_reveal_flap (HDY_FLAP (container), TRUE);

      if ((sel_widget == separator ||
           gtk_widget_is_ancestor (sel_widget, separator)))
        hdy_flap_set_reveal_flap (HDY_FLAP (container), TRUE);
    }
  }
}

static void
project_changed_cb (GladeWidget *gwidget,
                    GParamSpec  *pspec,
                    gpointer     userdata)
{
  GladeProject *project = glade_widget_get_project (gwidget);
  GladeProject *old_project = g_object_get_data (G_OBJECT (gwidget),
                                                 "project-ptr");

  if (old_project)
    g_signal_handlers_disconnect_by_func (G_OBJECT (old_project),
                                          G_CALLBACK (selection_changed_cb),
                                          gwidget);

  if (project)
    g_signal_connect (G_OBJECT (project),
                      "selection-changed",
                      G_CALLBACK (selection_changed_cb),
                      gwidget);

  g_object_set_data (G_OBJECT (gwidget), "project-ptr", project);
}

void
glade_hdy_flap_post_create (GladeWidgetAdaptor *adaptor,
                            GObject            *container,
                            GladeCreateReason   reason)
{
  GladeWidget *gwidget = glade_widget_get_from_gobject (container);
  GtkWidget *child;
  GtkWidget *content;

  if (!hdy_flap_get_flap (HDY_FLAP (container))) {
    child = glade_placeholder_new ();
    g_object_set_data (G_OBJECT (child), "special-child-type", "flap");
    hdy_flap_set_flap (HDY_FLAP (container), child);
  }

  if (!hdy_flap_get_separator (HDY_FLAP (container))) {
    child = glade_placeholder_new ();
    g_object_set_data (G_OBJECT (child), "special-child-type", "separator");
    hdy_flap_set_separator (HDY_FLAP (container), child);
  }

  content = hdy_flap_get_content (HDY_FLAP (container));
  if (!content) {
    child = glade_placeholder_new ();
    g_object_set_data (G_OBJECT (child), "special-child-type", "content");
    hdy_flap_set_content (HDY_FLAP (container), child);
  } else {
    g_object_set_data (G_OBJECT (content), "special-child-type", "content");
  }

  g_signal_connect (G_OBJECT (gwidget),
                    "notify::project",
                    G_CALLBACK (project_changed_cb),
                    NULL);

  project_changed_cb (gwidget, NULL, NULL);
}

void
glade_hdy_flap_add_child (GladeWidgetAdaptor *adaptor,
                          GObject            *parent,
                          GObject            *child)
{
  gchar *special_child_type = g_object_get_data (child, "special-child-type");

  if (special_child_type && !strcmp (special_child_type, "flap")) {
    hdy_flap_set_flap (HDY_FLAP (parent), GTK_WIDGET (child));

    return;
  }

  if (special_child_type && !strcmp (special_child_type, "separator")) {
    hdy_flap_set_separator (HDY_FLAP (parent), GTK_WIDGET (child));

    return;
  }

  hdy_flap_set_content (HDY_FLAP (parent), GTK_WIDGET (child));
}

void
glade_hdy_flap_remove_child (GladeWidgetAdaptor *adaptor,
                             GObject            *object,
                             GObject            *child)
{
  gchar *special_child_type = g_object_get_data (child, "special-child-type");
  GtkWidget *replacement = glade_placeholder_new ();

  if (special_child_type && !strcmp (special_child_type, "flap")) {
    g_object_set_data (G_OBJECT (replacement), "special-child-type", "flap");
    hdy_flap_set_flap (HDY_FLAP (object), replacement);

    return;
  }

  if (special_child_type && !strcmp (special_child_type, "separator")) {
    g_object_set_data (G_OBJECT (replacement), "special-child-type", "separator");
    hdy_flap_set_separator (HDY_FLAP (object), replacement);

    return;
  }

  g_object_set_data (G_OBJECT (replacement), "special-child-type", "content");
  hdy_flap_set_content (HDY_FLAP (object), replacement);
}

void
glade_hdy_flap_replace_child (GladeWidgetAdaptor *adaptor,
                              GObject            *container,
                              GObject            *current,
                              GObject            *new_widget)
{
  gchar *special_child_type =
    g_object_get_data (G_OBJECT (current), "special-child-type");

  if (special_child_type && !strcmp (special_child_type, "flap")) {
    g_object_set_data (G_OBJECT (new_widget), "special-child-type", "flap");
    hdy_flap_set_flap (HDY_FLAP (container), GTK_WIDGET (new_widget));

    return;
  }

  if (special_child_type && !strcmp (special_child_type, "separator")) {
    g_object_set_data (G_OBJECT (new_widget), "special-child-type", "separator");
    hdy_flap_set_separator (HDY_FLAP (container), GTK_WIDGET (new_widget));

    return;
  }

  g_object_set_data (G_OBJECT (new_widget), "special-child-type", "content");
  hdy_flap_set_content (HDY_FLAP (container), GTK_WIDGET (new_widget));
}
