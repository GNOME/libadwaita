/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-enum-value-object.h"

/**
 * SECTION:hdy-enum-value-object
 * @short_description: An object representing a #GEnumValue.
 * @Title: HdyEnumValueObject
 *
 * The #HdyEnumValueObject object represents a #GEnumValue, allowing it to be
 * used with #GListModel.
 *
 * Since: 0.0.6
 */

struct _HdyEnumValueObject
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

G_DEFINE_TYPE (HdyEnumValueObject, hdy_enum_value_object, G_TYPE_OBJECT)

static void
hdy_enum_value_object_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  HdyEnumValueObject *self = HDY_ENUM_VALUE_OBJECT (object);

  switch (prop_id) {
  case PROP_VALUE:
    g_value_set_int (value, hdy_enum_value_object_get_value (self));
    break;
  case PROP_NAME:
    g_value_set_string (value, hdy_enum_value_object_get_name (self));
    break;
  case PROP_NICK:
    g_value_set_string (value, hdy_enum_value_object_get_nick (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_enum_value_object_class_init (HdyEnumValueObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = hdy_enum_value_object_get_property;

  props[PROP_VALUE] =
    g_param_spec_int ("value",
                      _("Value"),
                      _("The enum object value"),
                      G_MININT, G_MAXINT, 0,
                      G_PARAM_READABLE);

  props[PROP_NAME] =
    g_param_spec_string ("name",
                         _("Name"),
                         _("The enum object name"),
                         NULL,
                         G_PARAM_READABLE);

  props[PROP_NICK] =
    g_param_spec_string ("nick",
                         _("Nick"),
                         _("The enum object nick"),
                         NULL,
                         G_PARAM_READABLE);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
hdy_enum_value_object_init (HdyEnumValueObject *self)
{
}

HdyEnumValueObject *
hdy_enum_value_object_new (GEnumValue *enum_value)
{
  HdyEnumValueObject *self = g_object_new (HDY_TYPE_ENUM_VALUE_OBJECT, NULL);

  self->enum_value = *enum_value;

  return self;
}

gint
hdy_enum_value_object_get_value (HdyEnumValueObject *self)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (self), 0);

  return self->enum_value.value;
}

const gchar *
hdy_enum_value_object_get_name (HdyEnumValueObject *self)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (self), NULL);

  return self->enum_value.value_name;
}

const gchar *
hdy_enum_value_object_get_nick (HdyEnumValueObject *self)
{
  g_return_val_if_fail (HDY_IS_ENUM_VALUE_OBJECT (self), NULL);

  return self->enum_value.value_nick;
}
