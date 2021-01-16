/*
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-keypad.h"
#include "adw-keypad-button-private.h"

/**
 * SECTION:adw-keypad
 * @short_description: A keypad for dialing numbers
 * @Title: AdwKeypad
 *
 * The #AdwKeypad widget is a keypad for entering numbers such as phone numbers
 * or PIN codes.
 *
 * # CSS nodes
 *
 * #AdwKeypad has a single CSS node with name keypad.
 *
 * Since: 1.0
 */

struct _AdwKeypad {
  GtkWidget parent_instance;

  GtkEntry *entry;
  GtkWidget *grid;
  GtkWidget *label_asterisk;
  GtkWidget *label_hash;
  guint16 row_spacing;
  guint16 column_spacing;
  gboolean symbols_visible;
  gboolean letters_visible;
};

G_DEFINE_TYPE (AdwKeypad, adw_keypad, GTK_TYPE_WIDGET)

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
symbol_clicked (AdwKeypad *self,
                gchar      symbol)
{
  g_autofree gchar *string = g_strdup_printf ("%c", symbol);
  GtkEntryBuffer *buffer;
  gint position;

  if (!self->entry)
    return;

  buffer = gtk_entry_get_buffer (self->entry);
  position = gtk_editable_get_position (GTK_EDITABLE (self->entry));
  gtk_entry_buffer_insert_text (buffer, position, string, 1);
  gtk_editable_set_position (GTK_EDITABLE (self->entry), position + 1);

  /* Set focus to the entry only when it can get focus
   * https://gitlab.gnome.org/GNOME/gtk/issues/2204
   */
  if (gtk_widget_get_can_focus (GTK_WIDGET (self->entry)))
    gtk_entry_grab_focus_without_selecting (self->entry);
}


static void
button_clicked_cb (AdwKeypad       *self,
                   AdwKeypadButton *btn)
{
  gchar digit = adw_keypad_button_get_digit (btn);
  symbol_clicked (self, digit);
  g_debug ("Button with number %c was pressed", digit);
}


static void
asterisk_button_clicked_cb (AdwKeypad *self,
                            GtkWidget *btn)
{
  symbol_clicked (self, '*');
  g_debug ("Button with * was pressed");
}


static void
hash_button_clicked_cb (AdwKeypad *self,
                        GtkWidget *btn)
{
  symbol_clicked (self, '#');
  g_debug ("Button with # was pressed");
}


static void
insert_text_cb (AdwKeypad   *self,
                gchar       *text,
                gint         length,
                gpointer     position,
                GtkEditable *editable)
{
  gchar *p = text;

  g_assert (g_utf8_validate (text, length, NULL));

  while (p != text + length) {
    gchar *q = p;

    p = g_utf8_next_char (p);

    if (g_ascii_isdigit (*q))
      continue;

    if (self->symbols_visible && strchr ("#*+", *q))
      continue;

    gtk_widget_error_bell (GTK_WIDGET (editable));

    g_signal_stop_emission_by_name (editable, "insert-text");

    return;
  }
}


static void
long_press_zero_cb (AdwKeypad  *self,
                    gdouble     x,
                    gdouble     y,
                    GtkGesture *gesture)
{
  if (!self->symbols_visible)
    return;

  g_debug ("Long press on zero button");
  symbol_clicked (self, '+');
  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
}


static void
adw_keypad_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwKeypad *self = ADW_KEYPAD (object);

  switch (property_id) {
  case PROP_ROW_SPACING:
    adw_keypad_set_row_spacing (self, g_value_get_uint (value));
    break;
  case PROP_COLUMN_SPACING:
    adw_keypad_set_column_spacing (self, g_value_get_uint (value));
    break;
  case PROP_LETTERS_VISIBLE:
    adw_keypad_set_letters_visible (self, g_value_get_boolean (value));
    break;
  case PROP_SYMBOLS_VISIBLE:
    adw_keypad_set_symbols_visible (self, g_value_get_boolean (value));
    break;
  case PROP_ENTRY:
    adw_keypad_set_entry (self, g_value_get_object (value));
    break;
  case PROP_END_ACTION:
    adw_keypad_set_end_action (self, g_value_get_object (value));
    break;
  case PROP_START_ACTION:
    adw_keypad_set_start_action (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
adw_keypad_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwKeypad *self = ADW_KEYPAD (object);

  switch (property_id) {
  case PROP_ROW_SPACING:
    g_value_set_uint (value, adw_keypad_get_row_spacing (self));
    break;
  case PROP_COLUMN_SPACING:
    g_value_set_uint (value, adw_keypad_get_column_spacing (self));
    break;
  case PROP_LETTERS_VISIBLE:
    g_value_set_boolean (value, adw_keypad_get_letters_visible (self));
    break;
  case PROP_SYMBOLS_VISIBLE:
    g_value_set_boolean (value, adw_keypad_get_symbols_visible (self));
    break;
  case PROP_ENTRY:
    g_value_set_object (value, adw_keypad_get_entry (self));
    break;
  case PROP_START_ACTION:
    g_value_set_object (value, adw_keypad_get_start_action (self));
    break;
  case PROP_END_ACTION:
    g_value_set_object (value, adw_keypad_get_end_action (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
adw_keypad_dispose (GObject *object)
{
  AdwKeypad *self = ADW_KEYPAD (object);

  gtk_widget_unparent (self->grid);

  G_OBJECT_CLASS (adw_keypad_parent_class)->dispose (object);
}


static void
adw_keypad_class_init (AdwKeypadClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_keypad_dispose;

  object_class->set_property = adw_keypad_set_property;
  object_class->get_property = adw_keypad_get_property;

  /**
   * AdwKeypad:row-spacing:
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
   * AdwKeypad:column-spacing:
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
   * AdwKeypad:letters-visible:
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
   * AdwKeypad:symbols-visible:
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
   * AdwKeypad:entry:
   *
   * The entry widget connected to the keypad. See adw_keypad_set_entry() for
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
   * AdwKeypad:end-action:
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
   * AdwKeypad:start-action:
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
                                               "/org/gnome/Adwaita/ui/adw-keypad.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwKeypad, grid);
  gtk_widget_class_bind_template_child (widget_class, AdwKeypad, label_asterisk);
  gtk_widget_class_bind_template_child (widget_class, AdwKeypad, label_hash);

  gtk_widget_class_bind_template_callback (widget_class, button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, asterisk_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, hash_button_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, long_press_zero_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "keypad");
}


static void
adw_keypad_init (AdwKeypad *self)
{
  self->row_spacing = 6;
  self->column_spacing = 6;
  self->letters_visible = TRUE;
  self->symbols_visible = TRUE;

  g_type_ensure (ADW_TYPE_KEYPAD_BUTTON);
  gtk_widget_init_template (GTK_WIDGET (self));
}


/**
 * adw_keypad_new:
 * @symbols_visible: whether the hash, plus, and asterisk symbols should be visible
 * @letters_visible: whether the letters below the digits should be visible
 *
 * Create a new #AdwKeypad widget.
 *
 * Returns: the newly created #AdwKeypad widget
 *
 * Since: 1.0
 */
GtkWidget *
adw_keypad_new (gboolean symbols_visible,
                gboolean letters_visible)
{
  return g_object_new (ADW_TYPE_KEYPAD,
                       "symbols-visible", symbols_visible,
                       "letters-visible", letters_visible,
                       NULL);
}

/**
 * adw_keypad_set_row_spacing:
 * @self: a #AdwKeypad
 * @spacing: the amount of space to insert between rows
 *
 * Sets the amount of space between rows of @self.
 *
 * Since: 1.0
 */
void
adw_keypad_set_row_spacing (AdwKeypad *self,
                            guint      spacing)
{
  g_return_if_fail (ADW_IS_KEYPAD (self));
  g_return_if_fail (spacing <= G_MAXINT16);

  if (self->row_spacing == spacing)
    return;

  self->row_spacing = spacing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ROW_SPACING]);
}


/**
 * adw_keypad_get_row_spacing:
 * @self: a #AdwKeypad
 *
 * Returns the amount of space between the rows of @self.
 *
 * Returns: the row spacing of @self
 *
 * Since: 1.0
 */
guint
adw_keypad_get_row_spacing (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), 0);

  return self->row_spacing;
}


/**
 * adw_keypad_set_column_spacing:
 * @self: a #AdwKeypad
 * @spacing: the amount of space to insert between columns
 *
 * Sets the amount of space between columns of @self.
 *
 * Since: 1.0
 */
void
adw_keypad_set_column_spacing (AdwKeypad *self,
                               guint      spacing)
{
  g_return_if_fail (ADW_IS_KEYPAD (self));
  g_return_if_fail (spacing <= G_MAXINT16);

  if (self->column_spacing == spacing)
    return;

  self->column_spacing = spacing;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLUMN_SPACING]);
}


/**
 * adw_keypad_get_column_spacing:
 * @self: a #AdwKeypad
 *
 * Returns the amount of space between the columns of @self.
 *
 * Returns: the column spacing of @self
 *
 * Since: 1.0
 */
guint
adw_keypad_get_column_spacing (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), 0);

  return self->column_spacing;
}


/**
 * adw_keypad_set_letters_visible:
 * @self: a #AdwKeypad
 * @letters_visible: whether the letters below the digits should be visible
 *
 * Sets whether @self should display the standard letters below the digits on
 * its buttons.
 *
 * Since: 1.0
 */
void
adw_keypad_set_letters_visible (AdwKeypad *self,
                                gboolean   letters_visible)
{
  g_return_if_fail (ADW_IS_KEYPAD (self));

  letters_visible = !!letters_visible;

  if (self->letters_visible == letters_visible)
    return;

  self->letters_visible = letters_visible;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LETTERS_VISIBLE]);
}


/**
 * adw_keypad_get_letters_visible:
 * @self: a #AdwKeypad
 *
 * Returns whether @self should display the standard letters below the digits on
 * its buttons.
 *
 * Returns: whether the letters below the digits should be visible
 *
 * Since: 1.0
 */
gboolean
adw_keypad_get_letters_visible (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), FALSE);

  return self->letters_visible;
}


/**
 * adw_keypad_set_symbols_visible:
 * @self: a #AdwKeypad
 * @symbols_visible: whether the hash, plus, and asterisk symbols should be visible
 *
 * Sets whether @self should display the hash and asterisk buttons, and should
 * display the plus symbol at the bottom of its 0 button.
 *
 * Since: 1.0
 */
void
adw_keypad_set_symbols_visible (AdwKeypad *self,
                                gboolean   symbols_visible)
{
  g_return_if_fail (ADW_IS_KEYPAD (self));

  symbols_visible = !!symbols_visible;

  if (self->symbols_visible == symbols_visible)
    return;

  self->symbols_visible = symbols_visible;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYMBOLS_VISIBLE]);
}


/**
 * adw_keypad_get_symbols_visible:
 * @self: a #AdwKeypad
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
adw_keypad_get_symbols_visible (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), FALSE);

  return self->symbols_visible;
}


/**
 * adw_keypad_set_entry:
 * @self: a #AdwKeypad
 * @entry: (nullable): a #GtkEntry
 *
 * Binds @entry to @self and blocks any input which wouldn't be possible to type
 * with with the keypad.
 *
 * Since: 1.0
 */
void
adw_keypad_set_entry (AdwKeypad *self,
                      GtkEntry  *entry)
{
  g_return_if_fail (ADW_IS_KEYPAD (self));
  g_return_if_fail (entry == NULL || GTK_IS_ENTRY (entry));

  if (entry == self->entry)
    return;

  g_clear_object (&self->entry);

  if (entry) {
    self->entry = g_object_ref (entry);

    gtk_widget_show (GTK_WIDGET (self->entry));
    /* Workaround: To keep the osk closed
     * https://gitlab.gnome.org/GNOME/gtk/merge_requests/978#note_546576 */
    g_object_set (self->entry, "im-module", "gtk-im-context-none", NULL);

    g_signal_connect_swapped (G_OBJECT (self->entry),
                              "insert-text",
                              G_CALLBACK (insert_text_cb),
                              self);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENTRY]);
}


/**
 * adw_keypad_get_entry:
 * @self: a #AdwKeypad
 *
 * Get the connected entry. See adw_keypad_set_entry() for details.
 *
 * Returns: (transfer none): the set #GtkEntry or %NULL if no widget was set
 *
 * Since: 1.0
 */
GtkEntry *
adw_keypad_get_entry (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), NULL);

  return self->entry;
}


/**
 * adw_keypad_set_start_action:
 * @self: a #AdwKeypad
 * @start_action: (nullable): the start action widget
 *
 * Sets the widget for the lower left corner (or right, in RTL locales) of
 * @self.
 *
 * Since: 1.0
 */
void
adw_keypad_set_start_action (AdwKeypad *self,
                             GtkWidget *start_action)
{
  GtkWidget *old_widget;

  g_return_if_fail (ADW_IS_KEYPAD (self));
  g_return_if_fail (start_action == NULL || GTK_IS_WIDGET (start_action));

  old_widget = gtk_grid_get_child_at (GTK_GRID (self->grid), 0, 3);

  if (old_widget == start_action)
    return;

  if (old_widget != NULL)
    gtk_grid_remove (GTK_GRID (self->grid), old_widget);

  if (start_action != NULL)
    gtk_grid_attach (GTK_GRID (self->grid), start_action, 0, 3, 1, 1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_START_ACTION]);
}


/**
 * adw_keypad_get_start_action:
 * @self: a #AdwKeypad
 *
 * Returns the widget for the lower left corner (or right, in RTL locales) of
 * @self.
 *
 * Returns: (transfer none) (nullable): the start action widget
 *
 * Since: 1.0
 */
GtkWidget *
adw_keypad_get_start_action (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), NULL);

  return gtk_grid_get_child_at (GTK_GRID (self->grid), 0, 3);
}


/**
 * adw_keypad_set_end_action:
 * @self: a #AdwKeypad
 * @end_action: (nullable): the end action widget
 *
 * Sets the widget for the lower right corner (or left, in RTL locales) of
 * @self.
 *
 * Since: 1.0
 */
void
adw_keypad_set_end_action (AdwKeypad *self,
                           GtkWidget *end_action)
{
  GtkWidget *old_widget;

  g_return_if_fail (ADW_IS_KEYPAD (self));
  g_return_if_fail (end_action == NULL || GTK_IS_WIDGET (end_action));

  old_widget = gtk_grid_get_child_at (GTK_GRID (self->grid), 2, 3);

  if (old_widget == end_action)
    return;

  if (old_widget != NULL)
    gtk_grid_remove (GTK_GRID (self->grid), old_widget);

  if (end_action != NULL)
    gtk_grid_attach (GTK_GRID (self->grid), end_action, 2, 3, 1, 1);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_END_ACTION]);
}


/**
 * adw_keypad_get_end_action:
 * @self: a #AdwKeypad
 *
 * Returns the widget for the lower right corner (or left, in RTL locales) of
 * @self.
 *
 * Returns: (transfer none) (nullable): the end action widget
 *
 * Since: 1.0
 */
GtkWidget *
adw_keypad_get_end_action (AdwKeypad *self)
{
  g_return_val_if_fail (ADW_IS_KEYPAD (self), NULL);

  return gtk_grid_get_child_at (GTK_GRID (self->grid), 2, 3);
}
