/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-view-switcher-button-private.h"

/**
 * PRIVATE:hdy-view-switcher-button
 * @short_description: Button used in #HdyViewSwitcher.
 * @title: HdyViewSwitcherButton
 * @See_also: #HdyViewSwitcher
 * @stability: Private
 *
 * #HdyViewSwitcherButton represents an application's view. It is designed to be
 * used exclusively internally by #HdyViewSwitcher.
 *
 * Since: 0.0.10
 */

enum {
  PROP_0,
  PROP_ICON_SIZE,
  PROP_ICON_NAME,
  PROP_NEEDS_ATTENTION,

  /* Overridden properties */
  PROP_LABEL,
  PROP_ORIENTATION,

  LAST_PROP = PROP_NEEDS_ATTENTION + 1,
};

struct _HdyViewSwitcherButton
{
  GtkRadioButton parent_instance;

  GtkBox *horizontal_box;
  GtkImage *horizontal_image;
  GtkLabel *horizontal_label_active;
  GtkLabel *horizontal_label_inactive;
  GtkStack *horizontal_label_stack;
  GtkStack *stack;
  GtkBox *vertical_box;
  GtkImage *vertical_image;
  GtkLabel *vertical_label_active;
  GtkLabel *vertical_label_inactive;
  GtkStack *vertical_label_stack;

  gchar *icon_name;
  GtkIconSize icon_size;
  gchar *label;
  GtkOrientation orientation;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE (HdyViewSwitcherButton, hdy_view_switcher_button, GTK_TYPE_RADIO_BUTTON,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static void
on_active_changed (HdyViewSwitcherButton *self)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self))) {
    gtk_stack_set_visible_child (self->horizontal_label_stack, GTK_WIDGET (self->horizontal_label_active));
    gtk_stack_set_visible_child (self->vertical_label_stack, GTK_WIDGET (self->vertical_label_active));
  } else {
    gtk_stack_set_visible_child (self->horizontal_label_stack, GTK_WIDGET (self->horizontal_label_inactive));
    gtk_stack_set_visible_child (self->vertical_label_stack, GTK_WIDGET (self->vertical_label_inactive));
  }
}

static GtkOrientation
get_orientation (HdyViewSwitcherButton *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self), GTK_ORIENTATION_HORIZONTAL);

  return self->orientation;
}

static void
set_orientation (HdyViewSwitcherButton *self,
                 GtkOrientation         orientation)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));

  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_stack_set_visible_child (self->stack,
                               GTK_WIDGET (self->orientation == GTK_ORIENTATION_VERTICAL ?
                                             self->vertical_box :
                                             self->horizontal_box));
}

static void
hdy_view_switcher_button_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  HdyViewSwitcherButton *self = HDY_VIEW_SWITCHER_BUTTON (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, hdy_view_switcher_button_get_icon_name (self));
    break;
  case PROP_ICON_SIZE:
    g_value_set_int (value, hdy_view_switcher_button_get_icon_size (self));
    break;
  case PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, hdy_view_switcher_button_get_needs_attention (self));
    break;
  case PROP_LABEL:
    g_value_set_string (value, hdy_view_switcher_button_get_label (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_button_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  HdyViewSwitcherButton *self = HDY_VIEW_SWITCHER_BUTTON (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    hdy_view_switcher_button_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_ICON_SIZE:
    hdy_view_switcher_button_set_icon_size (self, g_value_get_int (value));
    break;
  case PROP_NEEDS_ATTENTION:
    hdy_view_switcher_button_set_needs_attention (self, g_value_get_boolean (value));
    break;
  case PROP_LABEL:
    hdy_view_switcher_button_set_label (self, g_value_get_string (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_button_finalize (GObject *object)
{
  HdyViewSwitcherButton *self = HDY_VIEW_SWITCHER_BUTTON (object);

  g_free (self->icon_name);
  g_free (self->label);

  G_OBJECT_CLASS (hdy_view_switcher_button_parent_class)->finalize (object);
}

static void
hdy_view_switcher_button_class_init (HdyViewSwitcherButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_view_switcher_button_get_property;
  object_class->set_property = hdy_view_switcher_button_set_property;
  object_class->finalize = hdy_view_switcher_button_finalize;

  g_object_class_override_property (object_class,
                                    PROP_LABEL,
                                    "label");

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * HdyViewSwitcherButton:icon-name:
   *
   * The icon name representing the view, or %NULL for no icon.
   *
   * Since: 0.0.10
   */
  props[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         _("Icon Name"),
                         _("Icon name for image"),
                         "text-x-generic-symbolic",
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

  /**
   * HdyViewSwitcherButton:icon-size:
   *
   * The icon size.
   *
   * Since: 0.0.10
   */
  props[PROP_ICON_SIZE] =
    g_param_spec_int ("icon-size",
                      _("Icon Size"),
                      _("Symbolic size to use for named icon"),
                      0, G_MAXINT, GTK_ICON_SIZE_BUTTON,
                      G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

  /**
   * HdyViewSwitcherButton:needs-attention:
   *
   * Sets a flag specifying whether the view requires the user attention. This
   * is used by the HdyViewSwitcher to change the appearance of the
   * corresponding button when a view needs attention and it is not the current
   * one.
   *
   * Since: 0.0.10
   */
  props[PROP_NEEDS_ATTENTION] =
  g_param_spec_boolean ("needs-attention",
                        _("Needs attention"),
                        _("Hint the view needs attention"),
                        FALSE,
                        G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /* We probably should set the class's CSS name to "viewswitcherbutton"
   * here, but it doesn't work because GtkCheckButton hardcodes it to "button"
   * on instantiation, and the functions required to override it are private.
   * In the meantime, we can use the "viewswitcher > button" CSS selector as
   * a fairly safe fallback.
   */

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-view-switcher-button.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, horizontal_box);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, horizontal_image);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, horizontal_label_active);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, horizontal_label_inactive);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, horizontal_label_stack);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, stack);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, vertical_box);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, vertical_image);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, vertical_label_active);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, vertical_label_inactive);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherButton, vertical_label_stack);
  gtk_widget_class_bind_template_callback (widget_class, on_active_changed);
}

static void
hdy_view_switcher_button_init (HdyViewSwitcherButton *self)
{
  self->icon_size = GTK_ICON_SIZE_BUTTON;

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_stack_set_visible_child (GTK_STACK (self->stack), GTK_WIDGET (self->horizontal_box));

  gtk_widget_set_focus_on_click (GTK_WIDGET (self), FALSE);
  /* Make the button look like a regular button and not a radio button. */
  gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (self), FALSE);

  on_active_changed (self);
}

/**
 * hdy_view_switcher_button_new:
 *
 * Creates a new #HdyViewSwitcherButton widget.
 *
 * Returns: a new #HdyViewSwitcherButton
 *
 * Since: 0.0.10
 */
GtkWidget *
hdy_view_switcher_button_new (void)
{
  return g_object_new (HDY_TYPE_VIEW_SWITCHER_BUTTON, NULL);
}

/**
 * hdy_view_switcher_button_get_icon_name:
 * @self: a #HdyViewSwitcherButton
 *
 * Gets the icon name representing the view, or %NULL is no icon is set.
 *
 * Returns: (transfer none) (nullable): the icon name, or %NULL
 *
 * Since: 0.0.10
 **/
const gchar *
hdy_view_switcher_button_get_icon_name (HdyViewSwitcherButton *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self), NULL);

  return self->icon_name;
}

/**
 * hdy_view_switcher_button_set_icon_name:
 * @self: a #HdyViewSwitcherButton
 * @icon_name: (nullable): an icon name or %NULL
 *
 * Sets the icon name representing the view, or %NULL to disable the icon.
 *
 * Since: 0.0.10
 **/
void
hdy_view_switcher_button_set_icon_name (HdyViewSwitcherButton *self,
                                        const gchar           *icon_name)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));

  if (!g_strcmp0 (self->icon_name, icon_name))
    return;

  g_free (self->icon_name);
  self->icon_name = g_strdup (icon_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * hdy_view_switcher_button_get_icon_size:
 * @self: a #HdyViewSwitcherButton
 *
 * Gets the icon size used by @self.
 *
 * Returns: the icon size used by @self
 *
 * Since: 0.0.10
 **/
GtkIconSize
hdy_view_switcher_button_get_icon_size (HdyViewSwitcherButton *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self), GTK_ICON_SIZE_INVALID);

  return self->icon_size;
}

/**
 * hdy_view_switcher_button_set_icon_size:
 * @self: a #HdyViewSwitcherButton
 * @icon_size: the new icon size
 *
 * Sets the icon size used by @self.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_button_set_icon_size (HdyViewSwitcherButton *self,
                                        GtkIconSize            icon_size)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));

  if (self->icon_size == icon_size)
    return;

  self->icon_size = icon_size;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_SIZE]);
}

/**
 * hdy_view_switcher_button_get_needs_attention:
 * @self: a #HdyViewSwitcherButton
 *
 * Gets whether the view represented by @self requires the user attention.
 *
 * Returns: %TRUE if the view represented by @self requires the user attention, %FALSE otherwise
 *
 * Since: 0.0.10
 **/
gboolean
hdy_view_switcher_button_get_needs_attention (HdyViewSwitcherButton *self)
{
  GtkStyleContext *context;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self), FALSE);

  context = gtk_widget_get_style_context (GTK_WIDGET (self));

  return gtk_style_context_has_class (context, GTK_STYLE_CLASS_NEEDS_ATTENTION);
}

/**
 * hdy_view_switcher_button_set_needs_attention:
 * @self: a #HdyViewSwitcherButton
 * @needs_attention: the new icon size
 *
 * Sets whether the view represented by @self requires the user attention.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_button_set_needs_attention (HdyViewSwitcherButton *self,
                                              gboolean               needs_attention)
{
  GtkStyleContext *context;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));

  needs_attention = !!needs_attention;

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  if (gtk_style_context_has_class (context, GTK_STYLE_CLASS_NEEDS_ATTENTION) == needs_attention)
    return;

  if (needs_attention)
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_NEEDS_ATTENTION);
  else
    gtk_style_context_remove_class (context, GTK_STYLE_CLASS_NEEDS_ATTENTION);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NEEDS_ATTENTION]);
}

/**
 * hdy_view_switcher_button_get_label:
 * @self: a #HdyViewSwitcherButton
 *
 * Gets the label representing the view.
 *
 * Returns: (transfer none) (nullable): the label, or %NULL
 *
 * Since: 0.0.10
 **/
const gchar *
hdy_view_switcher_button_get_label (HdyViewSwitcherButton *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self), NULL);

  return self->label;
}

/**
 * hdy_view_switcher_button_set_label:
 * @self: a #HdyViewSwitcherButton
 * @label: (nullable): a label or %NULL
 *
 * Sets the label representing the view.
 *
 * Since: 0.0.10
 **/
void
hdy_view_switcher_button_set_label (HdyViewSwitcherButton *self,
                                    const gchar           *label)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));

  if (!g_strcmp0 (self->label, label))
    return;

  g_free (self->label);
  self->label = g_strdup (label);

  g_object_notify (G_OBJECT (self), "label");
}

/**
 * hdy_view_switcher_button_set_narrow_ellipsize:
 * @self: a #HdyViewSwitcherButton
 * @mode: a #PangoEllipsizeMode
 *
 * Set the mode used to ellipsize the text in narrow mode if there is not
 * enough space to render the entire string.
 *
 * Since: 0.0.10
 **/
void
hdy_view_switcher_button_set_narrow_ellipsize (HdyViewSwitcherButton *self,
                                               PangoEllipsizeMode     mode)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BUTTON (self));
  g_return_if_fail (mode >= PANGO_ELLIPSIZE_NONE && mode <= PANGO_ELLIPSIZE_END);

  gtk_label_set_ellipsize (self->vertical_label_active, mode);
  gtk_label_set_ellipsize (self->vertical_label_inactive, mode);
}

/**
 * hdy_view_switcher_button_get_size:
 * @self: a #HdyViewSwitcherButton
 * @h_min_width: (out) (nullable): the minimum width when horizontal
 * @h_nat_width: (out) (nullable): the natural width when horizontal
 * @v_min_width: (out) (nullable): the minimum width when vertical
 * @v_nat_width: (out) (nullable): the natural width when vertical
 *
 * Measure the size requests in both horizontal and vertical modes.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_button_get_size (HdyViewSwitcherButton *self,
                                   gint                  *h_min_width,
                                   gint                  *h_nat_width,
                                   gint                  *v_min_width,
                                   gint                  *v_nat_width)
{
  GtkStyleContext *context;
  GtkStateFlags state;
  GtkBorder border;

  /* gtk_widget_get_preferred_width() doesn't accept both its out parameters to
   * be NULL, so we must have guards.
   */
  if (h_min_width != NULL || h_nat_width != NULL)
    gtk_widget_get_preferred_width (GTK_WIDGET (self->horizontal_box), h_min_width, h_nat_width);
  if (v_min_width != NULL || v_nat_width != NULL)
    gtk_widget_get_preferred_width (GTK_WIDGET (self->vertical_box), v_min_width, v_nat_width);

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  state = gtk_style_context_get_state (context);
  gtk_style_context_get_border (context, state, &border);
  if (h_min_width != NULL)
    *h_min_width += border.left + border.right;
  if (h_nat_width != NULL)
    *h_nat_width += border.left + border.right;
  if (v_min_width != NULL)
    *v_min_width += border.left + border.right;
  if (v_nat_width != NULL)
    *v_nat_width += border.left + border.right;
}
