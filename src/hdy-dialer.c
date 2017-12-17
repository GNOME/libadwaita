/* hdy-dialer.c
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

#include <glib/gi18n.h>

#include "hdy-dialer.h"
#include "hdy-dialer-button.h"


/**
 * SECTION:hdy-dialer
 * @short_description: A dialer keypad for phone numbers
 * @Title: HdyDialer
 *
 * The #HdyDialer widget is a keypad for dialing mobile phone numbers.
 */


typedef struct
{
  HdyDialerButton *btn_0, *btn_1, *btn_2, *btn_3, *btn_4, *btn_5, *btn_6, *btn_7, *btn_8, *btn_9;
  HdyDialerButton *btn_hash, *btn_star;
  GtkLabel *display;
  GtkButton *btn_dial, *btn_del;
  GString *number;
} HdyDialerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyDialer, hdy_dialer, GTK_TYPE_GRID)


enum {
    HDY_DIALER_PROP_0 = 0,
    HDY_DIALER_PROP_NUMBER,
    HDY_DIALER_PROP_LAST_PROP,
};
static GParamSpec *props[HDY_DIALER_PROP_LAST_PROP] = { NULL, };

enum {
  DIALED = 0,
  LAST_SIGNAL,
};
static guint signals [LAST_SIGNAL];


static void
hdy_digit_button_clicked (HdyDialer *self,  HdyDialerButton *btn)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);
  int d;

  g_return_if_fail (HDY_IS_DIALER (self));
  g_return_if_fail (HDY_IS_DIALER_BUTTON (btn));

  d = hdy_dialer_button_get_digit(btn);
  g_string_append_printf(priv->number, "%d", d);
  g_object_notify_by_pspec (G_OBJECT (self), props[HDY_DIALER_PROP_NUMBER]);
}


static void
hdy_symbol_button_clicked (HdyDialer *self,  HdyDialerButton *btn)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);
  char* symbols;

  g_return_if_fail (HDY_IS_DIALER (self));
  g_return_if_fail (HDY_IS_DIALER_BUTTON (btn));

  symbols = hdy_dialer_button_get_letters(btn);
  g_string_append_printf(priv->number, "%c", symbols[0]);
  g_object_notify_by_pspec (G_OBJECT (self), props[HDY_DIALER_PROP_NUMBER]);
}


static void
hdy_dial_button_clicked (HdyDialer *self, GtkButton *btn)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);
  const char *number;

  g_return_if_fail (HDY_IS_DIALER (self));
  g_return_if_fail (GTK_IS_BUTTON (btn));

  number = gtk_label_get_label(priv->display);
  g_signal_emit (self, signals[DIALED], 0, number);
}


static void
hdy_del_button_clicked (HdyDialer *self, GtkButton *btn)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);

  g_return_if_fail (HDY_IS_DIALER (self));
  g_return_if_fail (GTK_IS_BUTTON (btn));

  if (!priv->number->len)
    return;

  g_string_truncate(priv->number, priv->number->len - 1);
  g_object_notify_by_pspec (G_OBJECT (self), props[HDY_DIALER_PROP_NUMBER]);
}


static void
hdy_update_label (HdyDialer *self, gpointer data)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);

  g_return_if_fail (HDY_IS_DIALER (self));

  gtk_label_set_label (priv->display, priv->number->str);
}


static void
hdy_dialer_finalize (GObject *object)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (HDY_DIALER(object));
  GObjectClass *parent_class = G_OBJECT_CLASS (hdy_dialer_parent_class);

  g_string_free (priv->number, TRUE);

  if (parent_class->finalize != NULL)
    parent_class->finalize (object);
}


static void
hdy_dialer_set_property (GObject *object,
			 guint property_id,
			 const GValue *value,
			 GParamSpec *pspec)
{
  HdyDialer *self = HDY_DIALER (object);
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private(self);

  switch (property_id) {

  case HDY_DIALER_PROP_NUMBER:
    g_string_overwrite(priv->number, 0, g_value_get_string(value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
hdy_dialer_get_property (GObject *object,
			 guint property_id,
			 GValue *value,
			 GParamSpec *pspec)
{
  HdyDialer *self = HDY_DIALER (object);
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private(self);

  switch (property_id) {
  case HDY_DIALER_PROP_NUMBER:
    g_value_set_string (value, priv->number->str);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
hdy_dialer_class_init (HdyDialerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hdy_dialer_finalize;

  object_class->set_property = hdy_dialer_set_property;
  object_class->get_property = hdy_dialer_get_property;

  /**
   * HdyDialer::dialed:
   * @self: The #HdyDialer instance.
   * @number: The number at the time of activation.
   *
   * This signal is emitted when the dialer's dial button is activated.
   * Connect to this signal to perform the dialing.
   */
  signals [DIALED] =
    g_signal_new ("dialed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (HdyDialerClass, dialed),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);


  props[HDY_DIALER_PROP_NUMBER] = g_param_spec_string ("number",
						       "phone number",
						       "phone number to dial",
						       "",
						       G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, HDY_DIALER_PROP_LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
					       "/org/sigxcpu/handy/dialer/ui/hdy-dialer.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_0);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_1);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_2);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_3);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_4);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_5);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_6);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_7);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_8);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_9);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_hash);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_star);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, display);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_dial);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialer, btn_del);
}


/**
 * hdy_dialer_new:
 *
 * Create a new #HdyDialer widget.
 *
 * Returns: the newly created #HdyDialer widget
 *
 */
GtkWidget *hdy_dialer_new (void)
{
  return g_object_new (HDY_TYPE_DIALER, NULL);
}


static void
hdy_dialer_init (HdyDialer *self)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);
  GtkWidget *image;

  gtk_widget_init_template (GTK_WIDGET (self));
  g_signal_connect_object (priv->btn_0,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_1,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_2,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_3,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_4,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_5,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_6,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_7,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_8,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_9,
                           "clicked",
                           G_CALLBACK (hdy_digit_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_star,
                           "clicked",
                           G_CALLBACK (hdy_symbol_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_hash,
                           "clicked",
                           G_CALLBACK (hdy_symbol_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_dial,
                           "clicked",
                           G_CALLBACK (hdy_dial_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->btn_del,
			   "clicked",
                           G_CALLBACK (hdy_del_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (self,
			   "notify::number",
			   G_CALLBACK (hdy_update_label),
			   NULL, 0);

  /* In GTK+4 we can just use the icon-name property */
  image = gtk_image_new_from_icon_name ("edit-clear-symbolic",
					GTK_ICON_SIZE_BUTTON);
  gtk_button_set_image(priv->btn_del, image);

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (),
                                    "/org/sigxcpu/handy/icons");
  image = gtk_image_new_from_icon_name ("phone-dial-symbolic",
					GTK_ICON_SIZE_BUTTON * 1.3);
  gtk_button_set_image(priv->btn_dial, image);

  priv->number = g_string_new(NULL);
}


/**
 * hdy_dialer_get_number:
 * @self: a #HdyDialer
 *
 * Get the currently displayed number.
 *
 * Returns: (transfer none): the current number in the display
 */
const char*
hdy_dialer_get_number(HdyDialer *self)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);
  return priv->number->str;
}


/**
 * hdy_dialer_set_number:
 * @self: a #HdyDialer
 * @newnumber: (transfer none): the number to set
 *
 * Set the currently displayed number.
 *
 */
void
hdy_dialer_set_number(HdyDialer *self, const char *newnumber)
{
  HdyDialerPrivate *priv = hdy_dialer_get_instance_private (self);

  g_string_overwrite(priv->number, 0, newnumber);
  g_object_notify_by_pspec (G_OBJECT (self), props[HDY_DIALER_PROP_NUMBER]);
}
