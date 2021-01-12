/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib-object.h>
#include "adw-swipeable.h"

G_BEGIN_DECLS

#define ADW_TYPE_SWIPE_GROUP (adw_swipe_group_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwSwipeGroup, adw_swipe_group, ADW, SWIPE_GROUP, GObject)

ADW_AVAILABLE_IN_ALL
AdwSwipeGroup *adw_swipe_group_new (void);

ADW_AVAILABLE_IN_ALL
void           adw_swipe_group_add_swipeable    (AdwSwipeGroup *self,
                                                 AdwSwipeable  *swipeable);
ADW_AVAILABLE_IN_ALL
GSList *       adw_swipe_group_get_swipeables   (AdwSwipeGroup *self);
ADW_AVAILABLE_IN_ALL
void           adw_swipe_group_remove_swipeable (AdwSwipeGroup *self,
                                                 AdwSwipeable  *swipeable);

G_END_DECLS
