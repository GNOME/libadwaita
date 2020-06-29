/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "hdy-header-bar.h"

G_BEGIN_DECLS

#define HDY_TYPE_HEADER_GROUP_CHILD (hdy_header_group_child_get_type())

G_DECLARE_FINAL_TYPE (HdyHeaderGroupChild, hdy_header_group_child, HDY, HEADER_GROUP_CHILD, GObject)

#define HDY_TYPE_HEADER_GROUP (hdy_header_group_get_type())

G_DECLARE_FINAL_TYPE (HdyHeaderGroup, hdy_header_group, HDY, HEADER_GROUP, GObject)

typedef enum {
  HDY_HEADER_GROUP_CHILD_TYPE_HEADER_BAR,
  HDY_HEADER_GROUP_CHILD_TYPE_GTK_HEADER_BAR,
  HDY_HEADER_GROUP_CHILD_TYPE_HEADER_GROUP,
} HdyHeaderGroupChildType;

HdyHeaderBar   *hdy_header_group_child_get_header_bar     (HdyHeaderGroupChild *self);
GtkHeaderBar   *hdy_header_group_child_get_gtk_header_bar (HdyHeaderGroupChild *self);
HdyHeaderGroup *hdy_header_group_child_get_header_group   (HdyHeaderGroupChild *self);

HdyHeaderGroupChildType hdy_header_group_child_get_child_type (HdyHeaderGroupChild *self);

HdyHeaderGroup *hdy_header_group_new (void);

void hdy_header_group_add_header_bar     (HdyHeaderGroup *self,
                                          HdyHeaderBar   *header_bar);
void hdy_header_group_add_gtk_header_bar (HdyHeaderGroup *self,
                                          GtkHeaderBar   *header_bar);
void hdy_header_group_add_header_group   (HdyHeaderGroup *self,
                                          HdyHeaderGroup *header_group);

GSList *hdy_header_group_get_children (HdyHeaderGroup *self);

void hdy_header_group_remove_header_bar     (HdyHeaderGroup *self,
                                             HdyHeaderBar   *header_bar);
void hdy_header_group_remove_gtk_header_bar (HdyHeaderGroup *self,
                                             GtkHeaderBar   *header_bar);
void hdy_header_group_remove_header_group   (HdyHeaderGroup *self,
                                             HdyHeaderGroup *header_group);
void hdy_header_group_remove_child          (HdyHeaderGroup      *self,
                                             HdyHeaderGroupChild *child);

gboolean hdy_header_group_get_decorate_all (HdyHeaderGroup *self);
void     hdy_header_group_set_decorate_all (HdyHeaderGroup *self,
                                            gboolean        decorate_all);

G_END_DECLS
