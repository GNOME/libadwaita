/*
 * Copyright (C) 2026 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_ICON_PROVIDER (adw_icon_provider_get_type())

G_DECLARE_FINAL_TYPE (AdwIconProvider, adw_icon_provider, ADW, ICON_PROVIDER, GObject)

AdwIconProvider *adw_icon_provider_new (GdkDisplay *display);

void adw_icon_provider_add_resource_path (AdwIconProvider *self,
                                          const char      *path);

char *adw_icon_provider_lookup_path (AdwIconProvider *self,
                                     const char      *icon_name);

G_END_DECLS
