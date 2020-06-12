/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * glade-gtk-list-box.c - GladeWidgetAdaptor for GtkListBox
 * Copyright (C) 2013 Kalev Lember
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-expander-row.h"

#include <gladeui/glade.h>
#include "glade-hdy-utils.h"

void
glade_hdy_expander_row_post_create (GladeWidgetAdaptor *adaptor,
                                    GObject            *container,
                                    GladeCreateReason   reason)
{
  g_object_set (container, "expanded", TRUE, NULL);
}

void
glade_hdy_expander_row_get_child_property (GladeWidgetAdaptor *adaptor,
                                           GObject            *container,
                                           GObject            *child,
                                           const gchar        *property_name,
                                           GValue             *value)
{
  if (strcmp (property_name, "position") == 0)
    g_value_set_int (value, glade_hdy_get_child_index (GTK_CONTAINER (container),
                                                       GTK_WIDGET (child)));
  else
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->child_get_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
}

void
glade_hdy_expander_row_set_child_property (GladeWidgetAdaptor *adaptor,
                                           GObject            *container,
                                           GObject            *child,
                                           const gchar        *property_name,
                                           GValue             *value)
{
  if (strcmp (property_name, "position") == 0)
    glade_hdy_reorder_child (GTK_CONTAINER (container),
                             GTK_WIDGET (child),
                             g_value_get_int (value));
  else
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->child_set_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
}

void
glade_hdy_expander_row_add_child (GladeWidgetAdaptor *adaptor,
                                  GObject            *object,
                                  GObject            *child)
{
  gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (child));

  glade_hdy_sync_child_positions (GTK_CONTAINER (object));
}

void
glade_hdy_expander_row_remove_child (GladeWidgetAdaptor *adaptor,
                                     GObject            *object,
                                     GObject            *child)
{
  gtk_container_remove (GTK_CONTAINER (object), GTK_WIDGET (child));

  glade_hdy_sync_child_positions (GTK_CONTAINER (object));
}

gboolean
glade_hdy_expander_row_add_verify (GladeWidgetAdaptor *adaptor,
                                   GtkWidget          *object,
                                   GtkWidget          *child,
                                   gboolean            user_feedback)
{
  if (GTK_IS_LIST_BOX_ROW (child))
    return TRUE;

  if (user_feedback) {
      GladeWidgetAdaptor *row_adaptor =
        glade_widget_adaptor_get_by_type (GTK_TYPE_LIST_BOX_ROW);

      glade_util_ui_message (glade_app_get_window (),
                             GLADE_UI_INFO, NULL,
                             ONLY_THIS_GOES_IN_THAT_MSG,
                             glade_widget_adaptor_get_title (row_adaptor),
                             glade_widget_adaptor_get_title (adaptor));
  }

  return FALSE;
}
