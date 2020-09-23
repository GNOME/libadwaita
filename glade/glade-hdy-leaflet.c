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

#include "glade-hdy-leaflet.h"

#include <gladeui/glade.h>
#include "glade-hdy-utils.h"

#define PAGE_DISABLED_MESSAGE _("This property only applies when the leaflet is folded")

static void
selection_changed_cb (GladeProject *project,
                      GladeWidget  *gwidget)
{
  GList *list;
  GtkWidget *page, *sel_widget;
  GtkContainer *container = GTK_CONTAINER (glade_widget_get_object (gwidget));
  gint index;

  if ((list = glade_project_selection_get (project)) != NULL &&
      g_list_length (list) == 1) {
    sel_widget = list->data;

    if (GTK_IS_WIDGET (sel_widget) &&
        gtk_widget_is_ancestor (sel_widget, GTK_WIDGET (container))) {
      g_autoptr (GList) children = gtk_container_get_children (container);
      GList *l;

      index = 0;
      for (l = children; l; l = l->next) {
        page = l->data;
        if (sel_widget == page ||
            gtk_widget_is_ancestor (sel_widget, page)) {
          glade_widget_property_set (gwidget, "page", index);

          break;
        }

        index++;
      }
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

static void
add_named (GtkContainer *container,
           GtkWidget    *child,
           const gchar  *name)
{
  gtk_container_add_with_properties (container,
                                     child,
                                     "name", name,
                                     NULL);
}

static void
folded_changed_cb (HdyLeaflet *leaflet,
                   GParamSpec *pspec,
                   gpointer    userdata)
{
  GladeWidget *gwidget = glade_widget_get_from_gobject (leaflet);
  gboolean folded = hdy_leaflet_get_folded (leaflet);

  glade_widget_property_set_sensitive (gwidget,
                                       "page",
                                       folded,
                                       folded ? NULL : PAGE_DISABLED_MESSAGE);
}

void
glade_hdy_leaflet_post_create (GladeWidgetAdaptor *adaptor,
                               GObject            *container,
                               GladeCreateReason   reason)
{
  GladeWidget *gwidget = glade_widget_get_from_gobject (container);

  if (reason == GLADE_CREATE_USER)
    add_named (GTK_CONTAINER (container),
               glade_placeholder_new (),
               "page0");

  g_signal_connect (G_OBJECT (gwidget),
                    "notify::project",
                    G_CALLBACK (project_changed_cb),
                    NULL);

  project_changed_cb (gwidget, NULL, NULL);

  if (HDY_IS_LEAFLET (container)) {
    g_signal_connect (container,
                      "notify::folded",
                      G_CALLBACK (folded_changed_cb),
                      NULL);

    folded_changed_cb (HDY_LEAFLET (container), NULL, NULL);
  }
}

static GtkWidget *
get_child_by_name (GtkContainer *container,
                   const gchar  *name)
{
  g_autoptr (GList) children = gtk_container_get_children (container);
  GList *l;

  for (l = children; l; l = l->next) {
    const gchar *child_name;

    gtk_container_child_get (container, l->data, "name", &child_name, NULL);

    if (child_name && !strcmp (child_name, name))
      return l->data;
  }

  return NULL;
}

static gchar *
get_unused_name (GtkContainer *container)
{
  gint i = 0;

  while (TRUE) {
    g_autofree gchar *name = g_strdup_printf ("page%d", i);

    if (get_child_by_name (container, name) == NULL)
      return g_steal_pointer (&name);

    i++;
  }

  return NULL;
}

void
glade_hdy_leaflet_child_action_activate (GladeWidgetAdaptor *adaptor,
                                         GObject            *container,
                                         GObject            *object,
                                         const gchar        *action_path)
{
  if (!strcmp (action_path, "insert_page_after") ||
      !strcmp (action_path, "insert_page_before")) {
    GladeWidget *parent = glade_widget_get_from_gobject (container);
    GladeProperty *property;
    g_autofree gchar *name = NULL;
    GtkWidget *new_widget;
    gint pages, index;

    glade_widget_property_get (parent, "pages", &pages);

    glade_command_push_group (_("Insert placeholder to %s"),
                              glade_widget_get_name (parent));

    index = glade_hdy_get_child_index (GTK_CONTAINER (container), GTK_WIDGET (object));

    if (!strcmp (action_path, "insert_page_after"))
      index++;

    name = get_unused_name (GTK_CONTAINER (container));
    new_widget = glade_placeholder_new ();
    add_named (GTK_CONTAINER (container), new_widget, name);
    glade_hdy_reorder_child (GTK_CONTAINER (container), new_widget, index);
    g_object_set (container, "visible-child", new_widget, NULL);

    glade_hdy_sync_child_positions (GTK_CONTAINER (container));

    property = glade_widget_get_property (parent, "pages");
    glade_command_set_property (property, pages + 1);

    property = glade_widget_get_property (parent, "page");
    glade_command_set_property (property, index);

    glade_command_pop_group ();
  } else if (strcmp (action_path, "remove_page") == 0) {
    GladeWidget *parent = glade_widget_get_from_gobject (container);
    GladeProperty *property;
    gint pages, index;

    glade_widget_property_get (parent, "pages", &pages);

    glade_command_push_group (_("Remove placeholder from %s"),
                              glade_widget_get_name (parent));
    g_assert (GLADE_IS_PLACEHOLDER (object));
    gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (object));

    glade_hdy_sync_child_positions (GTK_CONTAINER (container));

    property = glade_widget_get_property (parent, "pages");
    glade_command_set_property (property, pages - 1);

    glade_widget_property_get (parent, "page", &index);
    property = glade_widget_get_property (parent, "page");
    glade_command_set_property (property, index);

    glade_command_pop_group ();
  } else {
    GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->child_action_activate (adaptor,
                                                               container,
                                                               object,
                                                               action_path);
  }
}

typedef struct {
  gint size;
  gboolean include_placeholders;
} ChildData;

static void
count_child (GtkWidget *child,
             gpointer   data)
{
  ChildData *cdata = data;

  if (cdata->include_placeholders || !GLADE_IS_PLACEHOLDER (child))
    cdata->size++;
}

static gint
get_n_pages (GtkContainer *container,
             gboolean      include_placeholders)
{
  ChildData data;

  data.size = 0;
  data.include_placeholders = include_placeholders;
  gtk_container_foreach (container, count_child, &data);

  return data.size;
}

static void
set_n_pages (GObject      *object,
             const GValue *value)
{
  GladeWidget *gbox;
  GtkContainer *container = GTK_CONTAINER (object);
  GtkWidget *child;
  gint new_size = g_value_get_int (value);
  gint old_size = get_n_pages (container, TRUE);
  gint i, page;

  if (old_size == new_size)
    return;

  for (i = old_size; i < new_size; i++) {
    g_autofree gchar *name = get_unused_name (container);
    child = glade_placeholder_new ();
    add_named (container, child, name);
  }

  for (i = old_size; i > 0; i--) {
    if (old_size <= new_size)
      break;

    child = glade_hdy_get_nth_child (container, i - 1);
    if (GLADE_IS_PLACEHOLDER (child)) {
      gtk_container_remove (container, child);
      old_size--;
    }
  }

  gbox = glade_widget_get_from_gobject (container);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

static void
set_page (GObject      *object,
          const GValue *value)
{
  gint new_page = g_value_get_int (value);
  GtkWidget *child = glade_hdy_get_nth_child (GTK_CONTAINER (object), new_page);

  if (!child)
    return;

  g_object_set (object, "visible-child", child, NULL);
}

static gint
get_page (GtkContainer *container)
{
  GtkWidget *child;

  g_object_get (container, "visible-child", &child, NULL);

  return glade_hdy_get_child_index (container, child);
}

void
glade_hdy_leaflet_set_property (GladeWidgetAdaptor *adaptor,
                                GObject            *object,
                                const gchar        *id,
                                const GValue       *value)
{
  if (!strcmp (id, "pages"))
    set_n_pages (object, value);
  else if (!strcmp (id, "page"))
    set_page (object, value);
  else
    GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->set_property (adaptor, object, id, value);
}

void
glade_hdy_leaflet_get_property (GladeWidgetAdaptor *adaptor,
                                GObject            *object,
                                const gchar        *id,
                                GValue             *value)
{
  if (!strcmp (id, "pages")) {
    g_value_reset (value);
    g_value_set_int (value, get_n_pages (GTK_CONTAINER (object), TRUE));
  } else if (!strcmp (id, "page")) {
    g_value_reset (value);
    g_value_set_int (value, get_page (GTK_CONTAINER (object)));
  } else {
    GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->get_property (adaptor, object, id, value);
  }
}

static gboolean
verify_n_pages (GObject      *object,
                const GValue *value)
{
  gint new_size = g_value_get_int (value);
  gint old_size = get_n_pages (GTK_CONTAINER (object), FALSE);

  return old_size <= new_size;
}

static gboolean
verify_page (GObject      *object,
             const GValue *value)
{
  gint page = g_value_get_int (value);
  gint pages = get_n_pages (GTK_CONTAINER (object), TRUE);

  if (page < 0 && page >= pages)
    return FALSE;

  if (HDY_IS_LEAFLET (object)) {
    GtkWidget *child = glade_hdy_get_nth_child (GTK_CONTAINER (object), page);
    gboolean navigatable;

    gtk_container_child_get (GTK_CONTAINER (object), child,
                             "navigatable", &navigatable,
                             NULL);

    if (!navigatable)
      return FALSE;
  }

  return TRUE;
}

gboolean
glade_hdy_leaflet_verify_property (GladeWidgetAdaptor *adaptor,
                                   GObject            *object,
                                   const gchar        *id,
                                   const GValue       *value)
{
  if (!strcmp (id, "pages"))
    return verify_n_pages (object, value);
  else if (!strcmp (id, "page"))
    return verify_page (object, value);
  else if (GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->verify_property)
    return GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->verify_property (adaptor, object, id, value);

  return TRUE;
}


void
glade_hdy_leaflet_get_child_property (GladeWidgetAdaptor *adaptor,
                                      GObject            *container,
                                      GObject            *child,
                                      const gchar        *property_name,
                                      GValue             *value)
{
  if (strcmp (property_name, "position") == 0)
    g_value_set_int (value, glade_hdy_get_child_index (GTK_CONTAINER (container),
                                                       GTK_WIDGET (child)));
  else
    GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->child_get_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
}

void
glade_hdy_leaflet_set_child_property (GladeWidgetAdaptor *adaptor,
                                      GObject            *container,
                                      GObject            *child,
                                      const gchar        *property_name,
                                      GValue             *value)
{
  if (strcmp (property_name, "position") == 0) {
    glade_hdy_reorder_child (GTK_CONTAINER (container),
                             GTK_WIDGET (child),
                             g_value_get_int (value));

    glade_hdy_sync_child_positions (GTK_CONTAINER (container));
  } else {
    GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (GTK_TYPE_CONTAINER)->child_set_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
  }
}

void
glade_hdy_leaflet_add_child (GladeWidgetAdaptor *adaptor,
                             GObject            *object,
                             GObject            *child)
{
  GladeWidget *gbox, *gchild;
  gint pages, page;

  if (!glade_widget_superuser () && !GLADE_IS_PLACEHOLDER (child)) {
    g_autoptr (GList) children = gtk_container_get_children (GTK_CONTAINER (object));
    GList *l;

    for (l = g_list_last (children); l; l = l->prev) {
      GtkWidget *widget = l->data;
      if (GLADE_IS_PLACEHOLDER (widget)) {
        gtk_container_remove (GTK_CONTAINER (object), widget);

        break;
      }
    }
  }

  gtk_container_add (GTK_CONTAINER (object), GTK_WIDGET (child));

  glade_hdy_sync_child_positions (GTK_CONTAINER (object));

  gchild = glade_widget_get_from_gobject (child);
  if (gchild != NULL)
    glade_widget_set_pack_action_visible (gchild, "remove_page", FALSE);

  gbox = glade_widget_get_from_gobject (object);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_leaflet_remove_child (GladeWidgetAdaptor *adaptor,
                                GObject            *object,
                                GObject            *child)
{
  GladeWidget *gbox;
  gint pages, page;

  gtk_container_remove (GTK_CONTAINER (object), GTK_WIDGET (child));

  glade_hdy_sync_child_positions (GTK_CONTAINER (object));

  gbox = glade_widget_get_from_gobject (object);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_leaflet_replace_child (GladeWidgetAdaptor *adaptor,
                                 GObject            *container,
                                 GObject            *current,
                                 GObject            *new_widget)
{
  GladeWidget *gchild;
  GladeWidget *gbox;
  gint pages, page, index;

  index = glade_hdy_get_child_index (GTK_CONTAINER (container), GTK_WIDGET (current));
  gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (current));
  gtk_container_add (GTK_CONTAINER (container), GTK_WIDGET (new_widget));
  glade_hdy_reorder_child (GTK_CONTAINER (container), GTK_WIDGET (new_widget), index);

  glade_hdy_sync_child_positions (GTK_CONTAINER (container));

  gbox = glade_widget_get_from_gobject (container);

  gchild = glade_widget_get_from_gobject (new_widget);
  if (gchild != NULL)
    glade_widget_set_pack_action_visible (gchild, "remove_page", FALSE);

  /* NOTE: make sure to sync this at the end because new_widget could be
   * a placeholder and syncing these properties could destroy it.
   */
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}
