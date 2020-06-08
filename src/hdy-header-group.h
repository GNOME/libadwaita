/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-header-bar.h"

G_BEGIN_DECLS

#define HDY_TYPE_HEADER_GROUP_CHILD (hdy_header_group_child_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyHeaderGroupChild, hdy_header_group_child, HDY, HEADER_GROUP_CHILD, GObject)

#define HDY_TYPE_HEADER_GROUP (hdy_header_group_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyHeaderGroup, hdy_header_group, HDY, HEADER_GROUP, GObject)

typedef enum {
  HDY_HEADER_GROUP_CHILD_TYPE_HEADER_BAR,
  HDY_HEADER_GROUP_CHILD_TYPE_GTK_HEADER_BAR,
  HDY_HEADER_GROUP_CHILD_TYPE_HEADER_GROUP,
} HdyHeaderGroupChildType;

HDY_AVAILABLE_IN_ALL
HdyHeaderBar   *hdy_header_group_child_get_header_bar     (HdyHeaderGroupChild *self);
HDY_AVAILABLE_IN_ALL
GtkHeaderBar   *hdy_header_group_child_get_gtk_header_bar (HdyHeaderGroupChild *self);
HDY_AVAILABLE_IN_ALL
HdyHeaderGroup *hdy_header_group_child_get_header_group   (HdyHeaderGroupChild *self);

HDY_AVAILABLE_IN_ALL
HdyHeaderGroupChildType hdy_header_group_child_get_child_type (HdyHeaderGroupChild *self);

HDY_AVAILABLE_IN_ALL
HdyHeaderGroup *hdy_header_group_new (void);

HDY_AVAILABLE_IN_ALL
void hdy_header_group_add_header_bar     (HdyHeaderGroup *self,
                                          HdyHeaderBar   *header_bar);
HDY_AVAILABLE_IN_ALL
void hdy_header_group_add_gtk_header_bar (HdyHeaderGroup *self,
                                          GtkHeaderBar   *header_bar);
HDY_AVAILABLE_IN_ALL
void hdy_header_group_add_header_group   (HdyHeaderGroup *self,
                                          HdyHeaderGroup *header_group);

HDY_AVAILABLE_IN_ALL
GSList *hdy_header_group_get_children (HdyHeaderGroup *self);

HDY_AVAILABLE_IN_ALL
void hdy_header_group_remove_header_bar     (HdyHeaderGroup *self,
                                             HdyHeaderBar   *header_bar);
HDY_AVAILABLE_IN_ALL
void hdy_header_group_remove_gtk_header_bar (HdyHeaderGroup *self,
                                             GtkHeaderBar   *header_bar);
HDY_AVAILABLE_IN_ALL
void hdy_header_group_remove_header_group   (HdyHeaderGroup *self,
                                             HdyHeaderGroup *header_group);
HDY_AVAILABLE_IN_ALL
void hdy_header_group_remove_child          (HdyHeaderGroup      *self,
                                             HdyHeaderGroupChild *child);

HDY_AVAILABLE_IN_ALL
gboolean hdy_header_group_get_decorate_all (HdyHeaderGroup *self);
HDY_AVAILABLE_IN_ALL
void     hdy_header_group_set_decorate_all (HdyHeaderGroup *self,
                                            gboolean        decorate_all);

G_END_DECLS
