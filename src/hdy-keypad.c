/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-keypad.h"
#include "hdy-keypad-button-private.h"

/**
 * SECTION:hdy-keypad
 * @short_description: A keypad for dialing numbers
 * @Title: HdyKeypad
 *
 * The #HdyKeypad widget is a keypad for entering numbers such as phone numbers
 * or PIN codes.
 *
 * This widget should not be altered using the #GtkGrid and #GtkContainer APIs,
 * they are considered internal to this widget, using them externally will lead to unexpected results.
 *
 * # CSS nodes
 *
 * #HdyKeypad has a single CSS node with name keypad.
 */

typedef struct
{
  GtkWidget *entry;
  GtkWidget *label_asterisk;
  GtkWidget *label_hash;
  GtkGesture *long_press_zero_gesture;
  gboolean only_digits;
  gboolean show_symbols;
} HdyKeypadPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyKeypad, hdy_keypad, GTK_TYPE_GRID)

enum {
  PROP_0,
  PROP_SHOW_SYMBOLS,
  PROP_ONLY_DIGITS,
  PROP_ENTRY,
  PROP_RIGHT_ACTION,
  PROP_LEFT_ACTION,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

static void
symbol_clicked (HdyKeypad     *self,
                gchar          symbol)
{
  HdyKeypadPrivate *priv;
  g_autofree gchar *string = g_strdup_printf ("%c", symbol);
  g_return_if_fail (HDY_IS_KEYPAD (self));
  priv = hdy_keypad_get_instance_private (self);
  g_return_if_fail (priv->entry != NULL);
  g_signal_emit_by_name(GTK_ENTRY (priv->entry), "insert-at-cursor", string, NULL);
  /* Set focus to the entry only when it can get focus
   * https://gitlab.gnome.org/GNOME/gtk/issues/2204
   */
  if (gtk_widget_get_can_focus (priv->entry)) {
    gtk_entry_grab_focus_without_selecting (GTK_ENTRY (priv->entry));
  }
}


static void
button_clicked_cb (HdyKeypad       *self,
                      HdyKeypadButton *btn)
{
  gchar digit;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (HDY_IS_KEYPAD_BUTTON (btn));

  digit = hdy_keypad_button_get_digit (btn);
  symbol_clicked (self, digit);
  g_debug ("Button with number %c was pressed", digit);
}


static void
asterisk_button_clicked_cb (HdyKeypad *self,
                            GtkWidget *btn)
{
  g_return_if_fail (HDY_IS_KEYPAD (self));

  symbol_clicked (self, '*');
  g_debug ("Button with * was pressed");
}


static void
hash_button_clicked_cb (HdyKeypad *self,
                        GtkWidget *btn)
{
  g_return_if_fail (HDY_IS_KEYPAD (self));

  symbol_clicked (self, '#');
  g_debug ("Button with # was pressed");
}


static void
insert_text_cb (HdyKeypad *self,
               gchar       *text,
               gint         length,
               gpointer     position,
               GtkEditable  *editable)
{
  HdyKeypadPrivate *priv;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  priv = hdy_keypad_get_instance_private (self);

  g_return_if_fail (length == 1);

  if (g_ascii_isdigit (*text))
     return;

  if (!priv->only_digits && strchr("#*+", *text))
     return;

  g_signal_stop_emission_by_name (editable, "insert-text");
}


static void
long_press_zero_cb (HdyKeypad  *self,
                    gdouble     x,
                    gdouble     y,
                    GtkGesture *gesture)
{
  HdyKeypadPrivate *priv;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  priv = hdy_keypad_get_instance_private (self);

  if (priv->only_digits)
    return;

  g_debug ("Long press on zero button");
  symbol_clicked (self, '+');
  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}


static void
hdy_keypad_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  HdyKeypad *self = HDY_KEYPAD (object);
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);

  switch (property_id) {
  case PROP_SHOW_SYMBOLS:
    hdy_keypad_show_symbols (self, g_value_get_boolean (value));
    break;

  case PROP_ONLY_DIGITS:
    if (g_value_get_boolean (value) != priv->only_digits) {
      priv->only_digits = g_value_get_boolean (value);
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ONLY_DIGITS]);
    }
    break;
  case PROP_ENTRY:
    hdy_keypad_set_entry (self, g_value_get_object (value));
    break;

  case PROP_RIGHT_ACTION:
    hdy_keypad_set_right_action (self, g_value_get_object (value));
    break;

  case PROP_LEFT_ACTION:
    hdy_keypad_set_left_action (self, g_value_get_object (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
hdy_keypad_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  HdyKeypad *self = HDY_KEYPAD (object);
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);

  switch (property_id) {
  case PROP_SHOW_SYMBOLS:
    g_value_set_boolean (value, priv->show_symbols);
    break;

  case PROP_ONLY_DIGITS:
    g_value_set_boolean (value, priv->only_digits);
    break;

  case PROP_ENTRY:
    g_value_set_object (value, priv->entry);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
hdy_keypad_finalize (GObject *object)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (HDY_KEYPAD (object));

  if (priv->long_press_zero_gesture != NULL)
    g_object_unref (priv->long_press_zero_gesture);

  G_OBJECT_CLASS (hdy_keypad_parent_class)->finalize (object);
}


static void
hdy_keypad_class_init (HdyKeypadClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hdy_keypad_finalize;

  object_class->set_property = hdy_keypad_set_property;
  object_class->get_property = hdy_keypad_get_property;

  props[PROP_SHOW_SYMBOLS] =
    g_param_spec_boolean ("show-symbols",
                         _("Show Symbols"),
                         _("Whether the second line of symbols should be shown or not"),
                         TRUE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ONLY_DIGITS] =
    g_param_spec_boolean ("only-digits",
                         _("Only Digits"),
                         _("Whether the keypad should show only digits or also extra buttons for #, *"),
                         FALSE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_ENTRY] =
   g_param_spec_object ("entry",
                        _("Entry widget"),
                        _("The entry widget connected to the keypad"),
                        GTK_TYPE_WIDGET,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_RIGHT_ACTION] =
   g_param_spec_object ("right-action",
                        _("Right action widget"),
                        _("The right action widget"),
                        GTK_TYPE_WIDGET,
                        G_PARAM_WRITABLE | G_PARAM_EXPLICIT_NOTIFY);

  props[PROP_LEFT_ACTION] =
   g_param_spec_object ("left-action",
                        _("Left action widget"),
                        _("The left action widget"),
                        GTK_TYPE_WIDGET,
                        G_PARAM_WRITABLE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-keypad.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, label_asterisk);
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, label_hash);
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, long_press_zero_gesture);

  gtk_widget_class_bind_template_callback(widget_class, button_clicked_cb);
  gtk_widget_class_bind_template_callback(widget_class, asterisk_button_clicked_cb);
  gtk_widget_class_bind_template_callback(widget_class, hash_button_clicked_cb);
  gtk_widget_class_bind_template_callback(widget_class, long_press_zero_cb);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_DIAL);
  gtk_widget_class_set_css_name (widget_class, "keypad");
}


static void
hdy_keypad_init (HdyKeypad *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}


/**
 * hdy_keypad_new:
 * @only_digits: whether the keypad should show only digits or also extra buttons for #, *
 * @show_symbols: whether the keypad should show the second line or only the main digit
 *
 * Create a new #HdyKeypad widget.
 *
 * Returns: the newly created #HdyKeypad widget
 *
 */
GtkWidget *hdy_keypad_new (gboolean only_digits, gboolean show_symbols)
{
  return g_object_new (HDY_TYPE_KEYPAD,
                       "only-digits", only_digits,
                       "show-symbols", show_symbols,
                       NULL);
}


/**
 * hdy_keypad_show_symbols:
 * @self: a #HdyKeypad
 * @visible: whether the second line on buttons should be shown or not
 *
 * Sets the visibility of symbols (excluding the main digit) on each button in the #HdyKeypad
 *
 */
void
hdy_keypad_show_symbols (HdyKeypad *self, gboolean visible)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private(self);

  g_return_if_fail (HDY_IS_KEYPAD (self));

  if (visible == priv->show_symbols)
    return;

  priv->show_symbols = visible;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SHOW_SYMBOLS]);
}


/**
 * hdy_keypad_set_entry:
 * @self: a #HdyKeypad
 * @entry: a #GtkEntry
 *
 * Binds a #GtkEntry to the keypad and it blocks every
 * input which wouldn't be possible to type with with the keypad
 *
 */
void
hdy_keypad_set_entry (HdyKeypad *self, GtkEntry *entry)
{
  HdyKeypadPrivate *priv;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (GTK_IS_ENTRY (entry));

  priv = hdy_keypad_get_instance_private(self);
  if (priv->entry != NULL) {
    g_object_unref (priv->entry);
  }

  if (entry == NULL) {
    priv->entry = NULL;
    return;
  }

  priv->entry = GTK_WIDGET (g_object_ref (entry));

  gtk_widget_show (priv->entry);
  /* Workaround: To keep the osk closed
   * https://gitlab.gnome.org/GNOME/gtk/merge_requests/978#note_546576 */
  g_object_set (priv->entry, "im-module", "gtk-im-context-none", NULL);

  g_signal_connect_swapped (G_OBJECT (priv->entry),
                            "insert-text",
                            G_CALLBACK (insert_text_cb),
                            self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENTRY]);
}


/**
 * hdy_keypad_get_entry:
 * @self: a #HdyKeypad
 *
 * Get the connected entry. See hdy_keypad_set_entry () for details
 *
 * Returns: (transfer none): the set #GtkEntry or NULL if no widget was set
 *
 */
GtkWidget *
hdy_keypad_get_entry (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), NULL);

  priv = hdy_keypad_get_instance_private(self);

  return priv->entry;
}


/**
 * hdy_keypad_set_left_action:
 * @self: a #HdyKeypad
 * @widget: nullable: the widget which should be show in the left lower corner of #HdyKeypad
 *
 * Sets the widget for the left lower corner of #HdyKeypad replacing the existing widget, if NULL it just removes whatever widget is there
 *
 */
void
hdy_keypad_set_left_action (HdyKeypad *self, GtkWidget *widget)
{
  GtkWidget *old_widget;
  g_return_if_fail (HDY_IS_KEYPAD (self));

  old_widget = gtk_grid_get_child_at (GTK_GRID (self), 0, 3);

  if (old_widget == widget)
    return;

  if (old_widget != NULL)
    gtk_container_remove (GTK_CONTAINER (self), old_widget);

  if (widget != NULL)
    gtk_grid_attach (GTK_GRID (self), widget, 0, 3, 1, 1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LEFT_ACTION]);
}


/**
 * hdy_keypad_set_right_action:
 * @self: a #HdyKeypad
 * @widget: nullable: the widget which should be show in the right lower corner of #HdyKeypad
 *
 * Sets the widget for the right lower corner of #HdyKeypad replacing the existing widget, if NULL it just removes whatever widget is there
 *
 */
void
hdy_keypad_set_right_action (HdyKeypad *self, GtkWidget *widget)
{
  GtkWidget *old_widget;
  g_return_if_fail (HDY_IS_KEYPAD (self));

  old_widget = gtk_grid_get_child_at (GTK_GRID (self), 2, 3);

  if (old_widget == widget)
    return;

  if (old_widget != NULL)
    gtk_container_remove (GTK_CONTAINER (self), old_widget);

  if (widget != NULL)
    gtk_grid_attach (GTK_GRID (self), widget, 2, 3, 1, 1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_RIGHT_ACTION]);
}
