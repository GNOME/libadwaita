/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#if defined(HDY_DISABLE_DEPRECATION_WARNINGS) || defined(HANDY_COMPILATION)
#  define _HDY_DEPRECATED
#  define _HDY_DEPRECATED_FOR(f)
#  define _HDY_DEPRECATED_MACRO
#  define _HDY_DEPRECATED_MACRO_FOR(f)
#  define _HDY_DEPRECATED_ENUMERATOR
#  define _HDY_DEPRECATED_ENUMERATOR_FOR(f)
#  define _HDY_DEPRECATED_TYPE
#  define _HDY_DEPRECATED_TYPE_FOR(f)
#else
#  define _HDY_DEPRECATED                G_DEPRECATED
#  define _HDY_DEPRECATED_FOR(f)         G_DEPRECATED_FOR(f)
#  define _HDY_DEPRECATED_MACRO          G_DEPRECATED
#  define _HDY_DEPRECATED_MACRO_FOR(f)   G_DEPRECATED_FOR(f)
#  define _HDY_DEPRECATED_ENUMERATOR          G_DEPRECATED
#  define _HDY_DEPRECATED_ENUMERATOR_FOR(f)   G_DEPRECATED_FOR(f)
#  define _HDY_DEPRECATED_TYPE           G_DEPRECATED
#  define _HDY_DEPRECATED_TYPE_FOR(f)    G_DEPRECATED_FOR(f)
#endif
