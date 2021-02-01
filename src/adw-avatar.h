/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_AVATAR (adw_avatar_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwAvatar, adw_avatar, ADW, AVATAR, GtkWidget)

/**
 * AdwAvatarImageLoadFunc:
 * @size: the required size of the avatar
 * @user_data: (closure): user data
 *
 * The returned #GdkPixbuf is expected to be square with width and height set
 * to @size. The image is cropped to a circle without any scaling or transformation.
 *
 * Returns: (nullable) (transfer full): the #GdkPixbuf to use as a custom avatar
 * or %NULL to fallback to the generated avatar.
 */
typedef GdkPixbuf *(*AdwAvatarImageLoadFunc) (gint     size,
                                              gpointer user_data);


ADW_AVAILABLE_IN_ALL
GtkWidget   *adw_avatar_new                 (gint                    size,
                                             const char             *text,
                                             gboolean                show_initials);
ADW_AVAILABLE_IN_ALL
const char  *adw_avatar_get_icon_name       (AdwAvatar              *self);
ADW_AVAILABLE_IN_ALL
void         adw_avatar_set_icon_name       (AdwAvatar              *self,
                                             const char             *icon_name);
ADW_AVAILABLE_IN_ALL
const char  *adw_avatar_get_text            (AdwAvatar              *self);
ADW_AVAILABLE_IN_ALL
void         adw_avatar_set_text            (AdwAvatar              *self,
                                             const char             *text);
ADW_AVAILABLE_IN_ALL
gboolean     adw_avatar_get_show_initials   (AdwAvatar              *self);
ADW_AVAILABLE_IN_ALL
void         adw_avatar_set_show_initials   (AdwAvatar              *self,
                                             gboolean                show_initials);
ADW_AVAILABLE_IN_ALL
void         adw_avatar_set_image_load_func (AdwAvatar              *self,
                                             AdwAvatarImageLoadFunc  load_image,
                                             gpointer                user_data,
                                             GDestroyNotify          destroy);
ADW_AVAILABLE_IN_ALL
gint         adw_avatar_get_size            (AdwAvatar              *self);
ADW_AVAILABLE_IN_ALL
void         adw_avatar_set_size            (AdwAvatar              *self,
                                             gint                    size);
ADW_AVAILABLE_IN_ALL
GdkPixbuf   *adw_avatar_draw_to_pixbuf      (AdwAvatar              *self,
                                             gint                    size,
                                             gint                    scale_factor);

G_END_DECLS
