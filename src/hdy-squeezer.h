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
#include "hdy-enums.h"

G_BEGIN_DECLS

#define HDY_TYPE_SQUEEZER (hdy_squeezer_get_type ())

G_DECLARE_DERIVABLE_TYPE (HdySqueezer, hdy_squeezer, HDY, SQUEEZER, GtkContainer)

typedef enum {
  HDY_SQUEEZER_TRANSITION_TYPE_NONE,
  HDY_SQUEEZER_TRANSITION_TYPE_CROSSFADE,
} HdySqueezerTransitionType;

/**
 * HdySqueezerClass
 * @parent_class: The parent class
 */
struct _HdySqueezerClass
{
  GtkContainerClass parent_class;
};

HdySqueezer *hdy_squeezer_new (void);

gboolean hdy_squeezer_get_homogeneous (HdySqueezer *self);
void     hdy_squeezer_set_homogeneous (HdySqueezer *self,
                                       gboolean     homogeneous);

guint hdy_squeezer_get_transition_duration (HdySqueezer *self);
void  hdy_squeezer_set_transition_duration (HdySqueezer *self,
                                            guint        duration);

HdySqueezerTransitionType hdy_squeezer_get_transition_type (HdySqueezer *self);
void                      hdy_squeezer_set_transition_type (HdySqueezer               *self,
                                                            HdySqueezerTransitionType  transition);

gboolean hdy_squeezer_get_transition_running (HdySqueezer *self);

gboolean hdy_squeezer_get_interpolate_size (HdySqueezer *self);
void     hdy_squeezer_set_interpolate_size (HdySqueezer *self,
                                            gboolean     interpolate_size);

GtkWidget *hdy_squeezer_get_visible_child (HdySqueezer *self);

gboolean hdy_squeezer_get_child_enabled (HdySqueezer *self,
                                         GtkWidget   *child);
void     hdy_squeezer_set_child_enabled (HdySqueezer *self,
                                         GtkWidget   *child,
                                         gboolean     enabled);

G_END_DECLS
