/*
 * Copyright (C) 2019 Zander Brown <zbrown@gnome.org>
 * Copyright (C) 2019 Purism SPC
 *
 * Based on gtkstackswitcher.c, Copyright (c) 2013 Red Hat, Inc.
 * https://gitlab.gnome.org/GNOME/gtk/blob/a0129f556b1fd655215165739d0277d7f7a2c1a8/gtk/gtkstackswitcher.c
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-enums.h"
#include "hdy-style-private.h"
#include "hdy-view-switcher.h"
#include "hdy-view-switcher-button-private.h"

/**
 * SECTION:hdy-view-switcher
 * @short_description: An adaptive view switcher
 * @title: HdyViewSwitcher
 *
 * An adaptive view switcher, designed to switch between multiple views in a
 * similar fashion than a #GtkStackSwitcher.
 *
 * Depending on the available width, the view switcher can adapt from a wide
 * mode showing the view's icon and title side by side, to a narrow mode showing
 * the view's icon and title one on top of the other, in a more compact way.
 * This can be controlled via the policy property.
 *
 * To look good in a header bar, a #HdyViewSwitcher requires to fill its full
 * height. Contrary to #GtkHeaderBar, #HdyHeaderBar doesn't force a vertical
 * alignment on its title widget, so we recommend it over #GtkHeaderBar.
 *
 * Since: 0.0.10
 */

/**
 * HdyViewSwitcherPolicy:
 * @HDY_VIEW_SWITCHER_POLICY_AUTO: Automatically adapt to the best fitting mode
 * @HDY_VIEW_SWITCHER_POLICY_NARROW: Force the narrow mode
 * @HDY_VIEW_SWITCHER_POLICY_WIDE: Force the wide mode
 */

#define MIN_NAT_BUTTON_WIDTH 100
#define TIMEOUT_EXPAND 500

enum {
  PROP_0,
  PROP_POLICY,
  PROP_ICON_SIZE,
  PROP_NARROW_ELLIPSIZE,
  PROP_STACK,
  LAST_PROP,
};

typedef struct {
  GHashTable *buttons;
  gboolean in_child_changed;
  GtkWidget *switch_button;
  guint switch_timer;

  HdyViewSwitcherPolicy policy;
  GtkIconSize icon_size;
  PangoEllipsizeMode narrow_ellipsize;
  GtkStack *stack;
} HdyViewSwitcherPrivate;

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE_WITH_PRIVATE (HdyViewSwitcher, hdy_view_switcher, GTK_TYPE_BOX)

static void
set_visible_stack_child_for_button (HdyViewSwitcher       *self,
                                    HdyViewSwitcherButton *button)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  if (priv->in_child_changed)
    return;

  gtk_stack_set_visible_child (priv->stack, GTK_WIDGET (g_object_get_data (G_OBJECT (button), "stack-child")));
}

static void
update_button (HdyViewSwitcher       *self,
               GtkWidget             *widget,
               HdyViewSwitcherButton *button)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  g_autofree gchar *title = NULL;
  g_autofree gchar *icon_name = NULL;
  gboolean needs_attention;

  gtk_container_child_get (GTK_CONTAINER (priv->stack), widget,
                           "title", &title,
                           "icon-name", &icon_name,
                           "needs-attention", &needs_attention,
                           NULL);

  g_object_set (G_OBJECT (button),
                "icon-name", icon_name,
                "icon-size", priv->icon_size,
                "label", title,
                "needs-attention", needs_attention,
                NULL);

  gtk_widget_set_visible (GTK_WIDGET (button),
                          gtk_widget_get_visible (widget) && (title != NULL || icon_name != NULL));
}

static void
on_stack_child_updated (GtkWidget       *widget,
                        GParamSpec      *pspec,
                        HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  update_button (self, widget, g_hash_table_lookup (priv->buttons, widget));
}

static void
on_position_updated (GtkWidget       *widget,
                     GParamSpec      *pspec,
                     HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  GtkWidget *button = g_hash_table_lookup (priv->buttons, widget);
  gint position;

  gtk_container_child_get (GTK_CONTAINER (priv->stack), widget,
                           "position", &position,
                           NULL);
  gtk_box_reorder_child (GTK_BOX (self), button, position);
}

static void
remove_switch_timer (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  if (!priv->switch_timer)
    return;

  g_source_remove (priv->switch_timer);
  priv->switch_timer = 0;
}

static gboolean
hdy_view_switcher_switch_timeout (gpointer data)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (data);
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  GtkWidget *button = priv->switch_button;

  priv->switch_timer = 0;
  priv->switch_button = NULL;

  if (button)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  return G_SOURCE_REMOVE;
}

static gboolean
hdy_view_switcher_drag_motion (GtkWidget      *widget,
                              GdkDragContext *context,
                              gint            x,
                              gint            y,
                              guint           time)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (widget);
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  GtkAllocation allocation;
  GtkWidget *button;
  GHashTableIter iter;
  gpointer value;
  gboolean retval = FALSE;

  gtk_widget_get_allocation (widget, &allocation);

  x += allocation.x;
  y += allocation.y;

  button = NULL;
  g_hash_table_iter_init (&iter, priv->buttons);
  while (g_hash_table_iter_next (&iter, NULL, &value)) {
    gtk_widget_get_allocation (GTK_WIDGET (value), &allocation);
    if (x >= allocation.x && x <= allocation.x + allocation.width &&
        y >= allocation.y && y <= allocation.y + allocation.height) {
      button = GTK_WIDGET (value);
      retval = TRUE;

      break;
    }
  }

  if (button != priv->switch_button)
    remove_switch_timer (self);

  priv->switch_button = button;

  if (button && !priv->switch_timer) {
    priv->switch_timer = gdk_threads_add_timeout (TIMEOUT_EXPAND,
                                                  hdy_view_switcher_switch_timeout,
                                                  self);
    g_source_set_name_by_id (priv->switch_timer, "[gtk+] hdy_view_switcher_switch_timeout");
  }

  return retval;
}

static void
hdy_view_switcher_drag_leave (GtkWidget      *widget,
                              GdkDragContext *context,
                              guint           time)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (widget);

  remove_switch_timer (self);
}

static void
add_button_for_stack_child (HdyViewSwitcher *self,
                            GtkWidget       *stack_child)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  g_autoptr (GList) children = gtk_container_get_children (GTK_CONTAINER (self));
  HdyViewSwitcherButton *button = hdy_view_switcher_button_new ();

  g_object_set_data (G_OBJECT (button), "stack-child", stack_child);
  g_object_bind_property (self, "icon-size", button, "icon-size", G_BINDING_SYNC_CREATE);
  hdy_view_switcher_button_set_narrow_ellipsize (button, priv->narrow_ellipsize);

  update_button (self, stack_child, button);

  if (children != NULL)
    gtk_radio_button_join_group (GTK_RADIO_BUTTON (button), GTK_RADIO_BUTTON (children->data));

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (button));

  g_signal_connect_swapped (button, "clicked", G_CALLBACK (set_visible_stack_child_for_button), self);
  g_signal_connect (stack_child, "notify::visible", G_CALLBACK (on_stack_child_updated), self);
  g_signal_connect (stack_child, "child-notify::title", G_CALLBACK (on_stack_child_updated), self);
  g_signal_connect (stack_child, "child-notify::icon-name", G_CALLBACK (on_stack_child_updated), self);
  g_signal_connect (stack_child, "child-notify::needs-attention", G_CALLBACK (on_stack_child_updated), self);
  g_signal_connect (stack_child, "child-notify::position", G_CALLBACK (on_position_updated), self);

  g_hash_table_insert (priv->buttons, stack_child, button);
}

static void
add_button_for_stack_child_cb (GtkWidget       *stack_child,
                               HdyViewSwitcher *self)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));
  g_return_if_fail (GTK_IS_WIDGET (stack_child));

  add_button_for_stack_child (self, stack_child);
}

static void
remove_button_for_stack_child (HdyViewSwitcher *self,
                               GtkWidget       *stack_child)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  g_signal_handlers_disconnect_by_func (stack_child, on_stack_child_updated, self);
  g_signal_handlers_disconnect_by_func (stack_child, on_position_updated, self);
  gtk_container_remove (GTK_CONTAINER (self), g_hash_table_lookup (priv->buttons, stack_child));
  g_hash_table_remove (priv->buttons, stack_child);
}

static void
remove_button_for_stack_child_cb (GtkWidget       *stack_child,
                                  HdyViewSwitcher *self)
{
  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));
  g_return_if_fail (GTK_IS_WIDGET (stack_child));

  remove_button_for_stack_child (self, stack_child);
}

static void
update_active_button_for_visible_stack_child (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  GtkWidget *visible_stack_child = gtk_stack_get_visible_child (priv->stack);
  GtkWidget *button = g_hash_table_lookup (priv->buttons, visible_stack_child);

  if (button == NULL)
    return;

  priv->in_child_changed = TRUE;
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  priv->in_child_changed = FALSE;
}

static void
disconnect_stack_signals (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  g_signal_handlers_disconnect_by_func (priv->stack, add_button_for_stack_child, self);
  g_signal_handlers_disconnect_by_func (priv->stack, remove_button_for_stack_child, self);
  g_signal_handlers_disconnect_by_func (priv->stack, update_active_button_for_visible_stack_child, self);
  g_signal_handlers_disconnect_by_func (priv->stack, disconnect_stack_signals, self);
}

static void
connect_stack_signals (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  g_signal_connect_object (priv->stack, "add",
                           G_CALLBACK (add_button_for_stack_child), self,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->stack, "remove",
                           G_CALLBACK (remove_button_for_stack_child), self,
                           G_CONNECT_AFTER | G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->stack, "notify::visible-child",
                           G_CALLBACK (update_active_button_for_visible_stack_child), self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->stack, "destroy",
                           G_CALLBACK (disconnect_stack_signals), self,
                           G_CONNECT_SWAPPED);
}

static void
hdy_view_switcher_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_POLICY:
    g_value_set_enum (value, hdy_view_switcher_get_policy (self));
    break;
  case PROP_ICON_SIZE:
    g_value_set_int (value, hdy_view_switcher_get_icon_size (self));
    break;
  case PROP_NARROW_ELLIPSIZE:
    g_value_set_enum (value, hdy_view_switcher_get_narrow_ellipsize (self));
    break;
  case PROP_STACK:
    g_value_set_object (value, hdy_view_switcher_get_stack (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_POLICY:
    hdy_view_switcher_set_policy (self, g_value_get_enum (value));
    break;
  case PROP_ICON_SIZE:
    hdy_view_switcher_set_icon_size (self, g_value_get_int (value));
    break;
  case PROP_NARROW_ELLIPSIZE:
    hdy_view_switcher_set_narrow_ellipsize (self, g_value_get_enum (value));
    break;
  case PROP_STACK:
    hdy_view_switcher_set_stack (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
hdy_view_switcher_dispose (GObject *object)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (object);

  remove_switch_timer (self);
  hdy_view_switcher_set_stack (self, NULL);

  G_OBJECT_CLASS (hdy_view_switcher_parent_class)->dispose (object);
}

static void
hdy_view_switcher_finalize (GObject *object)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (object);
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);

  g_hash_table_destroy (priv->buttons);

  G_OBJECT_CLASS (hdy_view_switcher_parent_class)->finalize (object);
}

static void
hdy_view_switcher_get_preferred_width (GtkWidget *widget,
                                       gint      *min,
                                       gint      *nat)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (widget);
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  g_autoptr (GList) children = gtk_container_get_children (GTK_CONTAINER (self));
  gint max_h_min = 0, max_h_nat = 0, max_v_min = 0, max_v_nat = 0;
  gint n_children = 0;

  for (GList *l = children; l != NULL; l = g_list_next (l)) {
    gint h_min = 0, h_nat = 0, v_min = 0, v_nat = 0;

    if (!gtk_widget_get_visible (l->data))
      continue;

    hdy_view_switcher_button_get_size (HDY_VIEW_SWITCHER_BUTTON (l->data), &h_min, &h_nat, &v_min, &v_nat);
    max_h_min = MAX (h_min, max_h_min);
    max_h_nat = MAX (h_nat, max_h_nat);
    max_v_min = MAX (v_min, max_v_min);
    max_v_nat = MAX (v_nat, max_v_nat);

    n_children++;
  }

  /* Make the buttons ask at least a minimum arbitrary size for their natural
   * width. This prevents them from looking terribly narrow in a very wide bar.
   */
  max_h_nat = MAX (max_h_nat, MIN_NAT_BUTTON_WIDTH);
  max_v_nat = MAX (max_v_nat, MIN_NAT_BUTTON_WIDTH);

  switch (priv->policy) {
  case HDY_VIEW_SWITCHER_POLICY_NARROW:
    *min = max_v_min * n_children;
    *nat = max_v_nat * n_children;
    break;
  case HDY_VIEW_SWITCHER_POLICY_WIDE:
    *min = max_h_min * n_children;
    *nat = max_h_nat * n_children;
    break;
  case HDY_VIEW_SWITCHER_POLICY_AUTO:
  default:
    *min = max_v_min * n_children;
    *nat = max_h_nat * n_children;
    break;
  }
}

static gint
is_narrow (HdyViewSwitcher *self,
           gint             width)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  g_autoptr (GList) children = gtk_container_get_children (GTK_CONTAINER (self));
  gint max_h_min = 0;
  gint n_children = 0;

  if (priv->policy == HDY_VIEW_SWITCHER_POLICY_NARROW)
    return TRUE;

  if (priv->policy == HDY_VIEW_SWITCHER_POLICY_WIDE)
    return FALSE;

  for (GList *l = children; l != NULL; l = g_list_next (l)) {
    gint h_min = 0;

    hdy_view_switcher_button_get_size (HDY_VIEW_SWITCHER_BUTTON (l->data), &h_min, NULL, NULL, NULL);
    max_h_min = MAX (max_h_min, h_min);

    n_children++;
  }

  return (max_h_min * n_children) > width;
}

static void
hdy_view_switcher_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  g_autoptr (GList) children = gtk_container_get_children (GTK_CONTAINER (widget));
  GtkOrientation orientation = is_narrow (HDY_VIEW_SWITCHER (widget), allocation->width) ?
    GTK_ORIENTATION_VERTICAL :
    GTK_ORIENTATION_HORIZONTAL;

  for (GList *l = children; l != NULL; l = g_list_next (l))
    gtk_orientable_set_orientation (GTK_ORIENTABLE (l->data), orientation);

  GTK_WIDGET_CLASS (hdy_view_switcher_parent_class)->size_allocate (widget, allocation);
}

static void
hdy_view_switcher_class_init (HdyViewSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_view_switcher_get_property;
  object_class->set_property = hdy_view_switcher_set_property;
  object_class->dispose = hdy_view_switcher_dispose;
  object_class->finalize = hdy_view_switcher_finalize;

  widget_class->size_allocate = hdy_view_switcher_size_allocate;
  widget_class->get_preferred_width = hdy_view_switcher_get_preferred_width;
  widget_class->drag_motion = hdy_view_switcher_drag_motion;
  widget_class->drag_leave = hdy_view_switcher_drag_leave;

  /**
   * HdyViewSwitcher:policy:
   *
   * The #HdyViewSwitcherPolicy the view switcher should use to determine which
   * mode to use.
   *
   * Since: 0.0.10
   */
  props[PROP_POLICY] =
    g_param_spec_enum ("policy",
                       _("Policy"),
                       _("The policy to determine the mode to use"),
                       HDY_TYPE_VIEW_SWITCHER_POLICY, HDY_VIEW_SWITCHER_POLICY_AUTO,
                       G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * HdyViewSwitcher:icon-size:
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
   * HdyViewSwitcher:narrow-ellipsize:
   *
   * The preferred place to ellipsize the string, if the narrow mode label does
   * not have enough room to display the entire string, specified as a
   * #PangoEllipsizeMode.
   *
   * Note that setting this property to a value other than %PANGO_ELLIPSIZE_NONE
   * has the side-effect that the label requests only enough space to display
   * the ellipsis.
   *
   * Since: 0.0.10
   */
  props[PROP_NARROW_ELLIPSIZE] =
    g_param_spec_enum ("narrow-ellipsize",
                       _("Narrow ellipsize"),
                       _("The preferred place to ellipsize the string, if the narrow mode label does not have enough room to display the entire string"),
                       PANGO_TYPE_ELLIPSIZE_MODE,
                       PANGO_ELLIPSIZE_NONE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyViewSwitcher:stack:
   *
   * The #GtkStack the view switcher controls.
   *
   * Since: 0.0.10
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack",
                         _("Stack"),
                         _("Stack"),
                         GTK_TYPE_STACK,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "hdyviewswitcher");
}

static void
hdy_view_switcher_init (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv = hdy_view_switcher_get_instance_private (self);
  g_autoptr (GtkCssProvider) provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (provider, "/sm/puri/handy/style/hdy-view-switcher.css");
  gtk_style_context_add_provider (gtk_widget_get_style_context (GTK_WIDGET (self)),
                                  GTK_STYLE_PROVIDER (provider),
                                  HDY_STYLE_PROVIDER_PRIORITY);

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  priv->icon_size = GTK_ICON_SIZE_BUTTON;
  priv->buttons = g_hash_table_new (g_direct_hash, g_direct_equal);

  gtk_widget_set_valign (GTK_WIDGET (self), GTK_ALIGN_FILL);
  gtk_box_set_homogeneous (GTK_BOX (self), TRUE);

  gtk_drag_dest_set (GTK_WIDGET (self), 0, NULL, 0, 0);
  gtk_drag_dest_set_track_motion (GTK_WIDGET (self), TRUE);
}

/**
 * hdy_view_switcher_new:
 *
 * Creates a new #HdyViewSwitcher widget.
 *
 * Returns: a new #HdyViewSwitcher
 *
 * Since: 0.0.10
 */
HdyViewSwitcher *
hdy_view_switcher_new (void)
{
  return g_object_new (HDY_TYPE_VIEW_SWITCHER, NULL);
}

/**
 * hdy_view_switcher_get_policy:
 * @self: a #HdyViewSwitcher
 *
 * Gets the policy of @self.
 *
 * Returns: the policy of @self
 *
 * Since: 0.0.10
 */
HdyViewSwitcherPolicy
hdy_view_switcher_get_policy (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), HDY_VIEW_SWITCHER_POLICY_AUTO);

  priv = hdy_view_switcher_get_instance_private (self);

  return priv->policy;
}

/**
 * hdy_view_switcher_set_policy:
 * @self: a #HdyViewSwitcher
 * @policy: the new policy
 *
 * Sets the policy of @self.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_set_policy (HdyViewSwitcher       *self,
                              HdyViewSwitcherPolicy  policy)
{
  HdyViewSwitcherPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));

  priv = hdy_view_switcher_get_instance_private (self);

  if (priv->policy == policy)
    return;

  priv->policy = policy;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * hdy_view_switcher_get_icon_size:
 * @self: a #HdyViewSwitcher
 *
 * Get the icon size of the images used in the #HdyViewSwitcher.
 *
 * See: hdy_view_switcher_set_icon_size()
 *
 * Returns: the icon size of the images
 *
 * Since: 0.0.10
 */
GtkIconSize
hdy_view_switcher_get_icon_size (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), GTK_ICON_SIZE_BUTTON);

  priv = hdy_view_switcher_get_instance_private (self);

  return priv->icon_size;
}

/**
 * hdy_view_switcher_set_icon_size:
 * @self: a #HdyViewSwitcher
 * @icon_size: the new icon size
 *
 * Change the icon size hint for the icons in a #HdyViewSwitcher.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_set_icon_size (HdyViewSwitcher *self,
                                 GtkIconSize      icon_size)
{
  HdyViewSwitcherPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));

  priv = hdy_view_switcher_get_instance_private (self);

  if (priv->icon_size == icon_size)
    return;

  priv->icon_size = icon_size;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ICON_SIZE]);
}

/**
 * hdy_view_switcher_get_narrow_ellipsize:
 * @self: a #HdyViewSwitcher
 *
 * Get the ellipsizing position of the narrow mode label. See
 * hdy_view_switcher_set_narrow_ellipsize().
 *
 * Returns: #PangoEllipsizeMode
 *
 * Since: 0.0.10
 **/
PangoEllipsizeMode
hdy_view_switcher_get_narrow_ellipsize (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), PANGO_ELLIPSIZE_NONE);

  priv = hdy_view_switcher_get_instance_private (self);

  return priv->narrow_ellipsize;
}

/**
 * hdy_view_switcher_set_narrow_ellipsize:
 * @self: a #HdyViewSwitcher
 * @mode: a #PangoEllipsizeMode
 *
 * Set the mode used to ellipsize the text in narrow mode if there is not
 * enough space to render the entire string.
 *
 * Since: 0.0.10
 **/
void
hdy_view_switcher_set_narrow_ellipsize (HdyViewSwitcher    *self,
                                        PangoEllipsizeMode  mode)
{
  HdyViewSwitcherPrivate *priv;
  GHashTableIter iter;
  gpointer button;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));
  g_return_if_fail (mode >= PANGO_ELLIPSIZE_NONE && mode <= PANGO_ELLIPSIZE_END);

  priv = hdy_view_switcher_get_instance_private (self);

  if ((PangoEllipsizeMode) priv->narrow_ellipsize == mode)
    return;

  priv->narrow_ellipsize = mode;

  g_hash_table_iter_init (&iter, priv->buttons);
  while (g_hash_table_iter_next (&iter, NULL, &button))
    hdy_view_switcher_button_set_narrow_ellipsize (HDY_VIEW_SWITCHER_BUTTON (button), mode);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NARROW_ELLIPSIZE]);
}

/**
 * hdy_view_switcher_get_stack:
 * @self: a #HdyViewSwitcher
 *
 * Get the #GtkStack being controlled by the #HdyViewSwitcher.
 *
 * See: hdy_view_switcher_set_stack()
 *
 * Returns: (nullable) (transfer none): the #GtkStack, or %NULL if none has been set
 *
 * Since: 0.0.10
 */
GtkStack *
hdy_view_switcher_get_stack (HdyViewSwitcher *self)
{
  HdyViewSwitcherPrivate *priv;

  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), NULL);

  priv = hdy_view_switcher_get_instance_private (self);

  return priv->stack;
}

/**
 * hdy_view_switcher_set_stack:
 * @self: a #HdyViewSwitcher
 * @stack: (nullable): a #GtkStack
 *
 * Sets the #GtkStack to control.
 *
 * Since: 0.0.10
 */
void
hdy_view_switcher_set_stack (HdyViewSwitcher *self,
                             GtkStack        *stack)
{
  HdyViewSwitcherPrivate *priv;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));
  g_return_if_fail (stack == NULL || GTK_IS_STACK (stack));

  priv = hdy_view_switcher_get_instance_private (self);

  if (priv->stack == stack)
    return;

  if (priv->stack) {
    disconnect_stack_signals (self);
    gtk_container_foreach (GTK_CONTAINER (priv->stack), (GtkCallback) remove_button_for_stack_child_cb, self);
  }

  g_set_object (&priv->stack, stack);

  if (priv->stack) {
    gtk_container_foreach (GTK_CONTAINER (priv->stack), (GtkCallback) add_button_for_stack_child_cb, self);
    update_active_button_for_visible_stack_child (self);
    connect_stack_signals (self);
  }

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}
