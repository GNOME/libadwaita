/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * hdy-header-group.c - GladeWidgetAdaptor for HdyHeaderGroup
 * Copyright (C) 2018 Purism SPC
 * Copyright (C) 2013 Tristan Van Berkom
 */

#include <config.h>
#include <glib/gi18n-lib.h>

#include "glade-hdy-swipe-group.h"

#include <gladeui/glade.h>
#include "glade-hdy-utils.h"

#define PROP_SWIPEABLES "swipeables"
#define GLADE_TAG_SWIPEGROUP_SWIPEABLES "swipeables"
#define GLADE_TAG_SWIPEGROUP_SWIPEABLE "swipeable"

static void
glade_hdy_swipe_group_read_widgets (GladeWidget  *widget,
                                    GladeXmlNode *node)
{
  GladeXmlNode *widgets_node;
  GladeProperty *property;
  gchar *string = NULL;

  if ((widgets_node =
       glade_xml_search_child (node, GLADE_TAG_SWIPEGROUP_SWIPEABLES)) != NULL) {
    GladeXmlNode *n;

    for (n = glade_xml_node_get_children (widgets_node);
         n; n = glade_xml_node_next (n)) {
      gchar *widget_name, *tmp;

      if (!glade_xml_node_verify (n, GLADE_TAG_SWIPEGROUP_SWIPEABLE))
        continue;

      widget_name = glade_xml_get_property_string_required
        (n, GLADE_TAG_NAME, NULL);

      if (string == NULL) {
        string = widget_name;
      } else if (widget_name != NULL) {
        tmp =
          g_strdup_printf ("%s%s%s", string, GLADE_PROPERTY_DEF_OBJECT_DELIMITER,
                           widget_name);
        string = (g_free (string), tmp);
        g_free (widget_name);
      }
    }
  }

  if (string) {
    property = glade_widget_get_property (widget, PROP_SWIPEABLES);
    g_assert (property);

    /* we must synchronize this directly after loading this project
     * (i.e. lookup the actual objects after they've been parsed and
     * are present).
     */
    g_object_set_data_full (G_OBJECT (property),
                            "glade-loaded-object", string, g_free);
  }
}

void
glade_hdy_swipe_group_read_widget (GladeWidgetAdaptor *adaptor,
                                   GladeWidget        *widget,
                                   GladeXmlNode       *node)
{
  if (!(glade_xml_node_verify_silent (node, GLADE_XML_TAG_WIDGET) ||
        glade_xml_node_verify_silent (node, GLADE_XML_TAG_TEMPLATE)))
    return;

  /* First chain up and read in all the normal properties.. */
  GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (G_TYPE_OBJECT)->read_widget (adaptor, widget, node);

  glade_hdy_swipe_group_read_widgets (widget, node);
}

static void
glade_hdy_swipe_group_write_widgets (GladeWidget     *widget,
                                     GladeXmlContext *context,
                                     GladeXmlNode    *node)
{
  GladeXmlNode *widgets_node, *widget_node;
  GList *widgets = NULL, *list;
  GladeWidget *awidget;

  widgets_node = glade_xml_node_new (context, GLADE_TAG_SWIPEGROUP_SWIPEABLES);

  if (glade_widget_property_get (widget, PROP_SWIPEABLES, &widgets)) {
    for (list = widgets; list; list = list->next) {
      awidget = glade_widget_get_from_gobject (list->data);
      widget_node =
        glade_xml_node_new (context, GLADE_TAG_SWIPEGROUP_SWIPEABLE);
      glade_xml_node_append_child (widgets_node, widget_node);
      glade_xml_node_set_property_string (widget_node, GLADE_TAG_NAME,
                                          glade_widget_get_name (awidget));
    }
  }

  if (!glade_xml_node_get_children (widgets_node))
    glade_xml_node_delete (widgets_node);
  else
    glade_xml_node_append_child (node, widgets_node);
}

void
glade_hdy_swipe_group_write_widget (GladeWidgetAdaptor *adaptor,
                                    GladeWidget        *widget,
                                    GladeXmlContext    *context,
                                    GladeXmlNode       *node)
{
  if (!(glade_xml_node_verify_silent (node, GLADE_XML_TAG_WIDGET) ||
        glade_xml_node_verify_silent (node, GLADE_XML_TAG_TEMPLATE)))
    return;

  /* First chain up and read in all the normal properties.. */
  GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (G_TYPE_OBJECT)->write_widget (adaptor, widget, context, node);

  glade_hdy_swipe_group_write_widgets (widget, context, node);
}

void
glade_hdy_swipe_group_set_property (GladeWidgetAdaptor *adaptor,
                                    GObject            *object,
                                    const gchar        *property_name,
                                    const GValue       *value)
{
  if (!strcmp (property_name, PROP_SWIPEABLES)) {
    GSList *sg_widgets, *slist;
    GList *widgets, *list;

    /* remove old widgets */
    if ((sg_widgets =
         hdy_swipe_group_get_swipeables (HDY_SWIPE_GROUP (object))) != NULL) {
      /* copy since we are modifying an internal list */
      sg_widgets = g_slist_copy (sg_widgets);
      for (slist = sg_widgets; slist; slist = slist->next)
        hdy_swipe_group_remove_swipeable (HDY_SWIPE_GROUP (object),
                                          HDY_SWIPEABLE (slist->data));
      g_slist_free (sg_widgets);
    }

    /* add new widgets */
    if ((widgets = g_value_get_boxed (value)) != NULL) {
      for (list = widgets; list; list = list->next)
        hdy_swipe_group_add_swipeable (HDY_SWIPE_GROUP (object),
                                       HDY_SWIPEABLE (list->data));
    }
  } else {
    GLADE_WIDGET_ADAPTOR_GET_ADAPTOR_CLASS (G_TYPE_OBJECT)->set_property (adaptor, object,
                                                 property_name, value);
  }
}
