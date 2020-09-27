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
#include "hdy-view-switcher.h"
#include "hdy-view-switcher-button-private.h"

/**
 * SECTION:hdy-view-switcher
 * @short_description: An adaptive view switcher.
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
 * To look good in a header bar, an #HdyViewSwitcher requires to fill its full
 * height. Contrary to #GtkHeaderBar, #HdyHeaderBar doesn't force a vertical
 * alignment on its title widget, so we recommend it over #GtkHeaderBar.
 *
 * # CSS nodes
 *
 * #HdyViewSwitcher has a single CSS node with name viewswitcher.
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

enum {
  PROP_0,
  PROP_POLICY,
  PROP_NARROW_ELLIPSIZE,
  PROP_STACK,
  LAST_PROP,
};

struct _HdyViewSwitcher
{
  GtkWidget parent_instance;

  GtkStack *stack;
  GtkSelectionModel *pages;
  GHashTable *buttons;
  GtkBox *box;

  HdyViewSwitcherPolicy policy;
  PangoEllipsizeMode narrow_ellipsize;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (HdyViewSwitcher, hdy_view_switcher, GTK_TYPE_WIDGET)

static void
on_button_toggled (GtkWidget       *button,
                   GParamSpec      *pspec,
                   HdyViewSwitcher *self)
{
  gboolean active;
  guint index;

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
  index = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (button), "child-index"));

  if (active) {
      gtk_selection_model_select_item (self->pages, index, TRUE);
  } else {
    gboolean selected = gtk_selection_model_is_selected (self->pages, index);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);
  }
}

static void
update_button (HdyViewSwitcher *self,
               GtkStackPage    *page,
               GtkWidget       *button)
{
  g_autofree gchar *title = NULL;
  g_autofree gchar *icon_name = NULL;
  gboolean needs_attention;
  gboolean visible;
  gboolean use_underline;

  g_object_get (page,
                "title", &title,
                "icon-name", &icon_name,
                "needs-attention", &needs_attention,
                "visible", &visible,
                "use-underline", &use_underline,
                NULL);

  g_object_set (G_OBJECT (button),
                "icon-name", icon_name,
                "label", title,
                "needs-attention", needs_attention,
                NULL);

  gtk_widget_set_visible (button, visible && (title != NULL || icon_name != NULL));

  if (needs_attention)
    gtk_widget_add_css_class (button, "needs-attention");
  else
    gtk_widget_remove_css_class (button, "needs-attention");
}

static void
on_page_updated (GtkStackPage    *page,
                 GParamSpec      *pspec,
                 HdyViewSwitcher *self)
{
  GtkWidget *button;

  button = g_hash_table_lookup (self->buttons, page);
  update_button (self, page, button);
}

static void
add_child (HdyViewSwitcher *self,
           guint            position)
{
  HdyViewSwitcherButton *button = HDY_VIEW_SWITCHER_BUTTON (hdy_view_switcher_button_new ());
  GtkStackPage *page;
  gboolean selected;

  page = g_list_model_get_item (G_LIST_MODEL (self->pages), position);
  update_button (self, page, GTK_WIDGET (button));

  gtk_box_append (self->box, GTK_WIDGET (button));

  g_object_set_data (G_OBJECT (button), "child-index", GUINT_TO_POINTER (position));
  selected = gtk_selection_model_is_selected (self->pages, position);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);

  hdy_view_switcher_button_set_narrow_ellipsize (button, self->narrow_ellipsize);

  g_signal_connect (button, "notify::active", G_CALLBACK (on_button_toggled), self);
  g_signal_connect (page, "notify", G_CALLBACK (on_page_updated), self);

  g_hash_table_insert (self->buttons, g_object_ref (page), button);

  g_object_unref (page);
}

static void
populate_switcher (HdyViewSwitcher *self)
{
  guint i, n;

  n = g_list_model_get_n_items (G_LIST_MODEL (self->pages));
  for (i = 0; i < n; i++)
    add_child (self, i);
}

static void
clear_switcher (HdyViewSwitcher *self)
{
  GHashTableIter iter;
  GtkWidget *page;
  GtkWidget *button;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &button)) {
    gtk_box_remove (self->box, button);
    g_signal_handlers_disconnect_by_func (page, on_page_updated, self);
    g_hash_table_iter_remove (&iter);
  }
}


static void
items_changed_cb (HdyViewSwitcher *self)
{
  clear_switcher (self);
  populate_switcher (self);
}

static void
selection_changed_cb (HdyViewSwitcher   *self,
                      guint              position,
                      guint              n_items)
{
  guint i;

  for (i = position; i < position + n_items; i++) {
    GtkStackPage *page = NULL;
    GtkWidget *button;
    gboolean selected;

    page = g_list_model_get_item (G_LIST_MODEL (self->pages), i);
    button = g_hash_table_lookup (self->buttons, page);

    if (button) {
      selected = gtk_selection_model_is_selected (self->pages, i);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);
    }

    g_object_unref (page);
  }
}

static void
disconnect_stack_signals (HdyViewSwitcher *self)
{
  g_signal_handlers_disconnect_by_func (self->pages, items_changed_cb, self);
  g_signal_handlers_disconnect_by_func (self->pages, selection_changed_cb, self);
}

static void
connect_stack_signals (HdyViewSwitcher *self)
{
  g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (items_changed_cb), self);
  g_signal_connect_swapped (self->pages, "selection-changed", G_CALLBACK (selection_changed_cb), self);
}

static void
set_stack (HdyViewSwitcher *self,
           GtkStack        *stack)
{
  if (!stack)
    return;

  self->stack = g_object_ref (stack);
  self->pages = gtk_stack_get_pages (stack);
  populate_switcher (self);
  connect_stack_signals (self);
}

static void
unset_stack (HdyViewSwitcher *self)
{
  if (!self->stack)
    return;

  disconnect_stack_signals (self);
  clear_switcher (self);
  g_clear_object (&self->stack);
  g_clear_object (&self->pages);
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

  unset_stack (self);
  g_clear_pointer ((GtkWidget **) &self->box, gtk_widget_unparent);

  G_OBJECT_CLASS (hdy_view_switcher_parent_class)->dispose (object);
}

static void
hdy_view_switcher_finalize (GObject *object)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (object);

  g_hash_table_destroy (self->buttons);

  G_OBJECT_CLASS (hdy_view_switcher_parent_class)->finalize (object);
}

static void
hdy_view_switcher_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           gint            for_size,
                           gint           *minimum,
                           gint           *natural,
                           gint           *minimum_baseline,
                           gint           *natural_baseline)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (widget);
  GHashTableIter iter;
  GtkStackPage *page;
  HdyViewSwitcherButton *button;
  gint max_h_min = 0, max_h_nat = 0, max_v_min = 0, max_v_nat = 0;
  gint min = 0, nat = 0;
  gint n_children = 0;

  g_hash_table_iter_init (&iter, self->buttons);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &button)) {
      gint h_min = 0, h_nat = 0, v_min = 0, v_nat = 0;

      if (!gtk_stack_page_get_visible (page))
        continue;

      hdy_view_switcher_button_get_size (button, &h_min, &h_nat, &v_min, &v_nat);
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

    switch (self->policy) {
    case HDY_VIEW_SWITCHER_POLICY_NARROW:
      min = max_v_min * n_children;
      nat = max_v_nat * n_children;
      break;
    case HDY_VIEW_SWITCHER_POLICY_WIDE:
      min = max_h_min * n_children;
      nat = max_h_nat * n_children;
      break;
    case HDY_VIEW_SWITCHER_POLICY_AUTO:
    default:
      min = max_v_min * n_children;
      nat = max_h_nat * n_children;
      break;
    }
  } else {
    while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &button)) {
      gint child_min, child_nat;

      if (!gtk_stack_page_get_visible (page))
        continue;

      gtk_widget_measure (GTK_WIDGET (button), GTK_ORIENTATION_VERTICAL, -1,
                          &child_min, &child_nat, NULL, NULL);

      min = MAX (child_min, min);
      nat = MAX (child_nat, nat);
    }
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static gint
is_narrow (HdyViewSwitcher *self,
           gint             width)
{
  GHashTableIter iter;
  HdyViewSwitcherButton *button;
  gint max_h_min = 0;
  gint n_children = 0;

  if (self->policy == HDY_VIEW_SWITCHER_POLICY_NARROW)
    return TRUE;

  if (self->policy == HDY_VIEW_SWITCHER_POLICY_WIDE)
    return FALSE;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &button)) {
    gint h_min = 0;

    if (!gtk_widget_get_visible (GTK_WIDGET (button)))
      continue;

    hdy_view_switcher_button_get_size (button, &h_min, NULL, NULL, NULL);
    max_h_min = MAX (max_h_min, h_min);

    n_children++;
  }

  return (max_h_min * n_children) > width;
}

static void
hdy_view_switcher_size_allocate (GtkWidget *widget,
                                 gint       width,
                                 gint       height,
                                 gint       baseline)
{
  HdyViewSwitcher *self = HDY_VIEW_SWITCHER (widget);
  GtkOrientation orientation;
  GHashTableIter iter;
  HdyViewSwitcherButton *button;

  orientation = is_narrow (HDY_VIEW_SWITCHER (widget), width) ?
    GTK_ORIENTATION_VERTICAL :
    GTK_ORIENTATION_HORIZONTAL;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &button))
    gtk_orientable_set_orientation (GTK_ORIENTABLE (button), orientation);

  gtk_widget_allocate (GTK_WIDGET (self->box), width, height, baseline, NULL);
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
  widget_class->measure = hdy_view_switcher_measure;

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

  gtk_widget_class_set_css_name (widget_class, "viewswitcher");
}

static void
hdy_view_switcher_init (HdyViewSwitcher *self)
{
  self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
  gtk_box_set_homogeneous (self->box, TRUE);

  gtk_widget_set_parent (GTK_WIDGET (self->box), GTK_WIDGET (self));

  self->buttons = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, NULL);
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
GtkWidget *
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
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), HDY_VIEW_SWITCHER_POLICY_AUTO);

  return self->policy;
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
  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));

  if (self->policy == policy)
    return;

  self->policy = policy;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
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
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), PANGO_ELLIPSIZE_NONE);

  return self->narrow_ellipsize;
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
  GHashTableIter iter;
  gpointer button;

  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));
  g_return_if_fail (mode >= PANGO_ELLIPSIZE_NONE && mode <= PANGO_ELLIPSIZE_END);

  if ((PangoEllipsizeMode) self->narrow_ellipsize == mode)
    return;

  self->narrow_ellipsize = mode;

  g_hash_table_iter_init (&iter, self->buttons);
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
  g_return_val_if_fail (HDY_IS_VIEW_SWITCHER (self), NULL);

  return self->stack;
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
  g_return_if_fail (HDY_IS_VIEW_SWITCHER (self));
  g_return_if_fail (stack == NULL || GTK_IS_STACK (stack));

  if (self->stack == stack)
    return;

  unset_stack (self);
  set_stack (self, stack);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}
