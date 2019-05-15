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

#define HDY_TYPE_VIEW_SWITCHER (hdy_view_switcher_get_type())

struct _HdyViewSwitcherClass {
  GtkBoxClass parent_class;
};

G_DECLARE_DERIVABLE_TYPE (HdyViewSwitcher, hdy_view_switcher, HDY, VIEW_SWITCHER, GtkBox)

typedef enum {
  HDY_VIEW_SWITCHER_POLICY_AUTO,
  HDY_VIEW_SWITCHER_POLICY_NARROW,
  HDY_VIEW_SWITCHER_POLICY_WIDE,
} HdyViewSwitcherPolicy;

HdyViewSwitcher *hdy_view_switcher_new (void);

HdyViewSwitcherPolicy hdy_view_switcher_get_policy (HdyViewSwitcher *self);
void                  hdy_view_switcher_set_policy (HdyViewSwitcher       *self,
                                                    HdyViewSwitcherPolicy  policy);

GtkIconSize hdy_view_switcher_get_icon_size (HdyViewSwitcher *self);
void        hdy_view_switcher_set_icon_size (HdyViewSwitcher *self,
                                             GtkIconSize      icon_size);

PangoEllipsizeMode hdy_view_switcher_get_narrow_ellipsize (HdyViewSwitcher *self);
void               hdy_view_switcher_set_narrow_ellipsize (HdyViewSwitcher    *self,
                                                           PangoEllipsizeMode  mode);

GtkStack *hdy_view_switcher_get_stack (HdyViewSwitcher *self);
void      hdy_view_switcher_set_stack (HdyViewSwitcher *self,
                                       GtkStack        *stack);

G_END_DECLS
