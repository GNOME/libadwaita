/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 *
 * Based on
 * hdy-header-group.h - GladeWidgetAdaptor for HdyHeaderGroup
 * Copyright (C) 2018 Purism SPC
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>


void glade_hdy_swipe_group_set_property (GladeWidgetAdaptor *adaptor,
                                         GObject            *object,
                                         const gchar        *property_name,
                                         const GValue       *value);
void glade_hdy_swipe_group_write_widget (GladeWidgetAdaptor *adaptor,
                                          GladeWidget        *widget,
                                         GladeXmlContext    *context,
                                         GladeXmlNode       *node);
void glade_hdy_swipe_group_read_widget  (GladeWidgetAdaptor *adaptor,
                                         GladeWidget        *widget,
                                         GladeXmlNode       *node);
