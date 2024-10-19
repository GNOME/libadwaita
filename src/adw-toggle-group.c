/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-toggle-group.h"

#include "adw-gtkbuilder-utils-private.h"
#include "adw-widget-utils-private.h"

/**
 * AdwToggleGroup:
 *
 * A group of exclusive toggles.
 *
 * <picture>
 *   <source srcset="toggle-group-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toggle-group.png" alt="toggle-group">
 * </picture>
 *
 * `AdwToggleGroup` presents a set of exclusive toggles, represented as
 * [class@Toggle] objects. Each toggle can display an icon, a label, an icon
 * and a label, or a custom child.
 *
 * Toggles are indexed by their position, with the first toggle being equivalent
 * to 0, and so on. Use the [property@ToggleGroup:active] to get that position.
 *
 * Toggles can also have optional names, set via the [property@Toggle:name]
 * property. The name of the active toggle can be accessed via the
 * [property@ToggleGroup:active-name] property.
 *
 * `AdwToggle` objects can be retrieved via their index or name, using
 * [method@ToggleGroup.get_toggle] or [method@ToggleGroup.get_toggle_by_name]
 * respectively. `AdwToggleGroup` also provides a [iface@Gtk.SelectionModel] of
 * its toggles via the [property@ToggleGroup:toggles] property.
 *
 * `AdwToggleGroup` is orientable, and the toggles can be displayed horizontally
 * or vertically. This is mostly useful for icon-only toggles.
 *
 * Use the [property@ToggleGroup:homogeneous] property to make the toggles take
 * the same size, and the [property@ToggleGroup:can-shrink] to control whether
 * the toggles can ellipsize.
 *
 * Example of an `AdwToggleGroup` UI definition:
 *
 * ```xml
 *  <object class="AdwToggleGroup">
 *    <property name="active-name">picture</property>
 *    <child>
 *      <object class="AdwToggle">
 *        <property name="icon-name">camera-photo-symbolic</property>
 *        <property name="tooltip" translatable="yes">Picture Mode</property>
 *        <property name="name">picture</property>
 *      </object>
 *    </child>
 *    <child>
 *      <object class="AdwToggle">
 *        <property name="icon-name">camera-video-symbolic</property>
 *        <property name="tooltip" translatable="yes">Recording Mode</property>
 *        <property name="name">recording</property>
 *      </object>
 *    </child>
 *  </object>
 * ```
 *
 * See also: [class@InlineViewSwitcher].
 *
 * ## CSS nodes
 *
 * `AdwToggleGroup` has a main CSS node with the name `toggle-group`.
 *
 * Its toggles have CSS nodes with the name `toggle`, and its separators have nodes
 * with the name `separator`.
 *
 * Toggle nodes will have a different style classes depending on their content:
 * `.text-button` for labels, `.image-button` for icons, `.image-text-button`
 * for both or no style class for custom children.
 *
 * The hidden separators use the `.hidden` style class.
 *
 * ## Style classes
 *
 * `AdwToggleGroup` can use the [`.flat`](style-classes.html#flat_1) style class
 * to remove its background and make it look like a group of buttons.
 *
 * <picture>
 *   <source srcset="toggle-group-flat-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toggle-group-flat.png" alt="toggle-group-flat">
 * </picture>
 *
 * It can also use the [`.round`](style-classes.html#round) style class to make
 * its toggles and the group itself rounded.
 *
 * <picture>
 *   <source srcset="toggle-group-round-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toggle-group-round.png" alt="toggle-group-round">
 * </picture>
 *
 * They can also be combined with each other.
 *
 * <picture>
 *   <source srcset="toggle-group-flat-round-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toggle-group-flat-round.png" alt="toggle-group-flat-round">
 * </picture>
 *
 * ## Accessibility
 *
 * `AdwToggleGroup` uses the `GTK_ACCESSIBLE_ROLE_RADIO_GROUP` role. Its toggles
 * use the `GTK_ACCESSIBLE_ROLE_RADIO` role.
 *
 * Since: 1.7
 */

/**
 * AdwToggle:
 *
 * A toggle within [class@ToggleGroup].
 *
 * `AdwToggle` can optionally have a name, set with [property@Toggle:name].
 * If the name is set, [property@ToggleGroup:active-name] can be used to access
 * toggles instead of index.
 *
 * Since: 1.7
 */

struct _AdwToggle {
  GObject parent_instance;

  AdwToggleGroup *group;
  guint index;

  char *name;
  char *label;
  char *icon_name;
  gboolean use_underline;
  char *tooltip;
  GtkWidget *child;
  gboolean enabled;

  GtkWidget *button;
  GtkWidget *separator;
};

G_DEFINE_FINAL_TYPE (AdwToggle, adw_toggle, G_TYPE_OBJECT)

enum {
  TOGGLE_PROP_0,
  TOGGLE_PROP_NAME,
  TOGGLE_PROP_LABEL,
  TOGGLE_PROP_USE_UNDERLINE,
  TOGGLE_PROP_ICON_NAME,
  TOGGLE_PROP_TOOLTIP,
  TOGGLE_PROP_CHILD,
  TOGGLE_PROP_ENABLED,
  LAST_TOGGLE_PROP
};

static GParamSpec *toggle_props[LAST_TOGGLE_PROP];

struct _AdwToggleGroup {
  GtkWidget parent_instance;

  GtkOrientation orientation;
  gboolean can_shrink;

  GPtrArray *toggles;
  GHashTable *name_to_toggle;

  guint active;

  GtkSizeGroup *size_group;

  GtkSelectionModel *toggles_model;

  guint delayed_active;
  char *delayed_active_name;
};

static void adw_toggle_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwToggleGroup, adw_toggle_group, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_toggle_group_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_N_TOGGLES,
  PROP_ACTIVE,
  PROP_ACTIVE_NAME,
  PROP_HOMOGENEOUS,
  PROP_CAN_SHRINK,
  PROP_TOGGLES,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_TOGGLES + 1,
};

static GParamSpec *props[LAST_PROP];

static void
update_content (AdwToggle *self)
{
  if (!self->button)
    return;

  gtk_widget_set_tooltip_markup (self->button, self->tooltip);

  if (self->label && *self->label) {
    if (self->use_underline) {
      char *stripped_label = adw_strip_mnemonic (self->label);
      gtk_accessible_update_property (GTK_ACCESSIBLE (self->button),
                                      GTK_ACCESSIBLE_PROPERTY_LABEL, stripped_label,
                                      -1);
      g_free (stripped_label);
    } else {
      gtk_accessible_update_property (GTK_ACCESSIBLE (self->button),
                                      GTK_ACCESSIBLE_PROPERTY_LABEL, self->label,
                                      -1);
    }
    /* Tooltip is automatically set as description */
  }

  if (self->child) {
    gtk_button_set_child (GTK_BUTTON (self->button), self->child);
    return;
  }

  if (self->icon_name && *self->icon_name && self->label && *self->label) {
    GtkWidget *content = adw_button_content_new ();

    adw_button_content_set_icon_name (ADW_BUTTON_CONTENT (content), self->icon_name);
    if (self->label && *self->label)
      adw_button_content_set_label (ADW_BUTTON_CONTENT (content), self->label);
    adw_button_content_set_use_underline (ADW_BUTTON_CONTENT (content),
                                          self->use_underline);

    g_object_bind_property (self->button, "can-shrink",
                            content, "can-shrink",
                            G_BINDING_SYNC_CREATE);

    gtk_button_set_child (GTK_BUTTON (self->button), content);
    return;
  }

  if (self->label && *self->label) {
    gtk_button_set_label (GTK_BUTTON (self->button), self->label);
    gtk_button_set_use_underline (GTK_BUTTON (self->button), self->use_underline);
    return;
  }

  if (self->tooltip && *self->tooltip) {
    char *tooltip_text;

    pango_parse_markup (self->tooltip, -1, 0, NULL, &tooltip_text, NULL, NULL);

    gtk_accessible_update_property (GTK_ACCESSIBLE (self->button),
                                    GTK_ACCESSIBLE_PROPERTY_LABEL, tooltip_text,
                                    GTK_ACCESSIBLE_PROPERTY_DESCRIPTION, NULL,
                                    -1);

    g_free (tooltip_text);
  } else {
    gtk_accessible_update_property (GTK_ACCESSIBLE (self->button),
                                    GTK_ACCESSIBLE_PROPERTY_LABEL, NULL,
                                    -1);
  }

  if (self->icon_name && *self->icon_name) {
    gtk_button_set_icon_name (GTK_BUTTON (self->button), self->icon_name);
    return;
  }

  gtk_button_set_child (GTK_BUTTON (self->button), NULL);
}

static void
adw_toggle_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwToggle *self = ADW_TOGGLE (object);

  switch (property_id) {
  case TOGGLE_PROP_NAME:
    g_value_set_string (value, adw_toggle_get_name (self));
    break;
  case TOGGLE_PROP_LABEL:
    g_value_set_string (value, adw_toggle_get_label (self));
    break;
  case TOGGLE_PROP_USE_UNDERLINE:
    g_value_set_boolean (value, adw_toggle_get_use_underline (self));
    break;
  case TOGGLE_PROP_ICON_NAME:
    g_value_set_string (value, adw_toggle_get_icon_name (self));
    break;
  case TOGGLE_PROP_TOOLTIP:
    g_value_set_string (value, adw_toggle_get_tooltip (self));
    break;
  case TOGGLE_PROP_CHILD:
    g_value_set_object (value, adw_toggle_get_child (self));
    break;
  case TOGGLE_PROP_ENABLED:
    g_value_set_boolean (value, adw_toggle_get_enabled (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_toggle_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwToggle *self = ADW_TOGGLE (object);

  switch (property_id) {
  case TOGGLE_PROP_NAME:
    adw_toggle_set_name (self, g_value_get_string (value));
    break;
  case TOGGLE_PROP_LABEL:
    adw_toggle_set_label (self, g_value_get_string (value));
    break;
  case TOGGLE_PROP_USE_UNDERLINE:
    adw_toggle_set_use_underline (self, g_value_get_boolean (value));
    break;
  case TOGGLE_PROP_ICON_NAME:
    adw_toggle_set_icon_name (self, g_value_get_string (value));
    break;
  case TOGGLE_PROP_TOOLTIP:
    adw_toggle_set_tooltip (self, g_value_get_string (value));
    break;
  case TOGGLE_PROP_CHILD:
    adw_toggle_set_child (self, g_value_get_object (value));
    break;
  case TOGGLE_PROP_ENABLED:
    adw_toggle_set_enabled (self, g_value_get_boolean (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
adw_toggle_finalize (GObject *object)
{
  AdwToggle *self = ADW_TOGGLE (object);

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->label, g_free);
  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->tooltip, g_free);

  G_OBJECT_CLASS (adw_toggle_parent_class)->finalize (object);
}

static void
adw_toggle_class_init (AdwToggleClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = adw_toggle_finalize;
  object_class->get_property = adw_toggle_get_property;
  object_class->set_property = adw_toggle_set_property;

  /**
   * AdwToggle:name:
   *
   * The toggle name.
   *
   * Allows accessing the toggle by its name instead of index.
   *
   * See [property@ToggleGroup:active-name].
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_NAME] =
    g_param_spec_string ("name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggle:label:
   *
   * The toggle label.
   *
   * The label will be displayed alone or next to the icon, unless
   * [property@Toggle:child] is set, but will still be read out by the screen
   * reader.
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_LABEL] =
    g_param_spec_string ("label", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggle:use-underline:
   *
   * Whether an embedded underline in the label indicates a mnemonic.
   *
   * See [property@Toggle:label].
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_USE_UNDERLINE] =
    g_param_spec_boolean ("use-underline", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggle:icon-name:
   *
   * The toggle icon name.
   *
   * The icon will be displayed alone or next to the label, unless
   * [property@Toggle:child] is set.
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_ICON_NAME] =
    g_param_spec_string ("icon-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggle:tooltip:
   *
   * The tooltip of the toggle.
   *
   * The tooltip can be marked up with the Pango text markup language.
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_TOOLTIP] =
    g_param_spec_string ("tooltip", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggle:child:
   *
   * The toggle child.
   *
   * When the child is set, icon and label are not displayed.
   *
   * It's recommended to still set the label, as it can still be used by the
   * screen reader.
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_CHILD] =
    g_param_spec_object ("child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggle:enabled:
   *
   * Whether this toggle is enabled.
   *
   * Since: 1.7
   */
  toggle_props[TOGGLE_PROP_ENABLED] =
    g_param_spec_boolean ("enabled", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_TOGGLE_PROP, toggle_props);
}

static void
adw_toggle_init (AdwToggle *self)
{
  self->enabled = TRUE;
  self->index = GTK_INVALID_LIST_POSITION;
  self->tooltip = g_strdup ("");
}

#define ADW_TYPE_TOGGLE_GROUP_TOGGLES (adw_toggle_group_toggles_get_type ())

G_DECLARE_FINAL_TYPE (AdwToggleGroupToggles, adw_toggle_group_toggles, ADW, TOGGLE_GROUP_TOGGLES, GObject)

struct _AdwToggleGroupToggles
{
  GObject parent_instance;

  AdwToggleGroup *group;
};

static GType
adw_toggle_group_toggles_get_item_type (GListModel *model)
{
  return ADW_TYPE_TOGGLE;
}

static guint
adw_toggle_group_toggles_get_n_items (GListModel *model)
{
  AdwToggleGroupToggles *self = ADW_TOGGLE_GROUP_TOGGLES (model);

  if (G_UNLIKELY (!ADW_IS_TOGGLE_GROUP (self->group)))
    return 0;

  return adw_toggle_group_get_n_toggles (self->group);
}

static gpointer
adw_toggle_group_toggles_get_item (GListModel *model,
                                   guint       position)
{
  AdwToggleGroupToggles *self = ADW_TOGGLE_GROUP_TOGGLES (model);
  AdwToggle *toggle;

  if (G_UNLIKELY (!ADW_IS_TOGGLE_GROUP (self->group)))
    return NULL;

  toggle = adw_toggle_group_get_toggle (self->group, position);
  if (!toggle)
    return NULL;

  return g_object_ref (toggle);
}

static void
adw_toggle_group_toggles_list_model_init (GListModelInterface *iface)
{
  iface->get_item_type = adw_toggle_group_toggles_get_item_type;
  iface->get_n_items = adw_toggle_group_toggles_get_n_items;
  iface->get_item = adw_toggle_group_toggles_get_item;
}

static gboolean
adw_toggle_group_toggles_is_selected (GtkSelectionModel *model,
                                      guint              position)
{
  AdwToggleGroupToggles *self = ADW_TOGGLE_GROUP_TOGGLES (model);

  if (G_UNLIKELY (!ADW_IS_TOGGLE_GROUP (self->group)))
    return FALSE;

  return position == self->group->active;
}

static gboolean
adw_toggle_group_toggles_select_item (GtkSelectionModel *model,
                                      guint              position,
                                      gboolean           exclusive)
{
  AdwToggleGroupToggles *self = ADW_TOGGLE_GROUP_TOGGLES (model);

  if (G_UNLIKELY (!ADW_IS_TOGGLE_GROUP (self->group)))
    return FALSE;

  if (position >= self->group->toggles->len)
    return FALSE;

  adw_toggle_group_set_active (self->group, position);

  return TRUE;
}

static void
adw_toggle_group_toggles_selection_model_init (GtkSelectionModelInterface *iface)
{
  iface->is_selected = adw_toggle_group_toggles_is_selected;
  iface->select_item = adw_toggle_group_toggles_select_item;
}

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwToggleGroupToggles, adw_toggle_group_toggles, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, adw_toggle_group_toggles_list_model_init)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SELECTION_MODEL, adw_toggle_group_toggles_selection_model_init))

static void
adw_toggle_group_toggles_dispose (GObject *object)
{
  AdwToggleGroupToggles *self = ADW_TOGGLE_GROUP_TOGGLES (object);

  g_clear_weak_pointer (&self->group);

  G_OBJECT_CLASS (adw_toggle_group_toggles_parent_class)->dispose (object);
}

static void
adw_toggle_group_toggles_class_init (AdwToggleGroupTogglesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_toggle_group_toggles_dispose;
}

static void
adw_toggle_group_toggles_init (AdwToggleGroupToggles *self)
{
}

static GtkSelectionModel *
adw_toggle_group_toggles_new (AdwToggleGroup *group)
{
  AdwToggleGroupToggles *toggles;

  toggles = g_object_new (ADW_TYPE_TOGGLE_GROUP_TOGGLES, NULL);
  g_set_weak_pointer (&toggles->group, group);

  return GTK_SELECTION_MODEL (toggles);
}

static void
set_active_toggle (AdwToggleGroup *self,
                   AdwToggle      *toggle)
{
  AdwToggle *prev_toggle = adw_toggle_group_get_toggle (self, self->active);
  gboolean has_name, prev_has_name;

  if (toggle && !toggle->enabled)
    toggle = NULL;

  if (prev_toggle == toggle)
    return;

  if (toggle)
    self->active = toggle->index;
  else
    self->active = GTK_INVALID_LIST_POSITION;

  prev_has_name = prev_toggle && prev_toggle->name;
  has_name = toggle && toggle->name;

  /* We set the active button after self->active to avoid calling this
   * function twice, see toggle_active_cb () */
  if (toggle && toggle->button) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle->button), TRUE);
    gtk_accessible_update_state (GTK_ACCESSIBLE (toggle->button),
                                 GTK_ACCESSIBLE_STATE_CHECKED, GTK_ACCESSIBLE_TRISTATE_TRUE,
                                 -1);
  }

  if (prev_toggle && prev_toggle->button) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (prev_toggle->button), FALSE);
    gtk_accessible_update_state (GTK_ACCESSIBLE (prev_toggle->button),
                                 GTK_ACCESSIBLE_STATE_CHECKED, GTK_ACCESSIBLE_TRISTATE_FALSE,
                                 -1);
  }

  if (self->toggles_model) {
    guint old_index = prev_toggle ? prev_toggle->index : GTK_INVALID_LIST_POSITION;
    guint new_index = self->active;

    if (old_index == GTK_INVALID_LIST_POSITION && new_index == GTK_INVALID_LIST_POSITION) {
      /* nothing to do */
    } else if (old_index == GTK_INVALID_LIST_POSITION) {
      gtk_selection_model_selection_changed (self->toggles_model, new_index, 1);
    } else if (new_index == GTK_INVALID_LIST_POSITION) {
      gtk_selection_model_selection_changed (self->toggles_model, old_index, 1);
    } else {
      gtk_selection_model_selection_changed (self->toggles_model,
                                             MIN (old_index, new_index),
                                             MAX (old_index, new_index) -
                                             MIN (old_index, new_index) + 1);
    }
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVE]);

  if (prev_has_name || has_name)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVE_NAME]);
}

static gboolean
should_hide_separators (GtkWidget *toggle)
{
  GtkStateFlags flags;

  if (!toggle)
    return TRUE;

  flags = gtk_widget_get_state_flags (toggle);

  if (flags & (GTK_STATE_FLAG_PRELIGHT |
               GTK_STATE_FLAG_SELECTED |
               GTK_STATE_FLAG_CHECKED)) {
    return TRUE;
  }

  if ((flags & GTK_STATE_FLAG_FOCUSED) &&
      (flags & GTK_STATE_FLAG_FOCUS_VISIBLE)) {
    return TRUE;
  }

  return FALSE;
}

static void
update_separator (AdwToggleGroup *self,
                  GtkWidget      *separator)
{
  GtkWidget *prev_button;
  GtkWidget *next_button;

  prev_button = gtk_widget_get_prev_sibling (separator);
  next_button = gtk_widget_get_next_sibling (separator);

  gtk_widget_set_visible (separator, prev_button && next_button);

  if (should_hide_separators (prev_button) ||
      should_hide_separators (next_button))
    gtk_widget_add_css_class (separator, "hidden");
  else
    gtk_widget_remove_css_class (separator, "hidden");
}

static void
toggle_active_cb (AdwToggle *toggle)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle->button)))
    set_active_toggle (toggle->group, toggle);
}

static void
state_flags_changed_cb (AdwToggleGroup *self,
                        GtkStateFlags   flags,
                        GtkWidget      *button)

{
  GtkWidget *prev_separator;
  GtkWidget *next_separator;

  prev_separator = gtk_widget_get_prev_sibling (button);
  next_separator = gtk_widget_get_next_sibling (button);

  if (prev_separator != NULL)
    update_separator (self, prev_separator);

  if (next_separator != NULL)
    update_separator (self, next_separator);
}

static void
add_toggle (AdwToggleGroup *self,
            AdwToggle      *toggle)
{
  GtkAccessibleRole accessible_role;

  if (toggle->name && adw_toggle_group_get_toggle_by_name (self, toggle->name)) {
    g_critical ("Trying to add a toggle with the name '%s' to an "
                "AdwToggleGroup, but such a toggle already exists", toggle->name);
    g_object_unref (toggle);
    return;
  }

  /* Special case for AdwInlineViewSwitcher */
  if (gtk_accessible_get_accessible_role (GTK_ACCESSIBLE (self)) == GTK_ACCESSIBLE_ROLE_TAB_LIST)
    accessible_role = GTK_ACCESSIBLE_ROLE_TAB;
  else
    accessible_role = GTK_ACCESSIBLE_ROLE_RADIO;

  toggle->group = self;

  toggle->button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                                 "accessible-role", accessible_role,
                                 "css-name", "toggle",
                                 NULL);
  gtk_widget_set_hexpand (toggle->button, TRUE);
  gtk_widget_set_vexpand (toggle->button, TRUE);
  gtk_button_set_can_shrink (GTK_BUTTON (toggle->button), self->can_shrink);
  gtk_widget_set_sensitive (toggle->button, toggle->enabled);

  toggle->separator = gtk_separator_new (self->orientation);

  update_content (toggle);

  if (self->toggles->len) {
    AdwToggle *first_toggle = g_ptr_array_index (self->toggles, 0);

    gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (toggle->button),
                                 GTK_TOGGLE_BUTTON (first_toggle->button));
  }

  if (self->size_group)
    gtk_size_group_add_widget (self->size_group, toggle->button);

  gtk_widget_set_parent (toggle->separator, GTK_WIDGET (self));
  gtk_widget_set_parent (toggle->button, GTK_WIDGET (self));

  g_signal_connect_swapped (toggle->button, "state-flags-changed",
                            G_CALLBACK (state_flags_changed_cb), self);
  g_signal_connect_swapped (toggle->button, "notify::active",
                            G_CALLBACK (toggle_active_cb), toggle);

  update_separator (self, toggle->separator);

  if (toggle->name)
    g_hash_table_insert (self->name_to_toggle, g_strdup (toggle->name), toggle);
  g_ptr_array_add (self->toggles, toggle);

  toggle->index = self->toggles->len - 1;

  if (self->active == GTK_INVALID_LIST_POSITION)
    set_active_toggle (self, toggle);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_TOGGLES]);

  if (self->toggles_model) {
    g_list_model_items_changed (G_LIST_MODEL (self->toggles_model),
                                self->toggles->len - 1, 0, 1);
  }
}

static void
set_orientation (AdwToggleGroup *self,
                 GtkOrientation  orientation)
{
  GtkLayoutManager *layout;
  int i;

  if (self->orientation == orientation)
    return;

  self->orientation = orientation;

  layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (layout), orientation);

  for (i = 0; i < self->toggles->len; i++) {
    AdwToggle *toggle = g_ptr_array_index (self->toggles, i);

    if (toggle->separator) {
      gtk_orientable_set_orientation (GTK_ORIENTABLE (toggle->separator),
                                      orientation);
    }
  }

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adw_toggle_group_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (object);

  switch (prop_id) {
  case PROP_N_TOGGLES:
    g_value_set_uint (value, adw_toggle_group_get_n_toggles (self));
    break;
  case PROP_ACTIVE:
    g_value_set_uint (value, adw_toggle_group_get_active (self));
    break;
  case PROP_ACTIVE_NAME:
    g_value_set_string (value, adw_toggle_group_get_active_name (self));
    break;
  case PROP_HOMOGENEOUS:
    g_value_set_boolean (value, adw_toggle_group_get_homogeneous (self));
    break;
  case PROP_CAN_SHRINK:
    g_value_set_boolean (value, adw_toggle_group_get_can_shrink (self));
    break;
  case PROP_TOGGLES:
    g_value_take_object (value, adw_toggle_group_get_toggles (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toggle_group_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (object);

  switch (prop_id) {
  case PROP_ACTIVE:
    adw_toggle_group_set_active (self, g_value_get_uint (value));
    break;
  case PROP_ACTIVE_NAME:
    adw_toggle_group_set_active_name (self, g_value_get_string (value));
    break;
  case PROP_HOMOGENEOUS:
    adw_toggle_group_set_homogeneous (self, g_value_get_boolean (value));
    break;
  case PROP_CAN_SHRINK:
    adw_toggle_group_set_can_shrink (self, g_value_get_boolean (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toggle_group_dispose (GObject *object)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (object);
  int i;

  if (self->toggles_model) {
    g_list_model_items_changed (G_LIST_MODEL (self->toggles_model), 0,
                                self->toggles->len, 0);
  }

  for (i = 0; i < self->toggles->len; i++) {
    AdwToggle *toggle = g_ptr_array_index (self->toggles, i);

    g_clear_pointer (&toggle->button, gtk_widget_unparent);
    g_clear_pointer (&toggle->separator, gtk_widget_unparent);
  }

  g_clear_object (&self->size_group);
  g_clear_pointer (&self->toggles, g_ptr_array_unref);
  g_clear_pointer (&self->name_to_toggle, g_hash_table_unref);

  G_OBJECT_CLASS (adw_toggle_group_parent_class)->dispose (object);
}

static void
adw_toggle_group_finalize (GObject *object)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (object);

  g_clear_weak_pointer (&self->toggles_model);

  G_OBJECT_CLASS (adw_toggle_group_parent_class)->finalize (object);
}

static gboolean
adw_toggle_group_focus (GtkWidget        *widget,
                        GtkDirectionType  direction)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (widget);

  if (!gtk_widget_get_focus_child (widget)) {
    AdwToggle *toggle = adw_toggle_group_get_toggle (self, self->active);

    if (toggle && toggle->button)
      return gtk_widget_child_focus (toggle->button, direction);

    return adw_widget_focus_child (widget, direction);
  }

  if (direction == GTK_DIR_TAB_FORWARD || direction == GTK_DIR_TAB_BACKWARD)
    return GDK_EVENT_PROPAGATE;

  return adw_widget_focus_child (widget, direction);
}

static gboolean
adw_toggle_group_grab_focus (GtkWidget *widget)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (widget);
  AdwToggle *toggle = adw_toggle_group_get_toggle (self, self->active);

  if (toggle && toggle->button)
    return gtk_widget_grab_focus (toggle->button);

  return adw_widget_grab_focus_child (widget);
}

static void
adw_toggle_group_class_init (AdwToggleGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_toggle_group_get_property;
  object_class->set_property = adw_toggle_group_set_property;
  object_class->dispose = adw_toggle_group_dispose;
  object_class->finalize = adw_toggle_group_finalize;

  widget_class->focus = adw_toggle_group_focus;
  widget_class->grab_focus = adw_toggle_group_grab_focus;

  /**
   * AdwToggleGroup:n-toggles:
   *
   * The number of toggles within the group.
   *
   * Since: 1.7
   */
  props[PROP_N_TOGGLES] =
    g_param_spec_uint ("n-toggles", NULL, NULL,
                       0, G_MAXUINT, 0,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwToggleGroup:active:
   *
   * The index of the active toggle.
   *
   * Setting the index to a larger value than the number of toggles in the group
   * unsets the current active toggle.
   *
   * If no toggle is active, the property will be set to
   * [const@Gtk.INVALID_LIST_POSITION].
   *
   * Since: 1.7
   */
  props[PROP_ACTIVE] =
    g_param_spec_uint ("active", NULL, NULL,
                       0, G_MAXUINT, GTK_INVALID_LIST_POSITION,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggleGroup:active-name:
   *
   * The name of the active toggle.
   *
   * The name can be set via [property@Toggle:name]. If the currently active
   * toggle doesn't have a name, the property will be set to `NULL`.
   *
   * Set it to `NULL` to unset the current active toggle.
   *
   * Since: 1.7
   */
  props[PROP_ACTIVE_NAME] =
    g_param_spec_string ("active-name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggleGroup:homogeneous:
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
   * AdwToggleGroup:can-shrink:
   *
   * Whether the toggles can be smaller than the natural size of their contents.
   *
   * If set to `TRUE`, the toggle labels will ellipsize.
   *
   * See [property@Gtk.Button:can-shrink].
   *
   * Since: 1.7
   */
  props[PROP_CAN_SHRINK] =
    g_param_spec_boolean ("can-shrink", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggleGroup:toggles:
   *
   * A selection model with the groups's toggles.
   *
   * This can be used to keep an up-to-date view. The model also implements
   * [iface@Gtk.SelectionModel] and can be used to track and change the active
   * toggle.
   *
   * Since: 1.7
   */
  props[PROP_TOGGLES] =
    g_param_spec_object ("toggles", NULL, NULL,
                         GTK_TYPE_SELECTION_MODEL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "toggle-group");

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_RADIO_GROUP);
}

static void
adw_toggle_group_init (AdwToggleGroup *self)
{
  self->active = GTK_INVALID_LIST_POSITION;
  self->toggles = g_ptr_array_new_full (0, g_object_unref);
  self->name_to_toggle = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  self->can_shrink = TRUE;
  self->delayed_active = GTK_INVALID_LIST_POSITION;
}

static void
adw_toggle_group_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  if (ADW_IS_TOGGLE (child)) {
    adw_toggle_group_add (ADW_TOGGLE_GROUP (buildable),
                          g_object_ref (ADW_TOGGLE (child)));
  } else if (GTK_IS_WIDGET (child)) {
    g_critical ("Trying to add %s as a child to an AdwToggleGroup, "
                "but only AdwToggle is allowed", G_OBJECT_TYPE_NAME (child));
  } else {
    parent_buildable_iface->add_child (buildable, builder, child, type);
  }
}

static void
adw_toggle_group_buildable_set_buildable_property (GtkBuildable *buildable,
                                                   GtkBuilder   *builder,
                                                   const char   *name,
                                                   const GValue *value)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (buildable);

  if (g_strcmp0 (name, "active") == 0)
    self->delayed_active = g_value_get_uint (value);
  else if (g_strcmp0 (name, "active-name") == 0)
    g_set_str (&self->delayed_active_name, g_value_get_string (value));
  else
    g_object_set_property (G_OBJECT (buildable), name, value);
}

static void
adw_toggle_group_buildable_parser_finished (GtkBuildable *buildable,
                                            GtkBuilder   *builder)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (buildable);

  /* We set the active toggle after the toggles have been loaded */
  if (self->delayed_active != GTK_INVALID_LIST_POSITION)
    adw_toggle_group_set_active (self, self->delayed_active);

  if (self->delayed_active_name != NULL)
    adw_toggle_group_set_active_name (self, self->delayed_active_name);

  self->delayed_active = GTK_INVALID_LIST_POSITION;
  g_clear_pointer (&self->delayed_active_name, g_free);
}

static void
adw_toggle_group_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_toggle_group_buildable_add_child;
  iface->set_buildable_property = adw_toggle_group_buildable_set_buildable_property;
  iface->parser_finished = adw_toggle_group_buildable_parser_finished;
}

/**
 * adw_toggle_new:
 *
 * Creates a new `AdwToggle`.
 *
 * Returns: the newly created `AdwToggle`
 *
 * Since: 1.7
 */
AdwToggle *
adw_toggle_new (void)
{
  return g_object_new (ADW_TYPE_TOGGLE, NULL);
}

/**
 * adw_toggle_get_name:
 * @self: a toggle
 *
 * Gets the name of @self.
 *
 * Returns: the toggle name
 *
 * Since: 1.7
 */
const char *
adw_toggle_get_name (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), NULL);

  return self->name;
}

/**
 * adw_toggle_set_name:
 * @self: a toggle
 * @name: (nullable): a name
 *
 * Sets the name of @self to @name.
 *
 * Allows accessing @self by its name instead of index.
 *
 * See [property@ToggleGroup:active-name].
 *
 * Since: 1.7
 */
void
adw_toggle_set_name (AdwToggle  *self,
                     const char *name)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));

  if (self->group && name && adw_toggle_group_get_toggle_by_name (self->group, name)) {
    g_critical ("Duplicate toggle name in AdwToggleGroup: %s", name);
    return;
  }

  if (!g_strcmp0 (self->name, name))
    return;

  if (self->group && self->name)
    g_hash_table_remove (self->group->name_to_toggle, self->name);

  g_set_str (&self->name, name);

  if (self->group && self->name)
    g_hash_table_insert (self->group->name_to_toggle, g_strdup (self->name), self);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_NAME]);

  if (self->group && adw_toggle_group_get_active (self->group) == self->index) {
    g_object_notify_by_pspec (G_OBJECT (self->group),
                              props[PROP_ACTIVE_NAME]);
  }
}

/**
 * adw_toggle_get_label:
 * @self: a toggle
 *
 * Gets the label of @self.
 *
 * Returns: (nullable): the toggle label
 *
 * Since: 1.7
 */
const char *
adw_toggle_get_label (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), NULL);

  return self->label;
}

/**
 * adw_toggle_set_label:
 * @self: a toggle
 * @label: (nullable): a label
 *
 * Sets the label of @self to @label.
 *
 * The label will be displayed alone or next to the icon, unless
 * [property@Toggle:child] is set, but will still be read out by the screen
 * reader.
 *
 * Since: 1.7
 */
void
adw_toggle_set_label (AdwToggle  *self,
                      const char *label)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));

  if (!g_set_str (&self->label, label))
    return;

  update_content (self);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_LABEL]);
}

/**
 * adw_toggle_get_use_underline:
 * @self: a toggle
 *
 * Gets whether @self uses underlines.
 *
 * Returns: whether the toggle uses underlines
 *
 * Since: 1.7
 */
gboolean
adw_toggle_get_use_underline (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), FALSE);

  return self->use_underline;
}

/**
 * adw_toggle_set_use_underline:
 * @self: a toggle
 * @use_underline: whether an underline in the label indicates a mnemonic
 *
 * Sets whether an embedded underline in the label indicates a mnemonic.
 *
 * See [property@Toggle:label].
 *
 * Since: 1.7
 */
void
adw_toggle_set_use_underline (AdwToggle *self,
                              gboolean   use_underline)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));

  use_underline = !!use_underline;

  if (self->use_underline == use_underline)
    return;

  self->use_underline = use_underline;

  update_content (self);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_USE_UNDERLINE]);
}

/**
 * adw_toggle_get_icon_name:
 * @self: a toggle
 *
 * Gets the icon name of @self.
 *
 * Returns: (nullable): the toggle icon name
 *
 * Since: 1.7
 */
const char *
adw_toggle_get_icon_name (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), NULL);

  return self->icon_name;
}

/**
 * adw_toggle_set_icon_name:
 * @self: a toggle
 * @icon_name: (nullable): the icon name
 *
 * Sets the icon name of @self to @icon_name.
 *
 * The icon will be displayed alone or next to the label, unless
 * [property@Toggle:child] is set.
 *
 * Since: 1.7
 */
void
adw_toggle_set_icon_name (AdwToggle  *self,
                          const char *icon_name)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));

  if (!g_set_str (&self->icon_name, icon_name))
    return;

  update_content (self);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_ICON_NAME]);
}

/**
 * adw_toggle_get_tooltip:
 * @self: a toggle
 *
 * Gets the tooltip of @self.
 *
 * Returns: the toggle tooltip
 *
 * Since: 1.7
 */
const char *
adw_toggle_get_tooltip (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), NULL);

  return self->tooltip;
}

/**
 * adw_toggle_set_tooltip:
 * @self: a toggle
 * @tooltip: the tooltip
 *
 * Sets the tooltip of @self to @tooltip.
 *
 * @tooltip can be marked up with the Pango text markup language.
 *
 * Since: 1.7
 */
void
adw_toggle_set_tooltip (AdwToggle  *self,
                        const char *tooltip)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));

  if (!g_set_str (&self->tooltip, tooltip))
    return;

  update_content (self);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_TOOLTIP]);
}

/**
 * adw_toggle_get_child:
 * @self: a toggle
 *
 * Gets the child widget of @self.
 *
 * Returns: (transfer none) (nullable): the toggle child
 *
 * Since: 1.7
 */
GtkWidget *
adw_toggle_get_child (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), NULL);

  return self->child;
}

/**
 * adw_toggle_set_child:
 * @self: a toggle
 * @child: (nullable): a child widget
 *
 * Sets the child of @self to @child.
 *
 * When the child is set, icon and label are not displayed.
 *
 * It's recommended to still set the label, as it can still be used by the
 * screen reader.
 *
 * Since: 1.7
 */
void
adw_toggle_set_child (AdwToggle *self,
                      GtkWidget *child)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  if (child == self->child)
    return;

  g_clear_object (&self->child);
  if (child)
    self->child = g_object_ref_sink (child);

  update_content (self);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_CHILD]);
}

/**
 * adw_toggle_get_enabled:
 * @self: a toggle
 *
 * Gets whether @self is enabled.
 *
 * Returns: whether the toggle is enabled
 *
 * Since: 1.7
 */
gboolean
adw_toggle_get_enabled (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), FALSE);

  return self->enabled;
}

/**
 * adw_toggle_set_enabled:
 * @self: a toggle
 * @enabled: whether the toggle should be enbled
 *
 * Sets whether @self is enabled.
 *
 * Since: 1.7
 */
void
adw_toggle_set_enabled (AdwToggle *self,
                        gboolean   enabled)
{
  g_return_if_fail (ADW_IS_TOGGLE (self));

  enabled = !!enabled;

  if (self->enabled == enabled)
    return;

  self->enabled = enabled;

  if (!enabled && self->group && self->index == self->group->active)
    set_active_toggle (self->group, NULL);

  if (self->button)
    gtk_widget_set_sensitive (self->button, enabled);

  g_object_notify_by_pspec (G_OBJECT (self), toggle_props[TOGGLE_PROP_ENABLED]);
}

/**
 * adw_toggle_get_index:
 * @self: a toggle
 *
 * Gets the index of @self within its toggle group.
 *
 * Returns: the index, or `GTK_INVALID_LIST_POSITION` if it's not in a group
 *
 * Since: 1.7
 */
guint
adw_toggle_get_index (AdwToggle *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE (self), 0);

  return self->index;
}

/**
 * adw_toggle_group_new:
 *
 * Creates a new `AdwToggleGroup`.
 *
 * Returns: the newly created `AdwToggleGroup`
 *
 * Since: 1.7
 */
GtkWidget *
adw_toggle_group_new (void)
{
  return g_object_new (ADW_TYPE_TOGGLE_GROUP, NULL);
}

/**
 * adw_toggle_group_add:
 * @self: a toggle group
 * @toggle: (transfer full): the toggle to add
 *
 * Adds a toggle to @self.
 *
 * Since: 1.7
 */
void
adw_toggle_group_add (AdwToggleGroup *self,
                      AdwToggle      *toggle)
{
  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (ADW_IS_TOGGLE (toggle));
  g_return_if_fail (toggle->group == NULL);

  add_toggle (self, toggle);
}

/**
 * adw_toggle_group_remove:
 * @self: a toggle group
 * @toggle: a toggle to remove
 *
 * Removes @toggle from @self.
 *
 * Since: 1.7
 */
void
adw_toggle_group_remove (AdwToggleGroup *self,
                         AdwToggle      *toggle)
{
  GtkWidget *next_separator;
  guint i;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (ADW_IS_TOGGLE (toggle));
  g_return_if_fail (toggle->group == self);

  next_separator = gtk_widget_get_next_sibling (toggle->button);

  if (self->size_group)
    gtk_size_group_remove_widget (self->size_group, toggle->button);

  g_object_freeze_notify (G_OBJECT (self));

  if (self->active == toggle->index)
    set_active_toggle (self, NULL);

  g_object_ref (toggle);

  g_clear_pointer (&toggle->button, gtk_widget_unparent);
  g_clear_pointer (&toggle->separator, gtk_widget_unparent);

  g_ptr_array_remove (self->toggles, toggle);
  if (toggle->name)
    g_hash_table_remove (self->name_to_toggle, toggle->name);

  for (i = toggle->index; i < self->toggles->len; i++) {
    AdwToggle *t = g_ptr_array_index (self->toggles, i);

    t->index--;
  }

  if (self->active > toggle->index && self->active != GTK_INVALID_LIST_POSITION) {
    self->active--;

    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVE]);
  }

  if (self->toggles_model)
    g_list_model_items_changed (G_LIST_MODEL (self->toggles_model), toggle->index, 1, 0);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_TOGGLES]);

  g_object_thaw_notify (G_OBJECT (self));

  toggle->index = GTK_INVALID_LIST_POSITION;
  toggle->group = NULL;

  if (next_separator != NULL)
    update_separator (self, next_separator);

  g_object_unref (toggle);
}

/**
 * adw_toggle_group_remove_all:
 * @self: a toggle group
 *
 * Removes all toggles from @self.
 *
 * Since: 1.7
 */
void
adw_toggle_group_remove_all (AdwToggleGroup *self)
{
  int i;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));

  if (!self->toggles->len)
    return;

  for (i = 0; i < self->toggles->len; i++) {
    AdwToggle *toggle = g_ptr_array_index (self->toggles, i);

    if (self->size_group)
      gtk_size_group_remove_widget (self->size_group, toggle->button);

    g_clear_pointer (&toggle->button, gtk_widget_unparent);
    g_clear_pointer (&toggle->separator, gtk_widget_unparent);
  }

  g_object_freeze_notify (G_OBJECT (self));

  set_active_toggle (self, NULL);

  g_ptr_array_remove_range (self->toggles, 0, self->toggles->len);
  g_hash_table_remove_all (self->name_to_toggle);

  if (self->toggles_model) {
    g_list_model_items_changed (G_LIST_MODEL (self->toggles_model),
                                0, self->toggles->len, 0);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_TOGGLES]);

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_toggle_group_get_toggle:
 * @self: a toggle group
 * @index: toggle's index
 *
 * Gets the toggle with @index from @self.
 *
 * Returns: (transfer none) (nullable): the toggle
 *
 * Since: 1.7
 */
AdwToggle *
adw_toggle_group_get_toggle (AdwToggleGroup *self,
                             guint           index)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), NULL);

  if (index >= self->toggles->len)
    return NULL;

  return g_ptr_array_index (self->toggles, index);
}

/**
 * adw_toggle_group_get_toggle_by_name:
 * @self: a toggle group
 * @name: toggle name
 *
 * Gets the toggle with the name @name from @self.
 *
 * Returns: (transfer none) (nullable): the toggle
 *
 * Since: 1.7
 */
AdwToggle *
adw_toggle_group_get_toggle_by_name (AdwToggleGroup *self,
                                     const char     *name)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return g_hash_table_lookup (self->name_to_toggle, name);
}

/**
 * adw_toggle_group_get_n_toggles:
 * @self: a toggle group
 *
 * Gets the number of toggles within @self.
 *
 * Returns: the number of toggles
 *
 * Since: 1.7
 */
guint
adw_toggle_group_get_n_toggles (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), 0);

  return self->toggles->len;
}

/**
 * adw_toggle_group_get_active:
 * @self: a toggle group
 *
 * Gets the index of the active toggle in @self.
 *
 * Returns `GTK_INVALID_LIST_POSITION` if no toggle is active.
 *
 * Returns: the active toggle index
 *
 * Since: 1.7
 */
guint
adw_toggle_group_get_active (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), 0);

  return self->active;
}

/**
 * adw_toggle_group_set_active:
 * @self: a toggle group
 * @active: toggle index
 *
 * Sets the active toggle for @self.
 *
 * If the index is larger than the number of toggles in @self, unsets the
 * current active toggle.
 *
 * Since: 1.7
 */
void
adw_toggle_group_set_active (AdwToggleGroup *self,
                             guint           active)
{
  AdwToggle *toggle;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));

  toggle = adw_toggle_group_get_toggle (self, active);

  set_active_toggle (self, toggle);
}

/**
 * adw_toggle_group_get_active_name:
 * @self: a toggle group
 *
 * Gets the name of the active toggle in @self.
 *
 * Can be `NULL` if the currently active toggle doesn't have a name.
 *
 * See [property@Toggle:name].
 *
 * Returns: (nullable): the active toggle name
 *
 * Since: 1.7
 */
const char *
adw_toggle_group_get_active_name (AdwToggleGroup *self)
{
  AdwToggle *toggle;
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), NULL);

  if (self->active == GTK_INVALID_LIST_POSITION)
    return NULL;

  toggle = adw_toggle_group_get_toggle (self, self->active);

  g_assert (toggle != NULL);

  return toggle->name;
}

/**
 * adw_toggle_group_set_active_name:
 * @self: a toggle group
 * @name: (nullable): toggle name
 *
 * Sets the active toggle for @self.
 *
 * The name can be set via [property@Toggle:name].
 *
 * If @name is `NULL`, unset the current active toggle instead.
 *
 * Since: 1.7
 */
void
adw_toggle_group_set_active_name (AdwToggleGroup *self,
                                  const char     *name)
{
  AdwToggle *toggle;

  if (!name) {
    set_active_toggle (self, NULL);
    return;
  }

  toggle = adw_toggle_group_get_toggle_by_name (self, name);

  if (!toggle) {
    g_critical ("Trying to set an active toggle with the name '%s' in an "
                "AdwToggleGroup, but such a toggle does not exist", name);
    return;
  }

  set_active_toggle (self, toggle);
}

/**
 * adw_toggle_group_get_homogeneous:
 * @self: a toggle group
 *
 * Gets whether all toggles take the same size.
 *
 * Returns: whether all toggles take the same size
 *
 * Since: 1.7
 */
gboolean
adw_toggle_group_get_homogeneous (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), FALSE);

  return self->size_group != NULL;
}

/**
 * adw_toggle_group_set_homogeneous:
 * @self: a toggle group
 * @homogeneous: whether all toggles should take the same size
 *
 * Sets whether all toggles take the same size.
 *
 * Since: 1.7
 */
void
adw_toggle_group_set_homogeneous (AdwToggleGroup *self,
                                  gboolean        homogeneous)
{
  int i;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));

  homogeneous = !!homogeneous;

  if (homogeneous == adw_toggle_group_get_homogeneous (self))
    return;

  if (homogeneous) {
    self->size_group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);

    for (i = 0; i < self->toggles->len; i++) {
      AdwToggle *toggle = g_ptr_array_index (self->toggles, i);

      gtk_size_group_add_widget (self->size_group, toggle->button);
    }
  } else {
    for (i = 0; i < self->toggles->len; i++) {
      AdwToggle *toggle = g_ptr_array_index (self->toggles, i);

      gtk_size_group_remove_widget (self->size_group, toggle->button);
    }

    g_clear_object (&self->size_group);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HOMOGENEOUS]);
}

/**
 * adw_toggle_group_get_can_shrink:
 * @self: a toggle group
 *
 * Gets whether the toggles can be smaller than the natural size of their
 * contents.
 *
 * Returns: whether the toggles can shrink
 *
 * Since: 1.7
 */
gboolean
adw_toggle_group_get_can_shrink (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), FALSE);

  return self->can_shrink;
}

/**
 * adw_toggle_group_set_can_shrink:
 * @self: a toggle group
 * @can_shrink: whether the toggles can shrink
 *
 * Sets whether the toggles can be smaller than the natural size of their
 * contents.
 *
 * If @can_shrink is `TRUE`, the toggle labels will ellipsize.
 *
 * See [property@Gtk.Button:can-shrink].
 *
 * Since: 1.7
 */
void
adw_toggle_group_set_can_shrink (AdwToggleGroup *self,
                                 gboolean        can_shrink)
{
  int i;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));

  can_shrink = !!can_shrink;

  if (self->can_shrink == can_shrink)
    return;

  self->can_shrink = can_shrink;

  for (i = 0; i < self->toggles->len; i++) {
    AdwToggle *toggle = g_ptr_array_index (self->toggles, i);

    gtk_button_set_can_shrink (GTK_BUTTON (toggle->button), can_shrink);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAN_SHRINK]);
}

/**
 * adw_toggle_group_get_toggles:
 * @self: a toggle group
 *
 * Returns a [iface@Gio.ListModel] that contains the toggles of the group.
 *
 * This can be used to keep an up-to-date view. The model also implements
 * [iface@Gtk.SelectionModel] and can be used to track and change the active
 * toggle.
 *
 * Returns: (transfer full): a `GtkSelectionModel` for the group's toggles
 */
GtkSelectionModel *
adw_toggle_group_get_toggles (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), NULL);

  if (self->toggles_model)
    return g_object_ref (self->toggles_model);

  g_set_weak_pointer (&self->toggles_model,
                      GTK_SELECTION_MODEL (adw_toggle_group_toggles_new (self)));

  return self->toggles_model;
}
