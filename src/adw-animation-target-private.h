/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <glib-object.h>

G_BEGIN_DECLS

typedef void (*AdwAnimationTargetFunc) (double   value,
                                        gpointer user_data);

#define ADW_TYPE_ANIMATION_TARGET (adw_animation_target_get_type())

G_DECLARE_FINAL_TYPE (AdwAnimationTarget, adw_animation_target, ADW, ANIMATION_TARGET, GObject)

AdwAnimationTarget *adw_animation_target_new (AdwAnimationTargetFunc callback,
                                              gpointer               user_data);

void adw_animation_target_set_value (AdwAnimationTarget *self,
                                     double              value);

G_END_DECLS
