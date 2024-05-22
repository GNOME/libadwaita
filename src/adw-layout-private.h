/*
 * Copyright (C) 2023 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-layout.h"

#include "adw-multi-layout-view.h"

G_BEGIN_DECLS

void adw_layout_set_view (AdwLayout          *self,
                          AdwMultiLayoutView *view);

G_END_DECLS
