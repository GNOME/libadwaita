/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_WINDOW_HANDLE (hdy_window_handle_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyWindowHandle, hdy_window_handle, HDY, WINDOW_HANDLE, GtkEventBox)

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_window_handle_new (void);

G_END_DECLS
