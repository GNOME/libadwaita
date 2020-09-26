/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include "hdy-preferences-group.h"

G_BEGIN_DECLS

GListModel *hdy_preferences_group_get_rows (HdyPreferencesGroup *self);

G_END_DECLS
