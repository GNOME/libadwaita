/*
 * Copyright (C) 2020 Felix Häcker <haeckerfelix@gnome.org>
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
#include "adw-fold-threshold-policy.h"

G_BEGIN_DECLS

#define ADW_TYPE_FLAP (adw_flap_get_type ())

ADW_AVAILABLE_IN_ALL
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

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_flap_new (void) G_GNUC_WARN_UNUSED_RESULT;

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_flap_get_content (AdwFlap   *self);
ADW_AVAILABLE_IN_ALL
void       adw_flap_set_content (AdwFlap   *self,
                                 GtkWidget *content);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_flap_get_flap (AdwFlap   *self);
ADW_AVAILABLE_IN_ALL
void       adw_flap_set_flap (AdwFlap   *self,
                              GtkWidget *flap);

ADW_AVAILABLE_IN_ALL
GtkWidget *adw_flap_get_separator (AdwFlap   *self);
ADW_AVAILABLE_IN_ALL
void       adw_flap_set_separator (AdwFlap   *self,
                                   GtkWidget *separator);

ADW_AVAILABLE_IN_ALL
GtkPackType adw_flap_get_flap_position (AdwFlap     *self);
ADW_AVAILABLE_IN_ALL
void        adw_flap_set_flap_position (AdwFlap     *self,
                                        GtkPackType  position);

ADW_AVAILABLE_IN_ALL
gboolean adw_flap_get_reveal_flap (AdwFlap  *self);
ADW_AVAILABLE_IN_ALL
void     adw_flap_set_reveal_flap (AdwFlap  *self,
                                   gboolean  reveal_flap);

ADW_AVAILABLE_IN_ALL
guint adw_flap_get_reveal_duration (AdwFlap *self);
ADW_AVAILABLE_IN_ALL
void  adw_flap_set_reveal_duration (AdwFlap *self,
                                    guint    duration);

ADW_AVAILABLE_IN_ALL
double adw_flap_get_reveal_progress (AdwFlap *self);

ADW_AVAILABLE_IN_ALL
AdwFlapFoldPolicy adw_flap_get_fold_policy (AdwFlap           *self);
ADW_AVAILABLE_IN_ALL
void              adw_flap_set_fold_policy (AdwFlap           *self,
                                            AdwFlapFoldPolicy  policy);

ADW_AVAILABLE_IN_ALL
guint adw_flap_get_fold_duration (AdwFlap *self);
ADW_AVAILABLE_IN_ALL
void  adw_flap_set_fold_duration (AdwFlap *self,
                                  guint    duration);

ADW_AVAILABLE_IN_ALL
gboolean adw_flap_get_folded (AdwFlap *self);

ADW_AVAILABLE_IN_ALL
gboolean adw_flap_get_locked (AdwFlap *self);
ADW_AVAILABLE_IN_ALL
void     adw_flap_set_locked (AdwFlap  *self,
                              gboolean  locked);

ADW_AVAILABLE_IN_ALL
AdwFlapTransitionType adw_flap_get_transition_type (AdwFlap               *self);
ADW_AVAILABLE_IN_ALL
void                  adw_flap_set_transition_type (AdwFlap               *self,
                                                    AdwFlapTransitionType  transition_type);

ADW_AVAILABLE_IN_ALL
gboolean adw_flap_get_modal (AdwFlap  *self);
ADW_AVAILABLE_IN_ALL
void     adw_flap_set_modal (AdwFlap  *self,
                             gboolean  modal);

ADW_AVAILABLE_IN_ALL
gboolean adw_flap_get_swipe_to_open (AdwFlap  *self);
ADW_AVAILABLE_IN_ALL
void     adw_flap_set_swipe_to_open (AdwFlap  *self,
                                     gboolean  swipe_to_open);

ADW_AVAILABLE_IN_ALL
gboolean adw_flap_get_swipe_to_close (AdwFlap  *self);
ADW_AVAILABLE_IN_ALL
void     adw_flap_set_swipe_to_close (AdwFlap  *self,
                                      gboolean  swipe_to_close);

ADW_AVAILABLE_IN_ALL
AdwFoldThresholdPolicy adw_flap_get_fold_threshold_policy (AdwFlap                *self);
ADW_AVAILABLE_IN_ALL
void                   adw_flap_set_fold_threshold_policy (AdwFlap                *self,
                                                           AdwFoldThresholdPolicy  policy);

G_END_DECLS
