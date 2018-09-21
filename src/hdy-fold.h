/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HDY_FOLD_H
#define HDY_FOLD_H

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <glib-object.h>
#include "hdy-enums.h"

G_BEGIN_DECLS

typedef enum {
  HDY_FOLD_UNFOLDED,
  HDY_FOLD_FOLDED,
} HdyFold;

G_END_DECLS

#endif /* HDY_FOLD_H */
