/*
 * Copyright (C) 2025 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

#include "adw-sidebar-section.h"

G_BEGIN_DECLS

typedef struct _AdwSidebarSection AdwSidebarSection;

#define ADW_TYPE_SIDEBAR_ITEM (adw_sidebar_item_get_type ())

ADW_AVAILABLE_IN_1_8
G_DECLARE_DERIVABLE_TYPE (AdwSidebarItem, adw_sidebar_item, ADW, SIDEBAR_ITEM, GObject)

/**
 * AdwSidebarItemClass:
 * @parent_class: The parent class
 */
struct _AdwSidebarItemClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer padding[4];
};

ADW_AVAILABLE_IN_1_8
AdwSidebarItem *adw_sidebar_item_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_1_8
const char *adw_sidebar_item_get_title (AdwSidebarItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_sidebar_item_set_title (AdwSidebarItem *self,
                                        const char     *title);

ADW_AVAILABLE_IN_1_8
const char *adw_sidebar_item_get_subtitle (AdwSidebarItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_sidebar_item_set_subtitle (AdwSidebarItem *self,
                                           const char     *subtitle);

ADW_AVAILABLE_IN_1_8
const char *adw_sidebar_item_get_icon_name (AdwSidebarItem *self);
ADW_AVAILABLE_IN_1_8
void        adw_sidebar_item_set_icon_name (AdwSidebarItem *self,
                                            const char     *icon_name);

ADW_AVAILABLE_IN_1_8
GdkPaintable *adw_sidebar_item_get_icon_paintable (AdwSidebarItem *self);
ADW_AVAILABLE_IN_1_8
void          adw_sidebar_item_set_icon_paintable (AdwSidebarItem *self,
                                                   GdkPaintable   *paintable);

ADW_AVAILABLE_IN_1_8
GtkWidget *adw_sidebar_item_get_suffix (AdwSidebarItem *self);
ADW_AVAILABLE_IN_1_8
void       adw_sidebar_item_set_suffix (AdwSidebarItem *self,
                                        GtkWidget      *suffix);

ADW_AVAILABLE_IN_1_8
gboolean adw_sidebar_item_get_enabled (AdwSidebarItem *self);
ADW_AVAILABLE_IN_1_8
void     adw_sidebar_item_set_enabled (AdwSidebarItem *self,
                                       gboolean        enabled);

ADW_AVAILABLE_IN_1_8
AdwSidebarSection *adw_sidebar_item_get_section (AdwSidebarItem *self);

ADW_AVAILABLE_IN_1_8
guint adw_sidebar_item_get_index (AdwSidebarItem *self);

G_END_DECLS
