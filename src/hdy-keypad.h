/*
 * Copyright (C) 2019 Purism SPC
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

#define HDY_TYPE_KEYPAD (hdy_keypad_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyKeypad, hdy_keypad, HDY, KEYPAD, GtkBin)

/**
 * HdyKeypadClass:
 * @parent_class: The parent class
 */
struct _HdyKeypadClass
{
  GtkBinClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_keypad_new                     (gboolean symbols_visible,
                                                     gboolean letters_visible);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_row_spacing         (HdyKeypad *self,
                                                     guint      spacing);
HDY_AVAILABLE_IN_ALL
guint            hdy_keypad_get_row_spacing         (HdyKeypad *self);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_column_spacing      (HdyKeypad *self,
                                                     guint      spacing);
HDY_AVAILABLE_IN_ALL
guint            hdy_keypad_get_column_spacing      (HdyKeypad *self);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_letters_visible     (HdyKeypad *self,
                                                     gboolean   letters_visible);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_keypad_get_letters_visible     (HdyKeypad *self);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_symbols_visible     (HdyKeypad *self,
                                                     gboolean   symbols_visible);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_keypad_get_symbols_visible     (HdyKeypad *self);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_entry               (HdyKeypad *self,
                                                     GtkEntry  *entry);
HDY_AVAILABLE_IN_ALL
GtkEntry        *hdy_keypad_get_entry               (HdyKeypad *self);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_start_action        (HdyKeypad *self,
                                                     GtkWidget *start_action);
HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_keypad_get_start_action        (HdyKeypad *self);
HDY_AVAILABLE_IN_ALL
void             hdy_keypad_set_end_action          (HdyKeypad *self,
                                                     GtkWidget *end_action);
HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_keypad_get_end_action          (HdyKeypad *self);


G_END_DECLS
