/* hdy-dialyer.h
 *
 * Copyright (C) 2017 Guido Günther <agx@sigxcpu.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HDY_DIALER_H
#define HDY_DIALER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_DIALER (hdy_dialer_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDialer, hdy_dialer, HDY, DIALER, GtkGrid)

/**
 * HdyDialerClass
 * @parent_class: The parent classqn
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

GtkWidget *      hdy_dialer_new                   (void);
const char *     hdy_dialer_get_number            (HdyDialer *self);
void             hdy_dialer_set_number            (HdyDialer *self,
						   const char* newnumber);

G_END_DECLS

#endif /* HDY_DIALER_H */
