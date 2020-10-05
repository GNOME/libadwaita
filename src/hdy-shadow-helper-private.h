/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_SHADOW_HELPER (hdy_shadow_helper_get_type())

G_DECLARE_FINAL_TYPE (HdyShadowHelper, hdy_shadow_helper, HDY, SHADOW_HELPER, GObject)

HdyShadowHelper *hdy_shadow_helper_new (GtkWidget *widget);

void             hdy_shadow_helper_clear (HdyShadowHelper *self);

void             hdy_shadow_helper_size_allocate (HdyShadowHelper *self,
                                                  gint             width,
                                                  gint             height,
                                                  gint             baseline,
                                                  gint             x,
                                                  gint             y,
                                                  gdouble          progress,
                                                  GtkPanDirection  direction);

void             hdy_shadow_helper_snapshot (HdyShadowHelper *self,
                                             GtkSnapshot     *snapshot);

G_END_DECLS
