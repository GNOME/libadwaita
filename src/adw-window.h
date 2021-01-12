/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_WINDOW (adw_window_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwWindow, adw_window, ADW, WINDOW, GtkWindow)

struct _AdwWindowClass
{
  GtkWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_window_new (void);

ADW_AVAILABLE_IN_ALL
void       adw_window_set_child (AdwWindow *self,
                                 GtkWidget *child);
ADW_AVAILABLE_IN_ALL
GtkWidget *adw_window_get_child (AdwWindow *self);

G_END_DECLS
