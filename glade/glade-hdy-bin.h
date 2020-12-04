/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>

void glade_hdy_bin_post_create (GladeWidgetAdaptor *adaptor,
                                GObject            *object,
                                GladeCreateReason   reason);

void glade_hdy_bin_add_child (GladeWidgetAdaptor *adaptor,
                              GObject            *object,
                              GObject            *child);
void glade_hdy_bin_remove_child (GladeWidgetAdaptor *adaptor,
                                 GObject            *object,
                                 GObject            *child);
void glade_hdy_bin_replace_child (GladeWidgetAdaptor *adaptor,
                                   GtkWidget          *object,
                                   GtkWidget          *current,
                                   GtkWidget          *new_widget);

GList *glade_hdy_bin_get_children (GladeWidgetAdaptor *adaptor,
                                   GObject            *object);

gboolean glade_hdy_bin_add_verify (GladeWidgetAdaptor *adaptor,
                                   GtkWidget          *object,
                                   GtkWidget          *child,
                                   gboolean            user_feedback);
