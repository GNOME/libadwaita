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

#include "hdy-preferences-row.h"

G_BEGIN_DECLS

#define HDY_TYPE_ACTION_ROW (hdy_action_row_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyActionRow, hdy_action_row, HDY, ACTION_ROW, HdyPreferencesRow)

/**
 * HdyActionRowClass
 * @parent_class: The parent class
 * @activate: Activates the row to trigger its main action.
 */
struct _HdyActionRowClass
{
  GtkListBoxRowClass parent_class;

  void (*activate) (HdyActionRow *self);

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_action_row_new (void);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_action_row_get_subtitle (HdyActionRow *self);
HDY_AVAILABLE_IN_ALL
void         hdy_action_row_set_subtitle (HdyActionRow *self,
                                          const gchar  *subtitle);

HDY_AVAILABLE_IN_ALL
const gchar *hdy_action_row_get_icon_name (HdyActionRow *self);
HDY_AVAILABLE_IN_ALL
void         hdy_action_row_set_icon_name (HdyActionRow *self,
                                           const gchar  *icon_name);

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_action_row_get_activatable_widget (HdyActionRow *self);
HDY_AVAILABLE_IN_ALL
void       hdy_action_row_set_activatable_widget (HdyActionRow *self,
                                                  GtkWidget    *widget);

HDY_AVAILABLE_IN_ALL
gboolean hdy_action_row_get_use_underline (HdyActionRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_action_row_set_use_underline (HdyActionRow *self,
                                           gboolean      use_underline);

HDY_AVAILABLE_IN_1_1
gint hdy_action_row_get_title_lines (HdyActionRow *self);
HDY_AVAILABLE_IN_1_1
void hdy_action_row_set_title_lines (HdyActionRow *self,
                                     gint          title_lines);

HDY_AVAILABLE_IN_1_1
gint hdy_action_row_get_subtitle_lines (HdyActionRow *self);
HDY_AVAILABLE_IN_1_1
void hdy_action_row_set_subtitle_lines (HdyActionRow *self,
                                        gint          subtitle_lines);

HDY_AVAILABLE_IN_ALL
void hdy_action_row_add_prefix (HdyActionRow *self,
                                GtkWidget    *widget);

HDY_AVAILABLE_IN_ALL
void hdy_action_row_activate (HdyActionRow *self);

G_END_DECLS
