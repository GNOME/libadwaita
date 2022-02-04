/* adw-tagged-entry.h: Tagged entry widget
 *
 * SPDX-FileCopyrightText: 2022 Emmanuele Bassi
 * SPDX-FileCopyrightText: 2019 Matthias Clasen
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include "adw-tag.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_TAGGED_ENTRY (adw_tagged_entry_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwTaggedEntry, adw_tagged_entry, ADW, TAGGED_ENTRY, GtkWidget)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_tagged_entry_new (void);

ADW_AVAILABLE_IN_ALL
void        adw_tagged_entry_add_tag         (AdwTaggedEntry *self,
                                              AdwTag         *tag);
ADW_AVAILABLE_IN_ALL
void        adw_tagged_entry_remove_tag      (AdwTaggedEntry *self,
                                              AdwTag         *tag);
ADW_AVAILABLE_IN_ALL
GListModel *adw_tagged_entry_get_tags        (AdwTaggedEntry *self);
ADW_AVAILABLE_IN_ALL
void        adw_tagged_entry_remove_all_tags (AdwTaggedEntry *self);

ADW_AVAILABLE_IN_ALL
const char *adw_tagged_entry_get_placeholder_text (AdwTaggedEntry *self);
ADW_AVAILABLE_IN_ALL
void        adw_tagged_entry_set_placeholder_text (AdwTaggedEntry *self,
                                                   const char     *text);

ADW_AVAILABLE_IN_ALL
const char *adw_tagged_entry_get_delimiter_chars (AdwTaggedEntry *self);
ADW_AVAILABLE_IN_ALL
void        adw_tagged_entry_set_delimiter_chars (AdwTaggedEntry *self,
                                                  const char     *delimiters);

ADW_AVAILABLE_IN_ALL
GListModel *adw_tagged_entry_get_match_model (AdwTaggedEntry *self);
ADW_AVAILABLE_IN_ALL
void        adw_tagged_entry_set_match_model (AdwTaggedEntry *self,
                                              GListModel     *model);

ADW_AVAILABLE_IN_ALL
GtkExpression *adw_tagged_entry_get_match_expression (AdwTaggedEntry *self);
ADW_AVAILABLE_IN_ALL
void           adw_tagged_entry_set_match_expression (AdwTaggedEntry *self,
                                                      GtkExpression  *expression);

/**
 * AdwTaggedEntryMatchFunc:
 * @self: the tagged entry
 * @text: the text in the tagged entry
 * @item: (type GObject): the item from the match model
 *
 * Matches the given text from the tagged entry with an item from the
 * match model.
 *
 * If @text matches @item, this function returns the [class@Tag] that
 * should be added to the tagged entry.
 *
 * Returns: (nullable): the tag that matches the item, if any
 *
 * Since: 1.2
 */
typedef AdwTag *(* AdwTaggedEntryMatchFunc) (AdwTaggedEntry *self,
                                             const char     *text,
                                             gpointer        item,
                                             gpointer        user_data);

ADW_AVAILABLE_IN_ALL
void adw_tagged_entry_set_match_func (AdwTaggedEntry          *self,
                                      AdwTaggedEntryMatchFunc  match_func,
                                      gpointer                 user_data,
                                      GDestroyNotify           notify);

G_END_DECLS
