/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-keypad-button-private.h"

/**
 * PRIVATE:adw-keypad-button
 * @short_description: A button on a #AdwKeypad keypad
 * @Title: AdwKeypadButton
 *
 * The #AdwKeypadButton widget is a single button on an #AdwKeypad. It
 * can represent a single symbol (typically a digit) plus an arbitrary
 * number of symbols that are displayed below it.
 */

enum {
  PROP_0,
  PROP_DIGIT,
  PROP_SYMBOLS,
  PROP_SHOW_SYMBOLS,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

struct _AdwKeypadButton
{
  GtkButton parent_instance;

  GtkWidget *box;
  GtkLabel *label;
  GtkLabel *secondary_label;
  gchar *symbols;
};

G_DEFINE_TYPE (AdwKeypadButton, adw_keypad_button, GTK_TYPE_BUTTON)

static void
format_label(AdwKeypadButton *self)
{
  g_autofree gchar *text = NULL;
  gchar *secondary_text = NULL;

  if (self->symbols != NULL && *(self->symbols) != '\0') {
    secondary_text = g_utf8_find_next_char (self->symbols, NULL);
    text = g_strndup (self->symbols, 1);
  }

  gtk_label_set_label (self->label, text);
  gtk_label_set_label (self->secondary_label, secondary_text);
}

static void
adw_keypad_button_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwKeypadButton *self = ADW_KEYPAD_BUTTON (object);

  switch (property_id) {
  case PROP_SYMBOLS:
    if (g_strcmp0 (self->symbols, g_value_get_string (value)) != 0) {
      g_free (self->symbols);
      self->symbols = g_value_dup_string (value);
      format_label(self);
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYMBOLS]);
    }
    break;

  case PROP_SHOW_SYMBOLS:
    adw_keypad_button_show_symbols (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_keypad_button_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwKeypadButton *self = ADW_KEYPAD_BUTTON (object);

  switch (property_id) {
  case PROP_DIGIT:
    g_value_set_schar (value, adw_keypad_button_get_digit (self));
    break;

  case PROP_SYMBOLS:
    g_value_set_string (value, adw_keypad_button_get_symbols (self));
    break;

  case PROP_SHOW_SYMBOLS:
    g_value_set_boolean (value, gtk_widget_is_visible (GTK_WIDGET (self->secondary_label)));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_keypad_button_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           int             for_size,
                           int            *minimum,
                           int            *natural,
                           int            *minimum_baseline,
                           int            *natural_baseline)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (adw_keypad_button_parent_class);
  gint min1, min2, nat1, nat2;

  if (for_size < 0) {
    widget_class->measure (widget, GTK_ORIENTATION_HORIZONTAL, -1, &min1, &nat1, NULL, NULL);
    widget_class->measure (widget, GTK_ORIENTATION_VERTICAL, -1, &min2, &nat2, NULL, NULL);
  } else {
    widget_class->measure (widget, orientation, for_size, &min1, &nat1, NULL, NULL);
    min2 = nat2 = for_size;
  }

  if (minimum)
    *minimum = MAX (min1, min2);
  if (natural)
    *natural = MAX (nat1, nat2);
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static GtkSizeRequestMode
adw_keypad_button_get_request_mode (GtkWidget *widget)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (adw_keypad_button_parent_class);
  gint min1, min2;

  widget_class->measure (widget, GTK_ORIENTATION_HORIZONTAL, -1, &min1, NULL, NULL, NULL);
  widget_class->measure (widget, GTK_ORIENTATION_VERTICAL, -1, &min2, NULL, NULL, NULL);

  if (min1 < min2)
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
  else
    return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
adw_keypad_button_dispose (GObject *object)
{
  AdwKeypadButton *self = ADW_KEYPAD_BUTTON (object);

  g_clear_pointer (&self->box, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_keypad_button_parent_class)->dispose (object);
}

static void
adw_keypad_button_finalize (GObject *object)
{
  AdwKeypadButton *self = ADW_KEYPAD_BUTTON (object);

  g_clear_pointer (&self->symbols, g_free);

  G_OBJECT_CLASS (adw_keypad_button_parent_class)->finalize (object);
}

static void
adw_keypad_button_class_init (AdwKeypadButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = adw_keypad_button_set_property;
  object_class->get_property = adw_keypad_button_get_property;
  object_class->dispose = adw_keypad_button_dispose;
  object_class->finalize = adw_keypad_button_finalize;

  widget_class->get_request_mode = adw_keypad_button_get_request_mode;
  widget_class->measure = adw_keypad_button_measure;

  props[PROP_DIGIT] =
    g_param_spec_int ("digit",
                      _("Digit"),
                      _("The keypad digit of the button"),
                      -1, INT_MAX, 0,
                      G_PARAM_READABLE);

  props[PROP_SYMBOLS] =
    g_param_spec_string ("symbols",
                         _("Symbols"),
                         _("The keypad symbols of the button. The first symbol is used as the digit"),
                         "",
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_SHOW_SYMBOLS] =
    g_param_spec_boolean ("show-symbols",
                         _("Show symbols"),
                         _("Whether the second line of symbols should be shown or not"),
                         TRUE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-keypad-button.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwKeypadButton, box);
  gtk_widget_class_bind_template_child (widget_class, AdwKeypadButton, label);
  gtk_widget_class_bind_template_child (widget_class, AdwKeypadButton, secondary_label);
}

static void
adw_keypad_button_init (AdwKeypadButton *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->symbols = NULL;
}

/**
 * adw_keypad_button_new:
 * @symbols: (nullable): the symbols displayed on the #AdwKeypadButton
 *
 * Create a new #AdwKeypadButton which displays @symbols,
 * where the first char is used as the main and the other symbols are shown below
 *
 * Returns: the newly created #AdwKeypadButton widget
 */
GtkWidget *
adw_keypad_button_new (const gchar *symbols)
{
  return g_object_new (ADW_TYPE_KEYPAD_BUTTON, "symbols", symbols, NULL);
}

/**
 * adw_keypad_button_get_digit:
 * @self: a #AdwKeypadButton
 *
 * Get the #AdwKeypadButton's digit.
 *
 * Returns: the button's digit
 */
char
adw_keypad_button_get_digit (AdwKeypadButton *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD_BUTTON (self), '\0');

  if (self->symbols == NULL)
    return ('\0');

  return *(self->symbols);
}

/**
 * adw_keypad_button_get_symbols:
 * @self: a #AdwKeypadButton
 *
 * Get the #AdwKeypadButton's symbols.
 *
 * Returns: the button's symbols including the digit.
 */
const char*
adw_keypad_button_get_symbols (AdwKeypadButton *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD_BUTTON (self), NULL);

  return self->symbols;
}

/**
 * adw_keypad_button_show_symbols:
 * @self: a #AdwKeypadButton
 * @visible: whether the second line should be shown or not
 *
 * Sets the visibility of the second line of symbols for #AdwKeypadButton
 *
 */
void
adw_keypad_button_show_symbols (AdwKeypadButton *self, gboolean visible)
{
  gboolean old_visible;

  g_return_if_fail (ADW_IS_KEYPAD_BUTTON (self));

  old_visible = gtk_widget_get_visible (GTK_WIDGET (self->secondary_label));

  if (old_visible != visible) {
    gtk_widget_set_visible (GTK_WIDGET (self->secondary_label), visible);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_SYMBOLS]);
  }
}
