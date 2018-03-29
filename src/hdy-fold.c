/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "hdy-fold.h"

/**
 * HdyFold:
 * @HDY_FOLD_UNFOLDED: The element isn't folded
 * @HDY_FOLD_FOLDED: The element is folded
 *
 * Represents the fold of widgets and other objects which can be switched
 * between folded and unfolded state on the fly, like HdyLeaflet.
 */

GType
hdy_fold_get_type (void)
{
  static volatile gsize hdy_fold_type = 0;

  if (g_once_init_enter (&hdy_fold_type)) {
    static const GEnumValue values[] = {
      { HDY_FOLD_UNFOLDED, "HDY_FOLD_UNFOLDED", "unfolded" },
      { HDY_FOLD_FOLDED, "HDY_FOLD_FOLDED", "folded" },
      { 0, NULL, NULL },
    };
    GType type;

    type = g_enum_register_static ("HdyFold", values);

    g_once_init_leave (&hdy_fold_type, type);
  }

  return hdy_fold_type;
}
