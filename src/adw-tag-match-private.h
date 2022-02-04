/* adw-tag-match-private.h
 *
 * SPDX-FileCopyrightText: 2022  Emmanuele Bassi
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "adw-tag-private.h"

G_BEGIN_DECLS

#define ADW_TYPE_TAG_MATCH (adw_tag_match_get_type())
G_DECLARE_FINAL_TYPE (AdwTagMatch, adw_tag_match, ADW, TAG_MATCH, GObject)

AdwTagMatch *adw_tag_match_new (gpointer    item,
                                const char *str);

gpointer adw_tag_match_get_item (AdwTagMatch *self);

const char *adw_tag_match_get_string (AdwTagMatch *self);

void    adw_tag_match_set_tag (AdwTagMatch *self,
                               AdwTag      *tag);
AdwTag *adw_tag_match_get_tag (AdwTagMatch *self);

G_END_DECLS
