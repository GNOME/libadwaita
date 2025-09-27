/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-window.h"

G_BEGIN_DECLS

void adw_window_prepare_mobile_screenshot (AdwWindow *self);

GdkTexture *adw_window_take_mobile_screenshot (AdwWindow *self);

G_END_DECLS
