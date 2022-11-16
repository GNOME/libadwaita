/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define ADW_CRITICAL_CANNOT_REMOVE_CHILD(parent, child) \
G_STMT_START { \
  g_critical ("%s:%d: tried to remove non-child %p of type '%s' from %p of type '%s'", \
              __FILE__, __LINE__, \
              (child), \
              G_OBJECT_TYPE_NAME ((GObject*) (child)), \
              (parent), \
              G_OBJECT_TYPE_NAME ((GObject*) (parent))); \
} G_STMT_END

G_END_DECLS
