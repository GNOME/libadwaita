/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

gboolean hdy_get_enable_animations (GtkWidget *widget);

gdouble hdy_lerp (gdouble a, gdouble b, gdouble t);

gdouble hdy_ease_out_cubic (gdouble t);

G_END_DECLS
