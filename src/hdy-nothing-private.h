/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_NOTHING (hdy_nothing_get_type())

G_DECLARE_FINAL_TYPE (HdyNothing, hdy_nothing, HDY, NOTHING, GtkWidget)

GtkWidget *hdy_nothing_new (void);

G_END_DECLS
