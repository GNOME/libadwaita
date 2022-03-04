#include "config.h"

#include "adw-tag-private.h"

/**
 * AdwTag:
 *
 * A tag inside a tagged widget.
 *
 * Since: 1.2
 */
struct _AdwTag
{
  GObject parent_instance;

  char *action_name;
  GVariant *action_target;

  AdwTagIconType icon_type;
  GIcon *gicon;
  GdkPaintable *paintable;

  char *label;
  gboolean show_close;
};

/* {{{ AdwTag */
enum
{
  PROP_TAG_LABEL = 1,
  PROP_TAG_GICON,
  PROP_TAG_PAINTABLE,
  PROP_TAG_SHOW_CLOSE,
  PROP_TAG_ACTION_NAME,
  PROP_TAG_ACTION_TARGET,
  PROP_TAG_HAS_ICON,

  N_TAG_PROPS
};

static GParamSpec *tag_props[N_TAG_PROPS];

G_DEFINE_TYPE (AdwTag, adw_tag, G_TYPE_OBJECT)

static void
adw_tag_dispose (GObject *gobject)
{
  AdwTag *self = ADW_TAG (gobject);

  g_clear_pointer (&self->action_name, g_free);
  g_clear_pointer (&self->action_target, g_variant_unref);

  g_clear_pointer (&self->label, g_free);
  g_clear_object (&self->gicon);
  g_clear_object (&self->paintable);

  G_OBJECT_CLASS (adw_tag_parent_class)->dispose (gobject);
}

static void
adw_tag_set_property (GObject      *gobject,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  AdwTag *self = ADW_TAG (gobject);

  switch (prop_id) {
  case PROP_TAG_LABEL:
    adw_tag_set_label (self, g_value_get_string (value));
    break;

  case PROP_TAG_SHOW_CLOSE:
    adw_tag_set_show_close (self, g_value_get_boolean (value));
    break;

  case PROP_TAG_GICON:
    adw_tag_set_gicon (self, g_value_get_object (value));
    break;

  case PROP_TAG_PAINTABLE:
    adw_tag_set_paintable (self, g_value_get_object (value));
    break;

  case PROP_TAG_ACTION_NAME:
    adw_tag_set_action_name (self, g_value_get_string (value));
    break;

  case PROP_TAG_ACTION_TARGET:
    adw_tag_set_action_target_value (self, g_value_get_variant (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tag_get_property (GObject    *gobject,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  AdwTag *self = ADW_TAG (gobject);

  switch (prop_id) {
  case PROP_TAG_LABEL:
    g_value_set_string (value, self->label);
    break;

  case PROP_TAG_SHOW_CLOSE:
    g_value_set_boolean (value, self->show_close);
    break;

  case PROP_TAG_GICON:
    g_value_set_object (value, self->gicon);
    break;

  case PROP_TAG_PAINTABLE:
    g_value_set_object (value, self->paintable);
    break;

  case PROP_TAG_ACTION_NAME:
    g_value_set_string (value, self->action_name);
    break;

  case PROP_TAG_ACTION_TARGET:
    g_value_set_variant (value, self->action_target);
    break;

  case PROP_TAG_HAS_ICON:
    g_value_set_boolean (value, adw_tag_has_icon (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tag_class_init (AdwTagClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = adw_tag_dispose;
  gobject_class->set_property = adw_tag_set_property;
  gobject_class->get_property = adw_tag_get_property;

  /**
   * AdwTag:gicon:
   *
   * The icon of the tag.
   *
   * Setting this property will also set the [property@Tag:has-icon] as a
   * side effect.
   *
   * Since: 1.2
   */
  tag_props[PROP_TAG_GICON] =
    g_param_spec_object ("gicon", NULL, NULL,
                         G_TYPE_ICON,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);
  tag_props[PROP_TAG_PAINTABLE] =
    g_param_spec_object ("paintable", NULL, NULL,
                         GDK_TYPE_PAINTABLE,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTag:label:
   *
   * The user readable name of the tag.
   *
   * Since: 1.2
   */
  tag_props[PROP_TAG_LABEL] =
    g_param_spec_string ("label", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS |
                         G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTag:show-close:
   *
   * Whether the tag should show a close button to remove itself
   * from the entry.
   *
   * Since: 1.2
   */
  tag_props[PROP_TAG_SHOW_CLOSE] =
    g_param_spec_boolean ("show-close", NULL, NULL,
                          TRUE,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTag:action-name:
   *
   * The name of the associated action.
   *
   * It will be activated when clicking the button.
   *
   * See [property@Tag:action-target].
   *
   * Since: 1.2
   */
  tag_props[PROP_TAG_ACTION_NAME] =
    g_param_spec_string ("action-name",
                         "Action Name",
                         "The name of the associated action",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);
  /**
   * AdwTag:action-target:
   *
   * The parameter for action invocations.
   *
   * See [property@Tag:action-name].
   *
   * Since: 1.2
   */
  tag_props[PROP_TAG_ACTION_TARGET] =
    g_param_spec_variant ("action-target",
                          "Action Target Value",
                          "The parameter for action invocations",
                          G_VARIANT_TYPE_ANY,
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwTag:has-icon:
   *
   * Whether the tag has a visible icon.
   *
   * Since: 1.2
   */
  tag_props[PROP_TAG_HAS_ICON] =
    g_param_spec_boolean ("has-icon", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (gobject_class, N_TAG_PROPS, tag_props);
}

static void
adw_tag_init (AdwTag *self)
{
  self->show_close = TRUE;
}

AdwTag *
adw_tag_new (void)
{
  return g_object_new (ADW_TYPE_TAG, NULL);
}

/**
 * adw_tag_get_label:
 * @self: the tag we want to query
 *
 * Retrieves the user readable label of the tag.
 *
 * Returns: (transfer none): the label of the tag
 */
const char *
adw_tag_get_label (AdwTag *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), NULL);

  return self->label;
}

/**
 * adw_tag_set_label:
 * @self: the tag we want to update
 * @label: (not nullable): the label of the tag
 *
 * Sets the user readable label of the tag.
 */
void
adw_tag_set_label (AdwTag     *self,
                   const char *label)
{
  g_return_if_fail (ADW_IS_TAG (self));
  g_return_if_fail (label != NULL);

  if (g_strcmp0 (self->label, label) == 0)
    return;

  g_free (self->label);
  self->label = g_strdup (label);

  g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_LABEL]);
}

/**
 * adw_tag_get_show_close:
 * @self: the tag we want to query
 *
 * Checks whether the tag should show a close button or not.
 *
 * Returns: true if the tag has a visible close button
 */
gboolean
adw_tag_get_show_close (AdwTag *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), FALSE);

  return self->show_close;
}

/**
 * adw_tag_set_show_close:
 * @self: the tag we want to update
 *
 * Sets whether the tag should show a close button or not.
 */
void
adw_tag_set_show_close (AdwTag   *self,
                        gboolean  show_close)
{
  g_return_if_fail (ADW_IS_TAG (self));

  show_close = !!show_close;

  if (self->show_close != show_close) {
    self->show_close = show_close;

    g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_SHOW_CLOSE]);
  }
}

GIcon *
adw_tag_get_gicon (AdwTag *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), NULL);

  return self->gicon;
}

static void
update_icon_type (AdwTag *self)
{
  if (self->paintable != NULL)
    self->icon_type = ADW_TAG_ICON_PAINTABLE;
  else if (self->gicon != NULL)
    self->icon_type = ADW_TAG_ICON_GICON;
  else
    self->icon_type = ADW_TAG_ICON_NONE;
}

void
adw_tag_set_gicon (AdwTag *self,
                   GIcon  *icon)
{
  g_return_if_fail (ADW_IS_TAG (self));
  g_return_if_fail (icon == NULL || G_IS_ICON (icon));

  if (g_set_object (&self->gicon, icon)) {
    update_icon_type (self);
    g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_GICON]);
    g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_HAS_ICON]);
  }
}

GdkPaintable *
adw_tag_get_paintable (AdwTag       *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), NULL);

  return self->paintable;
}

void
adw_tag_set_paintable (AdwTag       *self,
                       GdkPaintable *paintable)
{
  g_return_if_fail (ADW_IS_TAG (self));
  g_return_if_fail (paintable == NULL || GDK_IS_PAINTABLE (paintable));

  if (g_set_object (&self->paintable, paintable)) {
    update_icon_type (self);
    g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_PAINTABLE]);
    g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_HAS_ICON]);
  }
}

gboolean
adw_tag_has_icon (AdwTag *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), FALSE);

  return self->icon_type != ADW_TAG_ICON_NONE;
}

AdwTagIconType
adw_tag_get_icon_type (AdwTag *self)
{
  return self->icon_type;
}

const char *
adw_tag_get_action_name (AdwTag *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), NULL);

  return self->action_name;
}

void
adw_tag_set_action_name (AdwTag     *self,
                         const char *action_name)
{
  g_return_if_fail (ADW_IS_TAG (self));

  if (!g_strcmp0 (self->action_name, action_name))
    return;

  g_clear_pointer (&self->action_name, g_free);
  self->action_name = g_strdup (action_name);

  g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_ACTION_NAME]);
}

GVariant *
adw_tag_get_action_target_value (AdwTag *self)
{
  g_return_val_if_fail (ADW_IS_TAG (self), NULL);

  return self->action_target;
}

void
adw_tag_set_action_target_value (AdwTag   *self,
                                 GVariant *action_target)
{
  g_return_if_fail (ADW_IS_TAG (self));

  if (action_target == self->action_target)
    return;

  if (action_target && self->action_target &&
      g_variant_equal (action_target, self->action_target))
    return;

  g_clear_pointer (&self->action_target, g_variant_unref);
  self->action_target = g_variant_ref_sink (action_target);

  g_object_notify_by_pspec (G_OBJECT (self), tag_props[PROP_TAG_ACTION_TARGET]);
}

void
adw_tag_set_action_target (AdwTag     *self,
                           const char *format_string,
                           ...)
{
  va_list args;

  va_start (args, format_string);
  adw_tag_set_action_target_value (self,
                                   g_variant_new_va (format_string,
                                                     NULL,
                                                     &args));
  va_end (args);
}

void
adw_tag_set_detailed_action_name (AdwTag     *self,
                                  const char *detailed_action_name)
{
  char *name;
  GVariant *target;
  GError *error = NULL;

  g_return_if_fail (ADW_IS_TAG (self));

  if (detailed_action_name == NULL) {
    adw_tag_set_action_name (self, NULL);
    adw_tag_set_action_target_value (self, NULL);
    return;
  }

  if (g_action_parse_detailed_name (detailed_action_name, &name, &target, &error)) {
    adw_tag_set_action_name (self, name);
    adw_tag_set_action_target_value (self, target);
  } else {
    g_critical ("Couldn't parse detailed action name: %s", error->message);
  }

  g_clear_error (&error);
  g_clear_pointer (&target, g_variant_unref);
  g_clear_pointer (&name, g_free);
}

/* }}} */
