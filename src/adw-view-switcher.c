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

#include "adw-enums.h"
#include "adw-view-switcher.h"
#include "adw-view-switcher-button-private.h"

/**
 * AdwViewSwitcher:
 *
 * An adaptive view switcher.
 *
 * An adaptive view switcher designed to switch between multiple views
 * contained in a [class@Adw.ViewStack] in a similar fashion to
 * [class@Gtk.StackSwitcher].
 *
 * Depending on the available width, the view switcher can adapt from a wide
 * mode showing the view's icon and title side by side, to a narrow mode showing
 * the view's icon and title one on top of the other, in a more compact way.
 * This can be controlled via the [property@Adw.ViewSwitcher:policy] property.
 *
 * ## CSS nodes
 *
 * `AdwViewSwitcher` has a single CSS node with name `viewswitcher`.
 *
 * ## Accessibility
 *
 * `AdwViewSwitcher` uses the `GTK_ACCESSIBLE_ROLE_TAB_LIST` role and uses the
 * `GTK_ACCESSIBLE_ROLE_TAB` for its buttons.
 *
 * Since: 1.0
 */

/**
 * AdwViewSwitcherPolicy:
 * @ADW_VIEW_SWITCHER_POLICY_AUTO: Automatically adapt to the best fitting mode
 * @ADW_VIEW_SWITCHER_POLICY_NARROW: Force the narrow mode
 * @ADW_VIEW_SWITCHER_POLICY_WIDE: Force the wide mode
 *
 * Describes the adaptive modes of [class@Adw.ViewSwitcher].
 */

#define MIN_NAT_BUTTON_WIDTH 100

enum {
  PROP_0,
  PROP_POLICY,
  PROP_NARROW_ELLIPSIZE,
  PROP_STACK,
  LAST_PROP,
};

struct _AdwViewSwitcher
{
  GtkWidget parent_instance;

  AdwViewStack *stack;
  GtkSelectionModel *pages;
  GHashTable *buttons;
  GtkBox *box;

  AdwViewSwitcherPolicy policy;
  PangoEllipsizeMode narrow_ellipsize;
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (AdwViewSwitcher, adw_view_switcher, GTK_TYPE_WIDGET)

static void
on_button_toggled (GtkWidget       *button,
                   GParamSpec      *pspec,
                   AdwViewSwitcher *self)
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
update_button (AdwViewSwitcher  *self,
               AdwViewStackPage *page,
               GtkWidget        *button)
{
  g_autofree char *title = NULL;
  g_autofree char *icon_name = NULL;
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
                "use-underline", use_underline,
                NULL);

  gtk_widget_set_visible (button, visible && (title != NULL || icon_name != NULL));
}

static void
on_page_updated (AdwViewStackPage *page,
                 GParamSpec       *pspec,
                 AdwViewSwitcher  *self)
{
  GtkWidget *button;

  button = g_hash_table_lookup (self->buttons, page);
  update_button (self, page, button);
}

static void
add_child (AdwViewSwitcher *self,
           guint            position)
{
  AdwViewSwitcherButton *button = ADW_VIEW_SWITCHER_BUTTON (adw_view_switcher_button_new ());
  AdwViewStackPage *page;
  gboolean selected;

  page = g_list_model_get_item (G_LIST_MODEL (self->pages), position);
  update_button (self, page, GTK_WIDGET (button));

  gtk_box_append (self->box, GTK_WIDGET (button));

  g_object_set_data (G_OBJECT (button), "child-index", GUINT_TO_POINTER (position));
  selected = gtk_selection_model_is_selected (self->pages, position);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);

  gtk_accessible_update_state (GTK_ACCESSIBLE (button),
                               GTK_ACCESSIBLE_STATE_SELECTED, selected,
                               -1);

  adw_view_switcher_button_set_narrow_ellipsize (button, self->narrow_ellipsize);

  g_signal_connect (button, "notify::active", G_CALLBACK (on_button_toggled), self);
  g_signal_connect (page, "notify", G_CALLBACK (on_page_updated), self);

  g_hash_table_insert (self->buttons, g_object_ref (page), button);

  g_object_unref (page);
}

static void
populate_switcher (AdwViewSwitcher *self)
{
  guint i, n;

  n = g_list_model_get_n_items (G_LIST_MODEL (self->pages));
  for (i = 0; i < n; i++)
    add_child (self, i);
}

static void
clear_switcher (AdwViewSwitcher *self)
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
items_changed_cb (AdwViewSwitcher *self)
{
  clear_switcher (self);
  populate_switcher (self);
}

static void
selection_changed_cb (AdwViewSwitcher   *self,
                      guint              position,
                      guint              n_items)
{
  guint i;

  for (i = position; i < position + n_items; i++) {
    AdwViewStackPage *page = NULL;
    GtkWidget *button;
    gboolean selected;

    page = g_list_model_get_item (G_LIST_MODEL (self->pages), i);
    button = g_hash_table_lookup (self->buttons, page);

    if (button) {
      selected = gtk_selection_model_is_selected (self->pages, i);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), selected);

      gtk_accessible_update_state (GTK_ACCESSIBLE (button),
                                   GTK_ACCESSIBLE_STATE_SELECTED, selected,
                                   -1);
    }

    g_object_unref (page);
  }
}

static void
disconnect_stack_signals (AdwViewSwitcher *self)
{
  g_signal_handlers_disconnect_by_func (self->pages, items_changed_cb, self);
  g_signal_handlers_disconnect_by_func (self->pages, selection_changed_cb, self);
}

static void
connect_stack_signals (AdwViewSwitcher *self)
{
  g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (items_changed_cb), self);
  g_signal_connect_swapped (self->pages, "selection-changed", G_CALLBACK (selection_changed_cb), self);
}

static void
set_stack (AdwViewSwitcher *self,
           AdwViewStack    *stack)
{
  if (!stack)
    return;

  self->stack = g_object_ref (stack);
  self->pages = adw_view_stack_get_pages (stack);
  populate_switcher (self);
  connect_stack_signals (self);
}

static void
unset_stack (AdwViewSwitcher *self)
{
  if (!self->stack)
    return;

  disconnect_stack_signals (self);
  clear_switcher (self);
  g_clear_object (&self->stack);
  g_clear_object (&self->pages);
}

static void
adw_view_switcher_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_POLICY:
    g_value_set_enum (value, adw_view_switcher_get_policy (self));
    break;
  case PROP_NARROW_ELLIPSIZE:
    g_value_set_enum (value, adw_view_switcher_get_narrow_ellipsize (self));
    break;
  case PROP_STACK:
    g_value_set_object (value, adw_view_switcher_get_stack (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_POLICY:
    adw_view_switcher_set_policy (self, g_value_get_enum (value));
    break;
  case PROP_NARROW_ELLIPSIZE:
    adw_view_switcher_set_narrow_ellipsize (self, g_value_get_enum (value));
    break;
  case PROP_STACK:
    adw_view_switcher_set_stack (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_view_switcher_dispose (GObject *object)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  unset_stack (self);
  g_clear_pointer ((GtkWidget **) &self->box, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_view_switcher_parent_class)->dispose (object);
}

static void
adw_view_switcher_finalize (GObject *object)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (object);

  g_hash_table_destroy (self->buttons);

  G_OBJECT_CLASS (adw_view_switcher_parent_class)->finalize (object);
}

static void
adw_view_switcher_measure (GtkWidget      *widget,
                           GtkOrientation  orientation,
                           int             for_size,
                           int            *minimum,
                           int            *natural,
                           int            *minimum_baseline,
                           int            *natural_baseline)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (widget);
  GHashTableIter iter;
  AdwViewStackPage *page;
  AdwViewSwitcherButton *button;
  int max_h_min = 0, max_h_nat = 0, max_v_min = 0, max_v_nat = 0;
  int min = 0, nat = 0;
  int n_children = 0;

  g_hash_table_iter_init (&iter, self->buttons);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &button)) {
      int h_min = 0, h_nat = 0, v_min = 0, v_nat = 0;

      if (!adw_view_stack_page_get_visible (page))
        continue;

      adw_view_switcher_button_get_size (button, &h_min, &h_nat, &v_min, &v_nat);
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
    case ADW_VIEW_SWITCHER_POLICY_NARROW:
      min = max_v_min * n_children;
      nat = max_v_nat * n_children;
      break;
    case ADW_VIEW_SWITCHER_POLICY_WIDE:
      min = max_h_min * n_children;
      nat = max_h_nat * n_children;
      break;
    case ADW_VIEW_SWITCHER_POLICY_AUTO:
    default:
      min = max_v_min * n_children;
      nat = max_h_nat * n_children;
      break;
    }
  } else {
    while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &button)) {
      int child_min, child_nat;

      if (!adw_view_stack_page_get_visible (page))
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

static int
is_narrow (AdwViewSwitcher *self,
           int              width)
{
  GHashTableIter iter;
  AdwViewSwitcherButton *button;
  int max_h_min = 0;
  int n_children = 0;

  if (self->policy == ADW_VIEW_SWITCHER_POLICY_NARROW)
    return TRUE;

  if (self->policy == ADW_VIEW_SWITCHER_POLICY_WIDE)
    return FALSE;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &button)) {
    int h_min = 0;

    if (!gtk_widget_get_visible (GTK_WIDGET (button)))
      continue;

    adw_view_switcher_button_get_size (button, &h_min, NULL, NULL, NULL);
    max_h_min = MAX (max_h_min, h_min);

    n_children++;
  }

  return (max_h_min * n_children) > width;
}

static void
adw_view_switcher_size_allocate (GtkWidget *widget,
                                 int        width,
                                 int        height,
                                 int        baseline)
{
  AdwViewSwitcher *self = ADW_VIEW_SWITCHER (widget);
  GtkOrientation orientation;
  GHashTableIter iter;
  AdwViewSwitcherButton *button;

  orientation = is_narrow (ADW_VIEW_SWITCHER (widget), width) ?
    GTK_ORIENTATION_VERTICAL :
    GTK_ORIENTATION_HORIZONTAL;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, NULL, (gpointer *) &button))
    gtk_orientable_set_orientation (GTK_ORIENTABLE (button), orientation);

  gtk_widget_allocate (GTK_WIDGET (self->box), width, height, baseline, NULL);
}

static void
adw_view_switcher_class_init (AdwViewSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_view_switcher_get_property;
  object_class->set_property = adw_view_switcher_set_property;
  object_class->dispose = adw_view_switcher_dispose;
  object_class->finalize = adw_view_switcher_finalize;

  widget_class->size_allocate = adw_view_switcher_size_allocate;
  widget_class->measure = adw_view_switcher_measure;

  /**
   * AdwViewSwitcher:policy: (attributes org.gtk.Property.get=adw_view_switcher_get_policy org.gtk.Property.set=adw_view_switcher_set_policy)
   *
   * The policy to determine which mode to use.
   *
   * Since: 1.0
   */
  props[PROP_POLICY] =
    g_param_spec_enum ("policy",
                       "Policy",
                       "The policy to determine the mode to use",
                       ADW_TYPE_VIEW_SWITCHER_POLICY,
                       ADW_VIEW_SWITCHER_POLICY_AUTO,
                       G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwViewSwitcher:narrow-ellipsize: (attributes org.gtk.Property.get=adw_view_switcher_get_narrow_ellipsize org.gtk.Property.set=adw_view_switcher_set_narrow_ellipsize)
   *
   * The ellipsizing position for the titles.
   *
   * Note that setting this property to a value other than
   * `PANGO_ELLIPSIZE_NONE` has the side-effect that the label requests only
   * enough space to display the ellipsis.
   *
   * Since: 1.0
   */
  props[PROP_NARROW_ELLIPSIZE] =
    g_param_spec_enum ("narrow-ellipsize",
                       "Narrow ellipsize",
                       "The ellipsizing position for the titles",
                       PANGO_TYPE_ELLIPSIZE_MODE,
                       PANGO_ELLIPSIZE_NONE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwViewSwitcher:stack: (attributes org.gtk.Property.get=adw_view_switcher_get_stack org.gtk.Property.set=adw_view_switcher_set_stack)
   *
   * The stack the view switcher controls.
   *
   * Since: 1.0
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack",
                         "Stack",
                         "The stack the view switcher controls",
                         ADW_TYPE_VIEW_STACK,
                         G_PARAM_EXPLICIT_NOTIFY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "viewswitcher");
  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_TAB_LIST);
}

static void
adw_view_switcher_init (AdwViewSwitcher *self)
{
  self->box = GTK_BOX (gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0));
  gtk_box_set_homogeneous (self->box, TRUE);

  gtk_widget_set_parent (GTK_WIDGET (self->box), GTK_WIDGET (self));

  self->buttons = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, NULL);
}

/**
 * adw_view_switcher_new:
 *
 * Creates a new `AdwViewSwitcher`.
 *
 * Returns: the newly created `AdwViewSwitcher`
 *
 * Since: 1.0
 */
GtkWidget *
adw_view_switcher_new (void)
{
  return g_object_new (ADW_TYPE_VIEW_SWITCHER, NULL);
}

/**
 * adw_view_switcher_get_policy: (attributes org.gtk.Method.get_property=policy)
 * @self: a `AdwViewSwitcher`
 *
 * Gets the policy of @self.
 *
 * Returns: the policy of @self
 *
 * Since: 1.0
 */
AdwViewSwitcherPolicy
adw_view_switcher_get_policy (AdwViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER (self), ADW_VIEW_SWITCHER_POLICY_AUTO);

  return self->policy;
}

/**
 * adw_view_switcher_set_policy: (attributes org.gtk.Method.set_property=policy)
 * @self: a `AdwViewSwitcher`
 * @policy: the new policy
 *
 * Sets the policy of @self.
 *
 * Since: 1.0
 */
void
adw_view_switcher_set_policy (AdwViewSwitcher       *self,
                              AdwViewSwitcherPolicy  policy)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER (self));

  if (self->policy == policy)
    return;

  self->policy = policy;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POLICY]);

  gtk_widget_queue_resize (GTK_WIDGET (self));
}

/**
 * adw_view_switcher_get_narrow_ellipsize: (attributes org.gtk.Method.get_property=narrow-ellipsize)
 * @self: a `AdwViewSwitcher`
 *
 * Gets the ellipsizing position for the titles.
 *
 * Returns: the ellipsize mode.
 *
 * Since: 1.0
 */
PangoEllipsizeMode
adw_view_switcher_get_narrow_ellipsize (AdwViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER (self), PANGO_ELLIPSIZE_NONE);

  return self->narrow_ellipsize;
}

/**
 * adw_view_switcher_set_narrow_ellipsize: (attributes org.gtk.Method.set_property=narrow-ellipsize)
 * @self: a `AdwViewSwitcher`
 * @mode: the new value
 *
 * Sets the ellipsizing position for the titles.
 *
 * Since: 1.0
 */
void
adw_view_switcher_set_narrow_ellipsize (AdwViewSwitcher    *self,
                                        PangoEllipsizeMode  mode)
{
  GHashTableIter iter;
  gpointer button;

  g_return_if_fail (ADW_IS_VIEW_SWITCHER (self));
  g_return_if_fail (mode >= PANGO_ELLIPSIZE_NONE && mode <= PANGO_ELLIPSIZE_END);

  if ((PangoEllipsizeMode) self->narrow_ellipsize == mode)
    return;

  self->narrow_ellipsize = mode;

  g_hash_table_iter_init (&iter, self->buttons);
  while (g_hash_table_iter_next (&iter, NULL, &button))
    adw_view_switcher_button_set_narrow_ellipsize (ADW_VIEW_SWITCHER_BUTTON (button), mode);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NARROW_ELLIPSIZE]);
}

/**
 * adw_view_switcher_get_stack: (attributes org.gtk.Method.get_property=stack)
 * @self: a `AdwViewSwitcher`
 *
 * Gets the stack controlled by @self.
 *
 * Returns: (nullable) (transfer none): the stack
 *
 * Since: 1.0
 */
AdwViewStack *
adw_view_switcher_get_stack (AdwViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_VIEW_SWITCHER (self), NULL);

  return self->stack;
}

/**
 * adw_view_switcher_set_stack: (attributes org.gtk.Method.set_property=stack)
 * @self: a `AdwViewSwitcher`
 * @stack: (nullable): a stack
 *
 * Sets the stack controlled by @self.
 *
 * Since: 1.0
 */
void
adw_view_switcher_set_stack (AdwViewSwitcher *self,
                             AdwViewStack    *stack)
{
  g_return_if_fail (ADW_IS_VIEW_SWITCHER (self));
  g_return_if_fail (stack == NULL || ADW_IS_VIEW_STACK (stack));

  if (self->stack == stack)
    return;

  unset_stack (self);
  set_stack (self, stack);

  gtk_widget_queue_resize (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}
