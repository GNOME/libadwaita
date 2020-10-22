/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <glib-object.h>
#include "hdy-enum-value-object.h"

G_BEGIN_DECLS

#define HDY_TYPE_ENUM_LIST_MODEL (hdy_enum_list_model_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyEnumListModel, hdy_enum_list_model, HDY, ENUM_LIST_MODEL, GObject)

HDY_AVAILABLE_IN_ALL
HdyEnumListModel *hdy_enum_list_model_new (GType enum_type);

HDY_AVAILABLE_IN_ALL
GType hdy_enum_list_model_get_enum_type (HdyEnumListModel *self);

HDY_AVAILABLE_IN_ALL
guint hdy_enum_list_model_find_position (HdyEnumListModel *self,
                                         gint              value);

G_END_DECLS
