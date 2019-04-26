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

typedef enum
{
  HDY_ARROWS_DIRECTION_UP,
  HDY_ARROWS_DIRECTION_DOWN,
  HDY_ARROWS_DIRECTION_LEFT,
  HDY_ARROWS_DIRECTION_RIGHT,
} HdyArrowsDirection;

#define HDY_TYPE_ARROWS (hdy_arrows_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyArrows, hdy_arrows, HDY, ARROWS, GtkDrawingArea)

/**
 * HdyArrowsClass:
 * @parent_class: The parent class
 */
struct _HdyArrowsClass
{
  GtkDrawingAreaClass parent_class;
};


GtkWidget         *hdy_arrows_new                  (void);
guint              hdy_arrows_get_count            (HdyArrows  *self);
void               hdy_arrows_set_count            (HdyArrows  *self,
                                                    guint       count);
void               hdy_arrows_set_direction        (HdyArrows  *self,
                                                    HdyArrowsDirection direction);
HdyArrowsDirection hdy_arrows_get_direction        (HdyArrows  *self);
void               hdy_arrows_set_duration         (HdyArrows  *self,
                                                    guint duration);
guint              hdy_arrows_get_duration         (HdyArrows  *self);
void               hdy_arrows_animate              (HdyArrows  *self);

G_END_DECLS
