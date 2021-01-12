/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_KEYPAD_BUTTON (adw_keypad_button_get_type())

G_DECLARE_FINAL_TYPE (AdwKeypadButton, adw_keypad_button, ADW, KEYPAD_BUTTON, GtkButton)

struct _AdwKeypadButtonClass
{
  GtkButtonClass parent_class;
};

GtkWidget   *adw_keypad_button_new                   (const gchar     *symbols);
gchar        adw_keypad_button_get_digit             (AdwKeypadButton *self);
const gchar *adw_keypad_button_get_symbols           (AdwKeypadButton *self);
void         adw_keypad_button_show_symbols          (AdwKeypadButton *self,
                                                      gboolean         visible);

G_END_DECLS
