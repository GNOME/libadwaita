/* hdy-dialer-button.c
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

#include "hdy-dialer-button.h"

/**
 * SECTION:hdy-dialer-button
 * @short_description: A button on a #HdyDialer keypad
 * @Title: HdyDialerButton
 *
 * The #HdyDialerButton widget is a single button on an #HdyDialer. It
 * can represent a single digit (0-9) plus an arbitrary number of
 * letters that are displayed below the number.
 */


enum {
    HDY_DIALER_BUTTON_PROP_0 = 0,
    HDY_DIALER_BUTTON_PROP_DIGIT = 1,
    HDY_DIALER_BUTTON_PROP_LETTERS = 2,
};


typedef struct
{
  gint digit;
  gchar *letters;
} HdyDialerButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyDialerButton, hdy_dialer_button, GTK_TYPE_BUTTON)


void
format_label(HdyDialerButton *self)
{
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);
  GString *str;
  GtkLabel *label;
  g_autofree gchar *text;

  str = g_string_new(NULL);
  if (priv->digit >= 0) {
    g_string_sprintf(str, "%d", priv->digit);
    if (priv->letters)
      g_string_append_c(str, '\n');
  }

  if (priv->letters)
    g_string_append(str, priv->letters);

  text = g_string_free (str, FALSE);
  gtk_button_set_label (GTK_BUTTON(self), text);

  label = GTK_LABEL(gtk_bin_get_child (GTK_BIN (self)));
  gtk_label_set_xalign(label, 0.5);
  gtk_label_set_yalign(label, 0.5);
  gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_CENTER);
}


static void
hdy_dialer_button_set_property (GObject *object,
				guint property_id,
				const GValue *value,
				GParamSpec *pspec)
{
  HdyDialerButton *self = HDY_DIALER_BUTTON (object);
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);

  switch (property_id) {
  case HDY_DIALER_BUTTON_PROP_DIGIT:
    priv->digit = g_value_get_int (value);
    format_label(self);
    break;

  case HDY_DIALER_BUTTON_PROP_LETTERS:
    g_free (priv->letters);
    priv->letters = g_value_dup_string (value);
    format_label(self);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
hdy_dialer_button_get_property (GObject *object,
			       guint property_id,
			       GValue *value,
			       GParamSpec *pspec)
{
  HdyDialerButton *self = HDY_DIALER_BUTTON (object);
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);

  switch (property_id) {
  case HDY_DIALER_BUTTON_PROP_DIGIT:
    g_value_set_int (value, priv->digit);
    break;

  case HDY_DIALER_BUTTON_PROP_LETTERS:
    g_value_set_string (value, priv->letters);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
hdy_dialer_button_class_init (HdyDialerButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = hdy_dialer_button_set_property;
  object_class->get_property = hdy_dialer_button_get_property;

  g_object_class_install_property (object_class,
				   HDY_DIALER_BUTTON_PROP_DIGIT,
				   g_param_spec_int ("digit",
						     "dialer digit",
						     "dialer digit",
						     -1,
						     INT_MAX,
						     0,
						     G_PARAM_READWRITE));
  g_object_class_install_property (object_class,
				   HDY_DIALER_BUTTON_PROP_LETTERS,
				   g_param_spec_string ("letters",
							"dialer letters",
							"dialer letters",
							"",
							G_PARAM_READWRITE));
}


/**
 * hdy_dialer_button_new:
 * @digit: the digit displayed on the #HdyDialerButton
 * @letters: (nullable): the letters displayed on the #HdyDialerButton
 *
 * Create a new #HdyDialerButton which displays @digit and
 * @letters. If @digit is a negative no number will be displayed. If
 * @letters is %NULL no letters will be displayed.
 *
 * Returns: the newly created #HdyDialerButton widget
 */
GtkWidget *hdy_dialer_button_new (int digit, const gchar* letters)
{
  return g_object_new (HDY_TYPE_DIALER_BUTTON, "digit", digit, "letters", letters, NULL);
}


static void
hdy_dialer_button_init (HdyDialerButton *self)
{
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);

  priv->digit = -1;
  priv->letters = NULL;
}


/**
 * hdy_dialer_button_get_digit:
 * @self: a #HdyDialerButton
 *
 * Get the #HdyDialerButton's digit.
 *
 * Returns: the button's digit
 */
gint
hdy_dialer_button_get_digit(HdyDialerButton *self)
{
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);

  g_return_val_if_fail (HDY_IS_DIALER_BUTTON (self), -1);
  return priv->digit;
}

/**
 * hdy_dialer_button_get_letters:
 * @self: a #HdyDialerButton
 *
 * Get the #HdyDialerButton's letters.
 *
 * Returns: the button's letters.
 */
char*
hdy_dialer_button_get_letters(HdyDialerButton *self)
{
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);

  g_return_val_if_fail (HDY_IS_DIALER_BUTTON (self), NULL);
  return priv->letters;
}

