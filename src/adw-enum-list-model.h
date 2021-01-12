/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib-object.h>
#include "adw-enum-value-object.h"

G_BEGIN_DECLS

#define ADW_TYPE_ENUM_LIST_MODEL (adw_enum_list_model_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwEnumListModel, adw_enum_list_model, ADW, ENUM_LIST_MODEL, GObject)

ADW_AVAILABLE_IN_ALL
AdwEnumListModel *adw_enum_list_model_new (GType enum_type);

ADW_AVAILABLE_IN_ALL
GType adw_enum_list_model_get_enum_type (AdwEnumListModel *self);

ADW_AVAILABLE_IN_ALL
guint adw_enum_list_model_find_position (AdwEnumListModel *self,
                                         gint              value);

G_END_DECLS
