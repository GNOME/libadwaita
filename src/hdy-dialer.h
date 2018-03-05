/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef HDY_DIALER_H
#define HDY_DIALER_H

#if !defined(HANDY_INSIDE) && !defined(HANDY_COMPILATION)
#error "Only <handy.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_DIALER (hdy_dialer_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDialer, hdy_dialer, HDY, DIALER, GtkGrid)

/**
 * HdyDialerClass:
 * @parent_class: The parent class
 * @dialed: Class handler for the #HdyDialer::dialed signal
 */
struct _HdyDialerClass
{
  GtkButtonClass parent_class;

  /* Signals
   */
  void (*dialed)   (HdyDialer    *self,
                    const gchar  *number);
};

GtkWidget       *hdy_dialer_new                   (void);
const char      *hdy_dialer_get_number            (HdyDialer  *self);
void             hdy_dialer_set_number            (HdyDialer  *self,
                                                   const char *newnumber);

G_END_DECLS

#endif /* HDY_DIALER_H */
