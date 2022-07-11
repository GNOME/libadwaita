/*
 * Copyright 2022 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>

#include "adw-spin-row.h"

#include "adw-macros-private.h"
#include "adw-widget-utils-private.h"

#define MAX_DIGITS 20

/**
 * AdwSpinRow
 *
 * A [class@Gtk.ListBoxRow] with an embedded [@class.Gtk.SpinButton].
 *
 * The `AdwSpinRow` widget can have a title and subtitle, as well as prefix
 * and suffix widgets.
 *
 * ## AdwSpinRow as GtkBuildable
 *
 * The `AdwSpinRow` implementation of the [iface@Gtk.Buildable] interface
 * supports adding a child at its end by specifying "suffix" or omitting the
 * "type" attribute of a <child> element.
 *
 * It also supports adding a child as a prefix widget by specifying "prefix" as
 * the "type" attribute of a <child> element.
 *
 * ## CSS nodes
 *
 * `AdwSpinRow` has a snigle CSS node with the name `row` and the `.spin` style
 * class.
 *
 * Since: 1.2
 */

struct _AdwSpinRow
{
  AdwPreferencesRow parent_instance;

  GtkWidget *header;
  GtkWidget *title;
  GtkWidget *subtitle;
  GtkWidget *prefixes;
  GtkWidget *suffixes;
  GtkWidget *title_box;

  GtkWidget *spin_button;
};

static void adw_spin_row_buildable_init (GtkBuildableIface    *iface);
static void adw_spin_row_editable_init  (GtkEditableInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwSpinRow, adw_spin_row, ADW_TYPE_PREFERENCES_ROW,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_spin_row_buildable_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE, adw_spin_row_editable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_ADJUSTMENT,
  PROP_CLIMB_RATE,
  PROP_DIGITS,
  PROP_NUMERIC,
  PROP_SNAP_TO_TICKS,
  PROP_UPDATE_POLICY,
  PROP_VALUE,
  PROP_WRAP,
  PROP_LAST_PROP,
};

static GParamSpec *props[PROP_LAST_PROP];

enum {
  SIGNAL_INPUT,
  SIGNAL_OUTPUT,
  SIGNAL_VALUE_CHANGED,
  SIGNAL_WRAPPED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static gboolean
boolean_handled_accumulator (GSignalInvocationHint *ihint,
                             GValue                *return_accu,
                             const GValue          *handler_return,
                             gpointer               dummy)
{
  gboolean continue_emission;
  gboolean signal_handled;

  signal_handled = g_value_get_boolean (handler_return);
  g_value_set_boolean (return_accu, signal_handled);
  continue_emission = !signal_handled;

  return continue_emission;
}

static gboolean
string_is_not_empty (AdwSpinRow *self,
                     const char *string)
{
  return string && string[0];
}

static void
pressed_cb (GtkGesture  *gesture,
            int          n_press,
            double       x,
            double       y,
            AdwSpinRow *self)
{
  GtkWidget *picked;

  picked = gtk_widget_pick (GTK_WIDGET (self), x, y, GTK_PICK_DEFAULT);

  if (picked != GTK_WIDGET (self) &&
      picked != self->header &&
      picked != GTK_WIDGET (self->prefixes) &&
      picked != GTK_WIDGET (self->suffixes)) {
    gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);

    return;
  }

  gtk_widget_grab_focus (GTK_WIDGET (self->spin_button));

  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_CLAIMED);
}

static void
spin_button_state_flags_changed_cb (AdwSpinRow *self)
{
  GtkStateFlags flags = gtk_widget_get_state_flags (self->spin_button);

  if (flags & GTK_STATE_FLAG_FOCUS_WITHIN) {
    gtk_widget_add_css_class (GTK_WIDGET (self), "focused");
  } else {
    gtk_widget_remove_css_class (GTK_WIDGET (self), "focused");
  }
}

static gboolean
spin_button_keynav_failed_cb (AdwSpinRow       *self,
                              GtkDirectionType  direction)
{
  if (direction == GTK_DIR_LEFT || direction == GTK_DIR_RIGHT)
    return gtk_widget_child_focus (GTK_WIDGET (self), direction);

  return GDK_EVENT_PROPAGATE;
}

static int
spin_button_input_cb (AdwSpinRow *self,
                      double     *new_value)
{
  int return_value;

  g_signal_emit (self, signals[SIGNAL_INPUT], 0, &new_value, &return_value);

  return return_value;
}

static gboolean
spin_button_output_cb (AdwSpinRow *self)
{
  gboolean return_value;

  g_signal_emit (self, signals[SIGNAL_OUTPUT], 0, &return_value);

  return return_value;
}

static void
spin_button_value_changed_cb (AdwSpinRow *self)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);

  g_signal_emit (self, signals[SIGNAL_VALUE_CHANGED], 0);
}

static void
spin_button_wrapped_cb (AdwSpinRow *self)
{
  g_signal_emit (self, signals[SIGNAL_WRAPPED], 0);
}

static gboolean
adw_spin_row_grab_focus (GtkWidget *widget)
{
  AdwSpinRow *self = ADW_SPIN_ROW (widget);

  return gtk_widget_grab_focus (self->spin_button);
}

static void
adw_spin_row_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwSpinRow *self = ADW_SPIN_ROW (object);

  if (gtk_editable_delegate_get_property (object, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_ADJUSTMENT:
    g_value_set_object (value, adw_spin_row_get_adjustment (self));
    break;
  case PROP_CLIMB_RATE:
    g_value_set_double (value, adw_spin_row_get_climb_rate (self));
    break;
  case PROP_DIGITS:
    g_value_set_uint (value, adw_spin_row_get_digits (self));
    break;
  case PROP_NUMERIC:
    g_value_set_boolean (value, adw_spin_row_get_numeric (self));
    break;
  case PROP_SNAP_TO_TICKS:
    g_value_set_boolean (value, adw_spin_row_get_snap_to_ticks (self));
    break;
  case PROP_UPDATE_POLICY:
    g_value_set_enum (value, adw_spin_row_get_update_policy (self));
    break;
  case PROP_VALUE:
    g_value_set_double (value, adw_spin_row_get_value (self));
    break;
  case PROP_WRAP:
    g_value_set_boolean (value, adw_spin_row_get_wrap (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_spin_row_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwSpinRow *self = ADW_SPIN_ROW (object);

  if (gtk_editable_delegate_set_property (object, prop_id, value, pspec))
    return;

  switch (prop_id) {
  case PROP_ADJUSTMENT:
    adw_spin_row_set_adjustment (self, g_value_get_object (value));
    break;
  case PROP_CLIMB_RATE:
    adw_spin_row_set_climb_rate (self, g_value_get_double (value));
    break;
  case PROP_DIGITS:
    adw_spin_row_set_digits (self, g_value_get_uint (value));
    break;
  case PROP_NUMERIC:
    adw_spin_row_set_numeric (self, g_value_get_boolean (value));
    break;
  case PROP_SNAP_TO_TICKS:
    adw_spin_row_set_snap_to_ticks (self, g_value_get_boolean (value));
    break;
  case PROP_UPDATE_POLICY:
    adw_spin_row_set_update_policy (self, g_value_get_enum (value));
    break;
  case PROP_VALUE:
    adw_spin_row_set_value (self, g_value_get_double (value));
    break;
  case PROP_WRAP:
    adw_spin_row_set_wrap (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_spin_row_class_init (AdwSpinRowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_spin_row_get_property;
  object_class->set_property = adw_spin_row_set_property;

  widget_class->focus = adw_widget_focus_child;
  widget_class->grab_focus = adw_spin_row_grab_focus;

  /**
   * AdwSpinRow:adjustment: (attributes org.gtk.Property.get=adw_spin_row_get_adjustment org.gtk.Property.set=adw_spin_row_set_adjustment)
   *
   * The adjustment that holds the value of the spin row.
   *
   * Since: 1.2
   */
  props[PROP_ADJUSTMENT] =
    g_param_spec_object ("adjustment", NULL, NULL,
                         GTK_TYPE_ADJUSTMENT,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:climb-rate: (attributes org.gtk.Property.get=adw_spin_row_get_climb_rate org.gtk.Property.set=adw_spin_row_set_climb_rate)
   *
   * The acceleration rate when you hold down a button or key.
   *
   * Since: 1.2
   */
  props[PROP_CLIMB_RATE] =
    g_param_spec_double ("climb-rate", NULL, NULL,
                         0.0, G_MAXDOUBLE, 0.0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:digits: (attributes org.gtk.Property.get=adw_spin_row_get_digits org.gtk.Property.set=adw_spin_row_set_digits)
   *
   * The number of decimal places to display.
   *
   * Since: 1.2
   */
  props[PROP_DIGITS] =
    g_param_spec_uint ("digits", NULL, NULL,
                       0, MAX_DIGITS, 0,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:numeric: (attributes org.gtk.Property.get=adw_spin_row_get_numeric org.gtk.Property.set=adw_spin_row_set_numeric)
   *
   * Whether non-numeric characters should be ignored.
   *
   * Since: 1.2
   */
  props[PROP_NUMERIC] =
    g_param_spec_boolean ("numeric", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:snap-to-ticks: (attributes org.gtk.Property.get=adw_spin_row_get_snap_to_ticks org.gtk.Property.set=adw_spin_row_set_snap_to_ticks)
   *
   * Whether invalid values are snapped to the spin row's nearest step increment.
   *
   * Since: 1.2
   */
  props[PROP_SNAP_TO_TICKS] =
    g_param_spec_boolean ("snap-to-ticks", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:update-policy: (attributes org.gtk.Property.get=adw_spin_row_get_update_policy org.gtk.Property.set=adw_spin_row_set_update_policy)
   *
   * The policy for updating the spin row.
   *
   * The options are always, or only when the value is accpetable.
   *
   * Since: 1.2
   */
  props[PROP_UPDATE_POLICY] =
    g_param_spec_enum ("update-policy", NULL, NULL,
                       GTK_TYPE_SPIN_BUTTON_UPDATE_POLICY,
                       GTK_UPDATE_ALWAYS,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:value: (attributes: org.gtk.Property.get=adw_spin_row_get_value org.gtk.Property.set=adw_spin_row_set_value)
   *
   * The current value.
   *
   * Since: 1.2
   */
  props[PROP_VALUE] =
    g_param_spec_double ("value", NULL, NULL,
                         -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSpinRow:wrap: (attributes: org.gtk.Property.get=adw_spin_row_get_wrap org.gtk.Property.set=adw_spin_row_set_wrap)
   *
   * Whether a spin row should wrap upon reaching its limits.
   *
   * Since: 1.2
   */
  props[PROP_WRAP] =
    g_param_spec_boolean ("wrap", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_editable_install_properties (object_class, PROP_LAST_PROP);

  /**
   * AdwSpinRow::input:
   *
   * Emitted to convert the users input into a double value.
   *
   * See [signal@Gtk.SpinButton::input].
   *
   * Since: 1.2
   */
  signals[SIGNAL_INPUT] =
    g_signal_new ("input",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_INT, 1,
                  G_TYPE_POINTER);

  /**
   * AdwSpinRow::output:
   *
   * Emitted to tweak the formatting of the value for display.
   *
   * See [signal@Gtk.SpinButton::output].
   *
   * Since: 1.2
   */
  signals[SIGNAL_OUTPUT] =
    g_signal_new ("output",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  boolean_handled_accumulator, NULL,
                  NULL,
                  G_TYPE_BOOLEAN, 0);

  /**
   * AdwSpinRow::value-changed:
   *
   * Emitted when the value is changed.
   *
   * See also: [signal@SpinRow::output]
   *
   * Since: 1.2
   */
  signals[SIGNAL_VALUE_CHANGED] =
    g_signal_new ("value-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

  /**
   * AdwSpinRow::wrapped:
   *
   * Emitted right after the spinbutton wraps.
   *
   * See [signal@Gtk.SpinButton::wrapped].
   *
   * Since: 1.2
   */
  signals[SIGNAL_WRAPPED] =
    g_signal_new ("wrapped",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-spin-row.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, header);
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, prefixes);
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, suffixes);
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, title_box);
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, title);
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, subtitle);
  gtk_widget_class_bind_template_child (widget_class, AdwSpinRow, spin_button);

  gtk_widget_class_bind_template_callback (widget_class, string_is_not_empty);
  gtk_widget_class_bind_template_callback (widget_class, pressed_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_state_flags_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_keynav_failed_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_input_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_output_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_value_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, spin_button_wrapped_cb);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_TEXT_BOX);
}

static void
adw_spin_row_init (AdwSpinRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  gtk_editable_init_delegate (GTK_EDITABLE (self));
}

static void
adw_spin_row_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  AdwSpinRow *self = ADW_SPIN_ROW (buildable);

  if (!self->header)
    parent_buildable_iface->add_child (buildable, builder, child, type);
  else if (g_strcmp0 (type, "prefix") == 0)
    adw_spin_row_add_prefix (self, GTK_WIDGET (child));
  else if (g_strcmp0 (type, "suffix") == 0)
    adw_spin_row_add_suffix (self, GTK_WIDGET (child));
  else if (!type && GTK_IS_WIDGET (child))
    adw_spin_row_add_suffix (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_spin_row_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->add_child = adw_spin_row_buildable_add_child;
}

static GtkEditable *
adw_spin_row_editable_get_delegate (GtkEditable *editable)
{
  AdwSpinRow *self = ADW_SPIN_ROW (editable);
  return GTK_EDITABLE (self->spin_button);
}

void
adw_spin_row_editable_init (GtkEditableInterface *iface)
{
    iface->get_delegate = adw_spin_row_editable_get_delegate;
}

/**
 * adw_spin_row_new:
 * @adjustment: (nullable): the adjustment that this spin row should use
 * @climb_rate: the rate the value changes when holding a button or key
 * @digits: the number of decimal places to display
 *
 * Creates a new `AdwSpinRow`.
 *
 * Returns: the newly created `AdwSpinRow`
 *
 * Since: 1.2
 */
GtkWidget *
adw_spin_row_new (GtkAdjustment *adjustment,
                  double         climb_rate,
                  guint          digits)
{
  AdwSpinRow *self = g_object_new (ADW_TYPE_SPIN_ROW, NULL);

  adw_spin_row_configure (self, adjustment, climb_rate, digits);

  return GTK_WIDGET (self);
}

/**
 * adw_spin_row_new_with_range:
 * @min: minimum allowable value
 * @max: maximum allowable value
 * @step: increment added or subtracted by spinning the widget
 *
 * Creates a new `AdwSpinRow` with the given properties.
 *
 * This is a convenience constructor that allows creation
 * of a numeric `AdwSpinRow` without manually creating an
 * adjustment. The value is initially set to the minimum
 * value and a page increment of 10 * @step is the default.
 * The precision of the spin row is equivalent to the
 * precisions of @step.
 *
 * Note that the way in which the precision is derived works
 * best if @step is a power of ten. If the resulting precision
 * is not suitable for your needs, use [method@SpinRow.set_digits]
 * to correct it.
 *
 * Returns: the new `AdwSpinRow`
 *
 * Since: 1.2
 */
GtkWidget *
adw_spin_row_new_with_range (double min,
                             double max,
                             double step)
{

  AdwSpinRow *self = g_object_new (ADW_TYPE_SPIN_ROW, NULL);
  GtkAdjustment *adjustment;
  int digits;

  g_return_val_if_fail (min <= max, NULL);
  g_return_val_if_fail (step != 0.0, NULL);

  adjustment = gtk_adjustment_new (min, min, max, step, 10 * step, 0);

  if (fabs (step) >= 1.0 || step == 0.0) {
      digits = 0;
  } else {
      digits = abs ((int) floor (log10 (fabs (step))));
      if (digits > MAX_DIGITS)
        digits = MAX_DIGITS;
  }

  adw_spin_row_configure (self, adjustment, step, digits);

  adw_spin_row_set_numeric (self, TRUE);

  return GTK_WIDGET (self);
}

/**
 * adw_spin_row_configure:
 * @self: a spin row
 * @adjustment: (nullable): the adjustment that this spin row should use
 * @climb_rate: the new climb rate
 * @digits: the number of decimal places to display
 *
 * Changes the properties of an existing spin row.
 *
 * The adjustment, climb rate, and number of decimal places are updated
 * accordingly.
 *
 * Since: 1.2
 */
void
adw_spin_row_configure (AdwSpinRow    *self,
                        GtkAdjustment *adjustment,
                        double         climb_rate,
                        guint          digits)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  g_object_freeze_notify (G_OBJECT (self));

  adw_spin_row_set_adjustment (self, adjustment);
  adw_spin_row_set_climb_rate (self, climb_rate);
  adw_spin_row_set_digits (self, digits);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_spin_row_get_adjustment:
 * @self: a spin row
 *
 * Gets the adjustment that holds the value for the spin row.
 *
 * Returns: the adjustment that holds the spin row's value
 *
 * Since: 1.2
 */
GtkAdjustment *
adw_spin_row_get_adjustment (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), NULL);

  return gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_adjustment:
 * @self: a spin row
 * @adjustment: an adjustment
 *
 * Sets the adjustment that holds the value for the spin row.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_adjustment (AdwSpinRow    *self,
                             GtkAdjustment *adjustment)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (adjustment == adw_spin_row_get_adjustment (self))
    return;

  gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (self->spin_button), adjustment);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ADJUSTMENT]);
}

/**
 * adw_spin_row_get_climb_rate:
 * @self: a spin row
 *
 * Gets the acceleration rate when you hold down a button or key.
 *
 * Returns: the acceleration rate when you hold down a button or key
 *
 * Since: 1.2
 */
double
adw_spin_row_get_climb_rate (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), 0.0);

  return gtk_spin_button_get_climb_rate (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_climb_rate:
 * @self: a spin row
 * @climb_rate: the acceleration rate when you hold down a button or key
 *
 * Sets the acceleration rate when you hold down a button or key.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_climb_rate (AdwSpinRow *self,
                             double      climb_rate)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (climb_rate == adw_spin_row_get_climb_rate (self))
    return;

  gtk_spin_button_set_climb_rate (GTK_SPIN_BUTTON (self->spin_button), climb_rate);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CLIMB_RATE]);
}

/**
 * adw_spin_row_get_digits:
 * @self: a spin row
 *
 * Gets the number of decimal places to display.
 *
 * Returns: the number of decimal places to display
 *
 * Since: 1.2
 */
guint
adw_spin_row_get_digits (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), 0);

  return gtk_spin_button_get_digits (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_get_digits:
 * @self: a spin row
 * @digits: the number of decimal places to display
 *
 * Sets the number of decimal places to display.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_digits (AdwSpinRow *self,
                         guint       digits)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (digits == adw_spin_row_get_digits (self))
    return;

  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (self->spin_button), digits);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DIGITS]);
}

/**
 * adw_spin_row_get_numeric:
 * @self: a spin row
 *
 * Gets whether non-numeric characters should be ignored.
 *
 * Returns: whether non-numeric characters should be ignored.
 *
 * Since: 1.2
 */
gboolean
adw_spin_row_get_numeric (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), FALSE);

  return gtk_spin_button_get_numeric (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_numeric:
 * @self: a spin row
 * @numeric: whether non-numeric characters should be ignored
 *
 * Sets whether non-numeric characters should be ignored.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_numeric (AdwSpinRow *self,
                          gboolean    numeric)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (numeric == adw_spin_row_get_numeric (self))
    return;

  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (self->spin_button), numeric);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NUMERIC]);
}

/**
 * adw_spin_row_get_snap_to_ticks:
 * @self: a spin row
 *
 * Gets whether invalid values are snapped to the spin row's nearest step increment.
 *
 * Returns: whether invalid values are automatically changed to the spin row's nearest step increment
 *
 * Since: 1.2
 */
gboolean
adw_spin_row_get_snap_to_ticks (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), FALSE);

  return gtk_spin_button_get_snap_to_ticks (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_snap_to_ticks:
 * @self: a spin row
 * @snap_to_ticks: whether invalid values are automatically changed to the spin row's nearest step increment
 *
 * Sets whether invalid values are snapped to the spin row's nearest step increment.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_snap_to_ticks (AdwSpinRow *self,
                                gboolean    snap_to_ticks)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (snap_to_ticks == adw_spin_row_get_snap_to_ticks (self))
    return;

  gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (self->spin_button), snap_to_ticks);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SNAP_TO_TICKS]);
}

/**
 * adw_spin_row_get_update_policy:
 * @self: a spin row
 *
 * Gets the policy for updating the spin row.
 *
 * See [property@SpinRow:update-policy].
 *
 * Returns: the policy for updating the spin row
 *
 * Since: 1.2
 */
GtkSpinButtonUpdatePolicy
adw_spin_row_get_update_policy (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), GTK_UPDATE_ALWAYS);

  return gtk_spin_button_get_update_policy (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_update_policy:
 * @self: a spin row
 * @policy: the policy for updating the spin row
 *
 * Sets the policy for updating the spin row.
 *
 * See [property@SpinRow:update-policy].
 *
 * Since: 1.2
 */
void
adw_spin_row_set_update_policy (AdwSpinRow *self,
                                GtkSpinButtonUpdatePolicy policy)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (policy == adw_spin_row_get_update_policy (self))
    return;

  gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (self->spin_button), policy);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_UPDATE_POLICY]);
}

/**
 * adw_spin_row_get_value:
 * @self: a spin row
 *
 * Gets the current value.
 *
 * Returns: the current value
 *
 * Since: 1.2
 */
double
adw_spin_row_get_value (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), 0.0);

  return gtk_spin_button_get_value (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_value:
 * @self: a spin row
 * @value: a new value
 *
 * Sets the current value.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_value (AdwSpinRow *self,
                        double      value)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (value == adw_spin_row_get_value (self))
    return;

  gtk_spin_button_set_value (GTK_SPIN_BUTTON (self->spin_button), value);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VALUE]);
}

/**
 * adw_spin_row_get_wrap:
 * @self: a spin row
 *
 * Gets whether the spin row should wrap upon reaching its limits.
 *
 * Returns: whether the spin row should wrap upon reaching its limits
 *
 * Since: 1.2
 */
gboolean
adw_spin_row_get_wrap (AdwSpinRow *self)
{
  g_return_val_if_fail (ADW_IS_SPIN_ROW (self), FALSE);

  return gtk_spin_button_get_wrap (GTK_SPIN_BUTTON (self->spin_button));
}

/**
 * adw_spin_row_set_wrap:
 * @self: a spin row
 * @wrap: whether the spin row should wrap upon reaching its limits
 *
 * Sets whether the spin row should wrap upon reaching its limits.
 *
 * Since: 1.2
 */
void
adw_spin_row_set_wrap (AdwSpinRow *self,
                       gboolean    wrap)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  if (wrap == adw_spin_row_get_wrap (self))
    return;

  gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (self->spin_button), wrap);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WRAP]);
}

/**
 * adw_spin_row_add_prefix:
 * @self: a spin row
 * @widget: a widget
 *
 * Adds a prefix widget to @self.
 *
 * Since: 1.2
 */
void
adw_spin_row_add_prefix (AdwSpinRow *self,
                         GtkWidget  *widget)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_box_prepend (GTK_BOX (self->prefixes), widget);
  gtk_widget_show (self->prefixes);
}

/**
 * adw_spin_row_add_suffix:
 * @self: a spin row
 * @widget: a widget
 *
 * Adds a suffix widget to @self.
 *
 * Since: 1.2
 */
void
adw_spin_row_add_suffix (AdwSpinRow *self,
                         GtkWidget  *widget)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_box_prepend (GTK_BOX (self->suffixes), widget);
  gtk_widget_show (self->suffixes);
}

/**
 * adw_spin_row_remove:
 * @self: a spin row
 * @widget: the child to be removed
 *
 * Removes a child from @self.
 *
 * Since: 1.2
 */
void
adw_spin_row_remove (AdwSpinRow *self,
                     GtkWidget  *child)
{
  GtkWidget *parent;

  g_return_if_fail (ADW_IS_SPIN_ROW (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  parent = gtk_widget_get_parent (child);

  if (parent == self->prefixes || parent == self->suffixes) {
    gtk_box_remove (GTK_BOX (parent), child);
    gtk_widget_set_visible (parent, gtk_widget_get_first_child (parent) != NULL);
  } else {
    ADW_CRITICAL_CANNOT_REMOVE_CHILD (self, child);
  }
}

/**
 * adw_spin_row_update:
 * @self: a spin row
 *
 * Manually force an update of the spin row.
 *
 * Since: 1.2
 */
void
adw_spin_row_update (AdwSpinRow *self)
{
  g_return_if_fail (ADW_IS_SPIN_ROW (self));

  gtk_spin_button_update (GTK_SPIN_BUTTON (self->spin_button));
}
