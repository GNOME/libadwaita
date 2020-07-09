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

G_BEGIN_DECLS

#define HDY_TYPE_HEADER_GROUP (hdy_header_group_get_type())

G_DECLARE_FINAL_TYPE (HdyHeaderGroup, hdy_header_group, HDY, HEADER_GROUP, GObject)

HdyHeaderGroup *hdy_header_group_new (void);

void hdy_header_group_add_header_bar (HdyHeaderGroup *self,
                                      GtkHeaderBar   *header_bar);

GSList *hdy_header_group_get_children (HdyHeaderGroup *self);

void     hdy_header_group_remove_header_bar (HdyHeaderGroup *self,
                                             GtkHeaderBar   *header_bar);

gboolean hdy_header_group_get_decorate_all (HdyHeaderGroup *self);
void     hdy_header_group_set_decorate_all (HdyHeaderGroup *self,
                                            gboolean        decorate_all);

G_END_DECLS
