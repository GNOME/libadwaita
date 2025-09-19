/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-inline-view-switcher.h"

#include "adw-bin.h"
#include "adw-indicator-bin-private.h"
#include "adw-toggle-group.h"
#include "adw-widget-utils-private.h"

/**
 * AdwInlineViewSwitcherDisplayMode:
 * @ADW_INLINE_VIEW_SWITCHER_LABELS: Toggles only display labels.
 * @ADW_INLINE_VIEW_SWITCHER_ICONS: Toggles only display icons.
 * @ADW_INLINE_VIEW_SWITCHER_BOTH: Toggles display both icons and labels.
 *
 * Describes what [class@InlineViewSwitcher] toggles display.
 *
 * <picture>
 *   <source srcset="inline-view-switcher-display-modes-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="inline-view-switcher-display-modes.png" alt="inline-view-switcher-display-modes">
 * </picture>
 *
 * Since: 1.7
 */

/**
 * AdwInlineViewSwitcher:
 *
 * A view switcher that uses a toggle group.
 *
 * <picture>
 *   <source srcset="inline-view-switcher-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="inline-view-switcher.png" alt="inline-view-switcher">
 * </picture>
 *
 * A view switcher showing pages of an [class@ViewStack] within an
 * [class@ToggleGroup], similar to [class@ViewSwitcher].
 *
 * The toggles can display either an icon, a label or both. Use the
 * [property@InlineViewSwitcher:display-mode] to control this.
 *
 * <picture>
 *   <source srcset="inline-view-switcher-display-modes-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="inline-view-switcher-display-modes.png" alt="inline-view-switcher-display-modes">
 * </picture>
 *
 * ## CSS nodes
 *
 * `AdwInlineViewSwitcher` has a single CSS node with the name
 * `inline-view-switcher`.
 *
 * ## Style classes
 *
 * Like `AdwToggleGroup`, it can accept the [`.flat`](style-classes.html#flat_1)
 * and [`.round`](style-classes.html#round) style classes.
 *
 * <picture>
 *   <source srcset="inline-view-switcher-style-classes-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="inline-view-switcher-style-classes.png" alt="inline-view-switcher-style-classes">
 * </picture>
 *
 * ## Accessibility
 *
 * The internal toggle group uses the `GTK_ACCESSIBLE_ROLE_TAB_LIST` role. Its
 * toggles use the `GTK_ACCESSIBLE_ROLE_TAB` role.
 *
 * See also: [class@ViewSwitcher], [class@ViewSwitcherBar],
 * [class@ViewSwitcherSidebar].
 *
 * Since: 1.7
 */

#define TIMEOUT_EXPAND 500

struct _AdwInlineViewSwitcher
{
  GtkWidget parent_instance;

  GtkWidget *toggle_group;
  AdwInlineViewSwitcherDisplayMode display_mode;

  AdwViewStack *stack;
  GListModel *pages;
  GHashTable *toggles;

  int block_notify_active;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwInlineViewSwitcher, adw_inline_view_switcher, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_STACK,
  PROP_DISPLAY_MODE,
  PROP_HOMOGENEOUS,
  PROP_CAN_SHRINK,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_CAN_SHRINK + 1,
};

static GParamSpec *props[LAST_PROP];

static void
notify_active_cb (AdwInlineViewSwitcher *self)
{
  AdwToggle *toggle;
  guint active, index;

  if (self->block_notify_active)
    return;

  active = adw_toggle_group_get_active (ADW_TOGGLE_GROUP (self->toggle_group));
  toggle = adw_toggle_group_get_toggle (ADW_TOGGLE_GROUP (self->toggle_group), active);

  index = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (toggle), "child-index"));

  gtk_selection_model_select_item (GTK_SELECTION_MODEL (self->pages), index, TRUE);
}

static gboolean
transform_badge (GBinding     *binding,
                 const GValue *input,
                 GValue       *output,
                 gpointer      user_data)
{
  guint badge_number = g_value_get_uint (input);

  if (badge_number > 0)
    g_value_take_string (output, g_strdup_printf ("%u", badge_number));
  else
    g_value_set_string (output, NULL);

  return TRUE;
}

static gboolean
transform_icon_name (GBinding     *binding,
                     const GValue *input,
                     GValue       *output,
                     gpointer      user_data)
{
  const char *icon_name = g_value_get_string (input);

  if (icon_name && *icon_name)
    g_value_set_string (output, icon_name);
  else
    g_value_set_string (output, "image-missing");

  return TRUE;
}

static gboolean
transform_can_shrink (GBinding     *binding,
                      const GValue *input,
                      GValue       *output,
                      gpointer      user_data)
{
  gboolean can_shrink = g_value_get_boolean (input);

  if (can_shrink)
    g_value_set_enum (output, PANGO_ELLIPSIZE_END);
  else
    g_value_set_enum (output, PANGO_ELLIPSIZE_NONE);

  return TRUE;
}

static void
update_tooltip (AdwToggle        *toggle,
                GParamSpec       *pspec,
                AdwViewStackPage *page)
{
  AdwInlineViewSwitcher *self = g_object_get_data (G_OBJECT (toggle), "switcher");
  const char *title;
  char *stripped_title, *tooltip;

  if (!toggle)
    return;

  if (self->display_mode != ADW_INLINE_VIEW_SWITCHER_ICONS) {
    adw_toggle_set_tooltip (toggle, "");
    return;
  }

  title = adw_view_stack_page_get_title (page);

  if (adw_view_stack_page_get_use_underline (page))
    stripped_title = adw_strip_mnemonic (title);
  else
    stripped_title = g_strdup (title);

  tooltip = g_markup_escape_text (stripped_title, -1);
  adw_toggle_set_tooltip (toggle, tooltip);

  g_free (tooltip);
  g_free (stripped_title);
}

static void
switch_timeout_cb (AdwToggle *toggle)
{
  g_object_steal_data (G_OBJECT (toggle), "switch-timer");

  if (toggle) {
    AdwToggleGroup *group = g_object_get_data (G_OBJECT (toggle), "toggle-group");

    adw_toggle_group_set_active (group, adw_toggle_get_index (toggle));
  }
}

static void
clear_timer (gpointer data)
{
  if (data)
    g_source_remove (GPOINTER_TO_UINT (data));
}

static void
drag_enter_cb (AdwToggle *toggle)
{
  AdwToggleGroup *group = g_object_get_data (G_OBJECT (toggle), "toggle-group");
  guint active = adw_toggle_group_get_active (group);

  if (adw_toggle_get_index (toggle) != active) {
    guint switch_timer =
      g_timeout_add_once (TIMEOUT_EXPAND, (GSourceOnceFunc) switch_timeout_cb, toggle);

    g_object_set_data_full (G_OBJECT (toggle), "switch-timer",
                            GUINT_TO_POINTER (switch_timer), clear_timer);
  }
}

static void
drag_leave_cb (AdwToggle *toggle)
{
  guint switch_timer =
    GPOINTER_TO_UINT (g_object_steal_data (G_OBJECT (toggle), "switch-timer"));

  if (switch_timer)
    g_source_remove (switch_timer);
}

static void
update_toggle (AdwInlineViewSwitcher *self,
               AdwToggle             *toggle,
               AdwViewStackPage      *page)
{
  GtkWidget *child = adw_bin_new ();
  GtkEventController *controller;

  switch (self->display_mode) {
  case ADW_INLINE_VIEW_SWITCHER_LABELS:
    {
      GtkWidget *indicator = adw_indicator_bin_new ();
      GtkWidget *label = gtk_label_new (NULL);

      gtk_widget_set_halign (indicator, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (indicator, GTK_ALIGN_CENTER);

      g_object_bind_property (page, "needs-attention", indicator, "needs-attention", G_BINDING_SYNC_CREATE);
      g_object_bind_property_full (page, "badge-number", indicator, "badge", G_BINDING_SYNC_CREATE,
                                   transform_badge, NULL, NULL, NULL);
      g_object_bind_property (page, "title", label, "label", G_BINDING_SYNC_CREATE);
      g_object_bind_property (page, "use-underline", label, "use-underline", G_BINDING_SYNC_CREATE);
      g_object_bind_property_full (self, "can-shrink", label, "ellipsize", G_BINDING_SYNC_CREATE,
                                   transform_can_shrink, NULL, NULL, NULL);

      adw_toggle_set_tooltip (toggle, "");

      adw_indicator_bin_set_child (ADW_INDICATOR_BIN (indicator), label);
      adw_bin_set_child (ADW_BIN (child), indicator);
    }
    break;
  case ADW_INLINE_VIEW_SWITCHER_ICONS:
    {
      GtkWidget *indicator = adw_indicator_bin_new ();
      GtkWidget *image = gtk_image_new ();

      gtk_widget_set_halign (indicator, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (indicator, GTK_ALIGN_CENTER);

      g_object_bind_property (page, "needs-attention", indicator, "needs-attention", G_BINDING_SYNC_CREATE);
      g_object_bind_property_full (page, "badge-number", indicator, "badge", G_BINDING_SYNC_CREATE,
                                   transform_badge, NULL, NULL, NULL);
      g_object_bind_property_full (page, "icon-name", image, "icon-name", G_BINDING_SYNC_CREATE,
                                   transform_icon_name, NULL, NULL, NULL);

      adw_indicator_bin_set_child (ADW_INDICATOR_BIN (indicator), image);
      adw_bin_set_child (ADW_BIN (child), indicator);
    }
    break;
  case ADW_INLINE_VIEW_SWITCHER_BOTH:
    {
      GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
      GtkWidget *indicator = adw_indicator_bin_new ();
      GtkWidget *image = g_object_new (GTK_TYPE_IMAGE,
                                       "accessible-role", GTK_ACCESSIBLE_ROLE_PRESENTATION,
                                       NULL);
      GtkWidget *label = gtk_label_new (NULL);

      gtk_widget_set_halign (box, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (indicator, GTK_ALIGN_CENTER);

      g_object_bind_property (page, "needs-attention", indicator, "needs-attention", G_BINDING_SYNC_CREATE);
      g_object_bind_property_full (page, "badge-number", indicator, "badge", G_BINDING_SYNC_CREATE,
                                   transform_badge, NULL, NULL, NULL);
      g_object_bind_property_full (page, "icon-name", image, "icon-name", G_BINDING_SYNC_CREATE,
                                   transform_icon_name, NULL, NULL, NULL);
      g_object_bind_property (page, "title", label, "label", G_BINDING_SYNC_CREATE);
      g_object_bind_property (page, "use-underline", label, "use-underline", G_BINDING_SYNC_CREATE);
      g_object_bind_property_full (self, "can-shrink", label, "ellipsize", G_BINDING_SYNC_CREATE,
                                   transform_can_shrink, NULL, NULL, NULL);

      adw_toggle_set_tooltip (toggle, "");

      adw_indicator_bin_set_child (ADW_INDICATOR_BIN (indicator), image);
      gtk_box_append (GTK_BOX (box), indicator);
      gtk_box_append (GTK_BOX (box), label);
      adw_bin_set_child (ADW_BIN (child), box);
    }
    break;
  default:
    g_assert_not_reached ();
  }

  controller = gtk_drop_controller_motion_new ();
  g_signal_connect_swapped (controller, "enter", G_CALLBACK (drag_enter_cb), toggle);
  g_signal_connect_swapped (controller, "leave", G_CALLBACK (drag_leave_cb), toggle);
  gtk_widget_add_controller (child, controller);

  update_tooltip (toggle, NULL, page);

  adw_toggle_set_child (toggle, child);
}

static void
add_toggle (AdwInlineViewSwitcher *self,
            AdwViewStackPage      *page,
            guint                  index)
{
  AdwToggle *toggle = adw_toggle_new ();

  /* page ownership is passed into self->toggles */
  g_hash_table_insert (self->toggles, page, toggle);

  g_object_set_data (G_OBJECT (toggle), "switcher", self);
  g_object_set_data (G_OBJECT (toggle), "toggle-group", self->toggle_group);
  g_object_set_data (G_OBJECT (toggle), "child-index", GUINT_TO_POINTER (index));

  g_object_bind_property (page, "title", toggle, "label", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "icon-name", toggle, "icon-name", G_BINDING_SYNC_CREATE);
  g_object_bind_property (page, "use-underline", toggle, "use-underline", G_BINDING_SYNC_CREATE);

  g_signal_connect_object (toggle, "notify::label", G_CALLBACK (update_tooltip), page, 0);
  g_signal_connect_object (toggle, "notify::use-underline", G_CALLBACK (update_tooltip), page, 0);

  update_toggle (self, toggle, page);

  adw_toggle_group_add (ADW_TOGGLE_GROUP (self->toggle_group), toggle);
}

static void
populate_group (AdwInlineViewSwitcher *self)
{
  guint i, n = g_list_model_get_n_items (self->pages);
  GtkWidget *visible_child;
  AdwViewStackPage *page;
  guint index = GTK_INVALID_LIST_POSITION;

  self->block_notify_active++;

  for (i = 0; i < n; i++) {
    page = g_list_model_get_item (self->pages, i);

    /* page ownership is passed into add_toggle() */
    if (adw_view_stack_page_get_visible (page))
      add_toggle (self, page, i);
    else
      g_object_unref (page);
  }

  visible_child = adw_view_stack_get_visible_child (self->stack);
  if (visible_child) {
    page = adw_view_stack_get_page (self->stack, visible_child);

    if (adw_view_stack_page_get_visible (page)) {
      AdwToggle *toggle = g_hash_table_lookup (self->toggles, page);
      index = adw_toggle_get_index (toggle);
    }
  }

  adw_toggle_group_set_active (ADW_TOGGLE_GROUP (self->toggle_group), index);
  self->block_notify_active--;
}

static void
clear_group (AdwInlineViewSwitcher *self)
{
  self->block_notify_active++;

  g_hash_table_remove_all (self->toggles);

  adw_toggle_group_remove_all (ADW_TOGGLE_GROUP (self->toggle_group));

  self->block_notify_active--;
}

static void
recreate_toggles (AdwInlineViewSwitcher *self)
{
  clear_group (self);
  populate_group (self);
}

static void
items_changed_cb (AdwInlineViewSwitcher *self,
                  guint                  position,
                  guint                  removed,
                  guint                  added,
                  GListModel            *model)
{
  guint i;

  for (i = position; i < position + added; i++) {
    AdwViewStackPage *page = g_list_model_get_item (model, i);

    g_signal_connect_swapped (page, "notify::visible", G_CALLBACK (recreate_toggles), self);

    g_object_unref (page);
  }

  recreate_toggles (self);
}

static void
selection_changed_cb (AdwInlineViewSwitcher *self,
                      guint                  position,
                      guint                  n_items,
                      GtkSelectionModel     *model)
{
  GtkWidget *visible_child = adw_view_stack_get_visible_child (self->stack);
  AdwViewStackPage *page = adw_view_stack_get_page (self->stack, visible_child);
  guint index = GTK_INVALID_LIST_POSITION;

  if (adw_view_stack_page_get_visible (page)) {
    AdwToggle *toggle = g_hash_table_lookup (self->toggles, page);
    index = adw_toggle_get_index (toggle);
  }

  self->block_notify_active++;

  adw_toggle_group_set_active (ADW_TOGGLE_GROUP (self->toggle_group), index);

  self->block_notify_active--;
}

static void
set_stack (AdwInlineViewSwitcher *self,
           AdwViewStack          *stack)
{
  guint i, n;

  if (!stack)
    return;

  self->stack = g_object_ref (stack);
  self->pages = g_object_ref (G_LIST_MODEL (adw_view_stack_get_pages (stack)));

  populate_group (self);

  n = g_list_model_get_n_items (self->pages);

  for (i = 0; i < n; i++) {
    AdwViewStackPage *page = g_list_model_get_item (self->pages, i);

    g_signal_connect_swapped (page, "notify::visible", G_CALLBACK (recreate_toggles), self);

    g_object_unref (page);
  }

  g_signal_connect_swapped (self->pages, "items-changed", G_CALLBACK (items_changed_cb), self);
  g_signal_connect_swapped (self->pages, "selection-changed", G_CALLBACK (selection_changed_cb), self);
}

static void
unset_stack (AdwInlineViewSwitcher *self)
{
  guint i, n;

  if (!self->stack)
    return;

  clear_group (self);

  n = g_list_model_get_n_items (self->pages);

  for (i = 0; i < n; i++) {
    AdwViewStackPage *page = g_list_model_get_item (self->pages, i);

    g_signal_handlers_disconnect_by_func (page, recreate_toggles, self);

    g_object_unref (page);
  }

  g_signal_handlers_disconnect_by_func (self->pages, items_changed_cb, self);
  g_signal_handlers_disconnect_by_func (self->pages, selection_changed_cb, self);
  g_clear_object (&self->pages);
  g_clear_object (&self->stack);
}

static GtkOrientation
get_orientation (AdwInlineViewSwitcher *self)
{
  return gtk_orientable_get_orientation (GTK_ORIENTABLE (self->toggle_group));
}

static void
set_orientation (AdwInlineViewSwitcher *self,
                 GtkOrientation         orientation)
{
  if (orientation == gtk_orientable_get_orientation (GTK_ORIENTABLE (self->toggle_group)))
    return;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self->toggle_group),
                                  orientation);

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
css_classes_changed_cb (AdwInlineViewSwitcher *self)
{
  if (gtk_widget_has_css_class (GTK_WIDGET (self), "flat"))
    gtk_widget_add_css_class (self->toggle_group, "flat");
  else
    gtk_widget_remove_css_class (self->toggle_group, "flat");

  if (gtk_widget_has_css_class (GTK_WIDGET (self), "round"))
    gtk_widget_add_css_class (self->toggle_group, "round");
  else
    gtk_widget_remove_css_class (self->toggle_group, "round");

  if (gtk_widget_has_css_class (GTK_WIDGET (self), "osd"))
    gtk_widget_add_css_class (self->toggle_group, "osd");
  else
    gtk_widget_remove_css_class (self->toggle_group, "osd");
}

static void
adw_inline_view_switcher_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  AdwInlineViewSwitcher *self = ADW_INLINE_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_STACK:
    g_value_set_object (value, adw_inline_view_switcher_get_stack (self));
    break;
  case PROP_DISPLAY_MODE:
    g_value_set_enum (value, adw_inline_view_switcher_get_display_mode (self));
    break;
  case PROP_HOMOGENEOUS:
    g_value_set_boolean (value, adw_inline_view_switcher_get_homogeneous (self));
    break;
  case PROP_CAN_SHRINK:
    g_value_set_boolean (value, adw_inline_view_switcher_get_can_shrink (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_inline_view_switcher_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  AdwInlineViewSwitcher *self = ADW_INLINE_VIEW_SWITCHER (object);

  switch (prop_id) {
  case PROP_STACK:
    adw_inline_view_switcher_set_stack (self, g_value_get_object (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  case PROP_DISPLAY_MODE:
    adw_inline_view_switcher_set_display_mode (self, g_value_get_enum (value));
    break;
  case PROP_HOMOGENEOUS:
    adw_inline_view_switcher_set_homogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SHRINK:
    adw_inline_view_switcher_set_can_shrink (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_inline_view_switcher_dispose (GObject *object)
{
  AdwInlineViewSwitcher *self = ADW_INLINE_VIEW_SWITCHER (object);

  unset_stack (self);

  g_clear_pointer (&self->toggles, g_hash_table_unref);
  g_clear_pointer (&self->toggle_group, gtk_widget_unparent);

  G_OBJECT_CLASS (adw_inline_view_switcher_parent_class)->dispose (object);
}

static void
adw_inline_view_switcher_class_init (AdwInlineViewSwitcherClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_inline_view_switcher_get_property;
  object_class->set_property = adw_inline_view_switcher_set_property;
  object_class->dispose = adw_inline_view_switcher_dispose;

  /**
   * AdwInlineViewSwitcher:stack:
   *
   * The stack the view switcher controls.
   *
   * Since: 1.7
   */
  props[PROP_STACK] =
    g_param_spec_object ("stack", NULL, NULL,
                         ADW_TYPE_VIEW_STACK,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwInlineViewSwitcher:display-mode:
   *
   * The display mode.
   *
   * Determines what the toggles display: a label, an icon or both.
   *
   * <picture>
   *   <source srcset="inline-view-switcher-display-modes-dark.png" media="(prefers-color-scheme: dark)">
   *   <img src="inline-view-switcher-display-modes.png" alt="inline-view-switcher-display-modes">
   * </picture>
   *
   * Since: 1.7
   */
  props[PROP_DISPLAY_MODE] =
    g_param_spec_enum ("display-mode", NULL, NULL,
                       ADW_TYPE_INLINE_VIEW_SWITCHER_DISPLAY_MODE,
                       ADW_INLINE_VIEW_SWITCHER_LABELS,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwInlineViewSwitcher:homogeneous:
   *
   * Whether all toggles take the same size.
   *
   * Since: 1.7
   */
  props[PROP_HOMOGENEOUS] =
    g_param_spec_boolean ("homogeneous", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwInlineViewSwitcher:can-shrink:
   *
   * Whether the toggles can be smaller than the natural size of their contents.
   *
   * If set to `TRUE`, the toggle labels will ellipsize.
   *
   * See [property@ToggleGroup:can-shrink].
   *
   * Since: 1.7
   */
  props[PROP_CAN_SHRINK] =
    g_param_spec_boolean ("can-shrink", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "inline-view-switcher");
}

static void
adw_inline_view_switcher_init (AdwInlineViewSwitcher *self)
{
  self->toggle_group = g_object_new (ADW_TYPE_TOGGLE_GROUP,
                                     "accessible-role", GTK_ACCESSIBLE_ROLE_TAB_LIST,
                                     NULL);

  g_signal_connect_swapped (self->toggle_group, "notify::active",
                            G_CALLBACK (notify_active_cb), self);

  g_signal_connect (self, "notify::css-classes",
                    G_CALLBACK (css_classes_changed_cb), self);

  self->toggles = g_hash_table_new_full (g_direct_hash, g_direct_equal, g_object_unref, NULL);

  gtk_widget_set_parent (self->toggle_group, GTK_WIDGET (self));

  gtk_widget_add_css_class (self->toggle_group, "labels");
}

/**
 * adw_inline_view_switcher_new:
 *
 * Creates a new `AdwInlineViewSwitcher`.
 *
 * Returns: the newly created `AdwInlineViewSwitcher`
 *
 * Since: 1.7
 */
GtkWidget *
adw_inline_view_switcher_new (void)
{
  return g_object_new (ADW_TYPE_INLINE_VIEW_SWITCHER, NULL);
}

/**
 * adw_inline_view_switcher_get_stack:
 * @self: an inline stack switcher
 *
 * Gets the stack @self controls.
 *
 * Returns: (nullable) (transfer none): The stack of @self
 *
 * Since: 1.7
 */
AdwViewStack *
adw_inline_view_switcher_get_stack (AdwInlineViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self), NULL);

  return self->stack;
}

/**
 * adw_inline_view_switcher_set_stack:
 * @self: an inline stack switcher
 * @stack: (nullable): a stack
 *
 * Sets the stack to control.
 *
 * Since: 1.7
 */
void
adw_inline_view_switcher_set_stack (AdwInlineViewSwitcher *self,
                                    AdwViewStack          *stack)
{
  g_return_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self));
  g_return_if_fail (stack == NULL || ADW_IS_VIEW_STACK (stack));

  if (self->stack == stack)
    return;

  unset_stack (self);
  set_stack (self, stack);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_STACK]);
}

/**
 * adw_inline_view_switcher_get_display_mode:
 * @self: an inline stack switcher
 *
 * Gets the display mode of @self.
 *
 * Returns: the display mode
 *
 * Since: 1.7
 */
AdwInlineViewSwitcherDisplayMode
adw_inline_view_switcher_get_display_mode (AdwInlineViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self), ADW_INLINE_VIEW_SWITCHER_LABELS);

  return self->display_mode;
}

/**
 * adw_inline_view_switcher_set_display_mode:
 * @self: an inline stack switcher
 * @mode: the display mode
 *
 * Sets the display mode of @self.
 *
 * Determines what the toggles display: a label, an icon or both.
 *
 * <picture>
 *   <source srcset="inline-view-switcher-display-modes-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="inline-view-switcher-display-modes.png" alt="inline-view-switcher-display-modes">
 * </picture>
 *
 * Since: 1.7
 */
void
adw_inline_view_switcher_set_display_mode (AdwInlineViewSwitcher            *self,
                                           AdwInlineViewSwitcherDisplayMode  mode)
{
  GHashTableIter iter;
  AdwViewStackPage *page;
  AdwToggle *toggle;

  g_return_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self));
  g_return_if_fail (mode <= ADW_INLINE_VIEW_SWITCHER_BOTH);

  if (mode == self->display_mode)
    return;

  self->display_mode = mode;

  switch (self->display_mode) {
  case ADW_INLINE_VIEW_SWITCHER_LABELS:
    {
      gtk_widget_add_css_class (self->toggle_group, "labels");
      gtk_widget_remove_css_class (self->toggle_group, "icons");
      gtk_widget_remove_css_class (self->toggle_group, "both");
    }
    break;
  case ADW_INLINE_VIEW_SWITCHER_ICONS:
    {
      gtk_widget_add_css_class (self->toggle_group, "icons");
      gtk_widget_remove_css_class (self->toggle_group, "labels");
      gtk_widget_remove_css_class (self->toggle_group, "both");
    }
    break;
  case ADW_INLINE_VIEW_SWITCHER_BOTH:
    {
      gtk_widget_add_css_class (self->toggle_group, "both");
      gtk_widget_remove_css_class (self->toggle_group, "icons");
      gtk_widget_remove_css_class (self->toggle_group, "labels");
    }
    break;
  default:
    g_assert_not_reached ();
  }

  g_hash_table_iter_init (&iter, self->toggles);
  while (g_hash_table_iter_next (&iter, (gpointer *) &page, (gpointer *) &toggle))
    update_toggle (self, toggle, page);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DISPLAY_MODE]);
}

/**
 * adw_inline_view_switcher_get_homogeneous:
 * @self: an inline stack switcher
 *
 * Gets whether all toggles within @self take the same size.
 *
 * Returns: whether all toggles take the same size
 *
 * Since: 1.7
 */
gboolean
adw_inline_view_switcher_get_homogeneous (AdwInlineViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self), FALSE);

  return adw_toggle_group_get_homogeneous (ADW_TOGGLE_GROUP (self->toggle_group));
}

/**
 * adw_inline_view_switcher_set_homogeneous:
 * @self: an inline stack switcher
 * @homogeneous: whether all toggles should take the same size
 *
 * Sets whether all toggles within @self take the same size.
 *
 * Since: 1.7
 */
void
adw_inline_view_switcher_set_homogeneous (AdwInlineViewSwitcher *self,
                                           gboolean               homogeneous)
{
  g_return_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self));

  homogeneous = !!homogeneous;

  if (homogeneous == adw_inline_view_switcher_get_homogeneous (self))
    return;

  adw_toggle_group_set_homogeneous (ADW_TOGGLE_GROUP (self->toggle_group),
                                    homogeneous);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HOMOGENEOUS]);
}

/**
 * adw_inline_view_switcher_get_can_shrink:
 * @self: an inline stack switcher
 *
 * Gets whether the toggles can be smaller than the natural size of their
 * contents.
 *
 * Returns: whether the toggles can shrink
 *
 * Since: 1.7
 */
gboolean
adw_inline_view_switcher_get_can_shrink (AdwInlineViewSwitcher *self)
{
  g_return_val_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self), FALSE);

  return adw_toggle_group_get_can_shrink (ADW_TOGGLE_GROUP (self->toggle_group));
}

/**
 * adw_inline_view_switcher_set_can_shrink:
 * @self: an inline stack switcher
 * @can_shrink: whether the toggles can shrink
 *
 * Sets whether the toggles can be smaller than the natural size of their
 * contents.
 *
 * If @can_shrink is `TRUE`, the toggle labels will ellipsize.
 *
 * See [property@ToggleGroup:can-shrink].
 *
 * Since: 1.7
 */
void
adw_inline_view_switcher_set_can_shrink (AdwInlineViewSwitcher *self,
                                         gboolean               can_shrink)
{
  g_return_if_fail (ADW_IS_INLINE_VIEW_SWITCHER (self));

  can_shrink = !!can_shrink;

  if (can_shrink == adw_inline_view_switcher_get_can_shrink (self))
    return;

  adw_toggle_group_set_can_shrink (ADW_TOGGLE_GROUP (self->toggle_group),
                                   can_shrink);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SHRINK]);
}
