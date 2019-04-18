/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(3, 22, 0)
# error "libhandy requires gtk+-3.0 >= 3.22.0"
#endif

#if !GLIB_CHECK_VERSION(2, 50, 0)
# error "libhandy requires glib-2.0 >= 2.50.0"
#endif

#define _HANDY_INSIDE

#ifndef HANDY_USE_UNSTABLE_API
#error    libhandy is unstable API. You must define HANDY_USE_UNSTABLE_API before including handy.h
#endif

#include "hdy-version.h"
#include "hdy-action-row.h"
#include "hdy-arrows.h"
#include "hdy-column.h"
#include "hdy-combo-row.h"
#include "hdy-dialer-button.h"
#include "hdy-dialer-cycle-button.h"
#include "hdy-dialer.h"
#include "hdy-dialog.h"
#include "hdy-enum-value-object.h"
#include "hdy-expander-row.h"
#include "hdy-fold.h"
#include "hdy-header-bar.h"
#include "hdy-header-group.h"
#include "hdy-leaflet.h"
#include "hdy-list-box.h"
#include "hdy-main.h"
#include "hdy-preferences-group.h"
#include "hdy-preferences-page.h"
#include "hdy-preferences-row.h"
#include "hdy-preferences-window.h"
#include "hdy-search-bar.h"
#include "hdy-squeezer.h"
#include "hdy-string-utf8.h"
#include "hdy-title-bar.h"
#include "hdy-value-object.h"
#include "hdy-view-switcher.h"
#include "hdy-view-switcher-bar.h"

#undef _HANDY_INSIDE

G_END_DECLS
