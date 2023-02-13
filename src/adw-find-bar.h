/*
 * adw-find-bar.h
 *
 * Copyright 2023 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_FIND_BAR (adw_find_bar_get_type())

ADW_AVAILABLE_IN_1_3
G_DECLARE_FINAL_TYPE (AdwFindBar, adw_find_bar, ADW, FIND_BAR, GtkWidget)

ADW_AVAILABLE_IN_1_3
AdwFindBar *adw_find_bar_new (void);

ADW_AVAILABLE_IN_1_3
GtkWidget *adw_find_bar_get_child (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void       adw_find_bar_set_child (AdwFindBar *self,
                                   GtkWidget  *child);

ADW_AVAILABLE_IN_1_3
gboolean adw_find_bar_get_narrow (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_find_bar_set_narrow (AdwFindBar *self,
                                  gboolean    narrow);

ADW_AVAILABLE_IN_1_3
gboolean adw_find_bar_get_revealed (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_find_bar_set_revealed (AdwFindBar *self,
                                    gboolean    revealed);

ADW_AVAILABLE_IN_1_3
gboolean adw_find_bar_get_replace_enabled (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_find_bar_set_replace_enabled (AdwFindBar *self,
                                           gboolean    enabled);

ADW_AVAILABLE_IN_1_3
const char *adw_find_bar_get_find_text (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void        adw_find_bar_set_find_text (AdwFindBar *self,
                                        const char *text);

ADW_AVAILABLE_IN_1_3
const char *adw_find_bar_get_replace_text (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void        adw_find_bar_set_replace_text (AdwFindBar *self,
                                           const char *text);

ADW_AVAILABLE_IN_1_3
const char *adw_find_bar_get_find_placeholder_text (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void        adw_find_bar_set_find_placeholder_text (AdwFindBar *self,
                                                    const char *text);

ADW_AVAILABLE_IN_1_3
const char *adw_find_bar_get_replace_placeholder_text (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void        adw_find_bar_set_replace_placeholder_text (AdwFindBar *self,
                                                       const char *text);

ADW_AVAILABLE_IN_1_3
int  adw_find_bar_get_match_count (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void adw_find_bar_set_match_count (AdwFindBar *self,
                                   int         count);

ADW_AVAILABLE_IN_1_3
int  adw_find_bar_get_match_position (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void adw_find_bar_set_match_position (AdwFindBar *self,
                                      int         position);

ADW_AVAILABLE_IN_1_3
gboolean adw_find_bar_get_case_sensitive (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_find_bar_set_case_sensitive (AdwFindBar *self,
                                          gboolean    case_sensitive);

ADW_AVAILABLE_IN_1_3
gboolean adw_find_bar_get_use_regex (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_find_bar_set_use_regex (AdwFindBar *self,
                                     gboolean    use_regex);

ADW_AVAILABLE_IN_1_3
gboolean adw_find_bar_get_match_whole_words (AdwFindBar *self);
ADW_AVAILABLE_IN_1_3
void     adw_find_bar_set_match_whole_words (AdwFindBar *self,
                                             gboolean    match_whole_words);

G_END_DECLS
