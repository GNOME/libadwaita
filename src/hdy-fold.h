/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HDY_FOLD_H
#define HDY_FOLD_H

#if !defined(HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define HDY_TYPE_FOLD (hdy_fold_get_type ())

typedef enum {
  HDY_FOLD_UNFOLDED,
  HDY_FOLD_FOLDED,
} HdyFold;

GType hdy_fold_get_type (void);

G_END_DECLS

#endif /* HDY_FOLD_H */
