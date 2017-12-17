/* hdy-dialyer-button.h
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

#ifndef HDY_DIALER_BUTTON_H
#define HDY_DIALER_BUTTON_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define HDY_TYPE_DIALER_BUTTON (hdy_dialer_button_get_type())

G_DECLARE_DERIVABLE_TYPE (HdyDialerButton, hdy_dialer_button, HDY, DIALER_BUTTON, GtkButton)

struct _HdyDialerButtonClass
{
  GtkButtonClass parent_class;
};

GtkWidget *     hdy_dialer_button_new                   (int digit,
							 const gchar* letters);
gint            hdy_dialer_button_get_digit             (HdyDialerButton *self);
char *          hdy_dialer_button_get_letters           (HdyDialerButton *self);

G_END_DECLS

#endif /* HDY_DIALER_BUTTON_H */
