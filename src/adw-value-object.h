/*
 * Copyright (C) 2019 Red Hat Inc.
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

#define ADW_TYPE_VALUE_OBJECT (adw_value_object_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwValueObject, adw_value_object, ADW, VALUE_OBJECT, GObject)

ADW_AVAILABLE_IN_ALL
AdwValueObject *adw_value_object_new             (const GValue *value);
ADW_AVAILABLE_IN_ALL
AdwValueObject *adw_value_object_new_collect     (GType         type,
                                                  ...);
ADW_AVAILABLE_IN_ALL
AdwValueObject *adw_value_object_new_string      (const char   *string);
ADW_AVAILABLE_IN_ALL
AdwValueObject *adw_value_object_new_take_string (char         *string);

ADW_AVAILABLE_IN_ALL
const GValue *adw_value_object_get_value  (AdwValueObject *value);
ADW_AVAILABLE_IN_ALL
void          adw_value_object_copy_value (AdwValueObject *value,
                                           GValue         *dest);

ADW_AVAILABLE_IN_ALL
const char *adw_value_object_get_string (AdwValueObject *value);
ADW_AVAILABLE_IN_ALL
char       *adw_value_object_dup_string (AdwValueObject *value);

G_END_DECLS
