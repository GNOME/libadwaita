/*
 * Copyright (C) 2019 Red Hat Inc.
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

#define HDY_TYPE_VALUE_OBJECT (hdy_value_object_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyValueObject, hdy_value_object, HDY, VALUE_OBJECT, GObject)

HDY_AVAILABLE_IN_ALL
HdyValueObject *hdy_value_object_new             (const GValue *value);
HDY_AVAILABLE_IN_ALL
HdyValueObject *hdy_value_object_new_collect     (GType         type,
                                                  ...);
HDY_AVAILABLE_IN_ALL
HdyValueObject *hdy_value_object_new_string      (const gchar  *string);
HDY_AVAILABLE_IN_ALL
HdyValueObject *hdy_value_object_new_take_string (gchar        *string);

HDY_AVAILABLE_IN_ALL
const GValue*   hdy_value_object_get_value  (HdyValueObject *value);
HDY_AVAILABLE_IN_ALL
void            hdy_value_object_copy_value (HdyValueObject *value,
                                             GValue         *dest);
HDY_AVAILABLE_IN_ALL
const gchar*    hdy_value_object_get_string (HdyValueObject *value);
HDY_AVAILABLE_IN_ALL
gchar*          hdy_value_object_dup_string (HdyValueObject *value);

G_END_DECLS
