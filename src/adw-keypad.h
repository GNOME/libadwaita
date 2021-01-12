/*
 * Copyright (C) 2019 Purism SPC
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

#define ADW_TYPE_KEYPAD (adw_keypad_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwKeypad, adw_keypad, ADW, KEYPAD, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget       *adw_keypad_new                     (gboolean symbols_visible,
                                                     gboolean letters_visible);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_row_spacing         (AdwKeypad *self,
                                                     guint      spacing);
ADW_AVAILABLE_IN_ALL
guint            adw_keypad_get_row_spacing         (AdwKeypad *self);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_column_spacing      (AdwKeypad *self,
                                                     guint      spacing);
ADW_AVAILABLE_IN_ALL
guint            adw_keypad_get_column_spacing      (AdwKeypad *self);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_letters_visible     (AdwKeypad *self,
                                                     gboolean   letters_visible);
ADW_AVAILABLE_IN_ALL
gboolean         adw_keypad_get_letters_visible     (AdwKeypad *self);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_symbols_visible     (AdwKeypad *self,
                                                     gboolean   symbols_visible);
ADW_AVAILABLE_IN_ALL
gboolean         adw_keypad_get_symbols_visible     (AdwKeypad *self);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_entry               (AdwKeypad *self,
                                                     GtkEntry  *entry);
ADW_AVAILABLE_IN_ALL
GtkEntry        *adw_keypad_get_entry               (AdwKeypad *self);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_start_action        (AdwKeypad *self,
                                                     GtkWidget *start_action);
ADW_AVAILABLE_IN_ALL
GtkWidget       *adw_keypad_get_start_action        (AdwKeypad *self);
ADW_AVAILABLE_IN_ALL
void             adw_keypad_set_end_action          (AdwKeypad *self,
                                                     GtkWidget *end_action);
ADW_AVAILABLE_IN_ALL
GtkWidget       *adw_keypad_get_end_action          (AdwKeypad *self);


G_END_DECLS
