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

#include <glib.h>

G_BEGIN_DECLS

HDY_AVAILABLE_IN_ALL
void hdy_init (void);

G_END_DECLS
