/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

#define ONLY_THIS_GOES_IN_THAT_MSG _("Only objects of type %s can be added to objects of type %s.")


void glade_hdy_sync_child_positions (GtkContainer *container);

gint glade_hdy_get_child_index (GtkContainer *container,
                                GtkWidget    *child);

void glade_hdy_reorder_child (GtkContainer *container,
                              GtkWidget    *child,
                              gint          index);

GtkWidget *glade_hdy_get_nth_child (GtkContainer *container,
                                    gint          n);
