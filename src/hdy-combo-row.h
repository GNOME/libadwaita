/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-enum-value-object.h"
#include "hdy-action-row.h"

G_BEGIN_DECLS

#define HDY_TYPE_COMBO_ROW (hdy_combo_row_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyComboRow, hdy_combo_row, HDY, COMBO_ROW, HdyActionRow)

/**
 * HdyComboRowGetNameFunc:
 * @item: (type GObject): the item from the model from which to get a name
 * @user_data: (closure): user data
 *
 * Called for combo rows that are bound to a #GListModel with
 * hdy_combo_row_bind_name_model() for each item that gets added to the model.
 *
 * Returns: (transfer full): a newly allocated displayable name that represents @item
 */
typedef gchar * (*HdyComboRowGetNameFunc) (gpointer item,
                                           gpointer user_data);

/**
 * HdyComboRowGetEnumValueNameFunc:
 * @value: the value from the enum from which to get a name
 * @user_data: (closure): user data
 *
 * Called for combo rows that are bound to an enumeration with
 * hdy_combo_row_set_for_enum() for each value from that enumeration.
 *
 * Returns: (transfer full): a newly allocated displayable name that represents @value
 */
typedef gchar * (*HdyComboRowGetEnumValueNameFunc) (HdyEnumValueObject *value,
                                                    gpointer            user_data);

/**
 * HdyComboRowClass
 * @parent_class: The parent class
 */
struct _HdyComboRowClass
{
  HdyActionRowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_combo_row_new (void);

HDY_AVAILABLE_IN_ALL
GListModel *hdy_combo_row_get_model (HdyComboRow *self);
HDY_AVAILABLE_IN_ALL
void        hdy_combo_row_set_model (HdyComboRow *self,
                                     GListModel  *model);

HDY_AVAILABLE_IN_ALL
void  hdy_combo_row_set_selected (HdyComboRow *self,
                                  guint        position);
HDY_AVAILABLE_IN_ALL
guint hdy_combo_row_get_selected (HdyComboRow *self);

HDY_AVAILABLE_IN_ALL
gpointer hdy_combo_row_get_selected_item (HdyComboRow *self);

HDY_AVAILABLE_IN_ALL
GtkListItemFactory *hdy_combo_row_get_factory (HdyComboRow        *self);
HDY_AVAILABLE_IN_ALL
void                hdy_combo_row_set_factory (HdyComboRow        *self,
                                               GtkListItemFactory *factory);

HDY_AVAILABLE_IN_ALL
GtkListItemFactory *hdy_combo_row_get_list_factory (HdyComboRow        *self);
HDY_AVAILABLE_IN_ALL
void                hdy_combo_row_set_list_factory (HdyComboRow        *self,
                                                    GtkListItemFactory *factory);

HDY_AVAILABLE_IN_ALL
GtkExpression *hdy_combo_row_get_expression (HdyComboRow   *self);
HDY_AVAILABLE_IN_ALL
void           hdy_combo_row_set_expression (HdyComboRow   *self,
                                             GtkExpression *expression);

HDY_AVAILABLE_IN_ALL
gboolean hdy_combo_row_get_use_subtitle (HdyComboRow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_combo_row_set_use_subtitle (HdyComboRow *self,
                                         gboolean     use_subtitle);

G_END_DECLS
