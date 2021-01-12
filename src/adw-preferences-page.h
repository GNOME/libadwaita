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
#include "adw-preferences-group.h"

G_BEGIN_DECLS

#define ADW_TYPE_PREFERENCES_PAGE (adw_preferences_page_get_type())

ADW_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (AdwPreferencesPage, adw_preferences_page, ADW, PREFERENCES_PAGE, GtkWidget)

/**
 * AdwPreferencesPageClass
 * @parent_class: The parent class
 */
struct _AdwPreferencesPageClass
{
  GtkWidgetClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_ALL
GtkWidget   *adw_preferences_page_new (void);

ADW_AVAILABLE_IN_ALL
const gchar *adw_preferences_page_get_icon_name (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_ALL
void         adw_preferences_page_set_icon_name (AdwPreferencesPage *self,
                                                 const gchar        *icon_name);

ADW_AVAILABLE_IN_ALL
const gchar *adw_preferences_page_get_title (AdwPreferencesPage *self);
ADW_AVAILABLE_IN_ALL
void         adw_preferences_page_set_title (AdwPreferencesPage *self,
                                             const gchar        *title);

ADW_AVAILABLE_IN_ALL
void         adw_preferences_page_add (AdwPreferencesPage  *self,
                                       AdwPreferencesGroup *group);
ADW_AVAILABLE_IN_ALL
void         adw_preferences_page_remove (AdwPreferencesPage  *self,
                                          AdwPreferencesGroup *group);

G_END_DECLS
