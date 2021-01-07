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
 * # CSS nodes
 *
 * #HdyKeypad has a single CSS node with name keypad.
 *
 * Since: 0.0.12
 */

typedef struct
{
  GtkEntry *entry;
  GtkWidget *grid;
  GtkWidget *label_asterisk;
  GtkWidget *label_hash;
  GtkGesture *long_press_zero_gesture;
  guint16 row_spacing;
  guint16 column_spacing;
  gboolean symbols_visible;
  gboolean letters_visible;
} HdyKeypadPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyKeypad, hdy_keypad, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_ROW_SPACING,
  PROP_COLUMN_SPACING,
  PROP_LETTERS_VISIBLE,
  PROP_SYMBOLS_VISIBLE,
  PROP_ENTRY,
  PROP_END_ACTION,
  PROP_START_ACTION,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

static void
symbol_clicked (HdyKeypad *self,
                gchar      symbol)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);
  g_autofree gchar *string = g_strdup_printf ("%c", symbol);

  if (!priv->entry)
    return;

  g_signal_emit_by_name (priv->entry, "insert-at-cursor", string, NULL);
  /* Set focus to the entry only when it can get focus
   * https://gitlab.gnome.org/GNOME/gtk/issues/2204
   */
  if (gtk_widget_get_can_focus (GTK_WIDGET (priv->entry)))
    gtk_entry_grab_focus_without_selecting (priv->entry);
}


static void
button_clicked_cb (HdyKeypad       *self,
                   HdyKeypadButton *btn)
{
  gchar digit = hdy_keypad_button_get_digit (btn);
  symbol_clicked (self, digit);
  g_debug ("Button with number %c was pressed", digit);
}


static void
asterisk_button_clicked_cb (HdyKeypad *self,
                            GtkWidget *btn)
{
  symbol_clicked (self, '*');
  g_debug ("Button with * was pressed");
}


static void
hash_button_clicked_cb (HdyKeypad *self,
                        GtkWidget *btn)
{
  symbol_clicked (self, '#');
  g_debug ("Button with # was pressed");
}


static void
insert_text_cb (HdyKeypad   *self,
                gchar       *text,
                gint         length,
                gpointer     position,
                GtkEditable *editable)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);
  gchar *p = text;

  g_assert (g_utf8_validate (text, length, NULL));

  while (p != text + length) {
    gchar *q = p;

    p = g_utf8_next_char (p);

    if (g_ascii_isdigit (*q))
      continue;

    if (priv->symbols_visible && strchr ("#*+", *q))
      continue;

    gtk_widget_error_bell (GTK_WIDGET (editable));

    g_signal_stop_emission_by_name (editable, "insert-text");

    return;
  }
}


static void
long_press_zero_cb (HdyKeypad  *self,
                    gdouble     x,
                    gdouble     y,
                    GtkGesture *gesture)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);

  if (!priv->symbols_visible)
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

  switch (property_id) {
  case PROP_ROW_SPACING:
    hdy_keypad_set_row_spacing (self, g_value_get_uint (value));
    break;
  case PROP_COLUMN_SPACING:
    hdy_keypad_set_column_spacing (self, g_value_get_uint (value));
    break;
  case PROP_LETTERS_VISIBLE:
    hdy_keypad_set_letters_visible (self, g_value_get_boolean (value));
    break;
  case PROP_SYMBOLS_VISIBLE:
    hdy_keypad_set_symbols_visible (self, g_value_get_boolean (value));
    break;
  case PROP_ENTRY:
    hdy_keypad_set_entry (self, g_value_get_object (value));
    break;
  case PROP_END_ACTION:
    hdy_keypad_set_end_action (self, g_value_get_object (value));
    break;
  case PROP_START_ACTION:
    hdy_keypad_set_start_action (self, g_value_get_object (value));
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

  switch (property_id) {
  case PROP_ROW_SPACING:
    g_value_set_uint (value, hdy_keypad_get_row_spacing (self));
    break;
  case PROP_COLUMN_SPACING:
    g_value_set_uint (value, hdy_keypad_get_column_spacing (self));
    break;
  case PROP_LETTERS_VISIBLE:
    g_value_set_boolean (value, hdy_keypad_get_letters_visible (self));
    break;
  case PROP_SYMBOLS_VISIBLE:
    g_value_set_boolean (value, hdy_keypad_get_symbols_visible (self));
    break;
  case PROP_ENTRY:
    g_value_set_object (value, hdy_keypad_get_entry (self));
    break;
  case PROP_START_ACTION:
    g_value_set_object (value, hdy_keypad_get_start_action (self));
    break;
  case PROP_END_ACTION:
    g_value_set_object (value, hdy_keypad_get_end_action (self));
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

  /**
   * HdyKeypad:row-spacing:
   *
   * The amount of space between two consecutive rows.
   *
   * Since: 1.0
   */
  props[PROP_ROW_SPACING] =
    g_param_spec_uint ("row-spacing",
                       _("Row spacing"),
                       _("The amount of space between two consecutive rows"),
                       0, G_MAXINT16, 6,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyKeypad:column-spacing:
   *
   * The amount of space between two consecutive columns.
   *
   * Since: 1.0
   */
  props[PROP_COLUMN_SPACING] =
    g_param_spec_uint ("column-spacing",
                       _("Column spacing"),
                       _("The amount of space between two consecutive columns"),
                       0, G_MAXINT16, 6,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyKeypad:letters-visible:
   *
   * Whether the keypad should display the standard letters below the digits on
   * its buttons.
   *
   * Since: 1.0
   */
  props[PROP_LETTERS_VISIBLE] =
    g_param_spec_boolean ("letters-visible",
                         _("Letters visible"),
                         _("Whether the letters below the digits should be visible"),
                         TRUE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyKeypad:symbols-visible:
   *
   * Whether the keypad should display the hash and asterisk buttons, and should
   * display the plus symbol at the bottom of its 0 button.
   *
   * Since: 1.0
   */
  props[PROP_SYMBOLS_VISIBLE] =
    g_param_spec_boolean ("symbols-visible",
                         _("Symbols visible"),
                         _("Whether the hash, plus, and asterisk symbols should be visible"),
                         TRUE,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyKeypad:entry:
   *
   * The entry widget connected to the keypad. See hdy_keypad_set_entry() for
   * details.
   *
   * Since: 1.0
   */
  props[PROP_ENTRY] =
   g_param_spec_object ("entry",
                        _("Entry"),
                        _("The entry widget connected to the keypad"),
                        GTK_TYPE_ENTRY,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyKeypad:end-action:
   *
   * The widget for the lower end corner of @self.
   *
   * Since: 1.0
   */
  props[PROP_END_ACTION] =
   g_param_spec_object ("end-action",
                        _("End action"),
                        _("The end action widget"),
                        GTK_TYPE_WIDGET,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyKeypad:start-action:
   *
   * The widget for the lower start corner of @self.
   *
   * Since: 1.0
   */
  props[PROP_START_ACTION] =
   g_param_spec_object ("start-action",
                        _("Start action"),
                        _("The start action widget"),
                        GTK_TYPE_WIDGET,
                        G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-keypad.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, grid);
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, label_asterisk);
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, label_hash);
  gtk_widget_class_bind_template_child_private (widget_class, HdyKeypad, long_press_zero_gesture);

  gtk_widget_class_bind_template_callback (widget_class, button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, asterisk_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, hash_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, long_press_zero_cb);

  gtk_widget_class_set_accessible_role (widget_class, ATK_ROLE_DIAL);
  gtk_widget_class_set_css_name (widget_class, "keypad");
}


static void
hdy_keypad_init (HdyKeypad *self)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);

  priv->row_spacing = 6;
  priv->column_spacing = 6;
  priv->letters_visible = TRUE;
  priv->symbols_visible = TRUE;

  g_type_ensure (HDY_TYPE_KEYPAD_BUTTON);
  gtk_widget_init_template (GTK_WIDGET (self));
}


/**
 * hdy_keypad_new:
 * @symbols_visible: whether the hash, plus, and asterisk symbols should be visible
 * @letters_visible: whether the letters below the digits should be visible
 *
 * Create a new #HdyKeypad widget.
 *
 * Returns: the newly created #HdyKeypad widget
 *
 * Since: 0.0.12
 */
GtkWidget *
hdy_keypad_new (gboolean symbols_visible,
                gboolean letters_visible)
{
  return g_object_new (HDY_TYPE_KEYPAD,
                       "symbols-visible", symbols_visible,
                       "letters-visible", letters_visible,
                       NULL);
}

/**
 * hdy_keypad_set_row_spacing:
 * @self: a #HdyKeypad
 * @spacing: the amount of space to insert between rows
 *
 * Sets the amount of space between rows of @self.
 *
 * Since: 1.0
 */
void
hdy_keypad_set_row_spacing (HdyKeypad *self,
                            guint      spacing)
{
  HdyKeypadPrivate *priv;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (spacing <= G_MAXINT16);

  priv = hdy_keypad_get_instance_private (self);

  if (priv->row_spacing == spacing)
    return;

  priv->row_spacing = spacing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ROW_SPACING]);
}


/**
 * hdy_keypad_get_row_spacing:
 * @self: a #HdyKeypad
 *
 * Returns the amount of space between the rows of @self.
 *
 * Returns: the row spacing of @self
 *
 * Since: 1.0
 */
guint
hdy_keypad_get_row_spacing (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), 0);

  priv = hdy_keypad_get_instance_private (self);

  return priv->row_spacing;
}


/**
 * hdy_keypad_set_column_spacing:
 * @self: a #HdyKeypad
 * @spacing: the amount of space to insert between columns
 *
 * Sets the amount of space between columns of @self.
 *
 * Since: 1.0
 */
void
hdy_keypad_set_column_spacing (HdyKeypad *self,
                               guint      spacing)
{
  HdyKeypadPrivate *priv;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (spacing <= G_MAXINT16);

  priv = hdy_keypad_get_instance_private (self);

  if (priv->column_spacing == spacing)
    return;

  priv->column_spacing = spacing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLUMN_SPACING]);
}


/**
 * hdy_keypad_get_column_spacing:
 * @self: a #HdyKeypad
 *
 * Returns the amount of space between the columns of @self.
 *
 * Returns: the column spacing of @self
 *
 * Since: 1.0
 */
guint
hdy_keypad_get_column_spacing (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), 0);

  priv = hdy_keypad_get_instance_private (self);

  return priv->column_spacing;
}


/**
 * hdy_keypad_set_letters_visible:
 * @self: a #HdyKeypad
 * @letters_visible: whether the letters below the digits should be visible
 *
 * Sets whether @self should display the standard letters below the digits on
 * its buttons.
 *
 * Since: 1.0
 */
void
hdy_keypad_set_letters_visible (HdyKeypad *self,
                                gboolean   letters_visible)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);

  g_return_if_fail (HDY_IS_KEYPAD (self));

  letters_visible = !!letters_visible;

  if (priv->letters_visible == letters_visible)
    return;

  priv->letters_visible = letters_visible;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LETTERS_VISIBLE]);
}


/**
 * hdy_keypad_get_letters_visible:
 * @self: a #HdyKeypad
 *
 * Returns whether @self should display the standard letters below the digits on
 * its buttons.
 *
 * Returns: whether the letters below the digits should be visible
 *
 * Since: 1.0
 */
gboolean
hdy_keypad_get_letters_visible (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), FALSE);

  priv = hdy_keypad_get_instance_private (self);

  return priv->letters_visible;
}


/**
 * hdy_keypad_set_symbols_visible:
 * @self: a #HdyKeypad
 * @symbols_visible: whether the hash, plus, and asterisk symbols should be visible
 *
 * Sets whether @self should display the hash and asterisk buttons, and should
 * display the plus symbol at the bottom of its 0 button.
 *
 * Since: 1.0
 */
void
hdy_keypad_set_symbols_visible (HdyKeypad *self,
                                gboolean   symbols_visible)
{
  HdyKeypadPrivate *priv = hdy_keypad_get_instance_private (self);

  g_return_if_fail (HDY_IS_KEYPAD (self));

  symbols_visible = !!symbols_visible;

  if (priv->symbols_visible == symbols_visible)
    return;

  priv->symbols_visible = symbols_visible;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYMBOLS_VISIBLE]);
}


/**
 * hdy_keypad_get_symbols_visible:
 * @self: a #HdyKeypad
 *
 * Returns whether @self should display the standard letters below the digits on
 * its buttons.
 *
 * Returns Whether @self should display the hash and asterisk buttons, and
 * should display the plus symbol at the bottom of its 0 button.
 *
 * Returns: whether the hash, plus, and asterisk symbols should be visible
 *
 * Since: 1.0
 */
gboolean
hdy_keypad_get_symbols_visible (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), FALSE);

  priv = hdy_keypad_get_instance_private (self);

  return priv->symbols_visible;
}


/**
 * hdy_keypad_set_entry:
 * @self: a #HdyKeypad
 * @entry: (nullable): a #GtkEntry
 *
 * Binds @entry to @self and blocks any input which wouldn't be possible to type
 * with with the keypad.
 *
 * Since: 0.0.12
 */
void
hdy_keypad_set_entry (HdyKeypad *self,
                      GtkEntry  *entry)
{
  HdyKeypadPrivate *priv;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (entry == NULL || GTK_IS_ENTRY (entry));

  priv = hdy_keypad_get_instance_private (self);

  if (entry == priv->entry)
    return;

  g_clear_object (&priv->entry);

  if (entry) {
    priv->entry = g_object_ref (entry);

    gtk_widget_show (GTK_WIDGET (priv->entry));
    /* Workaround: To keep the osk closed
     * https://gitlab.gnome.org/GNOME/gtk/merge_requests/978#note_546576 */
    g_object_set (priv->entry, "im-module", "gtk-im-context-none", NULL);

    g_signal_connect_swapped (G_OBJECT (priv->entry),
                              "insert-text",
                              G_CALLBACK (insert_text_cb),
                              self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENTRY]);
}


/**
 * hdy_keypad_get_entry:
 * @self: a #HdyKeypad
 *
 * Get the connected entry. See hdy_keypad_set_entry() for details.
 *
 * Returns: (transfer none): the set #GtkEntry or %NULL if no widget was set
 *
 * Since: 1.0
 */
GtkEntry *
hdy_keypad_get_entry (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), NULL);

  priv = hdy_keypad_get_instance_private (self);

  return priv->entry;
}


/**
 * hdy_keypad_set_start_action:
 * @self: a #HdyKeypad
 * @start_action: (nullable): the start action widget
 *
 * Sets the widget for the lower left corner (or right, in RTL locales) of
 * @self.
 *
 * Since: 1.0
 */
void
hdy_keypad_set_start_action (HdyKeypad *self,
                             GtkWidget *start_action)
{
  HdyKeypadPrivate *priv;
  GtkWidget *old_widget;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (start_action == NULL || GTK_IS_WIDGET (start_action));

  priv = hdy_keypad_get_instance_private (self);

  old_widget = gtk_grid_get_child_at (GTK_GRID (priv->grid), 0, 3);

  if (old_widget == start_action)
    return;

  if (old_widget != NULL)
    gtk_container_remove (GTK_CONTAINER (priv->grid), old_widget);

  if (start_action != NULL)
    gtk_grid_attach (GTK_GRID (priv->grid), start_action, 0, 3, 1, 1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_START_ACTION]);
}


/**
 * hdy_keypad_get_start_action:
 * @self: a #HdyKeypad
 *
 * Returns the widget for the lower left corner (or right, in RTL locales) of
 * @self.
 *
 * Returns: (transfer none) (nullable): the start action widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_keypad_get_start_action (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), NULL);

  priv = hdy_keypad_get_instance_private (self);

  return gtk_grid_get_child_at (GTK_GRID (priv->grid), 0, 3);
}


/**
 * hdy_keypad_set_end_action:
 * @self: a #HdyKeypad
 * @end_action: (nullable): the end action widget
 *
 * Sets the widget for the lower right corner (or left, in RTL locales) of
 * @self.
 *
 * Since: 1.0
 */
void
hdy_keypad_set_end_action (HdyKeypad *self,
                           GtkWidget *end_action)
{
  HdyKeypadPrivate *priv;
  GtkWidget *old_widget;

  g_return_if_fail (HDY_IS_KEYPAD (self));
  g_return_if_fail (end_action == NULL || GTK_IS_WIDGET (end_action));

  priv = hdy_keypad_get_instance_private (self);

  old_widget = gtk_grid_get_child_at (GTK_GRID (priv->grid), 2, 3);

  if (old_widget == end_action)
    return;

  if (old_widget != NULL)
    gtk_container_remove (GTK_CONTAINER (priv->grid), old_widget);

  if (end_action != NULL)
    gtk_grid_attach (GTK_GRID (priv->grid), end_action, 2, 3, 1, 1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_END_ACTION]);
}


/**
 * hdy_keypad_get_end_action:
 * @self: a #HdyKeypad
 *
 * Returns the widget for the lower right corner (or left, in RTL locales) of
 * @self.
 *
 * Returns: (transfer none) (nullable): the end action widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_keypad_get_end_action (HdyKeypad *self)
{
  HdyKeypadPrivate *priv;

  g_return_val_if_fail (HDY_IS_KEYPAD (self), NULL);

  priv = hdy_keypad_get_instance_private (self);

  return gtk_grid_get_child_at (GTK_GRID (priv->grid), 2, 3);
}
