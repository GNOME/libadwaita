/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>


void glade_hdy_header_bar_post_create (GladeWidgetAdaptor *adaptor,
                                       GObject            *container,
                                       GladeCreateReason   reason);

void glade_hdy_header_bar_action_activate (GladeWidgetAdaptor *adaptor,
                                           GObject            *object,
                                           const gchar        *action_path);

void glade_hdy_header_bar_child_action_activate (GladeWidgetAdaptor *adaptor,
                                                 GObject            *container,
                                                 GObject            *object,
                                                 const gchar        *action_path);

void glade_hdy_header_bar_get_property (GladeWidgetAdaptor *adaptor,
                                        GObject            *object,
                                        const gchar        *id,
                                        GValue             *value);
void glade_hdy_header_bar_set_property (GladeWidgetAdaptor *adaptor,
                                        GObject            *object,
                                        const gchar        *id,
                                        const GValue       *value);

void glade_hdy_header_bar_add_child (GladeWidgetAdaptor *adaptor,
                                     GObject            *parent,
                                     GObject            *child);
void glade_hdy_header_bar_remove_child (GladeWidgetAdaptor *adaptor,
                                        GObject            *object,
                                        GObject            *child);
void glade_hdy_header_bar_replace_child (GladeWidgetAdaptor *adaptor,
                                         GObject            *container,
                                         GObject            *current,
                                         GObject            *new_widget);

gboolean glade_hdy_header_bar_verify_property (GladeWidgetAdaptor *adaptor,
                                               GObject            *object,
                                               const gchar        *id,
                                               const GValue       *value);

GList *glade_hdy_header_bar_get_children (GladeWidgetAdaptor *adaptor,
                                          GObject            *container);

void glade_hdy_header_bar_child_set_property (GladeWidgetAdaptor *adaptor,
                                              GObject            *container,
                                              GObject            *child,
                                              const gchar        *property_name,
                                              const GValue       *value);
