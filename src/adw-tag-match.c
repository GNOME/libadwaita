/* adw-tag-match.c:
 *
 * SPDX-FileCopyrightText: 2022  Emmanuele Bassi
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-tag-match-private.h"

struct _AdwTagMatch
{
  GObject parent_instance;

  /* type: GObject, owned */
  gpointer item;

  char *str;

  AdwTag *tag;
};

enum
{
  PROP_ITEM = 1,
  PROP_STRING,

  N_PROPS
};

static GParamSpec *obj_props[N_PROPS];

G_DEFINE_FINAL_TYPE (AdwTagMatch, adw_tag_match, G_TYPE_OBJECT)

static void
adw_tag_match_set_property (GObject      *gobject,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  AdwTagMatch *self = ADW_TAG_MATCH (gobject);

  switch (prop_id) {
  case PROP_ITEM:
    g_clear_object (&self->item);
    self->item = g_value_dup_object (value);
    break;

  case PROP_STRING:
    g_clear_pointer (&self->str, g_free);
    self->str = g_value_dup_string (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tag_match_get_property (GObject    *gobject,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  AdwTagMatch *self = ADW_TAG_MATCH (gobject);

  switch (prop_id) {
  case PROP_ITEM:
    g_value_set_object (value, self->item);
    break;

  case PROP_STRING:
    g_value_set_string (value, self->str);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

static void
adw_tag_match_dispose (GObject *gobject)
{
  AdwTagMatch *self = ADW_TAG_MATCH (gobject);

  g_clear_object (&self->tag);
  g_clear_object (&self->item);
  g_clear_pointer (&self->str, g_free);

  G_OBJECT_CLASS (adw_tag_match_parent_class)->dispose (gobject);
}

static void
adw_tag_match_class_init (AdwTagMatchClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = adw_tag_match_set_property;
  gobject_class->get_property = adw_tag_match_get_property;
  gobject_class->dispose = adw_tag_match_dispose;

  obj_props[PROP_ITEM] =
    g_param_spec_object ("item", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);
  obj_props[PROP_STRING] =
    g_param_spec_string ("string", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, obj_props);
}

static void
adw_tag_match_init (AdwTagMatch *self)
{
}

AdwTagMatch *
adw_tag_match_new (gpointer    item,
                   const char *str)
{
  return g_object_new (ADW_TYPE_TAG_MATCH,
                       "item", item,
                       "string", str,
                       NULL);
}

gpointer
adw_tag_match_get_item (AdwTagMatch *self)
{
  g_return_val_if_fail (ADW_IS_TAG_MATCH (self), NULL);

  return self->item;
}

const char *
adw_tag_match_get_string (AdwTagMatch *self)
{
  g_return_val_if_fail (ADW_IS_TAG_MATCH (self), NULL);

  return self->str;
}

void
adw_tag_match_set_tag (AdwTagMatch *self,
                       AdwTag      *tag)
{
  g_set_object (&self->tag, tag);
}

AdwTag *
adw_tag_match_get_tag (AdwTagMatch *self)
{
  return self->tag;
}
