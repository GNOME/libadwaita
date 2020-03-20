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

#define HDY_TYPE_WINDOW_HANDLE_CONTROLLER (hdy_window_handle_controller_get_type())

G_DECLARE_FINAL_TYPE (HdyWindowHandleController, hdy_window_handle_controller, HDY, WINDOW_HANDLE_CONTROLLER, GObject)

HdyWindowHandleController *hdy_window_handle_controller_new (GtkWidget *widget);

G_END_DECLS
