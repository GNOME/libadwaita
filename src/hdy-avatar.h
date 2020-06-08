/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_AVATAR (hdy_avatar_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (HdyAvatar, hdy_avatar, HDY, AVATAR, GtkDrawingArea)

/**
 * HdyAvatarImageLoadFunc:
 * @size: the required size of the avatar
 * @user_data: (closure): user data
 *
 * The returned #GdkPixbuf is expected to be square with width and height set
 * to @size. The image is cropped to a circle without any scaling or transformation.
 *
 * Returns: (nullable) (transfer full): the #GdkPixbuf to use as a custom avatar
 * or %NULL to fallback to the generated avatar.
 */
typedef GdkPixbuf *(*HdyAvatarImageLoadFunc) (gint     size,
                                              gpointer user_data);


HDY_AVAILABLE_IN_ALL
GtkWidget   *hdy_avatar_new                 (gint                    size,
                                             const gchar            *text,
                                             gboolean                show_initials);
HDY_AVAILABLE_IN_ALL
const gchar *hdy_avatar_get_icon_name       (HdyAvatar              *self);
HDY_AVAILABLE_IN_ALL
void         hdy_avatar_set_icon_name       (HdyAvatar              *self,
                                             const gchar            *icon_name);
HDY_AVAILABLE_IN_ALL
const gchar *hdy_avatar_get_text            (HdyAvatar              *self);
HDY_AVAILABLE_IN_ALL
void         hdy_avatar_set_text            (HdyAvatar              *self,
                                             const gchar            *text);
HDY_AVAILABLE_IN_ALL
gboolean     hdy_avatar_get_show_initials   (HdyAvatar              *self);
HDY_AVAILABLE_IN_ALL
void         hdy_avatar_set_show_initials   (HdyAvatar              *self,
                                             gboolean                show_initials);
HDY_AVAILABLE_IN_ALL
void         hdy_avatar_set_image_load_func (HdyAvatar              *self,
                                             HdyAvatarImageLoadFunc  load_image,
                                             gpointer                user_data,
                                             GDestroyNotify          destroy);
HDY_AVAILABLE_IN_ALL
gint         hdy_avatar_get_size            (HdyAvatar              *self);
HDY_AVAILABLE_IN_ALL
void         hdy_avatar_set_size            (HdyAvatar              *self,
                                             gint                    size);

G_END_DECLS
