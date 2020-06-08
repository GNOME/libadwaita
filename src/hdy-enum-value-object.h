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

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HDY_TYPE_ENUM_VALUE_OBJECT (hdy_enum_value_object_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyEnumValueObject, hdy_enum_value_object, HDY, ENUM_VALUE_OBJECT, GObject)

HDY_AVAILABLE_IN_ALL
HdyEnumValueObject *hdy_enum_value_object_new (GEnumValue *enum_value);

HDY_AVAILABLE_IN_ALL
gint         hdy_enum_value_object_get_value (HdyEnumValueObject *self);
HDY_AVAILABLE_IN_ALL
const gchar *hdy_enum_value_object_get_name  (HdyEnumValueObject *self);
HDY_AVAILABLE_IN_ALL
const gchar *hdy_enum_value_object_get_nick  (HdyEnumValueObject *self);

G_END_DECLS
