/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#if !defined(_HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include "hdy-deprecation-macros.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_DIALER (hdy_dialer_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDialer, hdy_dialer, HDY, DIALER, GtkBin)

/**
 * HdyDialerClass:
 * @parent_class: The parent class
 * @submitted: Class handler for the #HdyDialer::submitted signal
 */
_HDY_DEPRECATED
struct _HdyDialerClass
{
  GtkBinClass parent_class;

  /* Signals
   */
  void (*submitted)   (HdyDialer    *self,
                       const gchar  *number);
};

_HDY_DEPRECATED
GtkWidget       *hdy_dialer_new                     (void);
_HDY_DEPRECATED
const gchar     *hdy_dialer_get_number              (HdyDialer  *self);
_HDY_DEPRECATED
void             hdy_dialer_set_number              (HdyDialer  *self,
                                                     const char *number);
_HDY_DEPRECATED
void             hdy_dialer_clear_number            (HdyDialer  *self);

_HDY_DEPRECATED
gboolean         hdy_dialer_get_show_action_buttons (HdyDialer  *self);
_HDY_DEPRECATED
void             hdy_dialer_set_show_action_buttons (HdyDialer  *self,
                                                     gboolean    show);

_HDY_DEPRECATED
GtkReliefStyle   hdy_dialer_get_relief              (HdyDialer *self);
_HDY_DEPRECATED
void             hdy_dialer_set_relief              (HdyDialer      *self,
                                                     GtkReliefStyle  relief);

G_END_DECLS
