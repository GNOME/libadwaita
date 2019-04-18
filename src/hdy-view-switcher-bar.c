/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-enums.h"
#include "hdy-style-private.h"
#include "hdy-view-switcher-bar.h"

/**
 * SECTION:hdy-view-switcher-bar
 * @short_description: An view switcher action bar
 * @title: HdyViewSwitcherBar
 *
 * An action bar letting you switch between multiple views offered by a
 * #GtkStack, via a #HdyViewSwitcher. It is designed to be put at the bottom of
 * a window and to be revealed only on really narrow windows e.g. on mobile
 * phones.
 *
 * Since: 0.0.10
 */

enum {
  PROP_0,
  PROP_POLICY,
  PROP_ICON_SIZE,
  PROP_STACK,
  PROP_REVEAL,
  LAST_PROP,
};

typedef struct {
  GtkActionBar *action_bar;
  GtkRevealer *revealer;
  HdyViewSwitcher *view_switcher;

  HdyViewSwitcherPolicy policy;
  GtkIconSize icon_size;
  gboolean reveal;
} HdyViewSwitcherBarPrivate;

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_CODE (HdyViewSwitcherBar, hdy_view_switcher_bar, GTK_TYPE_BIN,
                         G_ADD_PRIVATE (HdyViewSwitcherBar))

static void
hdy_view_switcher_bar_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  HdyViewSwitcherBar *self = HDY_VIEW_SWITCHER_BAR (object);

  switch (prop_id) {
  case PROP_POLICY:
    g_value_set_enum (value, hdy_view_switcher_bar_get_policy (self));
    break;
  case PROP_ICON_SIZE:
    g_value_set_int (value, hdy_view_switcher_bar_get_icon_size (self));
    break;
  case PROP_STACK:
    g_value_set_object (value, hdy_view_switcher_bar_get_stack (self));
    break;
  case PROP_REVEAL:
    g_value_set_boolean (value, hdy_view_switcher_bar_get_reveal (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_bar_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  HdyViewSwitcherBar *self = HDY_VIEW_SWITCHER_BAR (object);

  switch (prop_id) {
  case PROP_POLICY:
    hdy_view_switcher_bar_set_policy (self, g_value_get_enum (value));
    break;
  case PROP_ICON_SIZE:
    hdy_view_switcher_bar_set_icon_size (self, g_value_get_int (value));
    break;
  case PROP_STACK:
    hdy_view_switcher_bar_set_stack (self, g_value_get_object (value));
    break;
  case PROP_REVEAL:
    hdy_view_switcher_bar_set_reveal (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_bar_class_init (HdyViewSwitcherBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_view_switcher_bar_get_property;
  object_class->set_property = hdy_view_switcher_bar_set_property;

  /**
   * HdyViewSwitcherBar:policy:
   *
   * The #HdyViewSwitcherPolicy the #HdyViewSwitcher should use to determine
   * which mode to use.
   *
   * Since: 0.0.10
   */
  props[PROP_POLICY] =
    g_param_spec_enum ("policy",
                       _("Policy"),
                       _("The policy to determine the mode to use"),
                       HDY_TYPE_VIEW_SWITCHER_POLICY, HDY_VIEW_SWITCHER_POLICY_NARROW,
                       G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherBar:icon-size:
   *
   * Use the "icon-size" property to hint the icons to use, you almost certainly
   * want to leave this as %GTK_ICON_SIZE_BUTTON.
   *
   * Since: 0.0.10
   */
  props[PROP_ICON_SIZE] =
    g_param_spec_int ("icon-size",
                      _("Icon Size"),
                      _("Symbolic size to use for named icon"),
                      0, G_MAXINT, GTK_ICON_SIZE_BUTTON,
                      G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherBar:stack:
   *
   * The #GtkStack the #HdyViewSwitcher controls.
   *
   * Since: 0.0.10
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack",
                         _("Stack"),
                         _("Stack"),
                         GTK_TYPE_STACK,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherBar:reveal:
   *
   * Whether the bar should be revealed or hidden.
   *
   * Since: 0.0.10
   */
  props[PROP_REVEAL] =
    g_param_spec_boolean ("reveal",
                         _("Reveal"),
                         _("Whether the view switcher is revealed"),
                         FALSE,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "hdyviewswitcherbar");

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-view-switcher-bar.ui");
  gtk_widget_class_bind_template_child_private (widget_class, HdyViewSwitcherBar, action_bar);
  gtk_widget_class_bind_template_child_private (widget_class, HdyViewSwitcherBar, view_switcher);
}

static void
hdy_view_switcher_bar_init (HdyViewSwitcherBar *self)
{
  HdyViewSwitcherBarPrivate *priv;
  g_autoptr (GtkCssProvider) provider = gtk_css_provider_new ();
  GtkWidget *box;

  priv = hdy_view_switcher_bar_get_instance_private (self);

  /* This must be initialized before the template so the embedded view switcher
   * can pick up the correct default value.
   */
  priv->policy = HDY_VIEW_SWITCHER_POLICY_NARROW;
  priv->icon_size = GTK_ICON_SIZE_BUTTON;

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->revealer = GTK_REVEALER (gtk_bin_get_child (GTK_BIN (priv->action_bar)));
  g_object_bind_property (self, "reveal", priv->revealer, "reveal-child", G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  gtk_revealer_set_transition_type (priv->revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);

  box = gtk_bin_get_child (GTK_BIN (priv->revealer));

  gtk_css_provider_load_from_resource (provider, "/sm/puri/handy/style/hdy-view-switcher-bar-box.css");
  gtk_style_context_add_provider (gtk_widget_get_style_context (box),
                                  GTK_STYLE_PROVIDER (provider),
                                  HDY_STYLE_PROVIDER_PRIORITY);
}

/**
 * hdy_view_switcher_bar_new:
 *
 * Creates a new #HdyViewSwitcherBar widget.
 *
 * Returns: a new #HdyViewSwitcherBar
 *
 * Since: 0.0.10
 */
HdyViewSwitcherBar *
hdy_view_switcher_bar_new (void)
{
  return g_object_new (HDY_TYPE_VIEW_SWITCHER_BAR, NULL);
}

/**
 * hdy_view_switcher_bar_get_policy:
 * @self: a #HdyViewSwitcherBar
 *
 * Gets the policy of @self.
 *
 * Returns: the policy of @self
 *
 * Since: 0.0.10
 */
HdyViewSwitcherPolicy
hdy_view_switcher_bar_get_policy (HdyViewSwitcherBar *self)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), HDY_VIEW_SWITCHER_POLICY_NARROW);

  priv = hdy_view_switcher_bar_get_instance_private (self);

  return priv->policy;
}

/**
 * hdy_view_switcher_bar_set_policy:
 * @self: a #HdyViewSwitcherBar
 * @policy: the new policy
 *
 * Sets the policy of @self.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_bar_set_policy (HdyViewSwitcherBar    *self,
                                  HdyViewSwitcherPolicy  policy)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));

  priv = hdy_view_switcher_bar_get_instance_private (self);

  if (priv->policy == policy)
    return;

  priv->policy = policy;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * hdy_view_switcher_bar_get_icon_size:
 * @self: a #HdyViewSwitcherBar
 *
 * Get the icon size of the images used in the #HdyViewSwitcher.
 *
 * Returns: the icon size of the images
 *
 * Since: 0.0.10
 */
GtkIconSize
hdy_view_switcher_bar_get_icon_size (HdyViewSwitcherBar *self)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), GTK_ICON_SIZE_BUTTON);

  priv = hdy_view_switcher_bar_get_instance_private (self);

  return priv->icon_size;
}

/**
 * hdy_view_switcher_bar_set_icon_size:
 * @self: a #HdyViewSwitcherBar
 * @icon_size: the new icon size
 *
 * Change the icon size hint for the icons in a #HdyViewSwitcher.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_bar_set_icon_size (HdyViewSwitcherBar *self,
                                     GtkIconSize         icon_size)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));

  priv = hdy_view_switcher_bar_get_instance_private (self);

  if (priv->icon_size == icon_size)
    return;

  priv->icon_size = icon_size;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_SIZE]);
}

/**
 * hdy_view_switcher_bar_get_stack:
 * @self: a #HdyViewSwitcherBar
 *
 * Get the #GtkStack being controlled by the #HdyViewSwitcher.
 *
 * Returns: (nullable) (transfer none): the #GtkStack, or %NULL if none has been set
 *
 * Since: 0.0.10
 */
GtkStack *
hdy_view_switcher_bar_get_stack (HdyViewSwitcherBar *self)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), NULL);

  priv = hdy_view_switcher_bar_get_instance_private (self);

  return hdy_view_switcher_get_stack (priv->view_switcher);
}

/**
 * hdy_view_switcher_bar_set_stack:
 * @self: a #HdyViewSwitcherBar
 * @stack: (nullable): a #GtkStack
 *
 * Sets the #GtkStack to control.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_bar_set_stack (HdyViewSwitcherBar *self,
                                 GtkStack           *stack)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));
  g_return_if_fail (stack == NULL || GTK_IS_STACK (stack));

  priv = hdy_view_switcher_bar_get_instance_private (self);

  if (hdy_view_switcher_get_stack (priv->view_switcher) == stack)
    return;

  hdy_view_switcher_set_stack (priv->view_switcher, stack);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * hdy_view_switcher_bar_get_reveal:
 * @self: a #HdyViewSwitcherBar
 *
 * Gets whether @self should be revealed or not.
 *
 * Returns: %TRUE if @self is revealed, %FALSE if not.
 *
 * Since: 0.0.10
 */
gboolean
hdy_view_switcher_bar_get_reveal (HdyViewSwitcherBar *self)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), FALSE);

  priv = hdy_view_switcher_bar_get_instance_private (self);

  return priv->reveal;
}

/**
 * hdy_view_switcher_bar_set_reveal:
 * @self: a #HdyViewSwitcherBar
 * @reveal: %TRUE to reveal @self
 *
 * Sets whether @self should be revealed or not.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_bar_set_reveal (HdyViewSwitcherBar *self,
                                  gboolean            reveal)
{
  HdyViewSwitcherBarPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));

  priv = hdy_view_switcher_bar_get_instance_private (self);

  reveal = !!reveal;

  if (priv->reveal == reveal)
    return;

  priv->reveal = reveal;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL]);
}
