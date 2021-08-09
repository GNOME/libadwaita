/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.> See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

gboolean adw_widget_focus_child (GtkWidget        *widget,
                                 GtkDirectionType  direction);

gboolean adw_widget_grab_focus_self  (GtkWidget *widget);
gboolean adw_widget_grab_focus_child (GtkWidget *widget);

void adw_widget_compute_expand (GtkWidget *widget,
                                gboolean  *hexpand_p,
                                gboolean  *vexpand_p);

void adw_widget_compute_expand_horizontal_only (GtkWidget *widget,
                                                gboolean  *hexpand_p,
                                                gboolean  *vexpand_p);

G_END_DECLS
