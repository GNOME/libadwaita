/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-enum-value-object-private.h"

/**
 * AdwEnumValueObject:
 *
 * `AdwEnumValueObject` is the type of items in a [class@Adw.EnumListModel].
 *
 * Since: 1.0
 */

struct _AdwEnumValueObject
{
  GObject parent_instance;

  GEnumValue enum_value;
};

enum {
  PROP_0,
  PROP_VALUE,
  PROP_NAME,
  PROP_NICK,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

G_DEFINE_TYPE (AdwEnumValueObject, adw_enum_value_object, G_TYPE_OBJECT)

static void
adw_enum_value_object_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwEnumValueObject *self = ADW_ENUM_VALUE_OBJECT (object);

  switch (prop_id) {
  case PROP_VALUE:
    g_value_set_int (value, adw_enum_value_object_get_value (self));
    break;
  case PROP_NAME:
    g_value_set_string (value, adw_enum_value_object_get_name (self));
    break;
  case PROP_NICK:
    g_value_set_string (value, adw_enum_value_object_get_nick (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_enum_value_object_class_init (AdwEnumValueObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = adw_enum_value_object_get_property;

  /**
   * AdwEnumValueObject:value: (attributes org.gtk.Property.get=adw_enum_value_object_get_value)
   *
   * The enum value.
   *
   * Since: 1.0
   */
  props[PROP_VALUE] =
    g_param_spec_int ("value",
                      "Value",
                      "The enum value",
                      G_MININT, G_MAXINT, 0,
                      G_PARAM_READABLE);

  /**
   * AdwEnumValueObject:name: (attributes org.gtk.Property.get=adw_enum_value_object_get_name)
   *
   * The enum value name.
   *
   * Since: 1.0
   */
  props[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "The enum value name",
                         NULL,
                         G_PARAM_READABLE);

  /**
   * AdwEnumValueObject:nick: (attributes org.gtk.Property.get=adw_enum_value_object_get_nick)
   *
   * The enum value nick.
   *
   * Since: 1.0
   */
  props[PROP_NICK] =
    g_param_spec_string ("nick",
                         "Nick",
                         "The enum value nick",
                         NULL,
                         G_PARAM_READABLE);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_enum_value_object_init (AdwEnumValueObject *self)
{
}

AdwEnumValueObject *
adw_enum_value_object_new (GEnumValue *enum_value)
{
  AdwEnumValueObject *self = g_object_new (ADW_TYPE_ENUM_VALUE_OBJECT, NULL);

  self->enum_value = *enum_value;

  return self;
}

/**
 * adw_enum_value_object_get_value: (attributes org.gtk.Method.get_property=value)
 *
 * Gets the enum value.
 *
 * Returns: the enum value
 *
 * Since: 1.0
 */
int
adw_enum_value_object_get_value (AdwEnumValueObject *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_VALUE_OBJECT (self), 0);

  return self->enum_value.value;
}

/**
 * adw_enum_value_object_get_name: (attributes org.gtk.Method.get_property=name)
 *
 * Gets the enum value name.
 *
 * Returns: the enum value name
 *
 * Since: 1.0
 */
const char *
adw_enum_value_object_get_name (AdwEnumValueObject *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_VALUE_OBJECT (self), NULL);

  return self->enum_value.value_name;
}

/**
 * adw_enum_value_object_get_nick: (attributes org.gtk.Method.get_property=nick)
 *
 * Gets the enum value nick.
 *
 * Returns: the enum value nick
 *
 * Since: 1.0
 */
const char *
adw_enum_value_object_get_nick (AdwEnumValueObject *self)
{
  g_return_val_if_fail (ADW_IS_ENUM_VALUE_OBJECT (self), NULL);

  return self->enum_value.value_nick;
}
