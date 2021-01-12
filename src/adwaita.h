/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(4, 0, 0)
# error "libadwaita requires gtk4 >= 4.0.0"
#endif

#if !GLIB_CHECK_VERSION(2, 50, 0)
# error "libadwaita requires glib-2.0 >= 2.50.0"
#endif

#define _ADWAITA_INSIDE

#include "adw-version.h"
#include "adw-action-row.h"
#include "adw-animation.h"
#include "adw-application-window.h"
#include "adw-avatar.h"
#include "adw-carousel.h"
#include "adw-carousel-indicator-dots.h"
#include "adw-carousel-indicator-lines.h"
#include "adw-clamp.h"
#include "adw-clamp-layout.h"
#include "adw-clamp-scrollable.h"
#include "adw-combo-row.h"
#include "adw-deprecation-macros.h"
#include "adw-enum-list-model.h"
#include "adw-enum-value-object.h"
#include "adw-expander-row.h"
#include "adw-flap.h"
#include "adw-header-bar.h"
#include "adw-header-group.h"
#include "adw-keypad.h"
#include "adw-leaflet.h"
#include "adw-main.h"
#include "adw-navigation-direction.h"
#include "adw-preferences-group.h"
#include "adw-preferences-page.h"
#include "adw-preferences-row.h"
#include "adw-preferences-window.h"
#include "adw-squeezer.h"
#include "adw-status-page.h"
#include "adw-swipe-group.h"
#include "adw-swipe-tracker.h"
#include "adw-swipeable.h"
#include "adw-types.h"
#include "adw-value-object.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-bar.h"
#include "adw-view-switcher-title.h"
#include "adw-window.h"

#undef _ADWAITA_INSIDE

G_END_DECLS
