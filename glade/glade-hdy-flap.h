/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>


void glade_hdy_flap_post_create (GladeWidgetAdaptor *adaptor,
                                 GObject            *container,
                                 GladeCreateReason   reason);

void glade_hdy_flap_add_child (GladeWidgetAdaptor *adaptor,
                               GObject            *parent,
                               GObject            *child);
void glade_hdy_flap_remove_child (GladeWidgetAdaptor *adaptor,
                                  GObject            *object,
                                  GObject            *child);
void glade_hdy_flap_replace_child (GladeWidgetAdaptor *adaptor,
                                   GObject            *container,
                                   GObject            *current,
                                   GObject            *new_widget);
