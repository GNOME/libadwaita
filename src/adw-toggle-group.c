/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-toggle-group.h"

#include "adw-gtkbuilder-utils-private.h"

/**
 * AdwToggleDisplayMode:
 * @ADW_TOGGLE_DISPLAY_MODE_LABEL: Only display labels.
 * @ADW_TOGGLE_DISPLAY_MODE_ICON: Only display icons.
 * @ADW_TOGGLE_DISPLAY_MODE_BOTH: Display both labels and icons.
 *
 * Describes what content to show in toggles.
 *
 * Since: 1.4
 */

/**
 * AdwToggleGroup:
 *
 * A group of toggles.
 *
 * <picture>
 *   <source srcset="toggle-group-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toggle-group.png" alt="toggle-group">
 * </picture>
 *
 * A group of toggles that can be displayed according to the
 * [property@ToggleGroup:display-mode] property.
 *
 * If [property@ToggleGroup:display-mode] is set to
 * `ADW_TOGGLE_DISPLAY_MODE_ICON` the toggle's label will be used as the
 * [property@Gtk.Widget:tooltip-text] of the respective widget, if the toggle
 * has `use_underline` set to true, underlines will be removed for the tooltip.
 *
 * ## AdwToggleGroup as GtkBuildable
 *
 * `AdwToggleGroup` supports adding toggles in UI definitions by via the
 * `<toggles>` element that may contain multiple `<toggle>` elements, each
 * respresenting a toggle.
 *
 * Each of the `<toggle>` elements must have the `id` attribute specifying the
 * toggle ID. The contents of the element are used as the response label.
 *
 * Response labels can be translated with the usual `translatable`, `context`
 * and `comments` attributes.
 *
 * The `<toggle>` elements can also have `icon-name` and/or `use-underline`
 * attributes.
 *
 * Example of a `AdwToggleGroup` UI definition:
 *
 * ```c
 *  <object class="AdwToggleGroup" id="toggle_group">
 *    <property name="display-mode">both</property>
 *    <toggles>
 *      <toggle id="first" use-underline="yes" icon-name="view-grid-symbolic" translatable="yes">_First</toggle>
 *      <toggle id="second" icon-name="view-grid-symbolic" translatable="yes">Second</toggle>
 *      <toggle id="third" icon-name="view-grid-symbolic" translatable="yes">Third</toggle>
 *      <toggle id="fourth" icon-name="view-grid-symbolic" translatable="yes">Fourth</toggle>
 *    </toggles>
 *  </object>
 * ```
 *
 * ## CSS nodes
 *
 * `AdwToggleGroup` has a main CSS node with name `togglegroup`.
 *
 * Its toggles have a main CSS node with name `toggle`, and its separators have
 * a main CSS node with name `toggleseparator`.
 *
 * When the separators are hidden they use the `.hidden` style class.
 *
 * ## Accessibility
 *
 * `AdwToggleGroup` uses the `GTK_ACCESSIBLE_ROLE_RADIO_GROUP` role. Its toggles
 * use the `GTK_ACCESSIBLE_ROLE_RADIO` role.
 *
 * Since: 1.4
 */

typedef struct {
  AdwToggleGroup *toggle_group;

  char *id;

  char *label;
  char *icon_name;
  gboolean use_underline;

  GtkWidget *button;
  GtkWidget *separator;
} ToggleInfo;

struct _AdwToggleGroup
{
  GtkWidget parent_instance;

  char *active;
  AdwToggleDisplayMode display_mode;
  GtkToggleButton *group;

  char *delayed_active;

  GHashTable *toggles;

  GtkOrientation orientation;
};

static void adw_toggle_group_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwToggleGroup, adw_toggle_group, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_toggle_group_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_ACTIVE,
  PROP_DISPLAY_MODE,

  /* Overridden properties */
  PROP_ORIENTATION,

  LAST_PROP = PROP_DISPLAY_MODE + 1,
};

static GParamSpec *props[LAST_PROP];

static GtkOrientation
get_orientation (AdwToggleGroup *self)
{
  return self->orientation;
}

static void
set_orientation (AdwToggleGroup *self,
                 GtkOrientation  orientation)
{
  if (self->orientation == orientation)
    return;

  GtkLayoutManager *layout;
  self->orientation = orientation;
  layout = gtk_widget_get_layout_manager (GTK_WIDGET (self));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (layout), orientation);
  g_object_notify (G_OBJECT (self), "orientation");
}

static void
remove_and_free_toggle_info (ToggleInfo *info)
{
  gtk_widget_unparent (info->button);
  gtk_widget_unparent (info->separator);

  g_free (info->id);
  g_free (info->label);
  g_free (info->icon_name);

  g_free (info);
}

static void
toggle_set_content (AdwToggleGroup *self,
                    ToggleInfo     *info)
{
  switch (self->display_mode) {
  case ADW_TOGGLE_DISPLAY_MODE_LABEL:
    gtk_button_set_label (GTK_BUTTON (info->button), info->label);
    gtk_button_set_use_underline (GTK_BUTTON (info->button), info->use_underline);
    gtk_widget_set_tooltip_text (info->button, NULL);
    break;
  case ADW_TOGGLE_DISPLAY_MODE_ICON:
    if (info->icon_name && info->icon_name[0])
      gtk_button_set_icon_name (GTK_BUTTON (info->button), info->icon_name);
    else
      gtk_button_set_icon_name (GTK_BUTTON (info->button), "image-missing");

    if (!info->use_underline) {
      gtk_widget_set_tooltip_text (info->button, info->label);
    } else {
      GString *string = g_string_new (info->label);
      g_string_replace (string, "_", "", 0);
      char *str = g_string_free (string, FALSE);
      gtk_widget_set_tooltip_text (info->button, str);
      g_free (str);
    }
    break;
  case ADW_TOGGLE_DISPLAY_MODE_BOTH:
    AdwButtonContent *content = ADW_BUTTON_CONTENT (adw_button_content_new ());
    adw_button_content_set_label (content, info->label);
    if (info->icon_name && info->icon_name[0])
      adw_button_content_set_icon_name (content, info->icon_name);
    else
      adw_button_content_set_icon_name (content, "image-missing");

    adw_button_content_set_use_underline (content, info->use_underline);
    gtk_button_set_child (GTK_BUTTON (info->button), GTK_WIDGET (content));
    gtk_widget_set_tooltip_text (info->button, NULL);
    break;
  default:
    g_assert_not_reached ();
  }
}

static void
toggle_set_content_wrapper (gpointer        key,
                            ToggleInfo     *info,
                            AdwToggleGroup *toggle_group)
{
  toggle_set_content (toggle_group, info);
}

static gboolean
should_hide_separators (GtkWidget *widget)
{
  GtkStateFlags flags;

  if (widget == NULL)
    return true;

  flags = gtk_widget_get_state_flags (GTK_WIDGET (widget));

  if ((flags & (GTK_STATE_FLAG_PRELIGHT |
                GTK_STATE_FLAG_SELECTED |
                GTK_STATE_FLAG_CHECKED)) != 0)
    return TRUE;

  if ((flags & GTK_STATE_FLAG_FOCUSED) != 0 &&
      (flags & GTK_STATE_FLAG_FOCUS_VISIBLE) != 0)
    return TRUE;

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

  gtk_widget_set_visible (separator, prev_button != NULL && next_button != NULL);

  if (should_hide_separators (prev_button) ||
      should_hide_separators (next_button))
    gtk_widget_add_css_class (separator, "hidden");
  else
    gtk_widget_remove_css_class (separator, "hidden");
}

static gboolean
always_true (gpointer key,
             gpointer value,
             gpointer user_data)
{
  return true;
}

static inline ToggleInfo *
random_toggle (AdwToggleGroup *self)
{
  return g_hash_table_find (self->toggles, (GHRFunc) always_true, NULL);
}

static inline ToggleInfo *
find_toggle (AdwToggleGroup *self,
             const char     *id)
{
  return g_hash_table_lookup (self->toggles, id);
}

static void
toggle_active_cb (ToggleInfo *info)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (info->button)) &&
      g_strcmp0(info->toggle_group->active, info->id) != 0)
    adw_toggle_group_set_active (info->toggle_group, info->id);
}

static void
state_flags_changed_cb (GtkWidget      *button,
                        GtkStateFlags   flags,
                        AdwToggleGroup *self)
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
            const char     *id,
            const char     *label,
            const char     *icon_name)
{
  GtkWidget *button;
  GtkWidget *separator;
  ToggleInfo *info;

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "accessible-role", GTK_ACCESSIBLE_ROLE_RADIO,
                         "css-name", "toggle",
                         NULL);

  gtk_widget_add_css_class (button, "flat");

  gtk_toggle_button_set_group (GTK_TOGGLE_BUTTON (button), self->group);
  if (g_strcmp0 (self->active, "") == 0)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  separator = g_object_new (GTK_TYPE_SEPARATOR,
                            "css-name", "toggleseparator",
                            NULL);

  info = g_new0 (ToggleInfo, 1);
  info->toggle_group = self;
  info->button = button;
  info->id = g_strdup (id);
  info->icon_name = g_strdup (icon_name);
  info->use_underline = FALSE;
  info->label = g_strdup (label);
  info->separator = separator;

  gtk_widget_set_parent (separator, GTK_WIDGET (self));
  gtk_widget_set_parent (button, GTK_WIDGET (self));

  g_signal_connect_object (button, "state-flags-changed", G_CALLBACK (state_flags_changed_cb), self, 0);
  g_signal_connect_swapped (button, "notify::active", G_CALLBACK (toggle_active_cb), info);

  update_separator (self, separator);
  toggle_set_content (self, info);

  g_hash_table_insert (self->toggles, g_strdup (id), info);

  if (g_strcmp0 (self->active, "") == 0)
    adw_toggle_group_set_active (self, id);
}

static void
adw_toggle_group_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (object);

  switch (prop_id) {
  case PROP_ACTIVE:
    g_value_set_string (value, adw_toggle_group_get_active (self));
    break;
  case PROP_DISPLAY_MODE:
    g_value_set_enum (value, adw_toggle_group_get_display_mode (self));
    break;
  case PROP_ORIENTATION:
    g_value_set_enum (value, get_orientation (self));
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
    adw_toggle_group_set_active (self, g_value_get_string (value));
    break;
  case PROP_DISPLAY_MODE:
    adw_toggle_group_set_display_mode (self, g_value_get_enum (value));
    break;
  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toggle_group_finalize (GObject *object)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (object);

  g_free (self->active);
  g_free (self->delayed_active);
  g_hash_table_destroy (self->toggles);

  G_OBJECT_CLASS (adw_toggle_group_parent_class)->finalize (object);
}

static void
adw_toggle_group_init (AdwToggleGroup *self)
{
  self->active = g_strdup ("");
  self->group = GTK_TOGGLE_BUTTON (gtk_toggle_button_new ());
  self->toggles = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                         (GDestroyNotify) remove_and_free_toggle_info);
}

static void
adw_toggle_group_class_init (AdwToggleGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = adw_toggle_group_get_property;
  object_class->set_property = adw_toggle_group_set_property;
  object_class->finalize = adw_toggle_group_finalize;

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  /**
   * AdwToggleGroup:active: (attributes org.gtk.Property.get=adw_toggle_group_get_active org.gtk.Property.set=adw_toggle_group_set_active)
   *
   * The active toggle.
   *
   * Since: 1.4
   */
  props[PROP_ACTIVE] =
    g_param_spec_string ("active", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToggleGroup:display-mode: (attributes org.gtk.Property.get=adw_toggle_group_get_display_mode org.gtk.Property.set=adw_toggle_group_set_display_mode)
   *
   * The display mode.
   *
   * Since: 1.4
   */
  props[PROP_DISPLAY_MODE] =
    g_param_spec_enum ("display-mode", NULL, NULL,
                       ADW_TYPE_TOGGLE_DISPLAY_MODE,
                       ADW_TOGGLE_DISPLAY_MODE_LABEL,
                       G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BOX_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "togglegroup");

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_RADIO_GROUP);
}

typedef struct {
  GObject *object;
  GtkBuilder *builder;
  GSList *toggles;
} ToggleParserData;

typedef struct {
  char *id;

  GString *label;
  char *context;
  gboolean translatable;

  char *icon_name;
  gboolean use_underline;

  int line;
  int col;
} ToggleData;

static void
toggle_data_free (gpointer data)
{
  ToggleData *response = data;

  g_string_free (response->label, TRUE);
  g_free (response->id);
  g_free (response->context);
  g_free (response->icon_name);

  g_free (response);
}

static void
toggle_start_element (GtkBuildableParseContext  *context,
                      const char                *element_name,
                      const char               **names,
                      const char               **values,
                      gpointer                   user_data,
                      GError                   **error)
{
  ToggleParserData *data = user_data;

  if (strcmp (element_name, "toggle") == 0) {
    const char *id;
    const char *msg_context = NULL;
    gboolean translatable = FALSE;
    gboolean use_underline = FALSE;
    const char *icon_name = NULL;
    ToggleData *toggle;

    if (!_gtk_builder_check_parent (data->builder, context, "toggles", error))
      return;

    if (!g_markup_collect_attributes (element_name, names, values, error,
                                      G_MARKUP_COLLECT_STRING, "id", &id,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "icon-name", &icon_name,
                                      G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "translatable", &translatable,
                                      G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "use-underline", &use_underline,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "comments", NULL,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "context", &msg_context,
                                      G_MARKUP_COLLECT_INVALID)) {
      _gtk_builder_prefix_error (data->builder, context, error);
      return;
    }

    toggle = g_new (ToggleData, 1);
    toggle->id = g_strdup (id);
    toggle->context = g_strdup (msg_context);
    toggle->translatable = translatable;
    toggle->label = g_string_new ("");
    toggle->icon_name = g_strdup (icon_name);
    toggle->use_underline = use_underline;

    gtk_buildable_parse_context_get_position (context, &toggle->line, &toggle->col);
    data->toggles = g_slist_prepend (data->toggles, toggle);
  } else if (strcmp (element_name, "toggles") == 0) {
    if (!_gtk_builder_check_parent (data->builder, context, "object", error))
      return;

    if (!g_markup_collect_attributes (element_name, names, values, error,
                                      G_MARKUP_COLLECT_INVALID, NULL, NULL,
                                      G_MARKUP_COLLECT_INVALID))
      _gtk_builder_prefix_error (data->builder, context, error);
  } else {
    _gtk_builder_error_unhandled_tag (data->builder, context,
                                      "AdwToggleGroup", element_name,
                                      error);
  }
}

static void
toggle_text (GtkBuildableParseContext  *context,
             const char                *text,
             gsize                      text_len,
             gpointer                   user_data,
             GError                   **error)
{
  ToggleParserData *data = user_data;

  if (strcmp (gtk_buildable_parse_context_get_element (context), "toggle") == 0) {
    ToggleData *toggle = data->toggles->data;

    g_string_append_len (toggle->label, text, text_len);
  }
}

static const GtkBuildableParser toggle_parser = {
  toggle_start_element,
  NULL,
  toggle_text,
  NULL
};

static gboolean
adw_toggle_group_buildable_custom_tag_start (GtkBuildable       *buildable,
                                             GtkBuilder         *builder,
                                             GObject            *child,
                                             const char         *tagname,
                                             GtkBuildableParser *parser,
                                             gpointer           *parser_data)
{
  ToggleParserData *data;

  if (child)
    return FALSE;

  if (strcmp (tagname, "toggles") == 0) {
    data = g_new0 (ToggleParserData, 1);
    data->toggles = NULL;
    data->object = G_OBJECT (buildable);
    data->builder = builder;

    *parser = toggle_parser;
    *parser_data = data;

    return TRUE;
  }

  return parent_buildable_iface->custom_tag_start (buildable, builder, child,
                                                   tagname, parser, parser_data);
}

static void
adw_toggle_group_buildable_set_buildable_property (GtkBuildable *buildable,
                                                   GtkBuilder   *builder,
                                                   const char   *name,
                                                   const GValue *value)
{
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (buildable);

  if (g_strcmp0 (name, "active") == 0)
    self->delayed_active = g_value_dup_string (value);
  else
    g_object_set_property (G_OBJECT (buildable), name, value);
}

static void
adw_toggle_group_buildable_custom_finished (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const char   *tagname,
                                            gpointer      user_data)
{
  GSList *l;
  ToggleParserData *data;
  AdwToggleGroup *self = ADW_TOGGLE_GROUP (buildable);

  if (strcmp (tagname, "toggles") != 0) {
    parent_buildable_iface->custom_finished (buildable, builder, child,
                                             tagname, user_data);
    return;
  }

  data = (ToggleParserData*)user_data;
  data->toggles = g_slist_reverse (data->toggles);

  for (l = data->toggles; l; l = l->next) {
    ToggleData *toggle = l->data;
    const char *label;

    if (toggle->translatable && toggle->label->len)
      label = _gtk_builder_parser_translate (gtk_builder_get_translation_domain (builder),
                                             toggle->context,
                                             toggle->label->str);
    else
      label = toggle->label->str;

    adw_toggle_group_add_toggle (ADW_TOGGLE_GROUP (data->object),
                                 toggle->id, label, toggle->icon_name);
    if (toggle->use_underline)
      adw_toggle_group_set_use_underline (ADW_TOGGLE_GROUP (data->object),
                                          toggle->id,
                                          toggle->use_underline);
  }

  /* NOTE We set the active toggle *after* the toggles have been loaded */
  if (self->delayed_active)
    adw_toggle_group_set_active (self, self->delayed_active);

  g_slist_free_full (data->toggles, toggle_data_free);
  g_free (data);
}

static void
adw_toggle_group_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  /* TODO Will finish once the api is clear */
  iface->custom_tag_start = adw_toggle_group_buildable_custom_tag_start;
  iface->custom_finished = adw_toggle_group_buildable_custom_finished;
  /* FIXME if one uses bindings in the <property> tag it will error out
   * regardless, even if the property is set after the <toggles> section. */
  /* This is done to delay the "active" property until all toggles are set.
   * Please review. */
  iface->set_buildable_property = adw_toggle_group_buildable_set_buildable_property;
}

/**
 * adw_toggle_group_new:
 *
 * Creates a new `AdwToggleGroup`.
 *
 * Returns: the newly created `AdwToggleGroup`
 *
 * Since: 1.4
 */
GtkWidget *
adw_toggle_group_new (void)
{
  return g_object_new (ADW_TYPE_TOGGLE_GROUP, NULL);
}

/**
 * adw_toggle_group_add_toggle:
 * @self: a toggle group
 * @toggle: ID for the new toggle
 * @label: the toggle label
 * @icon_name: (nullable): the toggle icon name
 *
 * Adds a toggle to @self with a given identifier @toggle.
 *
 * Since: 1.4
 */
void
adw_toggle_group_add_toggle (AdwToggleGroup *self,
                             const char     *toggle,
                             const char     *label,
                             const char     *icon_name)
{
  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (toggle != NULL);
  /* NOTE Since we use labels as tooltip-text we expect one */
  g_return_if_fail (label != NULL);

  if (find_toggle (self, toggle)) {
    g_critical ("Trying to add a toggle with id '%s' to a "
                "AdwToggleGroup, but such a toggle already exists", toggle);
    return;
  }

  add_toggle (self, toggle, label, icon_name);
}

/**
 * adw_toggle_group_remove_toggle:
 * @self: a toggle group
 * @toggle: toggle ID
 *
 * Removes a toggle with ID @toggle from @self.
 *
 * Since: 1.4
 */
void
adw_toggle_group_remove_toggle (AdwToggleGroup *self,
                                const char     *toggle)
{
  GtkWidget *next_separator;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (toggle != NULL);

  ToggleInfo *info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to remove a toggle with id '%s' from a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return;
  }

  next_separator = gtk_widget_get_next_sibling (info->button);

  g_hash_table_remove (self->toggles, toggle);

  /* NOTE If we remove the active one we get a random toggle and set it as
   * active, if there is no such toggle we set active="". */
  if (g_strcmp0 (toggle, self->active) == 0) {
    ToggleInfo *random = random_toggle (self);
    if (random != NULL) {
      adw_toggle_group_set_active (self, random->id);
    } else {
      g_clear_pointer (&self->active, g_free);
      self->active = g_strdup ("");
      g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVE]);
    }
  }

  if (next_separator != NULL)
    update_separator (self, next_separator);
}

/**
 * adw_toggle_group_get_active: (attributes org.gtk.Method.get_property=active)
 * @self: a toggle group
 *
 * Gets the ID of the active toggle in @self.
 *
 * Returns the empty string if the group is empty.
 *
 * Returns: the active toggle ID
 *
 * Since: 1.4
 */
const char *
adw_toggle_group_get_active (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), NULL);

  return self->active;
}

/**
 * adw_toggle_group_set_active: (attributes org.gtk.Method.set_property=active)
 * @self: a toggle group
 * @toggle: toggle ID
 *
 * Sets the active toggle for @self.
 *
 * @active must be a valid toggle ID.
 *
 * Since: 1.4
 */
void
adw_toggle_group_set_active (AdwToggleGroup *self,
                             const char     *toggle)
{
  ToggleInfo *info;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (toggle != NULL);

  if (g_strcmp0 (toggle, self->active) == 0)
    return;

  info = find_toggle (self, toggle);

  if (!info) {
    g_critical ("Trying to set an active toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exists", toggle);
    return;
  }

  g_clear_pointer (&self->active, g_free);
  self->active = g_strdup (toggle);

  /* NOTE we set the active button after self->active to avoid calling this
   * function twice, see toggle_active_cb () */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (info->button), TRUE);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTIVE]);
}

/**
 * adw_toggle_group_get_display_mode: (attributes org.gtk.Method.get_property=display-mode)
 * @self: a toggle group
 *
 * Gets the display mode of @self.
 *
 * Returns: the display mode
 *
 * Since: 1.4
 */
AdwToggleDisplayMode
adw_toggle_group_get_display_mode (AdwToggleGroup *self)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), ADW_TOGGLE_DISPLAY_MODE_LABEL);

  return self->display_mode;
}

/**
 * adw_toggle_group_set_display_mode: (attributes org.gtk.Method.set_property=display-mode)
 * @self: a toggle group
 * @display_mode: the display mode
 *
 * Sets the display mode of @self.
 *
 * Since: 1.4
 */
void
adw_toggle_group_set_display_mode (AdwToggleGroup      *self,
                                   AdwToggleDisplayMode display_mode)
{
  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (display_mode <= ADW_TOGGLE_DISPLAY_MODE_BOTH);

  if (self->display_mode == display_mode)
    return;

  self->display_mode = display_mode;

  g_hash_table_foreach (self->toggles, (GHFunc) toggle_set_content_wrapper, self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DISPLAY_MODE]);
}

/**
 * adw_toggle_group_get_label:
 * @self: a toggle group
 * @toggle: toggle ID
 *
 * Gets the label of a toggle with ID @toggle.
 *
 * Returns: the toggle label.
 *
 * Since: 1.4
 */
const char *
adw_toggle_group_get_label (AdwToggleGroup *self,
                            const char     *toggle)
{
  ToggleInfo *info;

  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), FALSE);
  g_return_val_if_fail (toggle != NULL, NULL);

  info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to get the label of a toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return NULL;
  }

  return info->label;
}

/**
 * adw_toggle_group_set_label:
 * @self: a toggle group
 * @toggle: toggle ID
 * @label: a label
 *
 * Sets the label of the toggle with ID @toggle.
 *
 * Since: 1.4
 */
void
adw_toggle_group_set_label (AdwToggleGroup *self,
                            const char     *toggle,
                            const char     *label)
{
  ToggleInfo *info;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (toggle != NULL);
  g_return_if_fail (label != NULL);

  info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to get the label of a toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return;
  }

  if (g_strcmp0 (info->label, label) == 0)
    return;

  g_clear_pointer (&info->label, g_free);
  info->label = g_strdup (label);

  toggle_set_content (self, info);
}

/**
 * adw_toggle_group_get_icon_name:
 * @self: a toggle group
 * @toggle: toggle ID
 *
 * Gets the icon name of a toggle with ID @toggle.
 *
 * Returns: the toggle icon name.
 *
 * Since: 1.4
 */
const char *
adw_toggle_group_get_icon_name (AdwToggleGroup *self,
                                const char     *toggle)
{
  ToggleInfo *info;

  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), FALSE);
  g_return_val_if_fail (toggle != NULL, NULL);

  info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to get the icon name of a toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return NULL;
  }

  return info->icon_name;
}

/**
 * adw_toggle_group_set_icon_name:
 * @self: a toggle group
 * @toggle: toggle ID
 * @icon_name: (nullable): a label
 *
 * Sets the icon name of the toggle with ID @toggle.
 *
 * Since: 1.4
 */
void
adw_toggle_group_set_icon_name (AdwToggleGroup *self,
                                const char     *toggle,
                                const char     *icon_name)
{
  ToggleInfo *info;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (toggle != NULL);

  info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to get the icon name of a toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return;
  }

  if (g_strcmp0 (info->icon_name, icon_name) == 0)
    return;

  g_clear_pointer (&info->icon_name, g_free);
  info->icon_name = g_strdup (icon_name);

  toggle_set_content (self, info);
}

/**
 * adw_toggle_group_get_use_underline:
 * @self: a toggle group
 * @toggle: toggle ID
 *
 * Gets whether the toggle uses underlines.
 *
 * Returns: whether the toggle uses underlines
 *
 * Since: 1.4
 */
gboolean
adw_toggle_group_get_use_underline (AdwToggleGroup *self,
                                    const char     *toggle)
{
  ToggleInfo *info;

  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), FALSE);
  g_return_val_if_fail (toggle != NULL, FALSE);

  info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to get use_underline for a toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return FALSE;
  }

  return info->use_underline;
}

/**
 * adw_toggle_group_set_use_underline:
 * @self: a toggle group
 * @toggle: toggle ID
 * @use_underline: whether an underline in the text indicates a mnemonic
 *
 * Sets whether the toggle with ID @toggle uses underlines.
 *
 * Since: 1.4
 */
void
adw_toggle_group_set_use_underline (AdwToggleGroup *self,
                                    const char     *toggle,
                                    gboolean        use_underline)
{
  ToggleInfo *info;

  g_return_if_fail (ADW_IS_TOGGLE_GROUP (self));
  g_return_if_fail (toggle != NULL);

  info = find_toggle (self, toggle);
  if (!info) {
    g_critical ("Trying to set use_underline to a toggle with id '%s' in a "
                "AdwToggleGroup, but such a toggle does not exist", toggle);
    return;
  }

  if (info->use_underline != use_underline) {
    info->use_underline = use_underline;
    toggle_set_content (self, info);
  }
}

/**
 * adw_toggle_group_has_toggle:
 * @self: a toggle group
 * @toggle: toggle ID
 *
 * Gets whether @self has a toggle with the ID @toggle.
 *
 * Returns: whether @self has a toggle with the ID @toggle.
 *
 * Since: 1.4
 */
gboolean
adw_toggle_group_has_toggle (AdwToggleGroup *self,
                             const char     *toggle)
{
  g_return_val_if_fail (ADW_IS_TOGGLE_GROUP (self), FALSE);
  g_return_val_if_fail (toggle != NULL, FALSE);

  return g_hash_table_contains (self->toggles, toggle);
}
