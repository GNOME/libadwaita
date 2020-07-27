/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-preferences-row.h"

G_BEGIN_DECLS

#define HDY_TYPE_EXPANDER_ROW (hdy_expander_row_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyExpanderRow, hdy_expander_row, HDY, EXPANDER_ROW, HdyPreferencesRow)

/**
 * HdyExpanderRowClass
 * @parent_class: The parent class
 */
struct _HdyExpanderRowClass
{
  HdyPreferencesRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_expander_row_new (void);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_expander_row_get_subtitle (HdyExpanderRow *self);
HDY_AVAILABLE_IN_ALL
void         hdy_expander_row_set_subtitle (HdyExpanderRow *self,
                                            const gchar    *subtitle);

HDY_AVAILABLE_IN_ALL
gboolean hdy_expander_row_get_use_underline (HdyExpanderRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_expander_row_set_use_underline (HdyExpanderRow *self,
                                             gboolean        use_underline);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_expander_row_get_icon_name (HdyExpanderRow *self);
HDY_AVAILABLE_IN_ALL
void         hdy_expander_row_set_icon_name (HdyExpanderRow *self,
                                             const gchar    *icon_name);

HDY_AVAILABLE_IN_ALL
gboolean hdy_expander_row_get_expanded (HdyExpanderRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_expander_row_set_expanded (HdyExpanderRow *self,
                                        gboolean        expanded);

HDY_AVAILABLE_IN_ALL
gboolean hdy_expander_row_get_enable_expansion (HdyExpanderRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_expander_row_set_enable_expansion (HdyExpanderRow *self,
                                                gboolean        enable_expansion);

HDY_AVAILABLE_IN_ALL
gboolean hdy_expander_row_get_show_enable_switch (HdyExpanderRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_expander_row_set_show_enable_switch (HdyExpanderRow *self,
                                                  gboolean        show_enable_switch);

HDY_AVAILABLE_IN_ALL
void     hdy_expander_row_add_action (HdyExpanderRow *self,
                                      GtkWidget      *widget);
HDY_AVAILABLE_IN_ALL
void     hdy_expander_row_add_prefix (HdyExpanderRow *self,
                                      GtkWidget      *widget);

G_END_DECLS
