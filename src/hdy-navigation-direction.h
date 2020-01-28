/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <glib-object.h>
#include "hdy-enums.h"

G_BEGIN_DECLS

typedef enum {
  HDY_NAVIGATION_DIRECTION_BACK,
  HDY_NAVIGATION_DIRECTION_FORWARD,
} HdyNavigationDirection;

G_END_DECLS
