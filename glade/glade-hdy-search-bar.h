/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>

void glade_hdy_search_bar_post_create (GladeWidgetAdaptor *adaptor,
                                       GObject            *widget,
                                       GladeCreateReason   reason);

void glade_hdy_search_bar_add_child (GladeWidgetAdaptor *adaptor,
                                     GObject            *object,
                                     GObject            *child);
void glade_hdy_search_bar_remove_child (GladeWidgetAdaptor *adaptor,
                                        GObject            *object,
                                        GObject            *child);
void glade_hdy_search_bar_replace_child (GladeWidgetAdaptor *adaptor,
                                         GtkWidget          *container,
                                         GtkWidget          *current,
                                         GtkWidget          *new_widget);

GList *glade_hdy_search_bar_get_children (GladeWidgetAdaptor *adaptor,
                                          GObject            *widget);

gboolean glade_hdy_search_bar_add_verify (GladeWidgetAdaptor *adaptor,
                                          GtkWidget          *container,
                                          GtkWidget          *child,
                                          gboolean            user_feedback);
