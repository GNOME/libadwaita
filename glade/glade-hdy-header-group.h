/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#include <handy.h>


void glade_hdy_header_group_set_property (GladeWidgetAdaptor *adaptor,
                                          GObject            *object,
                                          const gchar        *property_name,
                                          const GValue       *value);
void glade_hdy_header_group_write_widget (GladeWidgetAdaptor *adaptor,
                                          GladeWidget        *widget,
                                          GladeXmlContext    *context,
                                          GladeXmlNode       *node);
void glade_hdy_header_group_read_widget  (GladeWidgetAdaptor *adaptor,
                                          GladeWidget        *widget,
                                          GladeXmlNode       *node);
