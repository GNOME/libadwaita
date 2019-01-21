/*
 * Copyright (C) 2019 Red Hat Inc.
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

#define HDY_TYPE_VALUE_OBJECT (hdy_value_object_get_type())

G_DECLARE_FINAL_TYPE (HdyValueObject, hdy_value_object, HDY, VALUE_OBJECT, GObject)

HdyValueObject *hdy_value_object_new             (const GValue *value);
HdyValueObject *hdy_value_object_new_collect     (GType         type,
                                                  ...);
HdyValueObject *hdy_value_object_new_string      (const gchar  *string);
HdyValueObject *hdy_value_object_new_take_string (gchar        *string);

const GValue*   hdy_value_object_get_value  (HdyValueObject *value);
void            hdy_value_object_copy_value (HdyValueObject *value,
                                             GValue         *dest);
const gchar*    hdy_value_object_get_string (HdyValueObject *value);
gchar*          hdy_value_object_dup_string (HdyValueObject *value);

G_END_DECLS
