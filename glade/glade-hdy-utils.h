/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>

#define ONLY_THIS_GOES_IN_THAT_MSG _("Only objects of type %s can be added to objects of type %s.")

/* Guess whether we are using a Glade version older than 3.36.
 *
 * If yes, redefine some symbols which got renamed.
 */
#ifndef GLADE_PROPERTY_DEF_OBJECT_DELIMITER
#define GLADE_PROPERTY_DEF_OBJECT_DELIMITER GPC_OBJECT_DELIMITER
#define glade_widget_action_get_def glade_widget_action_get_class
#endif

void glade_hdy_init (const gchar *name);

void glade_hdy_sync_child_positions (GtkContainer *container);

gint glade_hdy_get_child_index (GtkContainer *container,
                                GtkWidget    *child);

void glade_hdy_reorder_child (GtkContainer *container,
                              GtkWidget    *child,
                              gint          index);

GtkWidget *glade_hdy_get_nth_child (GtkContainer *container,
                                    gint          n);
