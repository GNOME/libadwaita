/*
 * Copyright (c) 2013 Red Hat, Inc.
 * Copyright (C) 2019 Purism SPC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_HEADER_BAR (hdy_header_bar_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyHeaderBar, hdy_header_bar, HDY, HEADER_BAR, GtkContainer)

typedef enum {
  HDY_CENTERING_POLICY_LOOSE,
  HDY_CENTERING_POLICY_STRICT,
} HdyCenteringPolicy;

/**
 * HdyHeaderBarClass
 * @parent_class: The parent class
 */
struct _HdyHeaderBarClass
{
  GtkContainerClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_header_bar_new (void);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_header_bar_get_title (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_set_title (HdyHeaderBar *self,
                                       const gchar  *title);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_header_bar_get_subtitle      (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_set_subtitle      (HdyHeaderBar *self,
                                               const gchar  *subtitle);

HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_header_bar_get_custom_title  (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_set_custom_title  (HdyHeaderBar *self,
                                               GtkWidget    *title_widget);

HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_pack_start        (HdyHeaderBar *self,
                                               GtkWidget    *child);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_pack_end          (HdyHeaderBar *self,
                                               GtkWidget    *child);

HDY_AVAILABLE_IN_ALL
gboolean     hdy_header_bar_get_show_close_button (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_set_show_close_button (HdyHeaderBar *self,
                                                   gboolean      setting);

HDY_AVAILABLE_IN_ALL
gboolean     hdy_header_bar_get_has_subtitle (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_set_has_subtitle (HdyHeaderBar *self,
                                              gboolean      setting);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_header_bar_get_decoration_layout (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void         hdy_header_bar_set_decoration_layout (HdyHeaderBar *self,
                                                   const gchar  *layout);

HDY_AVAILABLE_IN_ALL
HdyCenteringPolicy hdy_header_bar_get_centering_policy (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void               hdy_header_bar_set_centering_policy (HdyHeaderBar       *self,
                                                        HdyCenteringPolicy  centering_policy);

HDY_AVAILABLE_IN_ALL
guint hdy_header_bar_get_transition_duration (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void  hdy_header_bar_set_transition_duration (HdyHeaderBar *self,
                                              guint         duration);

HDY_AVAILABLE_IN_ALL
gboolean hdy_header_bar_get_transition_running (HdyHeaderBar *self);

HDY_AVAILABLE_IN_ALL
gboolean hdy_header_bar_get_interpolate_size (HdyHeaderBar *self);
HDY_AVAILABLE_IN_ALL
void     hdy_header_bar_set_interpolate_size (HdyHeaderBar *self,
                                              gboolean      interpolate_size);

G_END_DECLS
