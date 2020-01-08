/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>


void glade_hdy_leaflet_post_create (GladeWidgetAdaptor *adaptor,
                                    GObject            *container,
                                    GladeCreateReason   reason);

void glade_hdy_leaflet_child_action_activate (GladeWidgetAdaptor *adaptor,
                                              GObject            *container,
                                              GObject            *object,
                                              const gchar        *action_path);

void glade_hdy_leaflet_set_property (GladeWidgetAdaptor *adaptor,
                                     GObject            *object,
                                     const gchar        *id,
                                     const GValue       *value);
void glade_hdy_leaflet_get_property (GladeWidgetAdaptor *adaptor,
                                     GObject            *object,
                                     const gchar        *id,
                                     GValue             *value);
gboolean glade_hdy_leaflet_verify_property (GladeWidgetAdaptor *adaptor,
                                            GObject            *object,
                                            const gchar        *id,
                                            const GValue       *value);

void glade_hdy_leaflet_get_child_property (GladeWidgetAdaptor *adaptor,
                                           GObject            *container,
                                           GObject            *child,
                                           const gchar        *property_name,
                                           GValue             *value);
void glade_hdy_leaflet_set_child_property (GladeWidgetAdaptor *adaptor,
                                           GObject            *container,
                                           GObject            *child,
                                           const gchar        *property_name,
                                           GValue             *value);

void glade_hdy_leaflet_add_child (GladeWidgetAdaptor *adaptor,
                                  GObject            *container,
                                  GObject            *child);
void glade_hdy_leaflet_remove_child (GladeWidgetAdaptor *adaptor,
                                     GObject            *container,
                                     GObject            *child);
void glade_hdy_leaflet_replace_child (GladeWidgetAdaptor *adaptor,
                                      GObject            *container,
                                      GObject            *current,
                                      GObject            *new_widget);
