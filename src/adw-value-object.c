/*
 * Copyright (C) 2019 Red Hat Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <gobject/gvaluecollector.h>
#include "adw-value-object.h"

/**
 * SECTION:adw-value-object
 * @short_description: An object representing a #GValue.
 * @Title: AdwValueObject
 *
 * The #AdwValueObject object represents a #GValue, allowing it to be
 * used with #GListModel.
 *
 * Since: 1.0
 */

struct _AdwValueObject
{
  GObject parent_instance;

  GValue value;
};

G_DEFINE_TYPE (AdwValueObject, adw_value_object, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_VALUE,
  N_PROPS
};

static GParamSpec *props [N_PROPS];

/**
 * adw_value_object_new:
 * @value: the #GValue to store
 *
 * Create a new #AdwValueObject.
 *
 * Returns: a new #AdwValueObject
 *
 * Since: 1.0
 */
AdwValueObject *
adw_value_object_new (const GValue *value)
{
  return g_object_new (ADW_TYPE_VALUE_OBJECT,
                       "value", value,
                       NULL);
}

/**
 * adw_value_object_new_collect: (skip)
 * @type: the #GType of the value
 * @...: the value to store
 *
 * Creates a new #AdwValueObject. This is a convenience method which uses
 * the G_VALUE_COLLECT() macro internally.
 *
 * Returns: a new #AdwValueObject
 *
 * Since: 1.0
 */
AdwValueObject *
adw_value_object_new_collect (GType type, ...)
{
  g_auto(GValue) value = G_VALUE_INIT;
  g_autofree char *error = NULL;
  va_list var_args;

  va_start (var_args, type);

  G_VALUE_COLLECT_INIT (&value, type, var_args, 0, &error);

  va_end (var_args);

  if (error)
    g_critical ("%s: %s", G_STRFUNC, error);

  return g_object_new (ADW_TYPE_VALUE_OBJECT,
                       "value", &value,
                       NULL);
}

/**
 * adw_value_object_new_string: (skip)
 * @string: (transfer none): the string to store
 *
 * Creates a new #AdwValueObject. This is a convenience method to create a
 * #AdwValueObject that stores a string.
 *
 * Returns: a new #AdwValueObject
 *
 * Since: 1.0
 */
AdwValueObject *
adw_value_object_new_string (const char *string)
{
  g_auto(GValue) value = G_VALUE_INIT;

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, string);
  return adw_value_object_new (&value);
}

/**
 * adw_value_object_new_take_string: (skip)
 * @string: (transfer full): the string to store
 *
 * Creates a new #AdwValueObject. This is a convenience method to create a
 * #AdwValueObject that stores a string taking ownership of it.
 *
 * Returns: a new #AdwValueObject
 *
 * Since: 1.0
 */
AdwValueObject *
adw_value_object_new_take_string (char *string)
{
  g_auto(GValue) value = G_VALUE_INIT;

  g_value_init (&value, G_TYPE_STRING);
  g_value_take_string (&value, string);
  return adw_value_object_new (&value);
}

static void
adw_value_object_finalize (GObject *object)
{
  AdwValueObject *self = ADW_VALUE_OBJECT (object);

  g_value_unset (&self->value);

  G_OBJECT_CLASS (adw_value_object_parent_class)->finalize (object);
}

static void
adw_value_object_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwValueObject *self = ADW_VALUE_OBJECT (object);

  switch (prop_id)
    {
    case PROP_VALUE:
      g_value_set_boxed (value, &self->value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
adw_value_object_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwValueObject *self = ADW_VALUE_OBJECT (object);
  GValue *real_value;

  switch (prop_id)
    {
    case PROP_VALUE:
      /* construct only */
      real_value = g_value_get_boxed (value);
      g_value_init (&self->value, real_value->g_type);
      g_value_copy (real_value, &self->value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
adw_value_object_class_init (AdwValueObjectClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = adw_value_object_finalize;
  object_class->get_property = adw_value_object_get_property;
  object_class->set_property = adw_value_object_set_property;

  props[PROP_VALUE] =
    g_param_spec_boxed ("value",
                        "Value",
                        "The contained value",
                        G_TYPE_VALUE,
                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     N_PROPS,
                                     props);
}

static void
adw_value_object_init (AdwValueObject *self)
{
}

/**
 * adw_value_object_get_value:
 * @value: the #AdwValueObject
 *
 * Return the contained value.
 *
 * Returns: (transfer none): the contained #GValue
 *
 * Since: 1.0
 */
const GValue *
adw_value_object_get_value (AdwValueObject *value)
{
  return &value->value;
}

/**
 * adw_value_object_copy_value:
 * @value: the #AdwValueObject
 * @dest: #GValue with correct type to copy into
 *
 * Copy data from the contained #GValue into @dest.
 *
 * Since: 1.0
 */
void
adw_value_object_copy_value (AdwValueObject *value,
                             GValue         *dest)
{
  g_value_copy (&value->value, dest);
}

/**
 * adw_value_object_get_string:
 * @value: the #AdwValueObject
 *
 * Returns the contained string if the value is of type #G_TYPE_STRING.
 *
 * Returns: (transfer none): the contained string
 *
 * Since: 1.0
 */
const char *
adw_value_object_get_string (AdwValueObject *value)
{
  return g_value_get_string (&value->value);
}

/**
 * adw_value_object_dup_string:
 * @value: the #AdwValueObject
 *
 * Returns a copy of the contained string if the value is of type
 * #G_TYPE_STRING.
 *
 * Returns: (transfer full): a copy of the contained string
 *
 * Since: 1.0
 */
char *
adw_value_object_dup_string (AdwValueObject *value)
{
  return g_value_dup_string (&value->value);
}
