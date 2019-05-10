/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include "hdy-preferences-group.h"

G_BEGIN_DECLS

void hdy_preferences_group_add_preferences_to_model (HdyPreferencesGroup *self,
                                                     GListStore          *model);

G_END_DECLS
