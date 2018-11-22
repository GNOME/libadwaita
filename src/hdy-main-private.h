/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-main.h"

G_BEGIN_DECLS

/* Initializes the public GObject types, which is needed to ensure they are
 * discoverable, for example so they can easily be used with GtkBuilder.
 *
 * The function is implemented in hdy-public-types.c which is generated at
 * compile time by gen-public-types.sh
 */
void hdy_init_public_types (void);

G_END_DECLS
