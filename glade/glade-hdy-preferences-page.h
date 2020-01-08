/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>

void glade_hdy_preferences_page_post_create (GladeWidgetAdaptor *adaptor,
                                             GObject            *container,
                                             GladeCreateReason   reason);

gboolean glade_hdy_preferences_page_add_verify (GladeWidgetAdaptor *adaptor,
                                                GtkWidget          *object,
                                                GtkWidget          *child,
                                                gboolean            user_feedback);

void glade_hdy_preferences_page_add_child (GladeWidgetAdaptor *adaptor,
                                           GObject            *container,
                                           GObject            *child);
void glade_hdy_preferences_page_remove_child (GladeWidgetAdaptor *adaptor,
                                              GObject            *container,
                                              GObject            *child);
void glade_hdy_preferences_page_replace_child (GladeWidgetAdaptor *adaptor,
                                               GObject            *container,
                                               GObject            *current,
                                               GObject            *new_widget);

GList *glade_hdy_preferences_page_get_children (GladeWidgetAdaptor *adaptor,
                                                GObject            *object);

void glade_hdy_preferences_page_action_activate (GladeWidgetAdaptor *adaptor,
                                                 GObject            *object,
                                                 const gchar        *action_path);

void glade_hdy_preferences_page_child_set_property (GladeWidgetAdaptor *adaptor,
                                                    GObject            *container,
                                                    GObject            *child,
                                                    const gchar        *property_name,
                                                    const GValue       *value);

void glade_hdy_preferences_page_child_get_property (GladeWidgetAdaptor *adaptor,
                                                    GObject            *container,
                                                    GObject            *child,
                                                    const gchar        *property_name,
                                                    GValue             *value);
