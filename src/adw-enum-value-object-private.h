/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-enum-value-object.h"

G_BEGIN_DECLS

AdwEnumValueObject *adw_enum_value_object_new (GEnumValue *enum_value);

G_END_DECLS
