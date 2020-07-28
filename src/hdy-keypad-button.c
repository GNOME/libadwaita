/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-keypad-button-private.h"

/**
 * PRIVATE:hdy-keypad-button
 * @short_description: A button on a #HdyKeypad keypad
 * @Title: HdyKeypadButton
 *
 * The #HdyKeypadButton widget is a single button on an #HdyKeypad. It
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

struct _HdyKeypadButton
{
  GtkButton parent_instance;

  GtkLabel *label, *secondary_label;
  gchar *symbols;
};

G_DEFINE_TYPE (HdyKeypadButton, hdy_keypad_button, GTK_TYPE_BUTTON)

static void
format_label(HdyKeypadButton *self)
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
hdy_keypad_button_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdyKeypadButton *self = HDY_KEYPAD_BUTTON (object);

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
    hdy_keypad_button_show_symbols (self, g_value_get_boolean (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_keypad_button_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdyKeypadButton *self = HDY_KEYPAD_BUTTON (object);

  switch (property_id) {
  case PROP_DIGIT:
    g_value_set_schar (value, hdy_keypad_button_get_digit (self));
    break;

  case PROP_SYMBOLS:
    g_value_set_string (value, hdy_keypad_button_get_symbols (self));
    break;

  case PROP_SHOW_SYMBOLS:
    g_value_set_boolean (value, gtk_widget_is_visible (GTK_WIDGET (self->secondary_label)));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

/* This private method is prefixed by the call name because it will be a virtual
 * method in GTK 4.
 */
static void
hdy_keypad_button_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           int             for_size,
                           int            *minimum,
                           int            *natural,
                           int            *minimum_baseline,
                           int            *natural_baseline)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (hdy_keypad_button_parent_class);
  gint min1, min2, nat1, nat2;

  if (for_size < 0) {
    widget_class->get_preferred_width (widget, &min1, &nat1);
    widget_class->get_preferred_height (widget, &min2, &nat2);
  }
  else {
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      widget_class->get_preferred_width_for_height (widget, for_size, &min1, &nat1);
    else
      widget_class->get_preferred_height_for_width (widget, for_size, &min1, &nat1);
    min2 = nat2 = for_size;
  }

  if (minimum)
    *minimum = MAX (min1, min2);
  if (natural)
    *natural = MAX (nat1, nat2);
}

static GtkSizeRequestMode
hdy_keypad_button_get_request_mode (GtkWidget *widget)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (hdy_keypad_button_parent_class);
  gint min1, min2;
  widget_class->get_preferred_width (widget, &min1, NULL);
  widget_class->get_preferred_height (widget, &min2, NULL);
  if (min1 < min2)
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
  else
    return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
}

static void
hdy_keypad_button_get_preferred_width (GtkWidget *widget,
                                       gint      *minimum_width,
                                       gint      *natural_width)
{
  hdy_keypad_button_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                             minimum_width, natural_width, NULL, NULL);
}

static void
hdy_keypad_button_get_preferred_height (GtkWidget *widget,
                                        gint      *minimum_height,
                                        gint      *natural_height)
{
  hdy_keypad_button_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                             minimum_height, natural_height, NULL, NULL);
}

static void
hdy_keypad_button_get_preferred_width_for_height (GtkWidget *widget,
                                                  gint       height,
                                                  gint      *minimum_width,
                                                  gint      *natural_width)
{
  *minimum_width = height;
  *natural_width = height;
}

static void
hdy_keypad_button_get_preferred_height_for_width (GtkWidget *widget,
                                                  gint       width,
                                                  gint      *minimum_height,
                                                  gint      *natural_height)
{
  *minimum_height = width;
  *natural_height = width;
}


static void
hdy_keypad_button_finalize (GObject *object)
{
  HdyKeypadButton *self = HDY_KEYPAD_BUTTON (object);

  g_clear_pointer (&self->symbols, g_free);
  G_OBJECT_CLASS (hdy_keypad_button_parent_class)->finalize (object);
}


static void
hdy_keypad_button_class_init (HdyKeypadButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = hdy_keypad_button_set_property;
  object_class->get_property = hdy_keypad_button_get_property;

  object_class->finalize = hdy_keypad_button_finalize;

  widget_class->get_request_mode = hdy_keypad_button_get_request_mode;
  widget_class->get_preferred_width = hdy_keypad_button_get_preferred_width;
  widget_class->get_preferred_height = hdy_keypad_button_get_preferred_height;
  widget_class->get_preferred_width_for_height = hdy_keypad_button_get_preferred_width_for_height;
  widget_class->get_preferred_height_for_width = hdy_keypad_button_get_preferred_height_for_width;

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
                                               "/sm/puri/handy/ui/hdy-keypad-button.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyKeypadButton, label);
  gtk_widget_class_bind_template_child (widget_class, HdyKeypadButton, secondary_label);
}

static void
hdy_keypad_button_init (HdyKeypadButton *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->symbols = NULL;
}

/**
 * hdy_keypad_button_new:
 * @symbols: (nullable): the symbols displayed on the #HdyKeypadButton
 *
 * Create a new #HdyKeypadButton which displays @symbols,
 * where the first char is used as the main and the other symbols are shown below
 *
 * Returns: the newly created #HdyKeypadButton widget
 */
GtkWidget *
hdy_keypad_button_new (const gchar *symbols)
{
  return g_object_new (HDY_TYPE_KEYPAD_BUTTON, "symbols", symbols, NULL);
}

/**
 * hdy_keypad_button_get_digit:
 * @self: a #HdyKeypadButton
 *
 * Get the #HdyKeypadButton's digit.
 *
 * Returns: the button's digit
 */
char
hdy_keypad_button_get_digit (HdyKeypadButton *self)
{
  g_return_val_if_fail (HDY_IS_KEYPAD_BUTTON (self), '\0');

  if (self->symbols == NULL)
    return ('\0');

  return *(self->symbols);
}

/**
 * hdy_keypad_button_get_symbols:
 * @self: a #HdyKeypadButton
 *
 * Get the #HdyKeypadButton's symbols.
 *
 * Returns: the button's symbols including the digit.
 */
const char*
hdy_keypad_button_get_symbols (HdyKeypadButton *self)
{
  g_return_val_if_fail (HDY_IS_KEYPAD_BUTTON (self), NULL);

  return self->symbols;
}

/**
 * hdy_keypad_button_show_symbols:
 * @self: a #HdyKeypadButton
 * @visible: whether the second line should be shown or not
 *
 * Sets the visibility of the second line of symbols for #HdyKeypadButton
 *
 */
void
hdy_keypad_button_show_symbols (HdyKeypadButton *self, gboolean visible)
{
  gboolean old_visible;

  g_return_if_fail (HDY_IS_KEYPAD_BUTTON (self));

  old_visible = gtk_widget_get_visible (GTK_WIDGET (self->secondary_label));

  if (old_visible != visible) {
    gtk_widget_set_visible (GTK_WIDGET (self->secondary_label), visible);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_SYMBOLS]);
  }
}
