/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "hdy-action-row.h"

G_BEGIN_DECLS

#define HDY_TYPE_EXPANDER_ROW (hdy_expander_row_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyExpanderRow, hdy_expander_row, HDY, EXPANDER_ROW, HdyActionRow)

/**
 * HdyExpanderRowClass
 * @parent_class: The parent class
 */
struct _HdyExpanderRowClass
{
  HdyActionRowClass parent_class;
};

HdyExpanderRow *hdy_expander_row_new (void);

gboolean hdy_expander_row_get_expanded (HdyExpanderRow *self);
void     hdy_expander_row_set_expanded (HdyExpanderRow *self,
                                        gboolean        expanded);

gboolean hdy_expander_row_get_enable_expansion (HdyExpanderRow *self);
void     hdy_expander_row_set_enable_expansion (HdyExpanderRow *self,
                                                gboolean        enable_expansion);

gboolean hdy_expander_row_get_show_enable_switch (HdyExpanderRow *self);
void     hdy_expander_row_set_show_enable_switch (HdyExpanderRow *self,
                                                  gboolean        show_enable_switch);

G_END_DECLS
