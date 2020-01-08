/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>

void glade_hdy_expander_row_post_create (GladeWidgetAdaptor *adaptor,
                                         GObject            *container,
                                         GladeCreateReason   reason);

void glade_hdy_expander_row_get_child_property (GladeWidgetAdaptor *adaptor,
                                                GObject            *container,
                                                GObject            *child,
                                                const gchar        *property_name,
                                                GValue             *value);
void glade_hdy_expander_row_set_child_property (GladeWidgetAdaptor *adaptor,
                                                GObject            *container,
                                                GObject            *child,
                                                const gchar        *property_name,
                                                GValue             *value);

void glade_hdy_expander_row_add_child (GladeWidgetAdaptor *adaptor,
                                       GObject            *object,
                                       GObject            *child);

void glade_hdy_expander_row_remove_child (GladeWidgetAdaptor *adaptor,
                                          GObject            *object,
                                          GObject            *child);

gboolean glade_hdy_expander_row_add_verify (GladeWidgetAdaptor *adaptor,
                                            GtkWidget          *object,
                                            GtkWidget          *child,
                                            gboolean            user_feedback);
