/* adw-tag.h: A tag object for tagged widgets
 *
 * SPDX-FileCopyrightText: 2022 Emmanuele Bassi
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_TAG (adw_tag_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwTag, adw_tag, ADW, TAG, GObject)

ADW_AVAILABLE_IN_ALL
AdwTag *adw_tag_new           (void);
ADW_AVAILABLE_IN_ALL
AdwTag *adw_tag_new_with_name (const char *name);

ADW_AVAILABLE_IN_ALL
const char *adw_tag_get_label (AdwTag     *self);
ADW_AVAILABLE_IN_ALL
void        adw_tag_set_label (AdwTag     *self,
                               const char *label);

ADW_AVAILABLE_IN_ALL
gboolean adw_tag_get_show_close (AdwTag   *self);
ADW_AVAILABLE_IN_ALL
void     adw_tag_set_show_close (AdwTag   *self,
                                 gboolean  show_close);

ADW_AVAILABLE_IN_ALL
GIcon *       adw_tag_get_gicon     (AdwTag       *self);
ADW_AVAILABLE_IN_ALL
void          adw_tag_set_gicon     (AdwTag       *self,
                                     GIcon        *icon);
ADW_AVAILABLE_IN_ALL
GdkPaintable *adw_tag_get_paintable (AdwTag       *self);
ADW_AVAILABLE_IN_ALL
void          adw_tag_set_paintable (AdwTag       *self,
                                     GdkPaintable *paintable);
ADW_AVAILABLE_IN_ALL
gboolean      adw_tag_has_icon      (AdwTag       *self);

ADW_AVAILABLE_IN_ALL
const char *adw_tag_get_name (AdwTag     *self);

ADW_AVAILABLE_IN_ALL
const char *adw_tag_get_action_name (AdwTag *self);
ADW_AVAILABLE_IN_ALL
void        adw_tag_set_action_name (AdwTag   *self,
                                     const char *action_name);

ADW_AVAILABLE_IN_ALL
GVariant *adw_tag_get_action_target_value  (AdwTag     *self);
ADW_AVAILABLE_IN_ALL
void      adw_tag_set_action_target_value  (AdwTag     *self,
                                            GVariant   *action_target);
ADW_AVAILABLE_IN_ALL
void      adw_tag_set_action_target        (AdwTag     *self,
                                            const char *format_string,
                                            ...);
ADW_AVAILABLE_IN_ALL
void      adw_tag_set_detailed_action_name (AdwTag     *self,
                                            const char *detailed_action_name);

G_END_DECLS
