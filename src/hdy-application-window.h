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

#define HDY_TYPE_APPLICATION_WINDOW (hdy_application_window_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyApplicationWindow, hdy_application_window, HDY, APPLICATION_WINDOW, GtkApplicationWindow)

struct _HdyApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;
};

GtkWidget *hdy_application_window_new (void);

G_END_DECLS
