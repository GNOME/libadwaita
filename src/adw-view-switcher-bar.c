/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "adw-enums.h"
#include "adw-view-switcher-bar.h"

/**
 * SECTION:adw-view-switcher-bar
 * @short_description: A view switcher action bar.
 * @title: AdwViewSwitcherBar
 * @See_also: #AdwViewSwitcher, #AdwViewSwitcherTitle
 *
 * An action bar letting you switch between multiple views offered by a
 * #GtkStack, via an #AdwViewSwitcher. It is designed to be put at the bottom of
 * a window and to be revealed only on really narrow windows e.g. on mobile
 * phones. It can't be revealed if there are less than two pages.
 *
 * You can conveniently bind the #AdwViewSwitcherBar:reveal property to
 * #AdwViewSwitcherTitle:title-visible to automatically reveal the view switcher
 * bar when the title label is displayed in place of the view switcher.
 *
 * An example of the UI definition for a common use case:
 * |[
 * <object class="GtkWindow"/>
 *   <child type="titlebar">
 *     <object class="AdwHeaderBar">
 *       <property name="centering-policy">strict</property>
 *       <child type="title">
 *         <object class="AdwViewSwitcherTitle"
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
 *         <object class="AdwViewSwitcherBar">
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
 * #AdwViewSwitcherBar has a single CSS node with name viewswitcherbar.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_POLICY,
  PROP_STACK,
  PROP_REVEAL,
  LAST_PROP,
};

struct _AdwViewSwitcherBar
{
  GtkWidget parent_instance;

  GtkWidget *action_bar;
  GtkRevealer *revealer;
  AdwViewSwitcher *view_switcher;

  AdwViewSwitcherPolicy policy;
  GtkSelectionModel *pages;
  gboolean reveal;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (AdwViewSwitcherBar, adw_view_switcher_bar, GTK_TYPE_WIDGET)

static void
update_bar_revealed (AdwViewSwitcherBar *self) {
  int count = 0;

  if (!self->revealer)
    return;

  if (self->reveal && self->pages) {
    guint i, n;

    n = g_list_model_get_n_items (G_LIST_MODEL (self->pages));
    for (i = 0; i < n; i++) {
      GtkStackPage *page = g_list_model_get_item (G_LIST_MODEL (self->pages), i);

      if (gtk_stack_page_get_visible (page))
        count++;
    }
  }

  gtk_revealer_set_reveal_child (self->revealer, count > 1);
}

static void
adw_view_switcher_bar_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwViewSwitcherBar *self = ADW_VIEW_SWITCHER_BAR (object);

  switch (prop_id) {
  case PROP_POLICY:
    g_value_set_enum (value, adw_view_switcher_bar_get_policy (self));
    break;
  case PROP_STACK:
    g_value_set_object (value, adw_view_switcher_bar_get_stack (self));
    break;
  case PROP_REVEAL:
    g_value_set_boolean (value, adw_view_switcher_bar_get_reveal (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_bar_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwViewSwitcherBar *self = ADW_VIEW_SWITCHER_BAR (object);

  switch (prop_id) {
  case PROP_POLICY:
    adw_view_switcher_bar_set_policy (self, g_value_get_enum (value));
    break;
  case PROP_STACK:
    adw_view_switcher_bar_set_stack (self, g_value_get_object (value));
    break;
  case PROP_REVEAL:
    adw_view_switcher_bar_set_reveal (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_bar_dispose (GObject *object)
{
  AdwViewSwitcherBar *self = ADW_VIEW_SWITCHER_BAR (object);

  gtk_widget_unparent (self->action_bar);
  self->revealer = NULL;

  G_OBJECT_CLASS (adw_view_switcher_bar_parent_class)->dispose (object);
}

static void
adw_view_switcher_bar_class_init (AdwViewSwitcherBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_view_switcher_bar_get_property;
  object_class->set_property = adw_view_switcher_bar_set_property;
  object_class->dispose = adw_view_switcher_bar_dispose;

  /**
   * AdwViewSwitcherBar:policy:
   *
   * The #AdwViewSwitcherPolicy the #AdwViewSwitcher should use to determine
   * which mode to use.
   *
   * Since: 1.0
   */
  props[PROP_POLICY] =
    g_param_spec_enum ("policy",
                       _("Policy"),
                       _("The policy to determine the mode to use"),
                       ADW_TYPE_VIEW_SWITCHER_POLICY, ADW_VIEW_SWITCHER_POLICY_NARROW,
                       G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwViewSwitcherBar:stack:
   *
   * The #GtkStack the #AdwViewSwitcher controls.
   *
   * Since: 1.0
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack",
                         _("Stack"),
                         _("Stack"),
                         GTK_TYPE_STACK,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwViewSwitcherBar:reveal:
   *
   * Whether the bar should be revealed or hidden.
   *
   * Since: 1.0
   */
  props[PROP_REVEAL] =
    g_param_spec_boolean ("reveal",
                         _("Reveal"),
                         _("Whether the view switcher is revealed"),
                         FALSE,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "viewswitcherbar");
  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-view-switcher-bar.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherBar, action_bar);
  gtk_widget_class_bind_template_child (widget_class, AdwViewSwitcherBar, view_switcher);
}

static void
adw_view_switcher_bar_init (AdwViewSwitcherBar *self)
{
  /* This must be initialized before the template so the embedded view switcher
   * can pick up the correct default value.
   */
  self->policy = ADW_VIEW_SWITCHER_POLICY_NARROW;

  gtk_widget_init_template (GTK_WIDGET (self));

  self->revealer = GTK_REVEALER (gtk_widget_get_first_child (GTK_WIDGET (self->action_bar)));
  update_bar_revealed (self);
  gtk_revealer_set_transition_type (self->revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
}

/**
 * adw_view_switcher_bar_new:
 *
 * Creates a new #AdwViewSwitcherBar widget.
 *
 * Returns: a new #AdwViewSwitcherBar
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_switcher_bar_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_BAR, NULL);
}

/**
 * adw_view_switcher_bar_get_policy:
 * @self: a #AdwViewSwitcherBar
 *
 * Gets the policy of @self.
 *
 * Returns: the policy of @self
 *
 * Since: 1.0
 */
AdwViewSwitcherPolicy
adw_view_switcher_bar_get_policy (AdwViewSwitcherBar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self), ADW_VIEW_SWITCHER_POLICY_NARROW);

  return self->policy;
}

/**
 * adw_view_switcher_bar_set_policy:
 * @self: a #AdwViewSwitcherBar
 * @policy: the new policy
 *
 * Sets the policy of @self.
 *
 * Since: 1.0
 */
void
adw_view_switcher_bar_set_policy (AdwViewSwitcherBar    *self,
                                  AdwViewSwitcherPolicy  policy)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self));

  if (self->policy == policy)
    return;

  self->policy = policy;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * adw_view_switcher_bar_get_stack:
 * @self: a #AdwViewSwitcherBar
 *
 * Get the #GtkStack being controlled by the #AdwViewSwitcher.
 *
 * Returns: (nullable) (transfer none): the #GtkStack, or %NULL if none has been set
 *
 * Since: 1.0
 */
GtkStack *
adw_view_switcher_bar_get_stack (AdwViewSwitcherBar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self), NULL);

  return adw_view_switcher_get_stack (self->view_switcher);
}

/**
 * adw_view_switcher_bar_set_stack:
 * @self: a #AdwViewSwitcherBar
 * @stack: (nullable): a #GtkStack
 *
 * Sets the #GtkStack to control.
 *
 * Since: 1.0
 */
void
adw_view_switcher_bar_set_stack (AdwViewSwitcherBar *self,
                                 GtkStack           *stack)
{
  GtkStack *previous_stack;

  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self));
  g_return_if_fail (stack == NULL || GTK_IS_STACK (stack));

  previous_stack = adw_view_switcher_get_stack (self->view_switcher);

  if (previous_stack == stack)
    return;

  if (previous_stack) {
    g_signal_handlers_disconnect_by_func (self->pages, G_CALLBACK (update_bar_revealed), self);
    g_clear_object (&self->pages);
  }

  adw_view_switcher_set_stack (self->view_switcher, stack);

  if (stack) {
    self->pages = gtk_stack_get_pages (stack);

    g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (update_bar_revealed), self);
  }

  update_bar_revealed (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * adw_view_switcher_bar_get_reveal:
 * @self: a #AdwViewSwitcherBar
 *
 * Gets whether @self should be revealed or not.
 *
 * Returns: %TRUE if @self is revealed, %FALSE if not.
 *
 * Since: 1.0
 */
gboolean
adw_view_switcher_bar_get_reveal (AdwViewSwitcherBar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self), FALSE);

  return self->reveal;
}

/**
 * adw_view_switcher_bar_set_reveal:
 * @self: a #AdwViewSwitcherBar
 * @reveal: %TRUE to reveal @self
 *
 * Sets whether @self should be revealed or not.
 *
 * Since: 1.0
 */
void
adw_view_switcher_bar_set_reveal (AdwViewSwitcherBar *self,
                                  gboolean            reveal)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self));

  reveal = !!reveal;

  if (self->reveal == reveal)
    return;

  self->reveal = reveal;
  update_bar_revealed (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL]);
}
