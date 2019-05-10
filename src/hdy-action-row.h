/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-preferences-row.h"

G_BEGIN_DECLS

#define HDY_TYPE_ACTION_ROW (hdy_action_row_get_type())

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
};

HdyActionRow *hdy_action_row_new (void);

const gchar *hdy_action_row_get_title (HdyActionRow *self);
void         hdy_action_row_set_title (HdyActionRow *self,
                                       const gchar  *title);

const gchar *hdy_action_row_get_subtitle (HdyActionRow *self);
void         hdy_action_row_set_subtitle (HdyActionRow *self,
                                          const gchar  *subtitle);

const gchar *hdy_action_row_get_icon_name (HdyActionRow *self);
void         hdy_action_row_set_icon_name (HdyActionRow *self,
                                           const gchar  *icon_name);

GtkWidget *hdy_action_row_get_activatable_widget (HdyActionRow *self);
void       hdy_action_row_set_activatable_widget (HdyActionRow *self,
                                                  GtkWidget    *widget);

gboolean hdy_action_row_get_use_underline (HdyActionRow *self);
void     hdy_action_row_set_use_underline (HdyActionRow *self,
                                           gboolean      use_underline);

void hdy_action_row_add_action (HdyActionRow *self,
                                GtkWidget    *widget);

void hdy_action_row_add_prefix (HdyActionRow *self,
                                GtkWidget    *widget);

void hdy_action_row_activate (HdyActionRow *self);

G_END_DECLS
