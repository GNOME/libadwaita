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
#include "hdy-enum-value-object.h"
#include "hdy-action-row.h"

G_BEGIN_DECLS

#define HDY_TYPE_COMBO_ROW (hdy_combo_row_get_type())

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
};

HdyComboRow *hdy_combo_row_new (void);

GListModel *hdy_combo_row_get_model (HdyComboRow *self);

void hdy_combo_row_bind_model      (HdyComboRow                *self,
                                    GListModel                 *model,
                                    GtkListBoxCreateWidgetFunc  create_list_widget_func,
                                    GtkListBoxCreateWidgetFunc  create_current_widget_func,
                                    gpointer                    user_data,
                                    GDestroyNotify              user_data_free_func);
void hdy_combo_row_bind_name_model (HdyComboRow            *self,
                                    GListModel             *model,
                                    HdyComboRowGetNameFunc  get_name_func,
                                    gpointer                user_data,
                                    GDestroyNotify          user_data_free_func);
void hdy_combo_row_set_for_enum    (HdyComboRow                     *self,
                                    GType                            enum_type,
                                    HdyComboRowGetEnumValueNameFunc  get_name_func,
                                    gpointer                         user_data,
                                    GDestroyNotify                   user_data_free_func);

gint hdy_combo_row_get_selected_index (HdyComboRow *self);
void hdy_combo_row_set_selected_index (HdyComboRow *self,
                                       gint         selected_index);

gboolean hdy_combo_row_get_use_subtitle (HdyComboRow *self);
void     hdy_combo_row_set_use_subtitle (HdyComboRow *self,
                                         gboolean     use_subtitle);

void hdy_combo_row_set_get_name_func (HdyComboRow            *self,
                                      HdyComboRowGetNameFunc  get_name_func,
                                      gpointer                user_data,
                                      GDestroyNotify          user_data_free_func);

gchar *hdy_enum_value_row_name (HdyEnumValueObject *value,
                                gpointer            user_data);

G_END_DECLS
