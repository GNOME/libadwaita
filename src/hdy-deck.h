/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-version.h"

#include <gtk/gtk.h>
#include "hdy-navigation-direction.h"

G_BEGIN_DECLS

#define HDY_TYPE_DECK (hdy_deck_get_type())

HDY_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (HdyDeck, hdy_deck, HDY, DECK, GtkContainer)

typedef enum {
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

  /*< private >*/
  gpointer padding[4];
};

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_deck_new (void);
HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_deck_get_visible_child (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_visible_child (HdyDeck   *self,
                                             GtkWidget *visible_child);
HDY_AVAILABLE_IN_ALL
const gchar     *hdy_deck_get_visible_child_name (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_visible_child_name (HdyDeck     *self,
                                                  const gchar *name);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_deck_get_homogeneous (HdyDeck        *self,
                                           GtkOrientation  orientation);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_homogeneous (HdyDeck        *self,
                                           GtkOrientation  orientation,
                                           gboolean        homogeneous);
HDY_AVAILABLE_IN_ALL
HdyDeckTransitionType hdy_deck_get_transition_type (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_transition_type (HdyDeck               *self,
                                               HdyDeckTransitionType  transition);

HDY_AVAILABLE_IN_ALL
guint            hdy_deck_get_transition_duration (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_transition_duration (HdyDeck *self,
                                                   guint    duration);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_deck_get_transition_running (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_deck_get_interpolate_size (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_interpolate_size (HdyDeck  *self,
                                                gboolean  interpolate_size);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_deck_get_can_swipe_back (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_can_swipe_back (HdyDeck  *self,
                                              gboolean  can_swipe_back);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_deck_get_can_swipe_forward (HdyDeck *self);
HDY_AVAILABLE_IN_ALL
void             hdy_deck_set_can_swipe_forward (HdyDeck  *self,
                                                 gboolean  can_swipe_forward);

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_deck_get_adjacent_child (HdyDeck                *self,
                                              HdyNavigationDirection  direction);
HDY_AVAILABLE_IN_ALL
gboolean         hdy_deck_navigate (HdyDeck                *self,
                                    HdyNavigationDirection  direction);

HDY_AVAILABLE_IN_ALL
GtkWidget       *hdy_deck_get_child_by_name (HdyDeck     *self,
                                             const gchar *name);

G_END_DECLS
