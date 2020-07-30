/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-window.h"

G_BEGIN_DECLS

#define HDY_TYPE_PREFERENCES_WINDOW (hdy_preferences_window_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyPreferencesWindow, hdy_preferences_window, HDY, PREFERENCES_WINDOW, HdyWindow)

/**
 * HdyPreferencesWindowClass
 * @parent_class: The parent class
 */
struct _HdyPreferencesWindowClass
{
  HdyWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget *hdy_preferences_window_new (void);

HDY_AVAILABLE_IN_ALL
gboolean hdy_preferences_window_get_search_enabled (HdyPreferencesWindow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_preferences_window_set_search_enabled (HdyPreferencesWindow *self,
                                                    gboolean              search_enabled);

HDY_AVAILABLE_IN_ALL
gboolean hdy_preferences_window_get_can_swipe_back (HdyPreferencesWindow *self);
HDY_AVAILABLE_IN_ALL
void     hdy_preferences_window_set_can_swipe_back (HdyPreferencesWindow *self,
                                                    gboolean              can_swipe_back);

HDY_AVAILABLE_IN_ALL
void hdy_preferences_window_present_subpage (HdyPreferencesWindow *self,
                                             GtkWidget            *subpage);
HDY_AVAILABLE_IN_ALL
void hdy_preferences_window_close_subpage (HdyPreferencesWindow *self);

G_END_DECLS
