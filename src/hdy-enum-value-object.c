/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

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

G_DEFINE_TYPE (HdyEnumValueObject, hdy_enum_value_object, G_TYPE_OBJECT)

HdyEnumValueObject *
hdy_enum_value_object_new (GEnumValue *enum_value)
{
  HdyEnumValueObject *self = g_object_new (HDY_TYPE_ENUM_VALUE_OBJECT, NULL);

  self->enum_value = *enum_value;

  return self;
}

static void
hdy_enum_value_object_class_init (HdyEnumValueObjectClass *klass)
{
}

static void
hdy_enum_value_object_init (HdyEnumValueObject *self)
{
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
