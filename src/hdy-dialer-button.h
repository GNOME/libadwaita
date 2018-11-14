/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_DIALER_BUTTON (hdy_dialer_button_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDialerButton, hdy_dialer_button, HDY, DIALER_BUTTON, GtkButton)

struct _HdyDialerButtonClass
{
  GtkButtonClass parent_class;
};

GtkWidget      *hdy_dialer_button_new                   (const gchar     *symbols);
gint            hdy_dialer_button_get_digit             (HdyDialerButton *self);
const char     *hdy_dialer_button_get_symbols           (HdyDialerButton *self);

G_END_DECLS
