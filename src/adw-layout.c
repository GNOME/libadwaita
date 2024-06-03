/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include "config.h"

#include "adw-layout-private.h"

#include "adw-multi-layout-view.h"

/**
 * AdwLayout:
 *
 * An individual layout in [class@MultiLayoutView].
 *
 * Since: 1.6
 */

struct _AdwLayout
{
  GObject parent_instance;

  AdwMultiLayoutView *view;

  GtkWidget *content;

  char *name;
};

static void adw_layout_buildable_init (GtkBuildableIface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwLayout, adw_layout, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_layout_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_CONTENT,
  PROP_NAME,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static void
set_content (AdwLayout *self,
             GtkWidget *content)
{
  if (content == NULL)
    return;

  if (self->content)
    g_object_unref (self->content);

  self->content = g_object_ref_sink (content);
}

static void
adw_layout_dispose (GObject *object)
{
  AdwLayout *self = ADW_LAYOUT (object);

  g_clear_object (&self->content);

  G_OBJECT_CLASS (adw_layout_parent_class)->dispose (object);
}

static void
adw_layout_finalize (GObject *object)
{
  AdwLayout *self = ADW_LAYOUT (object);

  g_free (self->name);

  G_OBJECT_CLASS (adw_layout_parent_class)->finalize (object);
}

static void
adw_layout_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AdwLayout *self = ADW_LAYOUT (object);

  switch (prop_id) {
  case PROP_CONTENT:
    g_value_set_object (value, adw_layout_get_content (self));
    break;
  case PROP_NAME:
    g_value_set_string (value, adw_layout_get_name (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_layout_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AdwLayout *self = ADW_LAYOUT (object);

  switch (prop_id) {
  case PROP_CONTENT:
    set_content (self, g_value_get_object (value));
    break;
  case PROP_NAME:
    adw_layout_set_name (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_layout_class_init (AdwLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_layout_dispose;
  object_class->finalize = adw_layout_finalize;
  object_class->get_property = adw_layout_get_property;
  object_class->set_property = adw_layout_set_property;

  /**
   * AdwLayout:content:
   *
   * The content widget.
   *
   * Since: 1.6
   */
  props[PROP_CONTENT] =
    g_param_spec_object ("content", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * AdwLayout:name:
   *
   * The name of the layout.
   *
   * Since: 1.6
   */
  props[PROP_NAME] =
    g_param_spec_string ("name", NULL, NULL,
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_layout_init (AdwLayout *self)
{
}

static void
adw_layout_buildable_add_child (GtkBuildable *buildable,
                                GtkBuilder   *builder,
                                GObject      *child,
                                const char   *type)
{
  if (GTK_IS_WIDGET (child))
    set_content (ADW_LAYOUT (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_layout_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_layout_buildable_add_child;
}

/**
 * adw_layout_new:
 * @content: the content widget to use
 *
 * Creates a new `AdwLayout` that contains @content.
 *
 * Returns: a new `AdwLayout`
 *
 * Since: 1.6
 */
AdwLayout *
adw_layout_new (GtkWidget *content)
{
  g_return_val_if_fail (GTK_IS_WIDGET (content), NULL);

  return g_object_new (ADW_TYPE_LAYOUT, "content", content, NULL);
}

/**
 * adw_layout_get_content:
 * @self: a layout
 *
 * Gets the content widget.
 *
 * Returns: (transfer none): The content
 *
 * Since: 1.6
 */
GtkWidget *
adw_layout_get_content (AdwLayout *self)
{
  g_return_val_if_fail (ADW_IS_LAYOUT (self), NULL);

  return self->content;
}

/**
 * adw_layout_get_name:
 * @self: a layout
 *
 * Gets the name of the layout.
 *
 * Returns: (nullable): the name of the layout
 *
 * Since: 1.6
 */
const char *
adw_layout_get_name (AdwLayout *self)
{
  g_return_val_if_fail (ADW_IS_LAYOUT (self), NULL);

  return self->name;
}

/**
 * adw_layout_set_name:
 * @self: a layout
 * @name: (nullable): the layout name
 *
 * Sets the name of the layout.
 *
 * Since: 1.6
 */
void
adw_layout_set_name (AdwLayout  *self,
                     const char *name)
{
  g_return_if_fail (ADW_IS_LAYOUT (self));

  if (self->view) {
    AdwLayout *other_layout = adw_multi_layout_view_get_layout_by_name (self->view, name);

    if (other_layout && other_layout != self)
      g_warning ("Duplicate layout name in AdwMultiLayoutView: %s", name);
  }

  if (!g_set_str (&self->name, name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_NAME]);

  if (self->view && adw_multi_layout_view_get_layout (self->view) == self)
    g_object_notify (G_OBJECT (self->view), "layout-name");
}

void
adw_layout_set_view (AdwLayout          *self,
                     AdwMultiLayoutView *view)
{
  self->view = view;
}
