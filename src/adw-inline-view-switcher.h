/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-enums.h"
#include "adw-view-stack.h"

G_BEGIN_DECLS

typedef enum {
  ADW_INLINE_VIEW_SWITCHER_LABELS,
  ADW_INLINE_VIEW_SWITCHER_ICONS,
  ADW_INLINE_VIEW_SWITCHER_BOTH,
} AdwInlineViewSwitcherDisplayMode;

#define ADW_TYPE_INLINE_VIEW_SWITCHER (adw_inline_view_switcher_get_type ())

ADW_AVAILABLE_IN_1_7
G_DECLARE_FINAL_TYPE (AdwInlineViewSwitcher, adw_inline_view_switcher, ADW, INLINE_VIEW_SWITCHER, GtkWidget)

ADW_AVAILABLE_IN_1_7
GtkWidget *adw_inline_view_switcher_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_7
AdwViewStack *adw_inline_view_switcher_get_stack (AdwInlineViewSwitcher *self);
ADW_AVAILABLE_IN_1_7
void          adw_inline_view_switcher_set_stack (AdwInlineViewSwitcher *self,
                                                  AdwViewStack          *stack);

ADW_AVAILABLE_IN_1_7
AdwInlineViewSwitcherDisplayMode adw_inline_view_switcher_get_display_mode (AdwInlineViewSwitcher            *self);
ADW_AVAILABLE_IN_1_7
void                             adw_inline_view_switcher_set_display_mode (AdwInlineViewSwitcher            *self,
                                                                            AdwInlineViewSwitcherDisplayMode  mode);

ADW_AVAILABLE_IN_1_7
gboolean adw_inline_view_switcher_get_homogeneous (AdwInlineViewSwitcher *self);
ADW_AVAILABLE_IN_1_7
void     adw_inline_view_switcher_set_homogeneous (AdwInlineViewSwitcher *self,
                                                   gboolean               homogeneous);

ADW_AVAILABLE_IN_1_7
gboolean adw_inline_view_switcher_get_can_shrink (AdwInlineViewSwitcher *self);
ADW_AVAILABLE_IN_1_7
void     adw_inline_view_switcher_set_can_shrink (AdwInlineViewSwitcher *self,
                                                  gboolean               can_shrink);

G_END_DECLS
