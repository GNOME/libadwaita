/*
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

#define HDY_TYPE_PREFERENCES_PAGE (hdy_preferences_page_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyPreferencesPage, hdy_preferences_page, HDY, PREFERENCES_PAGE, GtkScrolledWindow)

/**
 * HdyPreferencesPageClass
 * @parent_class: The parent class
 */
struct _HdyPreferencesPageClass
{
  GtkScrolledWindowClass parent_class;
};

HdyPreferencesPage *hdy_preferences_page_new (void);

const gchar *hdy_preferences_page_get_icon_name (HdyPreferencesPage *self);
void         hdy_preferences_page_set_icon_name (HdyPreferencesPage *self,
                                                 const gchar        *icon_name);

const gchar *hdy_preferences_page_get_title (HdyPreferencesPage *self);
void         hdy_preferences_page_set_title (HdyPreferencesPage *self,
                                             const gchar        *title);

G_END_DECLS
