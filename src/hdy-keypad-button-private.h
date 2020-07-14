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

#define HDY_TYPE_KEYPAD_BUTTON (hdy_keypad_button_get_type())

G_DECLARE_FINAL_TYPE (HdyKeypadButton, hdy_keypad_button, HDY, KEYPAD_BUTTON, GtkButton)

struct _HdyKeypadButtonClass
{
  GtkButtonClass parent_class;
};

GtkWidget   *hdy_keypad_button_new                   (const gchar     *symbols);
gchar        hdy_keypad_button_get_digit             (HdyKeypadButton *self);
const gchar *hdy_keypad_button_get_symbols           (HdyKeypadButton *self);
void         hdy_keypad_button_show_symbols          (HdyKeypadButton *self,
                                                      gboolean         visible);

G_END_DECLS
