/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-header-bar.h"

G_BEGIN_DECLS

#define ADW_TYPE_HEADER_GROUP_CHILD (adw_header_group_child_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwHeaderGroupChild, adw_header_group_child, ADW, HEADER_GROUP_CHILD, GObject)

#define ADW_TYPE_HEADER_GROUP (adw_header_group_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwHeaderGroup, adw_header_group, ADW, HEADER_GROUP, GObject)

typedef enum {
  ADW_HEADER_GROUP_CHILD_TYPE_HEADER_BAR,
  ADW_HEADER_GROUP_CHILD_TYPE_GTK_HEADER_BAR,
  ADW_HEADER_GROUP_CHILD_TYPE_HEADER_GROUP,
} AdwHeaderGroupChildType;

ADW_AVAILABLE_IN_ALL
AdwHeaderBar   *adw_header_group_child_get_header_bar     (AdwHeaderGroupChild *self);
ADW_AVAILABLE_IN_ALL
GtkHeaderBar   *adw_header_group_child_get_gtk_header_bar (AdwHeaderGroupChild *self);
ADW_AVAILABLE_IN_ALL
AdwHeaderGroup *adw_header_group_child_get_header_group   (AdwHeaderGroupChild *self);

ADW_AVAILABLE_IN_ALL
AdwHeaderGroupChildType adw_header_group_child_get_child_type (AdwHeaderGroupChild *self);

ADW_AVAILABLE_IN_ALL
AdwHeaderGroup *adw_header_group_new (void);

ADW_AVAILABLE_IN_ALL
void adw_header_group_add_header_bar     (AdwHeaderGroup *self,
                                          AdwHeaderBar   *header_bar);
ADW_AVAILABLE_IN_ALL
void adw_header_group_add_gtk_header_bar (AdwHeaderGroup *self,
                                          GtkHeaderBar   *header_bar);
ADW_AVAILABLE_IN_ALL
void adw_header_group_add_header_group   (AdwHeaderGroup *self,
                                          AdwHeaderGroup *header_group);

ADW_AVAILABLE_IN_ALL
GSList *adw_header_group_get_children (AdwHeaderGroup *self);

ADW_AVAILABLE_IN_ALL
void adw_header_group_remove_header_bar     (AdwHeaderGroup *self,
                                             AdwHeaderBar   *header_bar);
ADW_AVAILABLE_IN_ALL
void adw_header_group_remove_gtk_header_bar (AdwHeaderGroup *self,
                                             GtkHeaderBar   *header_bar);
ADW_AVAILABLE_IN_ALL
void adw_header_group_remove_header_group   (AdwHeaderGroup *self,
                                             AdwHeaderGroup *header_group);
ADW_AVAILABLE_IN_ALL
void adw_header_group_remove_child          (AdwHeaderGroup      *self,
                                             AdwHeaderGroupChild *child);

ADW_AVAILABLE_IN_ALL
gboolean adw_header_group_get_decorate_all (AdwHeaderGroup *self);
ADW_AVAILABLE_IN_ALL
void     adw_header_group_set_decorate_all (AdwHeaderGroup *self,
                                            gboolean        decorate_all);

G_END_DECLS
