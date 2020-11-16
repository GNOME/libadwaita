/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-enum-value-object.h"

G_BEGIN_DECLS

HdyEnumValueObject *hdy_enum_value_object_new (GEnumValue *enum_value);

G_END_DECLS
