/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_VIEW_SWITCHER_BUTTON (hdy_view_switcher_button_get_type())

G_DECLARE_FINAL_TYPE (HdyViewSwitcherButton, hdy_view_switcher_button, HDY, VIEW_SWITCHER_BUTTON, GtkRadioButton)

GtkWidget   *hdy_view_switcher_button_new (void);

const gchar *hdy_view_switcher_button_get_icon_name (HdyViewSwitcherButton *self);
void         hdy_view_switcher_button_set_icon_name (HdyViewSwitcherButton *self,
                                                     const gchar           *icon_name);

GtkIconSize hdy_view_switcher_button_get_icon_size (HdyViewSwitcherButton *self);
void        hdy_view_switcher_button_set_icon_size (HdyViewSwitcherButton *self,
                                                    GtkIconSize            icon_size);

gboolean hdy_view_switcher_button_get_needs_attention (HdyViewSwitcherButton *self);
void     hdy_view_switcher_button_set_needs_attention (HdyViewSwitcherButton *self,
                                                       gboolean               needs_attention);

const gchar *hdy_view_switcher_button_get_label (HdyViewSwitcherButton *self);
void         hdy_view_switcher_button_set_label (HdyViewSwitcherButton *self,
                                                 const gchar           *label);

void hdy_view_switcher_button_set_narrow_ellipsize (HdyViewSwitcherButton *self,
                                                    PangoEllipsizeMode     mode);

void hdy_view_switcher_button_get_size (HdyViewSwitcherButton *self,
                                        gint                  *h_min_width,
                                        gint                  *h_nat_width,
                                        gint                  *v_min_width,
                                        gint                  *v_nat_width);

G_END_DECLS
