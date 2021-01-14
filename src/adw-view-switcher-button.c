/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-view-switcher-button-private.h"

/**
 * PRIVATE:adw-view-switcher-button
 * @short_description: Button used in #AdwViewSwitcher.
 * @title: AdwViewSwitcherButton
 * @See_also: #AdwViewSwitcher
 * @stability: Private
 *
 * #AdwViewSwitcherButton represents an application's view. It is designed to be
 * used exclusively internally by #AdwViewSwitcher.
 *
 * Since: 0.0.10
 */

#define TIMEOUT_EXPAND 500

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_NEEDS_ATTENTION,

  /* Overridden properties */
  PROP_LABEL,
  PROP_ORIENTATION,

  LAST_PROP = PROP_NEEDS_ATTENTION + 1,
};

struct _AdwViewSwitcherButton
{
  GtkToggleButton parent_instance;

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
  gchar *label;
  GtkOrientation orientation;

  guint switch_timer;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE (AdwViewSwitcherButton, adw_view_switcher_button, GTK_TYPE_TOGGLE_BUTTON,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

static gboolean
adw_view_switcher_button_switch_timeout (AdwViewSwitcherButton *self)
{
  self->switch_timer = 0;

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self), TRUE);

  return G_SOURCE_REMOVE;
}

static void
active_changed_cb (AdwViewSwitcherButton *self)
{
  g_assert (ADW_IS_VIEW_SWITCHER_BUTTON (self));

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self))) {
    gtk_stack_set_visible_child (self->horizontal_label_stack, GTK_WIDGET (self->horizontal_label_active));
    gtk_stack_set_visible_child (self->vertical_label_stack, GTK_WIDGET (self->vertical_label_active));
  } else {
    gtk_stack_set_visible_child (self->horizontal_label_stack, GTK_WIDGET (self->horizontal_label_inactive));
    gtk_stack_set_visible_child (self->vertical_label_stack, GTK_WIDGET (self->vertical_label_inactive));
  }
}

static void
drag_enter_cb (AdwViewSwitcherButton *self)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (self)))
    return;

  self->switch_timer = g_timeout_add (TIMEOUT_EXPAND,
                                      (GSourceFunc) adw_view_switcher_button_switch_timeout,
                                      self);
  g_source_set_name_by_id (self->switch_timer, "[gtk] adw_view_switcher_switch_timeout");
}

static void
drag_leave_cb (AdwViewSwitcherButton *self)
{
  g_clear_handle_id (&self->switch_timer, g_source_remove);
}

static GtkOrientation
get_orientation (AdwViewSwitcherButton *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self), GTK_ORIENTATION_HORIZONTAL);

  return self->orientation;
}

static void
set_orientation (AdwViewSwitcherButton *self,
                 GtkOrientation         orientation)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self));

  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  gtk_stack_set_visible_child (self->stack,
                               GTK_WIDGET (self->orientation == GTK_ORIENTATION_VERTICAL ?
                                             self->vertical_box :
                                             self->horizontal_box));
}

static void
adw_view_switcher_button_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  AdwViewSwitcherButton *self = ADW_VIEW_SWITCHER_BUTTON (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    g_value_set_string (value, adw_view_switcher_button_get_icon_name (self));
    break;
  case PROP_NEEDS_ATTENTION:
    g_value_set_boolean (value, adw_view_switcher_button_get_needs_attention (self));
    break;
  case PROP_LABEL:
    g_value_set_string (value, adw_view_switcher_button_get_label (self));
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
adw_view_switcher_button_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  AdwViewSwitcherButton *self = ADW_VIEW_SWITCHER_BUTTON (object);

  switch (prop_id) {
  case PROP_ICON_NAME:
    adw_view_switcher_button_set_icon_name (self, g_value_get_string (value));
    break;
  case PROP_NEEDS_ATTENTION:
    adw_view_switcher_button_set_needs_attention (self, g_value_get_boolean (value));
    break;
  case PROP_LABEL:
    adw_view_switcher_button_set_label (self, g_value_get_string (value));
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
adw_view_switcher_button_dispose (GObject *object)
{
  AdwViewSwitcherButton *self = ADW_VIEW_SWITCHER_BUTTON (object);

  g_clear_handle_id (&self->switch_timer, g_source_remove);

  G_OBJECT_CLASS (adw_view_switcher_button_parent_class)->dispose (object);
}

static void
adw_view_switcher_button_finalize (GObject *object)
{
  AdwViewSwitcherButton *self = ADW_VIEW_SWITCHER_BUTTON (object);

  g_free (self->icon_name);
  g_free (self->label);

  G_OBJECT_CLASS (adw_view_switcher_button_parent_class)->finalize (object);
}

static void
adw_view_switcher_button_class_init (AdwViewSwitcherButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_view_switcher_button_get_property;
  object_class->set_property = adw_view_switcher_button_set_property;
  object_class->dispose = adw_view_switcher_button_dispose;
  object_class->finalize = adw_view_switcher_button_finalize;

  g_object_class_override_property (object_class,
                                    PROP_LABEL,
                                    "label");

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdwViewSwitcherButton:icon-name:
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
   * AdwViewSwitcherButton:needs-attention:
   *
   * Sets a flag specifying whether the view requires the user attention. This
   * is used by the AdwViewSwitcher to change the appearance of the
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
                                               "/org/gnome/Adwaita/ui/adw-view-switcher-button.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, horizontal_box);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, horizontal_image);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, horizontal_label_active);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, horizontal_label_inactive);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, horizontal_label_stack);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, stack);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, vertical_box);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, vertical_image);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, vertical_label_active);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, vertical_label_inactive);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherButton, vertical_label_stack);
  gtk_widget_class_bind_template_callback (widget_class, active_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, drag_enter_cb);
  gtk_widget_class_bind_template_callback (widget_class, drag_leave_cb);
}

static void
adw_view_switcher_button_init (AdwViewSwitcherButton *self)
{
  self->icon_name = g_strdup ("image-missing");

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_stack_set_visible_child (GTK_STACK (self->stack), GTK_WIDGET (self->horizontal_box));

  gtk_widget_set_focus_on_click (GTK_WIDGET (self), FALSE);

  active_changed_cb (self);
}

/**
 * adw_view_switcher_button_new:
 *
 * Creates a new #AdwViewSwitcherButton widget.
 *
 * Returns: a new #AdwViewSwitcherButton
 *
 * Since: 0.0.10
 */
GtkWidget *
adw_view_switcher_button_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_BUTTON, NULL);
}

/**
 * adw_view_switcher_button_get_icon_name:
 * @self: a #AdwViewSwitcherButton
 *
 * Gets the icon name representing the view, or %NULL is no icon is set.
 *
 * Returns: (transfer none) (nullable): the icon name, or %NULL
 *
 * Since: 0.0.10
 **/
const gchar *
adw_view_switcher_button_get_icon_name (AdwViewSwitcherButton *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self), NULL);

  return self->icon_name;
}

/**
 * adw_view_switcher_button_set_icon_name:
 * @self: a #AdwViewSwitcherButton
 * @icon_name: (nullable): an icon name or %NULL
 *
 * Sets the icon name representing the view, or %NULL to disable the icon.
 *
 * Since: 0.0.10
 **/
void
adw_view_switcher_button_set_icon_name (AdwViewSwitcherButton *self,
                                        const gchar           *icon_name)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self));

  if (!g_strcmp0 (self->icon_name, icon_name))
    return;

  g_free (self->icon_name);

  if (icon_name && strlen (icon_name) > 0)
    self->icon_name = g_strdup (icon_name);
  else
    self->icon_name = g_strdup ("image-missing");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_NAME]);
}

/**
 * adw_view_switcher_button_get_needs_attention:
 * @self: a #AdwViewSwitcherButton
 *
 * Gets whether the view represented by @self requires the user attention.
 *
 * Returns: %TRUE if the view represented by @self requires the user attention, %FALSE otherwise
 *
 * Since: 0.0.10
 **/
gboolean
adw_view_switcher_button_get_needs_attention (AdwViewSwitcherButton *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self), FALSE);

  return gtk_widget_has_css_class (GTK_WIDGET (self), "needs-attention");
}

/**
 * adw_view_switcher_button_set_needs_attention:
 * @self: a #AdwViewSwitcherButton
 * @needs_attention: the new icon size
 *
 * Sets whether the view represented by @self requires the user attention.
 *
 * Since: 0.0.10
 */
void
adw_view_switcher_button_set_needs_attention (AdwViewSwitcherButton *self,
                                              gboolean               needs_attention)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self));

  needs_attention = !!needs_attention;

  if (gtk_widget_has_css_class (GTK_WIDGET (self), "needs-attention") == needs_attention)
    return;

  if (needs_attention)
    gtk_widget_add_css_class (GTK_WIDGET (self), "needs-attention");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (self), "needs-attention");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NEEDS_ATTENTION]);
}

/**
 * adw_view_switcher_button_get_label:
 * @self: a #AdwViewSwitcherButton
 *
 * Gets the label representing the view.
 *
 * Returns: (transfer none) (nullable): the label, or %NULL
 *
 * Since: 0.0.10
 **/
const gchar *
adw_view_switcher_button_get_label (AdwViewSwitcherButton *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self), NULL);

  return self->label;
}

/**
 * adw_view_switcher_button_set_label:
 * @self: a #AdwViewSwitcherButton
 * @label: (nullable): a label or %NULL
 *
 * Sets the label representing the view.
 *
 * Since: 0.0.10
 **/
void
adw_view_switcher_button_set_label (AdwViewSwitcherButton *self,
                                    const gchar           *label)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self));

  if (!g_strcmp0 (self->label, label))
    return;

  g_free (self->label);
  self->label = g_strdup (label);

  g_object_notify (G_OBJECT (self), "label");
}

/**
 * adw_view_switcher_button_set_narrow_ellipsize:
 * @self: a #AdwViewSwitcherButton
 * @mode: a #PangoEllipsizeMode
 *
 * Set the mode used to ellipsize the text in narrow mode if there is not
 * enough space to render the entire string.
 *
 * Since: 0.0.10
 **/
void
adw_view_switcher_button_set_narrow_ellipsize (AdwViewSwitcherButton *self,
                                               PangoEllipsizeMode     mode)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BUTTON (self));
  g_return_if_fail (mode >= PANGO_ELLIPSIZE_NONE && mode <= PANGO_ELLIPSIZE_END);

  gtk_label_set_ellipsize (self->vertical_label_active, mode);
  gtk_label_set_ellipsize (self->vertical_label_inactive, mode);
}

/**
 * adw_view_switcher_button_get_size:
 * @self: a #AdwViewSwitcherButton
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
adw_view_switcher_button_get_size (AdwViewSwitcherButton *self,
                                   gint                  *h_min_width,
                                   gint                  *h_nat_width,
                                   gint                  *v_min_width,
                                   gint                  *v_nat_width)
{
  /* gtk_widget_get_preferred_width() doesn't accept both its out parameters to
   * be NULL, so we must have guards.
   */
  if (h_min_width != NULL || h_nat_width != NULL)
    gtk_widget_measure (GTK_WIDGET (self->horizontal_box),
                        GTK_ORIENTATION_HORIZONTAL, -1,
                        h_min_width, h_nat_width, NULL, NULL);

  if (v_min_width != NULL || v_nat_width != NULL)
    gtk_widget_measure (GTK_WIDGET (self->vertical_box),
                        GTK_ORIENTATION_HORIZONTAL, -1,
                        v_min_width, v_nat_width, NULL, NULL);
}
