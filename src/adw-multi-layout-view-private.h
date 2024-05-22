/*
 * Copyright (C) 2019 Purism SPC
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-multi-layout-view.h"

#include "adw-layout.h"

G_BEGIN_DECLS

void adw_multi_layout_view_register_slot (AdwMultiLayoutView *self,
                                          const char         *id,
                                          GtkWidget          *slot);

G_END_DECLS
