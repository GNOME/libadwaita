/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include "hdy-navigation-direction.h"

G_BEGIN_DECLS

#define HDY_TYPE_DECK (hdy_deck_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDeck, hdy_deck, HDY, DECK, GtkContainer)

typedef enum {
  HDY_DECK_TRANSITION_TYPE_NONE,
  HDY_DECK_TRANSITION_TYPE_OVER,
  HDY_DECK_TRANSITION_TYPE_UNDER,
  HDY_DECK_TRANSITION_TYPE_SLIDE,
} HdyDeckTransitionType;

/**
 * HdyDeckClass
 * @parent_class: The parent class
 */
struct _HdyDeckClass
{
  GtkContainerClass parent_class;
};

GtkWidget       *hdy_deck_new (void);
GtkWidget       *hdy_deck_get_visible_child (HdyDeck *self);
void             hdy_deck_set_visible_child (HdyDeck   *self,
                                             GtkWidget *visible_child);
const gchar     *hdy_deck_get_visible_child_name (HdyDeck *self);
void             hdy_deck_set_visible_child_name (HdyDeck     *self,
                                                  const gchar *name);
gboolean         hdy_deck_get_homogeneous (HdyDeck        *self,
                                           GtkOrientation  orientation);
void             hdy_deck_set_homogeneous (HdyDeck        *self,
                                           GtkOrientation  orientation,
                                           gboolean        homogeneous);
HdyDeckTransitionType hdy_deck_get_transition_type (HdyDeck *self);
void             hdy_deck_set_transition_type (HdyDeck               *self,
                                               HdyDeckTransitionType  transition);

guint            hdy_deck_get_transition_duration (HdyDeck *self);
void             hdy_deck_set_transition_duration (HdyDeck *self,
                                                   guint    duration);
gboolean         hdy_deck_get_transition_running (HdyDeck *self);
gboolean         hdy_deck_get_interpolate_size (HdyDeck *self);
void             hdy_deck_set_interpolate_size (HdyDeck  *self,
                                                gboolean  interpolate_size);
gboolean         hdy_deck_get_can_swipe_back (HdyDeck *self);
void             hdy_deck_set_can_swipe_back (HdyDeck  *self,
                                              gboolean  can_swipe_back);
gboolean         hdy_deck_get_can_swipe_forward (HdyDeck *self);
void             hdy_deck_set_can_swipe_forward (HdyDeck  *self,
                                                 gboolean  can_swipe_forward);

GtkWidget       *hdy_deck_get_adjacent_child (HdyDeck                *self,
                                              HdyNavigationDirection  direction);
gboolean         hdy_deck_navigate (HdyDeck                *self,
                                    HdyNavigationDirection  direction);

G_END_DECLS
