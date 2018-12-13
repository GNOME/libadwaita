/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HDY_TYPE_ENUM_VALUE_OBJECT (hdy_enum_value_object_get_type())

G_DECLARE_FINAL_TYPE (HdyEnumValueObject, hdy_enum_value_object, HDY, ENUM_VALUE_OBJECT, GObject)

HdyEnumValueObject *hdy_enum_value_object_new (GEnumValue *enum_value);

gint         hdy_enum_value_object_get_value (HdyEnumValueObject *self);
const gchar *hdy_enum_value_object_get_name  (HdyEnumValueObject *self);
const gchar *hdy_enum_value_object_get_nick  (HdyEnumValueObject *self);

G_END_DECLS
