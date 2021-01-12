/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
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

#define ADW_TYPE_FLAP (adw_flap_get_type ())

ADW_AVAILABLE_IN_1_1
G_DECLARE_FINAL_TYPE (AdwFlap, adw_flap, ADW, FLAP, GtkWidget)

typedef enum {
  ADW_FLAP_FOLD_POLICY_NEVER,
  ADW_FLAP_FOLD_POLICY_ALWAYS,
  ADW_FLAP_FOLD_POLICY_AUTO,
} AdwFlapFoldPolicy;

typedef enum {
  ADW_FLAP_TRANSITION_TYPE_OVER,
  ADW_FLAP_TRANSITION_TYPE_UNDER,
  ADW_FLAP_TRANSITION_TYPE_SLIDE,
} AdwFlapTransitionType;

ADW_AVAILABLE_IN_1_1
GtkWidget *adw_flap_new (void);

ADW_AVAILABLE_IN_1_1
GtkWidget *adw_flap_get_content (AdwFlap *self);
ADW_AVAILABLE_IN_1_1
void       adw_flap_set_content (AdwFlap   *self,
                                 GtkWidget *content);

ADW_AVAILABLE_IN_1_1
GtkWidget *adw_flap_get_flap (AdwFlap   *self);
ADW_AVAILABLE_IN_1_1
void       adw_flap_set_flap (AdwFlap   *self,
                              GtkWidget *flap);

ADW_AVAILABLE_IN_1_1
GtkWidget *adw_flap_get_separator (AdwFlap   *self);
ADW_AVAILABLE_IN_1_1
void       adw_flap_set_separator (AdwFlap   *self,
                                   GtkWidget *separator);

ADW_AVAILABLE_IN_1_1
GtkPackType adw_flap_get_flap_position (AdwFlap *self);
ADW_AVAILABLE_IN_1_1
void        adw_flap_set_flap_position (AdwFlap     *self,
                                        GtkPackType  position);

ADW_AVAILABLE_IN_1_1
gboolean adw_flap_get_reveal_flap (AdwFlap *self);
ADW_AVAILABLE_IN_1_1
void     adw_flap_set_reveal_flap (AdwFlap  *self,
                                   gboolean  reveal_flap);

ADW_AVAILABLE_IN_1_1
guint adw_flap_get_reveal_duration (AdwFlap *self);
ADW_AVAILABLE_IN_1_1
void  adw_flap_set_reveal_duration (AdwFlap *self,
                                    guint    duration);

ADW_AVAILABLE_IN_1_1
gdouble adw_flap_get_reveal_progress (AdwFlap *self);

ADW_AVAILABLE_IN_1_1
AdwFlapFoldPolicy adw_flap_get_fold_policy (AdwFlap           *self);
ADW_AVAILABLE_IN_1_1
void              adw_flap_set_fold_policy (AdwFlap           *self,
                                            AdwFlapFoldPolicy  policy);

ADW_AVAILABLE_IN_1_1
guint adw_flap_get_fold_duration (AdwFlap *self);
ADW_AVAILABLE_IN_1_1
void  adw_flap_set_fold_duration (AdwFlap *self,
                                  guint    duration);

ADW_AVAILABLE_IN_1_1
gboolean adw_flap_get_folded (AdwFlap *self);

ADW_AVAILABLE_IN_1_1
gboolean adw_flap_get_locked (AdwFlap *self);
ADW_AVAILABLE_IN_1_1
void     adw_flap_set_locked (AdwFlap  *self,
                              gboolean  locked);

ADW_AVAILABLE_IN_1_1
AdwFlapTransitionType adw_flap_get_transition_type (AdwFlap               *self);
ADW_AVAILABLE_IN_1_1
void                  adw_flap_set_transition_type (AdwFlap               *self,
                                                    AdwFlapTransitionType  transition_type);

ADW_AVAILABLE_IN_1_1
gboolean adw_flap_get_modal (AdwFlap  *self);
ADW_AVAILABLE_IN_1_1
void     adw_flap_set_modal (AdwFlap  *self,
                             gboolean  modal);

ADW_AVAILABLE_IN_1_1
gboolean adw_flap_get_swipe_to_open (AdwFlap  *self);
ADW_AVAILABLE_IN_1_1
void     adw_flap_set_swipe_to_open (AdwFlap  *self,
                                     gboolean  swipe_to_open);

ADW_AVAILABLE_IN_1_1
gboolean adw_flap_get_swipe_to_close (AdwFlap  *self);
ADW_AVAILABLE_IN_1_1
void     adw_flap_set_swipe_to_close (AdwFlap  *self,
                                      gboolean  swipe_to_close);

G_END_DECLS
