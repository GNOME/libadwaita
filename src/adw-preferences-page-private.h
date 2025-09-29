/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "adw-preferences-page.h"

G_BEGIN_DECLS

GListModel *adw_preferences_page_get_rows (AdwPreferencesPage *self) G_GNUC_WARN_UNUSED_RESULT;

GtkWidget *adw_preferences_page_get_viewport (AdwPreferencesPage *self);

G_END_DECLS
