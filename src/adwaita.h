/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(4, 4, 0)
# error "libadwaita requires gtk4 >= 4.4.0"
#endif

#if !GLIB_CHECK_VERSION(2, 66, 0)
# error "libadwaita requires glib-2.0 >= 2.66.0"
#endif

#define _ADWAITA_INSIDE

#include "adw-version.h"
#include "adw-action-row.h"
#include "adw-animation.h"
#include "adw-animation-target.h"
#include "adw-animation-util.h"
#include "adw-application.h"
#include "adw-application-window.h"
#include "adw-avatar.h"
#include "adw-bin.h"
#include "adw-button-content.h"
#include "adw-carousel.h"
#include "adw-carousel-indicator-dots.h"
#include "adw-carousel-indicator-lines.h"
#include "adw-clamp.h"
#include "adw-clamp-layout.h"
#include "adw-clamp-scrollable.h"
#include "adw-combo-row.h"
#include "adw-deprecation-macros.h"
#include "adw-easing.h"
#include "adw-entry-row.h"
#include "adw-enum-list-model.h"
#include "adw-expander-row.h"
#include "adw-flap.h"
#include "adw-fold-threshold-policy.h"
#include "adw-header-bar.h"
#include "adw-leaflet.h"
#include "adw-main.h"
#include "adw-navigation-direction.h"
#include "adw-password-entry-row.h"
#include "adw-preferences-group.h"
#include "adw-preferences-page.h"
#include "adw-preferences-row.h"
#include "adw-preferences-window.h"
#include "adw-split-button.h"
#include "adw-spring-animation.h"
#include "adw-spring-params.h"
#include "adw-squeezer.h"
#include "adw-status-page.h"
#include "adw-style-manager.h"
#include "adw-swipe-tracker.h"
#include "adw-swipeable.h"
#include "adw-tab-bar.h"
#include "adw-tab-view.h"
#include "adw-timed-animation.h"
#include "adw-toast-overlay.h"
#include "adw-toast.h"
#include "adw-view-stack.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-bar.h"
#include "adw-view-switcher-title.h"
#include "adw-window.h"
#include "adw-window-title.h"

#undef _ADWAITA_INSIDE

G_END_DECLS
