/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-toast.h"

G_BEGIN_DECLS

gboolean adw_toast_get_added (AdwToast *self);
void     adw_toast_set_added (AdwToast *self,
                              gboolean  added);

G_END_DECLS
