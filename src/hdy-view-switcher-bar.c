/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-enums.h"
#include "hdy-view-switcher-bar.h"

/**
 * SECTION:hdy-view-switcher-bar
 * @short_description: A view switcher action bar.
 * @title: HdyViewSwitcherBar
 * @See_also: #HdyViewSwitcher, #HdyViewSwitcherTitle
 *
 * An action bar letting you switch between multiple views offered by a
 * #GtkStack, via an #HdyViewSwitcher. It is designed to be put at the bottom of
 * a window and to be revealed only on really narrow windows e.g. on mobile
 * phones. It can't be revealed if there are less than two pages.
 *
 * You can conveniently bind the #HdyViewSwitcherBar:reveal property to
 * #HdyViewSwitcherTitle:title-visible to automatically reveal the view switcher
 * bar when the title label is displayed in place of the view switcher.
 *
 * An example of the UI definition for a common use case:
 * |[
 * <object class="GtkWindow"/>
 *   <child type="titlebar">
 *     <object class="HdyHeaderBar">
 *       <property name="centering-policy">strict</property>
 *       <child type="title">
 *         <object class="HdyViewSwitcherTitle"
 *                 id="view_switcher_title">
 *           <property name="stack">stack</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="GtkBox">
 *       <child>
 *         <object class="GtkStack" id="stack"/>
 *       </child>
 *       <child>
 *         <object class="HdyViewSwitcherBar">
 *           <property name="stack">stack</property>
 *           <property name="reveal"
 *                     bind-source="view_switcher_title"
 *                     bind-property="title-visible"
 *                     bind-flags="sync-create"/>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 * </object>
 * ]|
 *
 * # CSS nodes
 *
 * #HdyViewSwitcherBar has a single CSS node with name viewswitcherbar.
 *
 * Since: 0.0.10
 */

enum {
  PROP_0,
  PROP_POLICY,
  PROP_STACK,
  PROP_REVEAL,
  LAST_PROP,
};

struct _HdyViewSwitcherBar
{
  GtkBin parent_instance;

  GtkActionBar *action_bar;
  GtkRevealer *revealer;
  HdyViewSwitcher *view_switcher;

  HdyViewSwitcherPolicy policy;
  gboolean reveal;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (HdyViewSwitcherBar, hdy_view_switcher_bar, GTK_TYPE_BIN)

static void
count_children_cb (GtkWidget *widget,
                   gint      *count)
{
  (*count)++;
}

static void
update_bar_revealed (HdyViewSwitcherBar *self) {
  GtkStack *stack = hdy_view_switcher_get_stack (self->view_switcher);
  gint count = 0;

  if (self->reveal && stack)
    gtk_container_foreach (GTK_CONTAINER (stack), (GtkCallback) count_children_cb, &count);

  gtk_revealer_set_reveal_child (self->revealer, count > 1);
}

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

  gtk_widget_class_set_css_name (widget_class, "viewswitcherbar");

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-view-switcher-bar.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherBar, action_bar);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherBar, view_switcher);
}

static void
hdy_view_switcher_bar_init (HdyViewSwitcherBar *self)
{
  /* This must be initialized before the template so the embedded view switcher
   * can pick up the correct default value.
   */
  self->policy = HDY_VIEW_SWITCHER_POLICY_NARROW;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->revealer = GTK_REVEALER (gtk_bin_get_child (GTK_BIN (self->action_bar)));
  update_bar_revealed (self);
  gtk_revealer_set_transition_type (self->revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
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
GtkWidget *
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
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), HDY_VIEW_SWITCHER_POLICY_NARROW);

  return self->policy;
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
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));

  if (self->policy == policy)
    return;

  self->policy = policy;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
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
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), NULL);

  return hdy_view_switcher_get_stack (self->view_switcher);
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
  GtkStack *previous_stack;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));
  g_return_if_fail (stack == NULL || GTK_IS_STACK (stack));

  previous_stack = hdy_view_switcher_get_stack (self->view_switcher);

  if (previous_stack == stack)
    return;

  if (previous_stack)
    g_signal_handlers_disconnect_by_func (previous_stack, G_CALLBACK (update_bar_revealed), self);

  hdy_view_switcher_set_stack (self->view_switcher, stack);

  if (stack) {
    g_signal_connect_swapped (stack, "add", G_CALLBACK (update_bar_revealed), self);
    g_signal_connect_swapped (stack, "remove", G_CALLBACK (update_bar_revealed), self);
  }

  update_bar_revealed (self);

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
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self), FALSE);

  return self->reveal;
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
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_BAR (self));

  reveal = !!reveal;

  if (self->reveal == reveal)
    return;

  self->reveal = reveal;
  update_bar_revealed (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL]);
}
