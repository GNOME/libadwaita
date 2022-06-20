/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-enums.h"
#include "adw-macros-private.h"
#include "adw-view-switcher-bar.h"

/**
 * AdwViewSwitcherBar:
 *
 * A view switcher action bar.
 *
 * <picture>
 *   <source srcset="view-switcher-bar-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="view-switcher-bar.png" alt="view-switcher-bar">
 * </picture>
 *
 * An action bar letting you switch between multiple views contained in a
 * [class@ViewStack], via an [class@ViewSwitcher]. It is designed to be put at
 * the bottom of a window and to be revealed only on really narrow windows, e.g.
 * on mobile phones. It can't be revealed if there are less than two pages.
 *
 * `AdwViewSwitcherBar` is intended to be used together with
 * [class@ViewSwitcherTitle].
 *
 * A common use case is to bind the [property@ViewSwitcherBar:reveal] property
 * to [property@ViewSwitcherTitle:title-visible] to automatically reveal the
 * view switcher bar when the title label is displayed in place of the view
 * switcher, as follows:
 *
 * ```xml
 * <object class="GtkWindow">
 *   <child type="titlebar">
 *     <object class="AdwHeaderBar">
 *       <property name="centering-policy">strict</property>
 *       <child type="title">
 *         <object class="AdwViewSwitcherTitle" id="title">
 *           <property name="stack">stack</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="GtkBox">
 *       <property name="orientation">vertical</property>
 *       <child>
 *         <object class="AdwViewStack" id="stack"/>
 *       </child>
 *       <child>
 *         <object class="AdwViewSwitcherBar">
 *           <property name="stack">stack</property>
 *           <binding name="reveal">
 *             <lookup name="title-visible">title</lookup>
 *           </binding>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 * </object>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwViewSwitcherBar` has a single CSS node with name` viewswitcherbar`.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_STACK,
  PROP_REVEAL,
  LAST_PROP,
};

struct _AdwViewSwitcherBar
{
  GtkWidget parent_instance;

  GtkWidget *action_bar;
  AdwViewSwitcher *view_switcher;

  GtkSelectionModel *pages;
  gboolean reveal;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_FINAL_TYPE (AdwViewSwitcherBar, adw_view_switcher_bar, GTK_TYPE_WIDGET)

static void
update_bar_revealed (AdwViewSwitcherBar *self)
{
  int count = 0;

  if (!self->action_bar)
    return;

  if (self->reveal && self->pages) {
    guint i, n;

    n = g_list_model_get_n_items (G_LIST_MODEL (self->pages));
    for (i = 0; i < n; i++) {
      AdwViewStackPage *page = g_list_model_get_item (G_LIST_MODEL (self->pages), i);

      if (adw_view_stack_page_get_visible (page))
        count++;

      g_object_unref (page);
    }
  }

  gtk_action_bar_set_revealed (GTK_ACTION_BAR (self->action_bar), count > 1);
}

static void
adw_view_switcher_bar_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwViewSwitcherBar *self = ADW_VIEW_SWITCHER_BAR (object);

  switch (prop_id) {
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

  adw_view_switcher_bar_set_stack (self, NULL);
  gtk_widget_unparent (self->action_bar);

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
   * AdwViewSwitcherBar:stack: (attributes org.gtk.Property.get=adw_view_switcher_bar_get_stack org.gtk.Property.set=adw_view_switcher_bar_set_stack)
   *
   * The stack the view switcher controls.
   *
   * Since: 1.0
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack", NULL, NULL,
                         ADW_TYPE_VIEW_STACK,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewSwitcherBar:reveal: (attributes org.gtk.Property.get=adw_view_switcher_bar_get_reveal org.gtk.Property.set=adw_view_switcher_bar_set_reveal)
   *
   * Whether the bar should be revealed or hidden.
   *
   * Since: 1.0
   */
  props[PROP_REVEAL] =
    g_param_spec_boolean ("reveal", NULL, NULL,
                         FALSE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

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
  gtk_widget_init_template (GTK_WIDGET (self));

  update_bar_revealed (self);
}

/**
 * adw_view_switcher_bar_new:
 *
 * Creates a new `AdwViewSwitcherBar`.
 *
 * Returns: the newly created `AdwViewSwitcherBar`
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_switcher_bar_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER_BAR, NULL);
}

/**
 * adw_view_switcher_bar_get_stack: (attributes org.gtk.Method.get_property=stack)
 * @self: a view switcher bar
 *
 * Gets the stack controlled by @self.
 *
 * Returns: (nullable) (transfer none): the stack
 *
 * Since: 1.0
 */
AdwViewStack *
adw_view_switcher_bar_get_stack (AdwViewSwitcherBar *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self), NULL);

  return adw_view_switcher_get_stack (self->view_switcher);
}

/**
 * adw_view_switcher_bar_set_stack: (attributes org.gtk.Method.set_property=stack)
 * @self: a view switcher bar
 * @stack: (nullable): a stack
 *
 * Sets the stack controlled by @self.
 *
 * Since: 1.0
 */
void
adw_view_switcher_bar_set_stack (AdwViewSwitcherBar *self,
                                 AdwViewStack       *stack)
{
  AdwViewStack *previous_stack;

  g_return_if_fail (ADW_IS_VIEW_SWITCHER_BAR (self));
  g_return_if_fail (stack == NULL || ADW_IS_VIEW_STACK (stack));

  previous_stack = adw_view_switcher_get_stack (self->view_switcher);

  if (previous_stack == stack)
    return;

  if (previous_stack) {
    g_signal_handlers_disconnect_by_func (self->pages, G_CALLBACK (update_bar_revealed), self);
    g_clear_object (&self->pages);
  }

  adw_view_switcher_set_stack (self->view_switcher, stack);

  if (stack) {
    self->pages = adw_view_stack_get_pages (stack);

    g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (update_bar_revealed), self);
  }

  update_bar_revealed (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * adw_view_switcher_bar_get_reveal: (attributes org.gtk.Method.get_property=reveal)
 * @self: a view switcher bar
 *
 * Gets whether @self should be revealed or hidden.
 *
 * Returns: whether @self is revealed
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
 * adw_view_switcher_bar_set_reveal: (attributes org.gtk.Method.set_property=reveal)
 * @self: a view switcher bar
 * @reveal: whether to reveal @self
 *
 * Sets whether @self should be revealed or hidden.
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
