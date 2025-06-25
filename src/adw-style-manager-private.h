/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-style-manager.h"

G_BEGIN_DECLS

void adw_style_manager_ensure (void);

void adw_style_manager_update_media_features (AdwStyleManager *self,
                                              GtkCssProvider  *css_provider);

G_END_DECLS
