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

#define HDY_TYPE_KEYPAD (hdy_keypad_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyKeypad, hdy_keypad, HDY, KEYPAD, GtkBin)

/**
 * HdyKeypadClass:
 * @parent_class: The parent class
 */
struct _HdyKeypadClass
{
  GtkBinClass parent_class;
};

GtkWidget       *hdy_keypad_new                     (gboolean symbols_visible,
                                                     gboolean letters_visible);
void             hdy_keypad_set_row_spacing         (HdyKeypad *self,
                                                     guint      spacing);
guint            hdy_keypad_get_row_spacing         (HdyKeypad *self);
void             hdy_keypad_set_column_spacing      (HdyKeypad *self,
                                                     guint      spacing);
guint            hdy_keypad_get_column_spacing      (HdyKeypad *self);
void             hdy_keypad_set_letters_visible     (HdyKeypad *self,
                                                     gboolean   letters_visible);
gboolean         hdy_keypad_get_letters_visible     (HdyKeypad *self);
void             hdy_keypad_set_symbols_visible     (HdyKeypad *self,
                                                     gboolean   symbols_visible);
gboolean         hdy_keypad_get_symbols_visible     (HdyKeypad *self);
void             hdy_keypad_set_entry               (HdyKeypad *self,
                                                     GtkEntry  *entry);
GtkWidget       *hdy_keypad_get_entry               (HdyKeypad *self);
void             hdy_keypad_set_left_action         (HdyKeypad *self,
                                                     GtkWidget *widget);
void             hdy_keypad_set_right_action        (HdyKeypad *self,
                                                     GtkWidget *widget);


G_END_DECLS
