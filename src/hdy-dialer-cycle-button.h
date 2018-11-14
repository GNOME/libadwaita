/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-dialer-button.h"

G_BEGIN_DECLS

#define HDY_TYPE_DIALER_CYCLE_BUTTON (hdy_dialer_cycle_button_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDialerCycleButton, hdy_dialer_cycle_button, HDY, DIALER_CYCLE_BUTTON, HdyDialerButton)

/**
 * HdyDialerCycleButtonClass:
 * @parent_class: The parent classqn
 * @cycle_start: Class handler for the #HdyDialerCycleButton::cycle-start signal
 * @cycle_end: Class handler for the #HdyDialerCycleButton::cycle-end signal
 */
struct _HdyDialerCycleButtonClass
{
  HdyDialerButtonClass parent_class;

  /* Signals */
  void (*cycle_start)   (HdyDialerCycleButton    *self);
  void (*cycle_end)     (HdyDialerCycleButton    *self);
};

GtkWidget     *hdy_dialer_cycle_button_new                   (const gchar          *symbols);
gunichar       hdy_dialer_cycle_button_get_current_symbol    (HdyDialerCycleButton *self);
gboolean       hdy_dialer_cycle_button_is_cycling            (HdyDialerCycleButton *self);
void           hdy_dialer_cycle_button_stop_cycle            (HdyDialerCycleButton *self);
gint           hdy_dialer_cycle_button_get_cycle_timeout     (HdyDialerCycleButton *self);
void           hdy_dialer_cycle_button_set_cycle_timeout     (HdyDialerCycleButton *self,
                                                              gint                  timeout);

G_END_DECLS
