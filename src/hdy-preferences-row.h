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

#define HDY_TYPE_PREFERENCES_ROW (hdy_preferences_row_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyPreferencesRow, hdy_preferences_row, HDY, PREFERENCES_ROW, GtkListBoxRow)

/**
 * HdyPreferencesRowClass
 * @parent_class: The parent class
 */
struct _HdyPreferencesRowClass
{
  GtkListBoxRowClass parent_class;
};

HdyPreferencesRow *hdy_preferences_row_new (void);

const gchar *hdy_preferences_row_get_title (HdyPreferencesRow *self);
void         hdy_preferences_row_set_title (HdyPreferencesRow *self,
                                            const gchar       *title);

gboolean hdy_preferences_row_get_use_underline (HdyPreferencesRow *self);
void     hdy_preferences_row_set_use_underline (HdyPreferencesRow *self,
                                                gboolean           use_underline);

G_END_DECLS
