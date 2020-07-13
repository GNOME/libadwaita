/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * glade-gtk-stack.c - GladeWidgetAdaptor for GtkStack
 * Copyright (C) 2014 Red Hat, Inc.
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-carousel.h"

#include <gladeui/glade.h>
#include "glade-hdy-utils.h"

#include <math.h>

static gint
hdy_carousel_get_page (HdyCarousel *carousel)
{
  return round (hdy_carousel_get_position (carousel));
}

static gboolean
hdy_carousel_is_transient (HdyCarousel *carousel)
{
  return fmod (hdy_carousel_get_position (carousel), 1.0) > 0.00001;
}

static gint
get_n_pages_excluding_placeholders (GtkContainer *container)
{
  GList *children, *l;
  gint n_pages;

  children = gtk_container_get_children (container);

  n_pages = 0;
  for (l = children; l; l = l->next)
    if (!GLADE_IS_PLACEHOLDER (l->data))
      n_pages++;

  g_list_free (children);


  return n_pages;
}

static void
selection_changed_cb (GladeProject *project,
                      GladeWidget  *gwidget)
{
  GList *list;
  GtkWidget *page, *sel_widget;
  GtkContainer *container;
  GList *children, *l;
  gint index;

  list = glade_project_selection_get (project);
  if (!list || g_list_length (list) != 1)
    return;

  sel_widget = list->data;

  container = GTK_CONTAINER (glade_widget_get_object (gwidget));

  if (!GTK_IS_WIDGET (sel_widget) ||
      !gtk_widget_is_ancestor (sel_widget, GTK_WIDGET (container)))
    return;

  children = gtk_container_get_children (container);
  for (l = children; l; l = l->next) {
    page = l->data;
    if (sel_widget == page || gtk_widget_is_ancestor (sel_widget, page)) {
      hdy_carousel_scroll_to (HDY_CAROUSEL (container), page);
      index = glade_hdy_get_child_index (container, page);
      glade_widget_property_set (gwidget, "page", index);
      break;
    }
  }
  g_list_free (children);
}

static void
project_changed_cb (GladeWidget *gwidget,
                    GParamSpec  *pspec,
                    gpointer     user_data)
{
  GladeProject *project, *old_project;

  project = glade_widget_get_project (gwidget);
  old_project = g_object_get_data (G_OBJECT (gwidget), "carousel-project-ptr");

  if (old_project)
    g_signal_handlers_disconnect_by_func (G_OBJECT (old_project),
                                          G_CALLBACK (selection_changed_cb),
                                          gwidget);

  if (project)
    g_signal_connect (G_OBJECT (project), "selection-changed",
                      G_CALLBACK (selection_changed_cb), gwidget);

  g_object_set_data (G_OBJECT (gwidget), "carousel-project-ptr", project);
}

static void
position_changed_cb (HdyCarousel *carousel,
                     GParamSpec  *pspec,
                     GladeWidget *gwidget)
{
  gint old_page, new_page;

  glade_widget_property_get (gwidget, "page", &old_page);
  new_page = hdy_carousel_get_page (carousel);

  if (old_page == new_page || hdy_carousel_is_transient (carousel))
    return;

  glade_widget_property_set (gwidget, "page", new_page);
}

void
glade_hdy_carousel_post_create (GladeWidgetAdaptor *adaptor,
                                GObject            *container,
                                GladeCreateReason   reason)
{
  GladeWidget *gwidget = glade_widget_get_from_gobject (container);

  if (reason == GLADE_CREATE_USER)
    gtk_container_add (GTK_CONTAINER (container), glade_placeholder_new ());

  g_signal_connect (G_OBJECT (gwidget), "notify::project",
                    G_CALLBACK (project_changed_cb), NULL);

  project_changed_cb (gwidget, NULL, NULL);

  g_signal_connect (G_OBJECT (container), "notify::position",
                    G_CALLBACK (position_changed_cb), gwidget);
}

void
glade_hdy_carousel_child_action_activate (GladeWidgetAdaptor *adaptor,
                                          GObject            *container,
                                          GObject            *object,
                                          const gchar        *action_path)
{
  if (!strcmp (action_path, "insert_page_after") ||
      !strcmp (action_path, "insert_page_before")) {
    GladeWidget *parent;
    GladeProperty *property;
    GtkWidget *placeholder;
    gint pages, index;

    parent = glade_widget_get_from_gobject (container);
    glade_widget_property_get (parent, "pages", &pages);

    glade_command_push_group (_("Insert placeholder to %s"),
                              glade_widget_get_name (parent));

    index = glade_hdy_get_child_index (GTK_CONTAINER (container), GTK_WIDGET (object));
    if (!strcmp (action_path, "insert_page_after"))
      index++;

    placeholder = glade_placeholder_new ();

    hdy_carousel_insert (HDY_CAROUSEL (container), placeholder, index);
    hdy_carousel_scroll_to (HDY_CAROUSEL (container), placeholder);

    glade_hdy_sync_child_positions (GTK_CONTAINER (container));

    property = glade_widget_get_property (parent, "pages");
    glade_command_set_property (property, pages + 1);

    property = glade_widget_get_property (parent, "page");
    glade_command_set_property (property, index);

    glade_command_pop_group ();
  } else if (strcmp (action_path, "remove_page") == 0) {
    GladeWidget *parent;
    GladeProperty *property;
    gint pages, position;

    parent = glade_widget_get_from_gobject (container);
    glade_widget_property_get (parent, "pages", &pages);

    glade_command_push_group (_("Remove placeholder from %s"),
                              glade_widget_get_name (parent));

    g_assert (GLADE_IS_PLACEHOLDER (object));
    gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (object));

    glade_hdy_sync_child_positions (GTK_CONTAINER (container));

    property = glade_widget_get_property (parent, "pages");
    glade_command_set_property (property, pages - 1);

    glade_widget_property_get (parent, "page", &position);
    property = glade_widget_get_property (parent, "page");
    glade_command_set_property (property, position);

    glade_command_pop_group ();
  } else
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->child_action_activate (adaptor,
                                                               container,
                                                               object,
                                                               action_path);
}

static void
set_n_pages (GObject      *container,
             const GValue *value)
{
  GladeWidget *gbox;
  GtkWidget *child;
  gint old_size, new_size, i, page;

  new_size = g_value_get_int (value);
  old_size = hdy_carousel_get_n_pages (HDY_CAROUSEL (container));

  if (old_size == new_size)
    return;

  for (i = old_size; i < new_size; i++)
    gtk_container_add (GTK_CONTAINER (container), glade_placeholder_new ());

  for (i = old_size; i > 0; i--) {
    if (old_size <= new_size)
      break;
    child = glade_hdy_get_nth_child (GTK_CONTAINER (container), i - 1);
    if (GLADE_IS_PLACEHOLDER (child)) {
      gtk_container_remove (GTK_CONTAINER (container), child);
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
  gint new_page;
  GtkWidget *child;

  new_page = g_value_get_int (value);
  child = glade_hdy_get_nth_child (GTK_CONTAINER (object), new_page);

  if (child)
    hdy_carousel_scroll_to (HDY_CAROUSEL (object), child);
}

void
glade_hdy_carousel_set_property (GladeWidgetAdaptor *adaptor,
                                 GObject            *object,
                                 const gchar        *id,
                                 const GValue       *value)
{
  if (!strcmp (id, "pages"))
    set_n_pages (object, value);
  else if (!strcmp (id, "page"))
    set_page (object, value);
  else {
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->set_property (adaptor, object, id, value);
  }
}

void
glade_hdy_carousel_get_property (GladeWidgetAdaptor *adaptor,
                                 GObject            *object,
                                 const gchar        *id,
                                 GValue             *value)
{
  if (!strcmp (id, "pages")) {
    g_value_reset (value);
    g_value_set_int (value, hdy_carousel_get_n_pages (HDY_CAROUSEL (object)));
  } else if (!strcmp (id, "page")) {
    g_value_reset (value);
    g_value_set_int (value, hdy_carousel_get_page (HDY_CAROUSEL (object)));
  } else {
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->get_property (adaptor, object, id, value);
  }
}

static gboolean
glade_hdy_carousel_verify_n_pages (GObject      *object,
                                   const GValue *value)
{
  gint new_size, old_size;

  new_size = g_value_get_int (value);
  old_size = get_n_pages_excluding_placeholders (GTK_CONTAINER (object));

  return old_size <= new_size;
}

static gboolean
glade_hdy_carousel_verify_page (GObject      *object,
                                const GValue *value)
{
  gint page, pages;

  page = g_value_get_int (value);
  pages = hdy_carousel_get_n_pages (HDY_CAROUSEL (object));

  return 0 <= page && page < pages;
}

gboolean
glade_hdy_carousel_verify_property (GladeWidgetAdaptor *adaptor,
                                    GObject            *object,
                                    const gchar        *id,
                                    const GValue       *value)
{
  if (!strcmp (id, "pages"))
    return glade_hdy_carousel_verify_n_pages (object, value);
  else if (!strcmp (id, "page"))
    return glade_hdy_carousel_verify_page (object, value);
  else if (GWA_GET_CLASS (GTK_TYPE_CONTAINER)->verify_property)
    return GWA_GET_CLASS (GTK_TYPE_CONTAINER)->verify_property (adaptor, object,
                                                                id, value);

  return TRUE;
}

void
glade_hdy_carousel_add_child (GladeWidgetAdaptor *adaptor,
                              GObject            *container,
                              GObject            *child)
{
  GladeWidget *gbox, *gchild;
  gint pages, page;

  if (!glade_widget_superuser () && !GLADE_IS_PLACEHOLDER (child)) {
    GList *l, *children;

    children = gtk_container_get_children (GTK_CONTAINER (container));

    for (l = g_list_last (children); l; l = l->prev) {
      GtkWidget *widget = l->data;

      if (GLADE_IS_PLACEHOLDER (widget)) {
        gtk_container_remove (GTK_CONTAINER (container), widget);
        break;
      }
    }

    g_list_free (children);
  }

  gtk_container_add (GTK_CONTAINER (container), GTK_WIDGET (child));

  gchild = glade_widget_get_from_gobject (child);
  if (gchild)
    glade_widget_set_pack_action_visible (gchild, "remove_page", FALSE);

  glade_hdy_sync_child_positions (GTK_CONTAINER (container));

  gbox = glade_widget_get_from_gobject (container);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_carousel_remove_child (GladeWidgetAdaptor *adaptor,
                                 GObject            *container,
                                 GObject            *child)
{
  GladeWidget *gbox;
  gint pages, page;

  gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (child));

  glade_hdy_sync_child_positions (GTK_CONTAINER (container));

  gbox = glade_widget_get_from_gobject (container);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_carousel_replace_child (GladeWidgetAdaptor *adaptor,
                                  GObject            *container,
                                  GObject            *current,
                                  GObject            *new_widget)
{
  GladeWidget *gbox, *gchild;
  gint pages, page, index;

  index = glade_hdy_get_child_index (GTK_CONTAINER (container), GTK_WIDGET (current));
  gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (current));
  hdy_carousel_insert (HDY_CAROUSEL (container), GTK_WIDGET (new_widget),
                       index);
  hdy_carousel_scroll_to_full (HDY_CAROUSEL (container),
                               GTK_WIDGET (new_widget), 0);

  glade_hdy_sync_child_positions (GTK_CONTAINER (container));

  gchild = glade_widget_get_from_gobject (new_widget);
  if (gchild)
    glade_widget_set_pack_action_visible (gchild, "remove_page", FALSE);

  /* NOTE: make sure to sync this at the end because new_widget could be
   * a placeholder and syncing these properties could destroy it.
   */
  gbox = glade_widget_get_from_gobject (container);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_carousel_get_child_property (GladeWidgetAdaptor *adaptor,
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
glade_hdy_carousel_set_child_property (GladeWidgetAdaptor *adaptor,
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
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->child_set_property (adaptor,
                                                            container,
                                                            child,
                                                            property_name,
                                                            value);
  }
}
