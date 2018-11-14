/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

G_BEGIN_DECLS

GString*     hdy_string_utf8_truncate   (GString         *string,
                                         gsize            len);
glong        hdy_string_utf8_len        (GString         *string);

G_END_DECLS
