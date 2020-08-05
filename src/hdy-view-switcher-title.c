/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-view-switcher-title.h"
#include "hdy-squeezer.h"

/**
 * SECTION:hdy-view-switcher-title
 * @short_description: A view switcher title.
 * @title: HdyViewSwitcherTitle
 * @See_also: #HdyHeaderBar, #HdyViewSwitcher, #HdyViewSwitcherBar
 *
 * A widget letting you switch between multiple views offered by a #GtkStack,
 * via an #HdyViewSwitcher. It is designed to be used as the title widget of a
 * #HdyHeaderBar, and will display the window's title when the window is too
 * narrow to fit the view switcher e.g. on mobile phones, or if there are less
 * than two views.
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
 * #HdyViewSwitcherTitle has a single CSS node with name viewswitchertitle.
 *
 * Since: 1.0
 */

enum {
  PROP_0,
  PROP_POLICY,
  PROP_STACK,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_VIEW_SWITCHER_ENABLED,
  PROP_TITLE_VISIBLE,
  LAST_PROP,
};

struct _HdyViewSwitcherTitle
{
  GtkBin parent_instance;

  HdySqueezer *squeezer;
  GtkLabel *subtitle_label;
  GtkBox *title_box;
  GtkLabel *title_label;
  HdyViewSwitcher *view_switcher;

  gboolean view_switcher_enabled;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (HdyViewSwitcherTitle, hdy_view_switcher_title, GTK_TYPE_BIN)

static void
update_subtitle_label (HdyViewSwitcherTitle *self)
{
  const gchar *subtitle = gtk_label_get_label (self->subtitle_label);

  gtk_widget_set_visible (GTK_WIDGET (self->subtitle_label), subtitle && subtitle[0]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
count_children_cb (GtkWidget *widget,
                   gint      *count)
{
  (*count)++;
}

static void
update_view_switcher_visible (HdyViewSwitcherTitle *self)
{
  GtkStack *stack = hdy_view_switcher_get_stack (self->view_switcher);
  gint count = 0;

  if (self->view_switcher_enabled && stack)
    gtk_container_foreach (GTK_CONTAINER (stack), (GtkCallback) count_children_cb, &count);

  hdy_squeezer_set_child_enabled (self->squeezer, GTK_WIDGET (self->view_switcher), count > 1);
}

static void
notify_squeezer_visible_child_cb (GObject *self)
{
  g_object_notify_by_pspec (self, props[PROP_TITLE_VISIBLE]);
}

static void
hdy_view_switcher_title_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  HdyViewSwitcherTitle *self = HDY_VIEW_SWITCHER_TITLE (object);

  switch (prop_id) {
  case PROP_POLICY:
    g_value_set_enum (value, hdy_view_switcher_title_get_policy (self));
    break;
  case PROP_STACK:
    g_value_set_object (value, hdy_view_switcher_title_get_stack (self));
    break;
  case PROP_TITLE:
    g_value_set_string (value, hdy_view_switcher_title_get_title (self));
    break;
  case PROP_SUBTITLE:
    g_value_set_string (value, hdy_view_switcher_title_get_subtitle (self));
    break;
  case PROP_VIEW_SWITCHER_ENABLED:
    g_value_set_boolean (value, hdy_view_switcher_title_get_view_switcher_enabled (self));
    break;
  case PROP_TITLE_VISIBLE:
    g_value_set_boolean (value, hdy_view_switcher_title_get_title_visible (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_title_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  HdyViewSwitcherTitle *self = HDY_VIEW_SWITCHER_TITLE (object);

  switch (prop_id) {
  case PROP_POLICY:
    hdy_view_switcher_title_set_policy (self, g_value_get_enum (value));
    break;
  case PROP_STACK:
    hdy_view_switcher_title_set_stack (self, g_value_get_object (value));
    break;
  case PROP_TITLE:
    hdy_view_switcher_title_set_title (self, g_value_get_string (value));
    break;
  case PROP_SUBTITLE:
    hdy_view_switcher_title_set_subtitle (self, g_value_get_string (value));
    break;
  case PROP_VIEW_SWITCHER_ENABLED:
    hdy_view_switcher_title_set_view_switcher_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_title_dispose (GObject *object) {
  HdyViewSwitcherTitle *self = (HdyViewSwitcherTitle *)object;

  if (self->view_switcher) {
    GtkStack *stack = hdy_view_switcher_get_stack (self->view_switcher);

    if (stack)
      g_signal_handlers_disconnect_by_func (stack, G_CALLBACK (update_view_switcher_visible), self);
  }

  G_OBJECT_CLASS (hdy_view_switcher_title_parent_class)->dispose (object);
}

static void
hdy_view_switcher_title_class_init (HdyViewSwitcherTitleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = hdy_view_switcher_title_dispose;
  object_class->get_property = hdy_view_switcher_title_get_property;
  object_class->set_property = hdy_view_switcher_title_set_property;

  /**
   * HdyViewSwitcherTitle:policy:
   *
   * The #HdyViewSwitcherPolicy the #HdyViewSwitcher should use to determine
   * which mode to use.
   *
   * Since: 1.0
   */
  props[PROP_POLICY] =
    g_param_spec_enum ("policy",
                       _("Policy"),
                       _("The policy to determine the mode to use"),
                       HDY_TYPE_VIEW_SWITCHER_POLICY, HDY_VIEW_SWITCHER_POLICY_AUTO,
                       G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherTitle:stack:
   *
   * The #GtkStack the #HdyViewSwitcher controls.
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
   * HdyViewSwitcherTitle:title:
   *
   * The title of the #HdyViewSwitcher.
   *
   * Since: 1.0
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("The title to display"),
                         NULL,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherTitle:subtitle:
   *
   * The subtitle of the #HdyViewSwitcher.
   *
   * Since: 1.0
   */
  props[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         _("Subtitle"),
                         _("The subtitle to display"),
                         NULL,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherTitle:view-switcher-enabled:
   *
   * Whether the bar should be revealed or hidden.
   *
   * Since: 1.0
   */
  props[PROP_VIEW_SWITCHER_ENABLED] =
    g_param_spec_boolean ("view-switcher-enabled",
                         _("View switcher enabled"),
                         _("Whether the view switcher is enabled"),
                         TRUE,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcherTitle:title-visible:
   *
   * Whether the bar should be revealed or hidden.
   *
   * Since: 1.0
   */
  props[PROP_TITLE_VISIBLE] =
    g_param_spec_boolean ("title-visible",
                         _("Title visible"),
                         _("Whether the title label is visible"),
                         TRUE,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "viewswitchertitle");

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-view-switcher-title.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherTitle, squeezer);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherTitle, subtitle_label);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherTitle, title_box);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherTitle, title_label);
  gtk_widget_class_bind_template_child (widget_class, HdyViewSwitcherTitle, view_switcher);
  gtk_widget_class_bind_template_callback (widget_class, notify_squeezer_visible_child_cb);
}

static void
hdy_view_switcher_title_init (HdyViewSwitcherTitle *self)
{
  /* This must be initialized before the template so the embedded view switcher
   * can pick up the correct default value.
   */
  self->view_switcher_enabled = TRUE;

  gtk_widget_init_template (GTK_WIDGET (self));

  update_subtitle_label (self);
  update_view_switcher_visible (self);
}

/**
 * hdy_view_switcher_title_new:
 *
 * Creates a new #HdyViewSwitcherTitle widget.
 *
 * Returns: a new #HdyViewSwitcherTitle
 *
 * Since: 1.0
 */
HdyViewSwitcherTitle *
hdy_view_switcher_title_new (void)
{
  return g_object_new (HDY_TYPE_VIEW_SWITCHER_TITLE, NULL);
}

/**
 * hdy_view_switcher_title_get_policy:
 * @self: a #HdyViewSwitcherTitle
 *
 * Gets the policy of @self.
 *
 * Returns: the policy of @self
 *
 * Since: 1.0
 */
HdyViewSwitcherPolicy
hdy_view_switcher_title_get_policy (HdyViewSwitcherTitle *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self), HDY_VIEW_SWITCHER_POLICY_NARROW);

  return hdy_view_switcher_get_policy (self->view_switcher);
}

/**
 * hdy_view_switcher_title_set_policy:
 * @self: a #HdyViewSwitcherTitle
 * @policy: the new policy
 *
 * Sets the policy of @self.
 *
 * Since: 1.0
 */
void
hdy_view_switcher_title_set_policy (HdyViewSwitcherTitle  *self,
                                    HdyViewSwitcherPolicy  policy)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self));

  if (hdy_view_switcher_get_policy (self->view_switcher) == policy)
    return;

  hdy_view_switcher_set_policy (self->view_switcher, policy);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * hdy_view_switcher_title_get_stack:
 * @self: a #HdyViewSwitcherTitle
 *
 * Get the #GtkStack being controlled by the #HdyViewSwitcher.
 *
 * Returns: (nullable) (transfer none): the #GtkStack, or %NULL if none has been set
 *
 * Since: 1.0
 */
GtkStack *
hdy_view_switcher_title_get_stack (HdyViewSwitcherTitle *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self), NULL);

  return hdy_view_switcher_get_stack (self->view_switcher);
}

/**
 * hdy_view_switcher_title_set_stack:
 * @self: a #HdyViewSwitcherTitle
 * @stack: (nullable): a #GtkStack
 *
 * Sets the #GtkStack to control.
 *
 * Since: 1.0
 */
void
hdy_view_switcher_title_set_stack (HdyViewSwitcherTitle *self,
                                   GtkStack             *stack)
{
  GtkStack *previous_stack;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self));
  g_return_if_fail (stack == NULL || GTK_IS_STACK (stack));

  previous_stack = hdy_view_switcher_get_stack (self->view_switcher);

  if (previous_stack == stack)
    return;

  if (previous_stack)
    g_signal_handlers_disconnect_by_func (previous_stack, G_CALLBACK (update_view_switcher_visible), self);

  hdy_view_switcher_set_stack (self->view_switcher, stack);

  if (stack) {
    g_signal_connect_swapped (stack, "add", G_CALLBACK (update_view_switcher_visible), self);
    g_signal_connect_swapped (stack, "remove", G_CALLBACK (update_view_switcher_visible), self);
  }

  update_view_switcher_visible (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * hdy_view_switcher_title_get_title:
 * @self: a #HdyViewSwitcherTitle
 *
 * Gets the title of @self. See hdy_view_switcher_title_set_title().
 *
 * Returns: (transfer none) (nullable): the title of @self, or %NULL.
 *
 * Since: 1.0
 */
const gchar *
hdy_view_switcher_title_get_title (HdyViewSwitcherTitle *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self), NULL);

  return gtk_label_get_label (self->title_label);
}

/**
 * hdy_view_switcher_title_set_title:
 * @self: a #HdyViewSwitcherTitle
 * @title: (nullable): a title, or %NULL
 *
 * Sets the title of @self. The title should give a user additional details. A
 * good title should not include the application name.
 *
 * Since: 1.0
 */
void
hdy_view_switcher_title_set_title (HdyViewSwitcherTitle *self,
                                   const gchar          *title)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self));

  if (g_strcmp0 (gtk_label_get_label (self->title_label), title) == 0)
    return;

  gtk_label_set_label (self->title_label, title);
  gtk_widget_set_visible (GTK_WIDGET (self->title_label), title && title[0]);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * hdy_view_switcher_title_get_subtitle:
 * @self: a #HdyViewSwitcherTitle
 *
 * Gets the subtitle of @self. See hdy_view_switcher_title_set_subtitle().
 *
 * Returns: (transfer none) (nullable): the subtitle of @self, or %NULL.
 *
 * Since: 1.0
 */
const gchar *
hdy_view_switcher_title_get_subtitle (HdyViewSwitcherTitle *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self), NULL);

  return gtk_label_get_label (self->subtitle_label);
}

/**
 * hdy_view_switcher_title_set_subtitle:
 * @self: a #HdyViewSwitcherTitle
 * @subtitle: (nullable): a subtitle, or %NULL
 *
 * Sets the subtitle of @self. The subtitle should give a user additional
 * details.
 *
 * Since: 1.0
 */
void
hdy_view_switcher_title_set_subtitle (HdyViewSwitcherTitle *self,
                                      const gchar          *subtitle)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self));

  if (g_strcmp0 (gtk_label_get_label (self->subtitle_label), subtitle) == 0)
    return;

  gtk_label_set_label (self->subtitle_label, subtitle);
  update_subtitle_label (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SUBTITLE]);
}

/**
 * hdy_view_switcher_title_get_view_switcher_enabled:
 * @self: a #HdyViewSwitcherTitle
 *
 * Gets whether @self's view switcher is enabled.
 *
 * See hdy_view_switcher_title_set_view_switcher_enabled().
 *
 * Returns: %TRUE if the view switcher is enabled, %FALSE otherwise.
 *
 * Since: 1.0
 */
gboolean
hdy_view_switcher_title_get_view_switcher_enabled (HdyViewSwitcherTitle *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self), FALSE);

  return self->view_switcher_enabled;
}

/**
 * hdy_view_switcher_title_set_view_switcher_enabled:
 * @self: a #HdyViewSwitcherTitle
 * @enabled: %TRUE to enable the view switcher, %FALSE to disable it
 *
 * Make @self enable or disable its view switcher. If it is disabled, the title
 * will be displayed instead. This allows to programmatically and prematurely
 * hide the view switcher of @self even if it fits in the available space.
 *
 * This can be used e.g. to ensure the view switcher is hidden below a certain
 * window width, or any other constraint you find suitable.
 *
 * Since: 1.0
 */
void
hdy_view_switcher_title_set_view_switcher_enabled (HdyViewSwitcherTitle *self,
                                                   gboolean              enabled)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self));

  enabled = !!enabled;

  if (self->view_switcher_enabled == enabled)
    return;

  self->view_switcher_enabled = enabled;
  update_view_switcher_visible (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_VIEW_SWITCHER_ENABLED]);
}

/**
 * hdy_view_switcher_title_get_title_visible:
 * @self: a #HdyViewSwitcherTitle
 *
 * Get whether the title label of @self is visible.
 *
 * Returns: %TRUE if the title label of @self is visible, %FALSE if not.
 *
 * Since: 1.0
 */
gboolean
hdy_view_switcher_title_get_title_visible (HdyViewSwitcherTitle *self)
{
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER_TITLE (self), FALSE);

  return hdy_squeezer_get_visible_child (self->squeezer) == (GtkWidget *) self->title_box;
}
