/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * glade-gtk-stack.c - GladeWidgetAdaptor for GtkStack
 * Copyright (C) 2014 Red Hat, Inc.
 */

#include "glade-hdy-carousel.h"

#include <config.h>
#include <glib/gi18n-lib.h>
#include <gladeui/glade.h>

#include <math.h>

#define CENTER_CONTENT_INSENSITIVE_MSG _("This property does not apply unless Show Indicators is set.")

static gint
hdy_paginator_get_page (HdyPaginator *paginator)
{
  return round (hdy_paginator_get_position (paginator));
}

static gboolean
hdy_paginator_is_transient (HdyPaginator *paginator)
{
  return fmod (hdy_paginator_get_position (paginator), 1.0) > 0.00001;
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

static gint
get_page_index (GtkContainer *container,
                GtkWidget    *child)
{
  GList *children;
  gint index;

  children = gtk_container_get_children (container);
  index = g_list_index (children, child);
  g_list_free (children);

  return index;
}

static GtkWidget *
get_nth_page (GtkContainer *container,
              gint          n)
{
  GList *children;
  GtkWidget *child;

  children = gtk_container_get_children (container);
  child = g_list_nth_data (children, n);
  g_list_free (children);

  return child;
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
      hdy_paginator_scroll_to (HDY_PAGINATOR (container), page);
      index = get_page_index (container, page);
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
  old_project = g_object_get_data (G_OBJECT (gwidget), "paginator-project-ptr");

  if (old_project)
    g_signal_handlers_disconnect_by_func (G_OBJECT (old_project),
                                          G_CALLBACK (selection_changed_cb),
                                          gwidget);

  if (project)
    g_signal_connect (G_OBJECT (project), "selection-changed",
                      G_CALLBACK (selection_changed_cb), gwidget);

  g_object_set_data (G_OBJECT (gwidget), "paginator-project-ptr", project);
}

static void
position_changed_cb (HdyPaginator *paginator,
                     GParamSpec   *pspec,
                     GladeWidget  *gwidget)
{
  gint old_page, new_page;

  glade_widget_property_get (gwidget, "page", &old_page);
  new_page = hdy_paginator_get_page (paginator);

  if (old_page == new_page || hdy_paginator_is_transient (paginator))
    return;

  glade_widget_property_set (gwidget, "page", new_page);
}

void
glade_hdy_paginator_post_create (GladeWidgetAdaptor *adaptor,
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

  glade_widget_property_set_sensitive (gwidget, "indicator-spacing",
                                       FALSE, CENTER_CONTENT_INSENSITIVE_MSG);
  glade_widget_property_set_sensitive (gwidget, "center-content",
                                       FALSE, CENTER_CONTENT_INSENSITIVE_MSG);
}

void
glade_hdy_paginator_child_action_activate (GladeWidgetAdaptor *adaptor,
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

    index = get_page_index (GTK_CONTAINER (container), GTK_WIDGET (object));
    if (!strcmp (action_path, "insert_page_after"))
      index++;

    placeholder = glade_placeholder_new ();

    hdy_paginator_insert (HDY_PAGINATOR (container), placeholder, index);
    hdy_paginator_scroll_to (HDY_PAGINATOR (container), placeholder);

    property = glade_widget_get_property (parent, "pages");
    glade_command_set_property (property, pages + 1);

    property = glade_widget_get_property (parent, "page");

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
  old_size = hdy_paginator_get_n_pages (HDY_PAGINATOR (container));

  if (old_size == new_size)
    return;

  for (i = old_size; i < new_size; i++)
    gtk_container_add (GTK_CONTAINER (container), glade_placeholder_new ());

  for (i = old_size; i > 0; i--) {
    if (old_size <= new_size)
      break;
    child = get_nth_page (GTK_CONTAINER (container), i - 1);
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
  child = get_nth_page (GTK_CONTAINER (object), new_page);

  if (child)
    hdy_paginator_scroll_to (HDY_PAGINATOR (object), child);
}

static void
set_indicator_style (GObject      *container,
                     const GValue *value)
{
  GladeWidget *gwidget;
  HdyPaginatorIndicatorStyle style;

  gwidget = glade_widget_get_from_gobject (container);
  style = g_value_get_enum (value);

  glade_widget_property_set_sensitive (gwidget, "indicator-spacing",
                                       style != HDY_PAGINATOR_INDICATOR_STYLE_NONE,
                                       CENTER_CONTENT_INSENSITIVE_MSG);
  glade_widget_property_set_sensitive (gwidget, "center-content",
                                       style != HDY_PAGINATOR_INDICATOR_STYLE_NONE,
                                       CENTER_CONTENT_INSENSITIVE_MSG);
}

void
glade_hdy_paginator_set_property (GladeWidgetAdaptor *adaptor,
                                  GObject            *object,
                                  const gchar        *id,
                                  const GValue       *value)
{
  if (!strcmp (id, "pages"))
    set_n_pages (object, value);
  else if (!strcmp (id, "page"))
    set_page (object, value);
  else {
    if (!strcmp (id, "indicator-style"))
      set_indicator_style (object, value);

    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->set_property (adaptor, object, id, value);
  }
}

void
glade_hdy_paginator_get_property (GladeWidgetAdaptor *adaptor,
                                  GObject            *object,
                                  const gchar        *id,
                                  GValue             *value)
{
  if (!strcmp (id, "pages")) {
    g_value_reset (value);
    g_value_set_int (value, hdy_paginator_get_n_pages (HDY_PAGINATOR (object)));
  } else if (!strcmp (id, "page")) {
    g_value_reset (value);
    g_value_set_int (value, hdy_paginator_get_page (HDY_PAGINATOR (object)));
  } else {
    GWA_GET_CLASS (GTK_TYPE_CONTAINER)->get_property (adaptor, object, id, value);
  }
}

static gboolean
glade_hdy_paginator_verify_n_pages (GObject      *object,
                                    const GValue *value)
{
  gint new_size, old_size;

  new_size = g_value_get_int (value);
  old_size = get_n_pages_excluding_placeholders (GTK_CONTAINER (object));

  return old_size <= new_size;
}

static gboolean
glade_hdy_paginator_verify_page (GObject      *object,
                                 const GValue *value)
{
  gint page, pages;

  page = g_value_get_int (value);
  pages = hdy_paginator_get_n_pages (HDY_PAGINATOR (object));

  return 0 <= page && page < pages;
}

gboolean
glade_hdy_paginator_verify_property (GladeWidgetAdaptor *adaptor,
                                     GObject            *object,
                                     const gchar        *id,
                                     const GValue       *value)
{
  if (!strcmp (id, "pages"))
    return glade_hdy_paginator_verify_n_pages (object, value);
  else if (!strcmp (id, "page"))
    return glade_hdy_paginator_verify_page (object, value);
  else if (GWA_GET_CLASS (GTK_TYPE_CONTAINER)->verify_property)
    return GWA_GET_CLASS (GTK_TYPE_CONTAINER)->verify_property (adaptor, object,
                                                                id, value);

  return TRUE;
}

void
glade_hdy_paginator_add_child (GladeWidgetAdaptor *adaptor,
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

  gbox = glade_widget_get_from_gobject (container);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_paginator_remove_child (GladeWidgetAdaptor *adaptor,
                                  GObject            *container,
                                  GObject            *child)
{
  GladeWidget *gbox;
  gint pages, page;

  gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (child));

  gbox = glade_widget_get_from_gobject (container);
  glade_widget_property_get (gbox, "pages", &pages);
  glade_widget_property_set (gbox, "pages", pages);
  glade_widget_property_get (gbox, "page", &page);
  glade_widget_property_set (gbox, "page", page);
}

void
glade_hdy_paginator_replace_child (GladeWidgetAdaptor *adaptor,
                                   GObject            *container,
                                   GObject            *current,
                                   GObject            *new_widget)
{
  GladeWidget *gbox, *gchild;
  gint pages, page, index;

  index = get_page_index (GTK_CONTAINER (container), GTK_WIDGET (current));
  gtk_container_remove (GTK_CONTAINER (container), GTK_WIDGET (current));
  hdy_paginator_insert (HDY_PAGINATOR (container), GTK_WIDGET (new_widget),
                        index);
  hdy_paginator_scroll_to_full (HDY_PAGINATOR (container),
                                GTK_WIDGET (new_widget), 0);

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
