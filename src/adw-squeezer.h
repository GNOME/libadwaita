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
#include "adw-enums.h"

G_BEGIN_DECLS

#define ADW_TYPE_SQUEEZER_PAGE (adw_squeezer_page_get_type ())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwSqueezerPage, adw_squeezer_page, ADW, SQUEEZER_PAGE, GObject)

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_squeezer_page_get_child (AdwSqueezerPage *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_squeezer_page_get_enabled (AdwSqueezerPage *self);
ADW_AVAILABLE_IN_ALL
void     adw_squeezer_page_set_enabled (AdwSqueezerPage *self,
                                        gboolean         enabled);

#define ADW_TYPE_SQUEEZER (adw_squeezer_get_type ())

ADW_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (AdwSqueezer, adw_squeezer, ADW, SQUEEZER, GtkWidget)

typedef enum {
  ADW_SQUEEZER_TRANSITION_TYPE_NONE,
  ADW_SQUEEZER_TRANSITION_TYPE_CROSSFADE,
} AdwSqueezerTransitionType;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_squeezer_new (void);

ADW_AVAILABLE_IN_ALL
AdwSqueezerPage *adw_squeezer_add (AdwSqueezer *self,
                                   GtkWidget   *child);
ADW_AVAILABLE_IN_ALL
void             adw_squeezer_remove (AdwSqueezer *self,
                                      GtkWidget   *child);

ADW_AVAILABLE_IN_ALL
AdwSqueezerPage *adw_squeezer_get_page (AdwSqueezer *self,
                                        GtkWidget   *child);

ADW_AVAILABLE_IN_ALL
gboolean adw_squeezer_get_homogeneous (AdwSqueezer *self);
ADW_AVAILABLE_IN_ALL
void     adw_squeezer_set_homogeneous (AdwSqueezer *self,
                                       gboolean     homogeneous);

ADW_AVAILABLE_IN_ALL
guint adw_squeezer_get_transition_duration (AdwSqueezer *self);
ADW_AVAILABLE_IN_ALL
void  adw_squeezer_set_transition_duration (AdwSqueezer *self,
                                            guint        duration);

ADW_AVAILABLE_IN_ALL
AdwSqueezerTransitionType adw_squeezer_get_transition_type (AdwSqueezer *self);
ADW_AVAILABLE_IN_ALL
void                      adw_squeezer_set_transition_type (AdwSqueezer               *self,
                                                            AdwSqueezerTransitionType  transition);

ADW_AVAILABLE_IN_ALL
gboolean adw_squeezer_get_transition_running (AdwSqueezer *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_squeezer_get_interpolate_size (AdwSqueezer *self);
ADW_AVAILABLE_IN_ALL
void     adw_squeezer_set_interpolate_size (AdwSqueezer *self,
                                            gboolean     interpolate_size);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_squeezer_get_visible_child (AdwSqueezer *self);

ADW_AVAILABLE_IN_ALL
gfloat adw_squeezer_get_xalign (AdwSqueezer *self);
ADW_AVAILABLE_IN_ALL
void   adw_squeezer_set_xalign (AdwSqueezer *self,
                                gfloat       xalign);

ADW_AVAILABLE_IN_ALL
gfloat adw_squeezer_get_yalign (AdwSqueezer *self);
ADW_AVAILABLE_IN_ALL
void   adw_squeezer_set_yalign (AdwSqueezer *self,
                                gfloat       yalign);

ADW_AVAILABLE_IN_ALL
GtkSelectionModel *adw_squeezer_get_pages (AdwSqueezer *self);

G_END_DECLS
