/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HDY_STRING_UTF8_H
#define HDY_STRING_UTF8_H

#if !defined(HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-dialer-button.h"

G_BEGIN_DECLS

GString*     hdy_string_utf8_truncate   (GString         *string,
                                         gsize            len);
glong        hdy_string_utf8_len        (GString         *string);

G_END_DECLS

#endif /* HDY_STRING_UTF8 */
