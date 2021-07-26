/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_APPLICATION_WINDOW (adw_application_window_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwApplicationWindow, adw_application_window, ADW, APPLICATION_WINDOW, GtkApplicationWindow)

struct _AdwApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_application_window_new (GtkApplication *app) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
void       adw_application_window_set_child (AdwApplicationWindow *self,
                                             GtkWidget            *child);
ADW_AVAILABLE_IN_ALL
GtkWidget *adw_application_window_get_child (AdwApplicationWindow *self);

G_END_DECLS
