/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define ADW_TYPE_ENUM_VALUE_OBJECT (adw_enum_value_object_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwEnumValueObject, adw_enum_value_object, ADW, ENUM_VALUE_OBJECT, GObject)

ADW_AVAILABLE_IN_ALL
int         adw_enum_value_object_get_value (AdwEnumValueObject *self);
ADW_AVAILABLE_IN_ALL
const char *adw_enum_value_object_get_name  (AdwEnumValueObject *self);
ADW_AVAILABLE_IN_ALL
const char *adw_enum_value_object_get_nick  (AdwEnumValueObject *self);

G_END_DECLS
