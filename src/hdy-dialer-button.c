/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-3.0+
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
  GtkLabel *label, *secondary_label;
  gint digit;
  gchar *letters;
} HdyDialerButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyDialerButton, hdy_dialer_button, GTK_TYPE_BUTTON)


void
format_label(HdyDialerButton *self)
{
  HdyDialerButtonPrivate *priv = hdy_dialer_button_get_instance_private(self);
  GString *str;
  g_autofree gchar *text;

  str = g_string_new(NULL);
  if (priv->digit >= 0) {
    g_string_sprintf (str, "%d", priv->digit);
  }

  text = g_string_free (str, FALSE);

  gtk_label_set_label (priv->label, text);
  gtk_label_set_label (priv->secondary_label, priv->letters);
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
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

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

  gtk_widget_class_set_template_from_resource (widget_class,
					       "/sm/puri/handy/dialer/ui/hdy-dialer-button.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialerButton, label);
  gtk_widget_class_bind_template_child_private (widget_class, HdyDialerButton, secondary_label);
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

  gtk_widget_init_template (GTK_WIDGET (self));

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
