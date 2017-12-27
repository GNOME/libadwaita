/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-3.0+
 */

#ifndef HANDY_H
#define HANDY_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(3, 22, 0)
# error "libhandy requires gtk+-3.0 >= 3.22.0"
#endif

#if !GLIB_CHECK_VERSION(2, 50, 0)
# error "libhandy requires glib-2.0 >= 2.50.0"
#endif

#define HANDY_INSIDE
#include "hdy-version.h"
#include "hdy-dialer-button.h"
#include "hdy-dialer.h"
#undef HANDY_INSIDE

G_END_DECLS

#endif /* HANDY_H */
