/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_SEARCH_BAR (hdy_search_bar_get_type())

G_DECLARE_DERIVABLE_TYPE (HdySearchBar, hdy_search_bar, HDY, SEARCH_BAR, GtkBin)

struct _HdySearchBarClass
{
  GtkBinClass parent_class;
};

GtkWidget      *hdy_search_bar_new                      (void);
void            hdy_search_bar_connect_entry            (HdySearchBar *self,
                                                         GtkEntry     *entry);
gboolean        hdy_search_bar_get_search_mode          (HdySearchBar *self);
void            hdy_search_bar_set_search_mode          (HdySearchBar *self,
                                                         gboolean      search_mode);
gboolean        hdy_search_bar_get_show_close_button    (HdySearchBar *self);
void            hdy_search_bar_set_show_close_button    (HdySearchBar *self,
                                                         gboolean      visible);
gboolean        hdy_search_bar_handle_event             (HdySearchBar *self,
                                                         GdkEvent     *event);

G_END_DECLS
