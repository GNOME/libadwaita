/*
 * Copyright (C) 2021 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-wrap-box.h"

#include "adw-enums.h"
#include "adw-widget-utils-private.h"

/**
 * AdwWrapBox:
 *
 * A box-like widget that can wrap into multiple lines.
 *
 * <picture>
 *   <source srcset="wrap-box-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="wrap-box.png" alt="wrap-box">
 * </picture>
 *
 * `AdwWrapBox` is similar to [class@Gtk.Box], but can wrap lines when the
 * widgets cannot fit otherwise. Unlike [class@Gtk.FlowBox], the children aren't
 * arranged into a grid and behave more like words in a wrapping label.
 *
 * Like `GtkBox`, `AdwWrapBox` is orientable and has spacing:
 *
 * - [property@WrapBox:child-spacing] between children in the same line;
 * - [property@WrapBox:line-spacing] between lines.
 *
 * ::: note
 *     Unlike `GtkBox`, `AdwWrapBox` cannot follow the CSS `border-spacing`
 *     property.
 *
 * Use the [property@WrapBox:natural-line-length] property to determine the
 * layout's natural size, e.g. when using it in a [class@Gtk.Popover].
 *
 * Normally, a horizontal `AdwWrapBox` wraps left to right and top to bottom
 * for left-to-right languages. Both of these directions can be reversed, using
 * the [property@WrapBox:pack-direction] and [property@WrapBox:wrap-reverse]
 * properties. Additionally, the alignment of each line can be controlled with
 * the [property@WrapBox:align] property.
 *
 * Lines can be justified using the [property@WrapBox:justify] property, filling
 * the entire line by either increasing child size or spacing depending on the
 * value. Set [property@WrapBox:justify-last-line] to justify the last line as
 * well.
 *
 * By default, `AdwWrapBox` wraps as soon as the previous line cannot fit any
 * more children without shrinking them past their natural size. Set
 * [property@WrapBox:wrap-policy] to [enum@Adw.WrapPolicy.MINIMUM] to only wrap
 * once all the children in the previous line have been shrunk to their minimum
 * size.
 *
 * To make each line take the same amount of space, set
 * [property@WrapBox:line-homogeneous] to `TRUE`.
 *
 * Spacing and natural line length can scale with the text scale factor, use the
 * [property@WrapBox:child-spacing-unit], [property@WrapBox:line-spacing-unit]
 * and/or [property@WrapBox:natural-line-length-unit] properties to enable that
 * behavior.
 *
 * See [class@WrapLayout].
 *
 * ## CSS nodes
 *
 * `AdwWrapBox` uses a single CSS node with name `wrap-box`.
 *
 * ## Accessibility
 *
 * `AdwWrapBox` uses the `GTK_ACCESSIBLE_ROLE_GROUP` role.
 *
 * Since: 1.7
 */

struct _AdwWrapBox
{
  GtkWidget parent_instance;
};

enum {
  PROP_0,
  PROP_CHILD_SPACING,
  PROP_CHILD_SPACING_UNIT,
  PROP_PACK_DIRECTION,
  PROP_ALIGN,
  PROP_JUSTIFY,
  PROP_JUSTIFY_LAST_LINE,
  PROP_LINE_SPACING,
  PROP_LINE_SPACING_UNIT,
  PROP_LINE_HOMOGENEOUS,
  PROP_NATURAL_LINE_LENGTH,
  PROP_NATURAL_LINE_LENGTH_UNIT,
  PROP_WRAP_REVERSE,
  PROP_WRAP_POLICY,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_WRAP_POLICY + 1,
};

static GParamSpec *props[LAST_PROP];

static void adw_wrap_box_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwWrapBox, adw_wrap_box, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_wrap_box_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

static GtkOrientation
get_orientation (AdwWrapBox *self)
{
  GtkLayoutManager *layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));

  return gtk_orientable_get_orientation (GTK_ORIENTABLE (layout));
}

static void
set_orientation (AdwWrapBox     *self,
                 GtkOrientation  orientation)
{
  GtkLayoutManager *layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));

  if (orientation == get_orientation (self))
    return;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (layout), orientation);

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adw_wrap_box_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwWrapBox *self = ADW_WRAP_BOX (object);

  switch (prop_id) {
  case PROP_CHILD_SPACING:
    g_value_set_int (value, adw_wrap_box_get_child_spacing (self));
    break;
  case PROP_CHILD_SPACING_UNIT:
    g_value_set_enum (value, adw_wrap_box_get_child_spacing_unit (self));
    break;
  case PROP_PACK_DIRECTION:
    g_value_set_enum (value, adw_wrap_box_get_pack_direction (self));
    break;
  case PROP_ALIGN:
    g_value_set_float (value, adw_wrap_box_get_align (self));
    break;
  case PROP_JUSTIFY:
    g_value_set_enum (value, adw_wrap_box_get_justify (self));
    break;
  case PROP_JUSTIFY_LAST_LINE:
    g_value_set_boolean (value, adw_wrap_box_get_justify_last_line (self));
    break;
  case PROP_LINE_SPACING:
    g_value_set_int (value, adw_wrap_box_get_line_spacing (self));
    break;
  case PROP_LINE_SPACING_UNIT:
    g_value_set_enum (value, adw_wrap_box_get_line_spacing_unit (self));
    break;
  case PROP_LINE_HOMOGENEOUS:
    g_value_set_boolean (value, adw_wrap_box_get_line_homogeneous (self));
    break;
  case PROP_NATURAL_LINE_LENGTH:
    g_value_set_int (value, adw_wrap_box_get_natural_line_length (self));
    break;
  case PROP_NATURAL_LINE_LENGTH_UNIT:
    g_value_set_enum (value, adw_wrap_box_get_natural_line_length_unit (self));
    break;
  case PROP_WRAP_REVERSE:
    g_value_set_boolean (value, adw_wrap_box_get_wrap_reverse (self));
    break;
  case PROP_WRAP_POLICY:
    g_value_set_enum (value, adw_wrap_box_get_wrap_policy (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_wrap_box_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwWrapBox *self = ADW_WRAP_BOX (object);

  switch (prop_id) {
  case PROP_CHILD_SPACING:
    adw_wrap_box_set_child_spacing (self, g_value_get_int (value));
    break;
  case PROP_CHILD_SPACING_UNIT:
    adw_wrap_box_set_child_spacing_unit (self, g_value_get_enum (value));
    break;
  case PROP_PACK_DIRECTION:
    adw_wrap_box_set_pack_direction (self, g_value_get_enum (value));
    break;
  case PROP_ALIGN:
    adw_wrap_box_set_align (self, g_value_get_float (value));
    break;
  case PROP_JUSTIFY:
    adw_wrap_box_set_justify (self, g_value_get_enum (value));
    break;
  case PROP_JUSTIFY_LAST_LINE:
    adw_wrap_box_set_justify_last_line (self, g_value_get_boolean (value));
    break;
  case PROP_LINE_SPACING:
    adw_wrap_box_set_line_spacing (self, g_value_get_int (value));
    break;
  case PROP_LINE_SPACING_UNIT:
    adw_wrap_box_set_line_spacing_unit (self, g_value_get_enum (value));
    break;
  case PROP_LINE_HOMOGENEOUS:
    adw_wrap_box_set_line_homogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_NATURAL_LINE_LENGTH:
    adw_wrap_box_set_natural_line_length (self, g_value_get_int (value));
    break;
  case PROP_NATURAL_LINE_LENGTH_UNIT:
    adw_wrap_box_set_natural_line_length_unit (self, g_value_get_enum (value));
    break;
  case PROP_WRAP_REVERSE:
    adw_wrap_box_set_wrap_reverse (self, g_value_get_boolean (value));
    break;
  case PROP_WRAP_POLICY:
    adw_wrap_box_set_wrap_policy (self, g_value_get_enum (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_wrap_box_dispose (GObject *object)
{
  GtkWidget *child;

  while ((child = gtk_widget_get_first_child (GTK_WIDGET (object))))
    gtk_widget_unparent (child);

  G_OBJECT_CLASS (adw_wrap_box_parent_class)->dispose (object);
}

static void
adw_wrap_box_class_init (AdwWrapBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_wrap_box_get_property;
  object_class->set_property = adw_wrap_box_set_property;
  object_class->dispose = adw_wrap_box_dispose;

  widget_class->compute_expand = adw_widget_compute_expand;

  /**
   * AdwWrapBox:child-spacing:
   *
   * The spacing between widgets on the same line.
   *
   * See [property@WrapBox:child-spacing-unit].
   *
   * Since: 1.7
   */
  props[PROP_CHILD_SPACING] =
    g_param_spec_int ("child-spacing", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:child-spacing-unit:
   *
   * The length unit for child spacing.
   *
   * Allows the spacing to vary depending on the text scale factor.
   *
   * See [property@WrapBox:child-spacing].
   *
   * Since: 1.7
   */
  props[PROP_CHILD_SPACING_UNIT] =
    g_param_spec_enum ("child-spacing-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_PX,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:pack-direction:
   *
   * The direction children are packed in each line.
   *
   * Since: 1.7
   */
  props[PROP_PACK_DIRECTION] =
    g_param_spec_enum ("pack-direction", NULL, NULL,
                       ADW_TYPE_PACK_DIRECTION,
                       ADW_PACK_START_TO_END,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:align:
   *
   * The alignment of the children within each line.
   *
   * 0 means the children are placed at the start of the line, 1 means they are
   * placed at the end of the line. 0.5 means they are placed in the middle of
   * the line.
   *
   * Alignment is only used when [property@WrapBox:justify] is set to
   * `ADW_JUSTIFY_NONE`, or on the last line when the
   * [property@WrapBox:justify-last-line] is `FALSE`.
   *
   * Since: 1.7
   */
  props[PROP_ALIGN] =
    g_param_spec_float ("align", NULL, NULL,
                        0, 1, 0,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:justify:
   *
   * Determines whether and how each complete line should be stretched to fill
   * the entire widget.
   *
   * If set to `ADW_JUSTIFY_FILL`, each widget in the line will be stretched,
   * keeping consistent spacing, so that the line fills the entire widget.
   *
   * If set to `ADW_JUSTIFY_SPREAD`, the spacing between widgets will be
   * increased, keeping widget sizes intact. The first and last widget will be
   * aligned with the beginning and end of the line. If the line only contains
   * a single widget, it will be stretched regardless.
   *
   * If set to `ADW_JUSTIFY_NONE`, the line will not be stretched and the
   * children will be placed together within the line, according to
   * [property@WrapBox:align].
   *
   * By default this doesn't affect the last line, as it will be incomplete. Use
   * [property@WrapBox:justify-last-line] to justify it as well.
   *
   * Since: 1.7
   */
  props[PROP_JUSTIFY] =
    g_param_spec_enum ("justify", NULL, NULL,
                       ADW_TYPE_JUSTIFY_MODE,
                       ADW_JUSTIFY_NONE,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:justify-last-line:
   *
   * Whether the last line should be stretched to fill the entire widget.
   *
   * See [property@WrapBox:justify].
   *
   * Since: 1.7
   */
  props[PROP_JUSTIFY_LAST_LINE] =
    g_param_spec_boolean ("justify-last-line", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:line-spacing:
   *
   * The spacing between lines.
   *
   * See [property@WrapBox:line-spacing-unit].
   *
   * Since: 1.7
   */
  props[PROP_LINE_SPACING] =
    g_param_spec_int ("line-spacing", NULL, NULL,
                      0, G_MAXINT, 0,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:line-spacing-unit:
   *
   * The length unit for line spacing.
   *
   * Allows the spacing to vary depending on the text scale factor.
   *
   * See [property@WrapBox:line-spacing].
   *
   * Since: 1.7
   */
  props[PROP_LINE_SPACING_UNIT] =
    g_param_spec_enum ("line-spacing-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_PX,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:line-homogeneous:
   *
   * Whether all lines should take the same amount of space.
   *
   * Since: 1.7
   */
  props[PROP_LINE_HOMOGENEOUS] =
    g_param_spec_boolean ("line-homogeneous", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:natural-line-length:
   *
   * Determines the natural size for each line.
   *
   * It should be used to limit the line lengths, for example when used in
   * popovers.
   *
   * See [property@WrapBox:natural-line-length-unit].
   *
   * Since: 1.7
   */
  props[PROP_NATURAL_LINE_LENGTH] =
    g_param_spec_int ("natural-line-length", NULL, NULL,
                      -1, G_MAXINT, -1,
                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:natural-line-length-unit:
   *
   * The length unit for natural line length.
   *
   * Allows the length to vary depending on the text scale factor.
   *
   * See [property@WrapBox:natural-line-length].
   *
   * Since: 1.7
   */
  props[PROP_NATURAL_LINE_LENGTH_UNIT] =
    g_param_spec_enum ("natural-line-length-unit", NULL, NULL,
                       ADW_TYPE_LENGTH_UNIT,
                       ADW_LENGTH_UNIT_PX,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:wrap-reverse:
   *
   * Whether wrap direction should be reversed.
   *
   * By default, lines wrap downwards in a horizontal box, and towards the end
   * in a vertical box. If set to `TRUE`, they wrap upwards or towards the start
   * respectively.
   *
   * Since: 1.7
   */
  props[PROP_WRAP_REVERSE] =
    g_param_spec_boolean ("wrap-reverse", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwWrapBox:wrap-policy:
   *
   * The policy for line wrapping.
   *
   + If set to `ADW_WRAP_NATURAL`, the box will wrap to the next line as soon as
   * the previous line cannot fit any more children without shrinking them past
   * their natural size.
   *
   * If set to `ADW_WRAP_MINIMUM`, the box will try to fit as many children into
   * each line as possible, shrinking them down to their minimum size before
   * wrapping to the next line.
   *
   * Since: 1.7
   */
  props[PROP_WRAP_POLICY] =
    g_param_spec_enum ("wrap-policy", NULL, NULL,
                       ADW_TYPE_WRAP_POLICY,
                       ADW_WRAP_NATURAL,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  g_object_class_override_property (object_class, PROP_ORIENTATION, "orientation");

  gtk_widget_class_set_layout_manager_type (widget_class, ADW_TYPE_WRAP_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "wrap-box");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_GROUP);
}

static void
adw_wrap_box_init (AdwWrapBox *self)
{
}

static void
adw_wrap_box_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  if (GTK_IS_WIDGET (child))
    adw_wrap_box_append (ADW_WRAP_BOX (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_wrap_box_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_wrap_box_buildable_add_child;
}

/**
 * adw_wrap_box_new:
 *
 * Creates a new `AdwWrapBox`.
 *
 * Returns: the newly created `AdwWrapBox`
 *
 * Since: 1.7
 */
GtkWidget *
adw_wrap_box_new (void)
{
  return g_object_new (ADW_TYPE_WRAP_BOX, NULL);
}

/**
 * adw_wrap_box_get_child_spacing:
 * @self: a wrap box
 *
 * Gets spacing between widgets on the same line.
 *
 * Returns: spacing between widgets on the same line
 *
 * Since: 1.7
 */
int
adw_wrap_box_get_child_spacing (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), 0);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_child_spacing (layout);
}

/**
 * adw_wrap_box_set_child_spacing:
 * @self: a wrap box
 * @child_spacing: the child spacing
 *
 * Sets the spacing between widgets on the same line.
 *
 * See [property@WrapBox:child-spacing-unit].
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_child_spacing (AdwWrapBox *self,
                                int         child_spacing)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  if (child_spacing < 0)
    child_spacing = 0;

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (child_spacing == adw_wrap_layout_get_child_spacing (layout))
    return;

  adw_wrap_layout_set_child_spacing (layout, child_spacing);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_SPACING]);
}

/**
 * adw_wrap_box_get_child_spacing_unit:
 * @self: a wrap box
 *
 * Gets the length unit for child spacing.
 *
 * Returns: the length unit
 *
 * Since: 1.7
 */
AdwLengthUnit
adw_wrap_box_get_child_spacing_unit (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), ADW_LENGTH_UNIT_PX);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_child_spacing_unit (layout);
}

/**
 * adw_wrap_box_set_child_spacing_unit:
 * @self: a wrap box
 * @unit: the length unit
 *
 * Sets the length unit for child spacing.
 *
 * Allows the spacing to vary depending on the text scale factor.
 *
 * See [property@WrapBox:child-spacing].
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_child_spacing_unit (AdwWrapBox    *self,
                                     AdwLengthUnit  unit)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (unit == adw_wrap_layout_get_child_spacing_unit (layout))
    return;

  adw_wrap_layout_set_child_spacing_unit (layout, unit);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CHILD_SPACING_UNIT]);
}

/**
 * adw_wrap_box_get_pack_direction:
 * @self: a wrap box
 *
 * Gets the direction children are packed in each line.
 *
 * Returns: the line direction
 *
 * Since: 1.7
 */
AdwPackDirection
adw_wrap_box_get_pack_direction (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), ADW_PACK_START_TO_END);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_pack_direction (layout);
}

/**
 * adw_wrap_box_set_pack_direction:
 * @self: a wrap box
 * @pack_direction: the new line direction
 *
 * Sets the direction children are packed in each line.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_pack_direction (AdwWrapBox       *self,
                                 AdwPackDirection  pack_direction)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (pack_direction >= ADW_PACK_START_TO_END);
  g_return_if_fail (pack_direction <= ADW_PACK_END_TO_START);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (pack_direction == adw_wrap_layout_get_pack_direction (layout))
    return;

  adw_wrap_layout_set_pack_direction (layout, pack_direction);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PACK_DIRECTION]);
}

/**
 * adw_wrap_box_get_align:
 * @self: a wrap box
 *
 * Gets the alignment of the children within each line.
 *
 * Returns: the child alignment
 *
 * Since: 1.7
 */
float
adw_wrap_box_get_align (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), 0.0f);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_align (layout);
}

/**
 * adw_wrap_box_set_align:
 * @self: a wrap box
 * @align: the child alignment
 *
 * Sets the alignment of the children within each line.
 *
 * 0 means the children are placed at the start of the line, 1 means they are
 * placed at the end of the line. 0.5 means they are placed in the middle of the
 * line.
 *
 * Alignment is only used when [property@WrapBox:justify] is set to
 * `ADW_JUSTIFY_NONE`, or on the last line when the
 * [property@WrapBox:justify-last-line] is `FALSE`.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_align (AdwWrapBox *self,
                        float       align)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (G_APPROX_VALUE (align, adw_wrap_layout_get_align (layout), FLT_EPSILON))
    return;

  adw_wrap_layout_set_align (layout, align);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALIGN]);
}

/**
 * adw_wrap_box_get_justify:
 * @self: a wrap box
 *
 * Gets whether and how each complete line is stretched to fill the entire widget.
 *
 * Returns: the justify mode
 *
 * Since: 1.7
 */
AdwJustifyMode
adw_wrap_box_get_justify (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), ADW_JUSTIFY_NONE);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_justify (layout);
}

/**
 * adw_wrap_box_set_justify:
 * @self: a wrap box
 * @justify: the justify mode
 *
 * Determines whether and how each complete line should be stretched to fill
 * the entire widget.
 *
 * If set to `ADW_JUSTIFY_FILL`, each widget in the line will be stretched,
 * keeping consistent spacing, so that the line fills the entire widget.
 *
 * If set to `ADW_JUSTIFY_SPREAD`, the spacing between widgets will be
 * increased, keeping widget sizes intact. The first and last widget will be
 * aligned with the beginning and end of the line. If the line only contains a
 * single widget, it will be stretched regardless.
 *
 * If set to `ADW_JUSTIFY_NONE`, the line will not be stretched and the children
 * will be placed together within the line, according to
 * [property@WrapBox:align].
 *
 * By default this doesn't affect the last line, as it will be incomplete. Use
 * [property@WrapBox:justify-last-line] to justify it as well.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_justify (AdwWrapBox     *self,
                          AdwJustifyMode  justify)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (justify >= ADW_JUSTIFY_NONE);
  g_return_if_fail (justify <= ADW_JUSTIFY_SPREAD);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (justify == adw_wrap_layout_get_justify (layout))
    return;

  adw_wrap_layout_set_justify (layout, justify);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_JUSTIFY]);
}

/**
 * adw_wrap_box_get_justify_last_line:
 * @self: a wrap box
 *
 * Gets whether the last line should be stretched to fill the entire widget.
 *
 * Returns: whether the last line is justified
 *
 * Since: 1.7
 */
gboolean
adw_wrap_box_get_justify_last_line (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), FALSE);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_justify_last_line (layout);
}

/**
 * adw_wrap_box_set_justify_last_line:
 * @self: a wrap box
 * @justify_last_line: whether to justify the last line
 *
 * Sets whether the last line should be stretched to fill the entire widget.
 *
 * See [property@WrapBox:justify].
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_justify_last_line (AdwWrapBox *self,
                                    gboolean    justify_last_line)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  justify_last_line = !!justify_last_line;

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (justify_last_line == adw_wrap_layout_get_justify_last_line (layout))
    return;

  adw_wrap_layout_set_justify_last_line (layout, justify_last_line);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_JUSTIFY_LAST_LINE]);
}

/**
 * adw_wrap_box_get_line_spacing:
 * @self: a wrap box
 *
 * Gets the spacing between lines.
 *
 * See [property@WrapBox:line-spacing-unit].
 *
 * Returns: the line spacing
 *
 * Since: 1.7
 */
int
adw_wrap_box_get_line_spacing (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), 0);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_line_spacing (layout);
}

/**
 * adw_wrap_box_set_line_spacing:
 * @self: a wrap box
 * @line_spacing: the line spacing
 *
 * Sets the spacing between lines.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_line_spacing (AdwWrapBox *self,
                               int         line_spacing)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (line_spacing < 0)
    line_spacing = 0;

  if (line_spacing == adw_wrap_layout_get_line_spacing (layout))
    return;

  adw_wrap_layout_set_line_spacing (layout, line_spacing);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINE_SPACING]);
}

/**
 * adw_wrap_box_get_line_spacing_unit:
 * @self: a wrap box
 *
 * Gets the length unit for line spacing.
 *
 * Returns: the length unit
 *
 * Since: 1.7
 */
AdwLengthUnit
adw_wrap_box_get_line_spacing_unit (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), ADW_LENGTH_UNIT_PX);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_line_spacing_unit (layout);
}

/**
 * adw_wrap_box_set_line_spacing_unit:
 * @self: a wrap box
 * @unit: the length unit
 *
 * Sets the length unit for line spacing.
 *
 * Allows the spacing to vary depending on the text scale factor.
 *
 * See [property@WrapBox:line-spacing].
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_line_spacing_unit (AdwWrapBox    *self,
                                    AdwLengthUnit  unit)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (unit == adw_wrap_layout_get_line_spacing_unit (layout))
    return;

  adw_wrap_layout_set_line_spacing_unit (layout, unit);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINE_SPACING_UNIT]);
}

/**
 * adw_wrap_box_get_line_homogeneous:
 * @self: a wrap box
 *
 * Gets whether all lines should take the same amount of space.
 *
 * Returns: whether lines should be homogeneous
 *
 * Since: 1.7
 */
gboolean
adw_wrap_box_get_line_homogeneous (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), FALSE);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_line_homogeneous (layout);
}

/**
 * adw_wrap_box_set_line_homogeneous:
 * @self: a wrap box
 * @homogeneous: whether lines should be homogeneous
 *
 * Sets whether all lines should take the same amount of space.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_line_homogeneous (AdwWrapBox *self,
                                   gboolean    homogeneous)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  homogeneous = !!homogeneous;

  if (homogeneous == adw_wrap_layout_get_line_homogeneous (layout))
    return;

  adw_wrap_layout_set_line_homogeneous (layout, homogeneous);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_LINE_HOMOGENEOUS]);
}

/**
 * adw_wrap_box_get_natural_line_length:
 * @self: a wrap box
 *
 * Gets the natural size for each line.
 *
 * Returns: the natural length
 *
 * Since: 1.7
 */
int
adw_wrap_box_get_natural_line_length (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), 0);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_natural_line_length (layout);
}

/**
 * adw_wrap_box_set_natural_line_length:
 * @self: a wrap box
 * @natural_line_length: the natural length
 *
 * Sets the natural size for each line.
 *
 * It should be used to limit the line lengths, for example when used in
 * popovers.
 *
 * See [property@WrapBox:natural-line-length-unit].
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_natural_line_length (AdwWrapBox *self,
                                      int         natural_line_length)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (natural_line_length < -1)
    natural_line_length = -1;

  if (natural_line_length == adw_wrap_layout_get_natural_line_length (layout))
    return;

  adw_wrap_layout_set_natural_line_length (layout, natural_line_length);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NATURAL_LINE_LENGTH]);
}

/**
 * adw_wrap_box_get_natural_line_length_unit:
 * @self: a wrap box
 *
 * Gets the length unit for line spacing.
 *
 * Returns: the length unit
 *
 * Since: 1.7
 */
AdwLengthUnit
adw_wrap_box_get_natural_line_length_unit (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), ADW_LENGTH_UNIT_PX);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_natural_line_length_unit (layout);
}

/**
 * adw_wrap_box_set_natural_line_length_unit:
 * @self: a wrap box
 * @unit: the length unit
 *
 * Sets the length unit for natural line length.
 *
 * Allows the length to vary depending on the text scale factor.
 *
 * See [property@WrapBox:natural-line-length].
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_natural_line_length_unit (AdwWrapBox    *self,
                                           AdwLengthUnit  unit)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (unit >= ADW_LENGTH_UNIT_PX);
  g_return_if_fail (unit <= ADW_LENGTH_UNIT_SP);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (unit == adw_wrap_layout_get_natural_line_length_unit (layout))
    return;

  adw_wrap_layout_set_natural_line_length_unit (layout, unit);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NATURAL_LINE_LENGTH_UNIT]);
}

/**
 * adw_wrap_box_get_wrap_reverse:
 * @self: a wrap box
 *
 * Gets whether wrap direction is reversed.
 *
 * Returns: whether wrap direction is reversed
 *
 * Since: 1.7
 */
gboolean
adw_wrap_box_get_wrap_reverse (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), FALSE);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_wrap_reverse (layout);
}

/**
 * adw_wrap_box_set_wrap_reverse:
 * @self: a wrap box
 * @wrap_reverse: whether to reverse wrap direction
 *
 * Sets whether wrap direction should be reversed.
 *
 * By default, lines wrap downwards in a horizontal box, and towards the end
 * in a vertical box. If set to `TRUE`, they wrap upwards or towards the start
 * respectively.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_wrap_reverse (AdwWrapBox *self,
                               gboolean    wrap_reverse)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  wrap_reverse = !!wrap_reverse;

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (wrap_reverse == adw_wrap_layout_get_wrap_reverse (layout))
    return;

  adw_wrap_layout_set_wrap_reverse (layout, wrap_reverse);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WRAP_REVERSE]);
}

/**
 * adw_wrap_box_get_wrap_policy:
 * @self: a wrap box
 *
 * Gets the policy for line wrapping.
 *
 * Returns: the wrap policy
 *
 * Since: 1.7
 */
AdwWrapPolicy
adw_wrap_box_get_wrap_policy (AdwWrapBox *self)
{
  AdwWrapLayout *layout;

  g_return_val_if_fail (ADW_IS_WRAP_BOX (self), ADW_WRAP_MINIMUM);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  return adw_wrap_layout_get_wrap_policy (layout);
}

/**
 * adw_wrap_box_set_wrap_policy:
 * @self: a wrap box
 * @wrap_policy: the new wrap policy
 *
 * Sets the policy for line wrapping.
 *
 * If set to `ADW_WRAP_NATURAL`, the box will wrap to the next line as soon as
 * the previous line cannot fit any more children without shrinking them past
 * their natural size.
 *
 * If set to `ADW_WRAP_MINIMUM`, the box will try to fit as many children into
 * each line as possible, shrinking them down to their minimum size before
 * wrapping to the next line.
 *
 * Since: 1.7
 */
void
adw_wrap_box_set_wrap_policy (AdwWrapBox    *self,
                              AdwWrapPolicy  wrap_policy)
{
  AdwWrapLayout *layout;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (wrap_policy >= ADW_WRAP_MINIMUM);
  g_return_if_fail (wrap_policy <= ADW_WRAP_NATURAL);

  layout = ADW_WRAP_LAYOUT (gtk_widget_get_layout_manager (GTK_WIDGET (self)));

  if (wrap_policy == adw_wrap_layout_get_wrap_policy (layout))
    return;

  adw_wrap_layout_set_wrap_policy (layout, wrap_policy);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_WRAP_POLICY]);
}

/**
 * adw_wrap_box_insert_child_after:
 * @self: a wrap box
 * @child: the widget to insert
 * @sibling: (nullable): the sibling after which to insert @child
 *
 * Inserts @child in the position after @sibling in the list of @self children.
 *
 * If @sibling is `NULL`, inserts @child at the first position.
 *
 * Since: 1.7
 */
void
adw_wrap_box_insert_child_after (AdwWrapBox *self,
                                 GtkWidget  *child,
                                 GtkWidget  *sibling)
{
  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  if (sibling) {
    g_return_if_fail (GTK_IS_WIDGET (sibling));
    g_return_if_fail (gtk_widget_get_parent (sibling) == GTK_WIDGET (self));
  }

  if (child == sibling)
    return;

  gtk_widget_insert_after (child, GTK_WIDGET (self), sibling);
}

/**
 * adw_wrap_box_reorder_child_after:
 * @self: a wrap box
 * @child: the widget to move, must be a child of @self
 * @sibling: (nullable): the sibling to move @child after
 *
 * Moves @child to the position after @sibling in the list of @self children.
 *
 * If @sibling is `NULL`, moves @child to the first position.
 *
 * Since: 1.7
 */
void
adw_wrap_box_reorder_child_after (AdwWrapBox *self,
                                  GtkWidget  *child,
                                  GtkWidget  *sibling)
{
  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));

  if (sibling) {
    g_return_if_fail (GTK_IS_WIDGET (sibling));
    g_return_if_fail (gtk_widget_get_parent (sibling) == GTK_WIDGET (self));
  }

  if (child == sibling)
    return;

  gtk_widget_insert_after (child, GTK_WIDGET (self), sibling);
}

/**
 * adw_wrap_box_append:
 * @self: a wrap box
 * @child: the widget to append
 *
 * Adds @child as the last child to @self.
 *
 * Since: 1.7
 */
void
adw_wrap_box_append (AdwWrapBox *self,
                     GtkWidget  *child)
{
  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  gtk_widget_insert_before (child, GTK_WIDGET (self), NULL);
}

/**
 * adw_wrap_box_prepend:
 * @self: a wrap box
 * @child: the widget to prepend
 *
 * Adds @child as the first child to @self.
 *
 * Since: 1.7
 */
void
adw_wrap_box_prepend (AdwWrapBox *self,
                      GtkWidget  *child)
{
  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  gtk_widget_insert_after (child, GTK_WIDGET (self), NULL);
}

/**
 * adw_wrap_box_remove:
 * @self: a wrap box
 * @child: the child to remove
 *
 * Removes a child widget from @self.
 *
 * The child must have been added before with [method@Adw.WrapBox.append],
 * [method@Adw.WrapBox.prepend], or [method@Adw.WrapBox.insert_child_after].
 *
 * Since: 1.7
 */
void
adw_wrap_box_remove (AdwWrapBox *self,
                     GtkWidget  *child)
{
  g_return_if_fail (ADW_IS_WRAP_BOX (self));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (gtk_widget_get_parent (child) == GTK_WIDGET (self));

  gtk_widget_unparent (child);
}

/**
 * adw_wrap_box_remove_all:
 * @self: a wrap box
 *
 * Removes all children from @self.
 *
 * Since: 1.8
 */
void
adw_wrap_box_remove_all (AdwWrapBox *self)
{
  GtkWidget *widget;

  g_return_if_fail (ADW_IS_WRAP_BOX (self));

  while ((widget = gtk_widget_get_first_child (GTK_WIDGET (self))))
    adw_wrap_box_remove (self, widget);
}
