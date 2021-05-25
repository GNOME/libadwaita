/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>
#include "adw-preferences-page.h"
#include "adw-window.h"

G_BEGIN_DECLS

#define ADW_TYPE_PREFERENCES_WINDOW (adw_preferences_window_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwPreferencesWindow, adw_preferences_window, ADW, PREFERENCES_WINDOW, AdwWindow)

/**
 * AdwPreferencesWindowClass
 * @parent_class: The parent class
 */
struct _AdwPreferencesWindowClass
{
  AdwWindowClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_preferences_window_new (void);

ADW_AVAILABLE_IN_ALL
gboolean adw_preferences_window_get_search_enabled (AdwPreferencesWindow *self);
ADW_AVAILABLE_IN_ALL
void     adw_preferences_window_set_search_enabled (AdwPreferencesWindow *self,
                                                    gboolean              search_enabled);

ADW_AVAILABLE_IN_ALL
gboolean adw_preferences_window_get_can_swipe_back (AdwPreferencesWindow *self);
ADW_AVAILABLE_IN_ALL
void     adw_preferences_window_set_can_swipe_back (AdwPreferencesWindow *self,
                                                    gboolean              can_swipe_back);

ADW_AVAILABLE_IN_ALL
void adw_preferences_window_present_subpage (AdwPreferencesWindow *self,
                                             GtkWidget            *subpage);
ADW_AVAILABLE_IN_ALL
void adw_preferences_window_close_subpage   (AdwPreferencesWindow *self);

ADW_AVAILABLE_IN_ALL
void adw_preferences_window_add    (AdwPreferencesWindow *self,
                                    AdwPreferencesPage   *page);
ADW_AVAILABLE_IN_ALL
void adw_preferences_window_remove (AdwPreferencesWindow *self,
                                    AdwPreferencesPage   *page);

G_END_DECLS
