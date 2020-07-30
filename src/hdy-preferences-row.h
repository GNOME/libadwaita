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

#define HDY_TYPE_PREFERENCES_ROW (hdy_preferences_row_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyPreferencesRow, hdy_preferences_row, HDY, PREFERENCES_ROW, GtkListBoxRow)

/**
 * HdyPreferencesRowClass
 * @parent_class: The parent class
 */
struct _HdyPreferencesRowClass
{
  GtkListBoxRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_preferences_row_new (void);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_preferences_row_get_title (HdyPreferencesRow *self);
HDY_AVAILABLE_IN_ALL
void         hdy_preferences_row_set_title (HdyPreferencesRow *self,
                                            const gchar       *title);

HDY_AVAILABLE_IN_ALL
gboolean hdy_preferences_row_get_use_underline (HdyPreferencesRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_preferences_row_set_use_underline (HdyPreferencesRow *self,
                                                gboolean           use_underline);

G_END_DECLS
