/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gladeui/glade.h>

#define HANDY_USE_UNSTABLE_API
#include <handy.h>


void glade_hdy_paginator_post_create (GladeWidgetAdaptor *adaptor,
                                      GObject            *container,
                                      GladeCreateReason   reason);

void glade_hdy_paginator_child_action_activate (GladeWidgetAdaptor *adaptor,
                                                GObject            *container,
                                                GObject            *object,
                                                const gchar        *action_path);

void glade_hdy_paginator_set_property         (GladeWidgetAdaptor *adaptor,
                                              GObject            *object,
                                              const gchar        *id,
                                              const GValue       *value);
void glade_hdy_paginator_get_property        (GladeWidgetAdaptor *adaptor,
                                              GObject            *object,
                                              const gchar        *id,
                                              GValue             *value);
gboolean glade_hdy_paginator_verify_property (GladeWidgetAdaptor *adaptor,
                                              GObject            *object,
                                              const gchar        *id,
                                              const GValue       *value);

void glade_hdy_paginator_add_child     (GladeWidgetAdaptor *adaptor,
                                        GObject            *container,
                                        GObject            *child);
void glade_hdy_paginator_remove_child  (GladeWidgetAdaptor *adaptor,
                                        GObject            *container,
                                        GObject            *child);
void glade_hdy_paginator_replace_child (GladeWidgetAdaptor *adaptor,
                                        GObject            *container,
                                        GObject            *current,
                                        GObject            *new_widget);
