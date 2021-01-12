/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#if defined(ADW_DISABLE_DEPRECATION_WARNINGS) || defined(ADWAITA_COMPILATION)
#  define _ADW_DEPRECATED
#  define _ADW_DEPRECATED_FOR(f)
#  define _ADW_DEPRECATED_MACRO
#  define _ADW_DEPRECATED_MACRO_FOR(f)
#  define _ADW_DEPRECATED_ENUMERATOR
#  define _ADW_DEPRECATED_ENUMERATOR_FOR(f)
#  define _ADW_DEPRECATED_TYPE
#  define _ADW_DEPRECATED_TYPE_FOR(f)
#else
#  define _ADW_DEPRECATED                G_DEPRECATED
#  define _ADW_DEPRECATED_FOR(f)         G_DEPRECATED_FOR(f)
#  define _ADW_DEPRECATED_MACRO          G_DEPRECATED
#  define _ADW_DEPRECATED_MACRO_FOR(f)   G_DEPRECATED_FOR(f)
#  define _ADW_DEPRECATED_ENUMERATOR          G_DEPRECATED
#  define _ADW_DEPRECATED_ENUMERATOR_FOR(f)   G_DEPRECATED_FOR(f)
#  define _ADW_DEPRECATED_TYPE           G_DEPRECATED
#  define _ADW_DEPRECATED_TYPE_FOR(f)    G_DEPRECATED_FOR(f)
#endif
