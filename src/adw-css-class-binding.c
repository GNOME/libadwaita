/* gbinding.c: Binding for object properties
 *
 * Copyright (C) 2010  Intel Corp.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Emmanuele Bassi <ebassi@linux.intel.com>
 */

#include "config.h"

#include "adw-css-class-binding.h"

#include "adw-marshalers.h"

/**
 * AdwCssClassBinding:
 *
 * A binding between a [class@GObject.Object] property and a CSS class on a
 * [class@Gtk.Widget].
 *
 * To create a binding, use [func@Adw.bind_property_to_css_class].
 *
 * Whenever the source property changes, a CSS class is toggled on the target
 * object; for instance, given the following binding:
 *
 * ```c
 * adw_bind_property_to_css_class (source, "property",
 *                                 target, "css-class",
 *                                 G_BINDING_DEFAULT);
 * ```
 *
 * when `property` is a boolean property, `css-class` will be set every time the
 * value of `property` is changed to `TRUE`, and vice versa.
 *
 * Using [func@Adw.bind_property_to_css_class_full], it's possible to define a
 * custom mapping between a property and the boolean enable state, that is,
 * defining a mapping between any arbitrary property type and a boolean type.
 * This allows for any property, not just boolean properties, to be bound to a
 * CSS class; for instance, the following binding
 *
 * ```c
 * adw_bind_property_to_css_class_full (source, "property",
 *                                      target, "css-class",
 *                                      G_BINDING_DEFAULT,
 *                                      int_to_bool,
 *                                      bool_to_int,
 *                                      NULL, NULL);
 * ```
 *
 * will map an integer-typed `property` to a boolean value, and use the value of
 * that boolean to toggle the CSS class.
 *
 * It is possible to invert the type of a boolean without using a custom map
 * function by using the [flags@GObject.BindingFlags.INVERT_BOOLEAN] flag.
 *
 * A binding will be removed, and any allocated resources freed, whenever either
 * one of the source or target instances are finalized, or when the binding
 * instances loses its last reference.
 *
 * Languages with garbage collection, or developers wanting to fully control the
 * lifecycle of a binding may use [method@CssClassBinding.unbind] to expliticly
 * release a binding, instead of relying on the last reference on the binding,
 * source, and target instances to drop.
 *
 * Since: 1.10
 */

struct _AdwCssClassBinding
{
  GObject parent_instance;

  GObject *source;
  GObject *target;

  GParamSpec *source_pspec;
  gchar *source_property;
  gchar *target_css_class;

  GBindingFlags flags;

  unsigned long source_notify;
  unsigned long target_notify;
  gboolean binding_removed;
  gboolean freeze;
  gboolean target_weak_ref;

  AdwCssClassBindingMapToClassFunc map_to_class;
  AdwCssClassBindingMapToPropertyFunc map_to_property;
  gpointer user_data;
  GDestroyNotify destroy_notify;
};

G_DEFINE_FINAL_TYPE (AdwCssClassBinding, adw_css_class_binding, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_SOURCE,
  PROP_SOURCE_PROPERTY,
  PROP_TARGET,
  PROP_TARGET_CSS_CLASS,
  PROP_FLAGS,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static gboolean
default_property_to_class_map (AdwCssClassBinding *self,
                               const GValue       *value,
                               gpointer            user_data)
{
  if (G_VALUE_HOLDS_BOOLEAN (value))
    return g_value_get_boolean (value);

  if (g_value_type_compatible (G_VALUE_TYPE (value), G_TYPE_BOOLEAN)) {
    GValue to_value = G_VALUE_INIT;
    gboolean ret;
    g_value_init (&to_value, G_TYPE_BOOLEAN);
    g_value_copy (value, &to_value);
    ret = g_value_get_boolean (&to_value);
    g_value_unset (&to_value);
    return ret;
  }

  if (g_value_type_transformable (G_VALUE_TYPE (value), G_TYPE_BOOLEAN)) {
    GValue to_value = G_VALUE_INIT;
    gboolean ret;
    g_value_init (&to_value, G_TYPE_BOOLEAN);

    if (g_value_transform (value, &to_value)) {
      ret = g_value_get_boolean (&to_value);
    } else {
      g_critical ("%s: Unable to convert a value of type %s to a value of type %s",
                  G_STRLOC,
                  g_type_name (G_VALUE_TYPE (value)),
                  g_type_name (G_TYPE_BOOLEAN));
      ret = FALSE;
    }

    g_value_unset (&to_value);
    return ret;
  }

  g_critical ("%s: Unable to convert a value of type %s to a value of type %s",
              G_STRLOC,
              g_type_name (G_VALUE_TYPE (value)),
              g_type_name (G_TYPE_BOOLEAN));

 return FALSE;
}

static gboolean
default_property_to_class_invert_map (AdwCssClassBinding *self,
                                      const GValue       *value,
                                      gpointer            user_data)
{
  gboolean ret;

  g_assert (G_VALUE_HOLDS_BOOLEAN (value));

  ret = g_value_get_boolean (value);
  ret = !ret;
  
  return ret;
}

static void
default_class_to_property_map (AdwCssClassBinding *self,
                               gboolean            class_value,
                               GValue             *property_value,
                               gpointer            user_data)
{
  g_assert (G_VALUE_HOLDS_BOOLEAN (property_value));

  g_value_set_boolean (property_value, class_value);
}

static void
default_class_to_property_invert_map (AdwCssClassBinding *self,
                                      gboolean            class_value,
                                      GValue             *property_value,
                                      gpointer            user_data)
{
  g_assert (G_VALUE_HOLDS_BOOLEAN (property_value));

  g_value_set_boolean (property_value, !class_value);
}

static void
weak_notify (gpointer  user_data,
             GObject  *where_the_object_was)
{
  AdwCssClassBinding *self = ADW_CSS_CLASS_BINDING (user_data);
  gboolean remove_binding = FALSE;

  if (self->source) {
    if (self->source != where_the_object_was) {
      g_object_weak_unref (G_OBJECT (self->source),
                           (GWeakNotify) weak_notify,
                           self);
    }

    if (self->source_notify != 0) {
      /* If the source and/or target are why weak_notify was called, then the signal handlers
       * are already disposed of, and we simply need to remove the objects. If the binding is what
       * is being disposed first, then disconnect the handlers */
      if (self->source != where_the_object_was)
        g_signal_handler_disconnect (self->source, self->source_notify);
      self->source_notify = 0;
    }

    self->source = NULL;
    remove_binding = TRUE;
  }

  if (self->target) {
    if (self->target_weak_ref && self->target != where_the_object_was) {
      g_object_weak_unref (G_OBJECT (self->target),
                           (GWeakNotify) weak_notify, 
                           self);
    }

    if (self->target_notify != 0) {
      if (self->target != where_the_object_was)
        g_signal_handler_disconnect (self->target, self->target_notify);
      self->target_notify = 0;
    }
    self->target = NULL;
    remove_binding = TRUE;
  }

  if (self->binding_removed || !remove_binding)
    return;

  if (self->destroy_notify)
    self->destroy_notify (self->user_data);
  g_object_unref (self);
}

static void
on_source_notify_cb (GObject            *source,
                     GParamSpec         *pspec,
                     AdwCssClassBinding *self)
{
  GObject *target;
  gboolean ret;
  GValue from_value = G_VALUE_INIT;

  if (self->freeze)
    return;

  target = self->target;
  if (!target)
    return;

  if (!self->map_to_class)
    return;

  g_value_init (&from_value, G_PARAM_SPEC_VALUE_TYPE (self->source_pspec));

  g_object_get_property (source, self->source_pspec->name, &from_value);

  ret = self->map_to_class (self, &from_value, self->user_data);

  self->freeze = TRUE;
  if (ret)
    gtk_widget_add_css_class (GTK_WIDGET (target), self->target_css_class);
  else
    gtk_widget_remove_css_class (GTK_WIDGET (target), self->target_css_class);
  self->freeze = FALSE;

  g_value_unset (&from_value);
}

static void
on_target_notify_cb (GObject            *target,
                     GParamSpec         *pspec,
                     AdwCssClassBinding *self)
{
  GObject *source;
  gboolean has_class;
  GValue to_value = G_VALUE_INIT;

  if (self->freeze)
    return;

  source = self->source;
  if (!source)
    return;

  if (!self->map_to_property)
    return;

  g_value_init (&to_value, G_PARAM_SPEC_VALUE_TYPE (self->source_pspec));

  has_class = gtk_widget_has_css_class (GTK_WIDGET (target), self->target_css_class);
  self->map_to_property (self, has_class, &to_value, self->user_data);

  self->freeze = TRUE;

  (void) g_param_value_validate (self->source_pspec, &to_value);
  g_object_set_property (self->source, self->source_pspec->name, &to_value);

  self->freeze = FALSE;

  g_value_unset (&to_value);
}

static void
adw_css_class_binding_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AdwCssClassBinding *self = ADW_CSS_CLASS_BINDING (object);

  switch (prop_id) {
  case PROP_FLAGS:
    g_value_set_flags (value, self->flags);
    break;
  case PROP_SOURCE:
    g_value_take_object (value, &self->source);
    break;
  case PROP_SOURCE_PROPERTY:
    g_value_set_string (value, self->source_property);
    break;
  case PROP_TARGET:
    g_value_take_object (value, &self->target);
    break;
  case PROP_TARGET_CSS_CLASS:
    g_value_set_string (value, self->target_css_class);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_css_class_binding_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AdwCssClassBinding *self = ADW_CSS_CLASS_BINDING (object);

  switch (prop_id) {
  case PROP_FLAGS:
    self->flags = g_value_get_flags (value);
    break;
  case PROP_SOURCE:
    self->source = g_value_get_object (value);
    g_object_weak_ref (G_OBJECT (self->source),
                       (GWeakNotify) weak_notify,
                       self);
    break;
  case PROP_SOURCE_PROPERTY:
    g_set_str (&self->source_property, g_value_get_string (value));
    break;
  case PROP_TARGET:
    self->target = g_value_get_object (value);
    if (self->source && (self->target != self->source)) {
      self->target_weak_ref = TRUE;
      g_object_weak_ref (G_OBJECT (self->target),
                         (GWeakNotify) weak_notify,
                         self);
    }
    break;
  case PROP_TARGET_CSS_CLASS:
    g_set_str (&self->target_css_class, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
adw_css_class_binding_constructed (GObject *object)
{
  AdwCssClassBinding *self = ADW_CSS_CLASS_BINDING (object);
  guint notify_signal_id;

  g_assert (self->source != NULL);
  g_assert (self->target != NULL);
  g_assert (self->source_property != NULL);
  g_assert (self->target_css_class != NULL);

  self->source_pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (self->source), self->source_property);
  g_assert (self->source_pspec != NULL);

  notify_signal_id = g_signal_lookup ("notify", G_OBJECT_TYPE (object));
  self->source_notify = g_signal_connect_closure_by_id (self->source,
                                                        notify_signal_id,
                                                        g_quark_from_string (self->source_property),
                                                        g_cclosure_new (G_CALLBACK (on_source_notify_cb),
                                                                        self,
                                                                        NULL),
                                                        FALSE);

  if (self->flags & G_BINDING_BIDIRECTIONAL) {
    self->target_notify = g_signal_connect_closure_by_id (self->target,
                                                          g_signal_lookup ("notify", G_OBJECT_TYPE (object)),
                                                          g_quark_from_string ("css-classes"),
                                                          g_cclosure_new (G_CALLBACK (on_target_notify_cb),
                                                                          self,
                                                                          NULL),
                                                          FALSE);
    }
}

static void
adw_css_class_binding_finalize (GObject *object)
{
  AdwCssClassBinding *self = ADW_CSS_CLASS_BINDING (object);

  self->binding_removed = TRUE;

  weak_notify (self, NULL);
  g_free (self->source_property);
  g_free (self->target_css_class);

  G_OBJECT_CLASS (adw_css_class_binding_parent_class)->finalize (object);
}

static void
adw_css_class_binding_class_init (AdwCssClassBindingClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_css_class_binding_constructed;
  object_class->get_property = adw_css_class_binding_get_property;
  object_class->set_property = adw_css_class_binding_set_property;
  object_class->finalize = adw_css_class_binding_finalize;

  /**
   * AdwCssClassBinding:source:
   *
   * The object to use as the source of the CSS class binding.
   *
   * Since: 1.10
   */
  props[PROP_SOURCE] =
    g_param_spec_object ("source", NULL, NULL,
                         G_TYPE_OBJECT,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwCssClassBinding:target:
   *
   * The widget to use as the target of the CSS class binding.
   *
   * Since: 1.10
   */
  props[PROP_TARGET] =
    g_param_spec_object ("target", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  
  /**
   * AdwCssClassBinding:source-property:
   *
   * The name of the property that shoudl be used as the source of the binding.
   *
   * Since: 1.10
   */
  props[PROP_SOURCE_PROPERTY] =
    g_param_spec_string ("source-property", NULL, NULL,
                         NULL,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * AdwCssClassBinding:target-css-class:
   *
   * The name of the CSS class that should be toggled on the target object.
   *
   * Since: 1.10
   */
  props[PROP_TARGET_CSS_CLASS] =
    g_param_spec_string ("target-css-class", NULL, NULL,
                         NULL,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
 
  /**
   * AdwCssClassBinding:flags:
   *
   * Flags to be used to control the binding.
   *
   * Since: 1.10
   */
  props[PROP_FLAGS] =
    g_param_spec_flags ("flags", NULL, NULL,
                        G_TYPE_BINDING_FLAGS,
                        G_BINDING_DEFAULT,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_css_class_binding_init (AdwCssClassBinding *self)
{
}

/**
 * adw_css_class_binding_get_flags:
 * @self: a binding
 *
 * Gets the flags passed when constructing the binding.
 *
 * Returns: the binding flags
 *
 * Since: 1.10
 */
GBindingFlags
adw_css_class_binding_get_flags (AdwCssClassBinding *self)
{
  g_return_val_if_fail (ADW_IS_CSS_CLASS_BINDING (self), G_BINDING_DEFAULT);

  return self->flags;
}

/**
 * adw_css_class_binding_get_source:
 * @self: a binding
 *
 * Gets the object instance passed as the source of the binding.
 *
 * While a binding can outlive the source as the binding doesn't hold a strong
 * reference to the source, the binding will try to finalize itself when it does
 * not have a valid source. If the source, however, is destroyed before the
 * binding is fully finalized then this function will return `NULL`.
 *
 * Returns: (transfer full) (nullable): the source, or `NULL` if it doesn't
 *   exist anymore
 *
 * Since: 1.10
 */
GObject *
adw_css_class_binding_get_source (AdwCssClassBinding *self)
{
  g_return_val_if_fail (ADW_IS_CSS_CLASS_BINDING (self), NULL);

  return self->source;
}

/**
 * adw_css_class_binding_get_target:
 * @self: a binding
 *
 * Gets the widget instance passed as the target of the binding.
 *
 * While a binding can outlive the target as the binding doesn't hold a strong
 * reference to the target, the binding will try to finalize itself when it does
 * not have a valid target. If the target, however, is destroyed before the
 * binding is fully finalized then this function will return `NULL`.
 *
 * Returns: (transfer full) (nullable): the target, or `NULL` if it doesn't
 *   exist anymore
 *
 * Since: 1.10
 */
GtkWidget *
adw_css_class_binding_get_target (AdwCssClassBinding *self)
{
  g_return_val_if_fail (ADW_IS_CSS_CLASS_BINDING (self), NULL);

  return GTK_WIDGET (self->target);
}

/**
 * adw_css_class_binding_get_source_property:
 * @self: a binding
 *
 * Gets the name of the property bound from [property@CssClassBinding:source].
 *
 * Returns: the name of the source property
 *
 * Since: 1.10
 */
const char *
adw_css_class_binding_get_source_property (AdwCssClassBinding *self)
{
  g_return_val_if_fail (ADW_IS_CSS_CLASS_BINDING (self), NULL);

  return self->source_property;
}

/**
 * adw_css_class_binding_get_target_css_class:
 * @self: a binding
 *
 * Gets the CSS class to toggle on [property@CssClassBinding:target].
 *
 * Returns: the name of the target CSS class
 *
 * Since: 1.10
 */
const char *
adw_css_class_binding_get_target_css_class (AdwCssClassBinding *self)
{
  g_return_val_if_fail (ADW_IS_CSS_CLASS_BINDING (self), NULL);

  return self->target_css_class;
}

/**
 * adw_css_class_binding_unbind:
 * @self: a binding
 *
 * Explicitly releases the binding between the source and the target.
 *
 * This function will release the reference that is being held on @self if the
 * binding is still bound; if you want to hold on to the binding instance after
 * calling this method, you will need to manually hold a reference to it.
 *
 * Note that this function does not take ownership of @self, it only unrefs the
 * instance that was initially created by [func@Adw.bind_property_to_css_class]
 * and is owned by the binding.
 *
 * Since: 1.10
 */
void
adw_css_class_binding_unbind (AdwCssClassBinding *self)
{
  g_return_if_fail (ADW_IS_CSS_CLASS_BINDING (self));

  weak_notify (self, NULL);
}

/**
 * adw_bind_property_to_css_class:
 * @source: (type GObject): the source object
 * @source_property:: the property on @source to bind
 * @target: the target widget
 * @target_css_class: the CSS class on @target to bind
 * @flags: flags to pass to @self
 *
 * Creates a binding between a property on @source and a CSS class on @target.
 *
 * Whenever @source_property is changed the @target_css_class is toggled either
 * on or off on @target using the boolean value of @source_property.
 *
 * For instance:
 *
 * ```c
 * adw_bind_property_to_css_class (action, "active", widget, "active-widget", 0);
 * ```
 *
 * will result in the `active-widget` CSS class on the widget instance being
 * applied when `active` is `TRUE`, and `active-widget` being removed when
 * `active` is `FALSE`.
 *
 * The binding will automatically be removed when either the @source or @target
 * instances are finalized. To remove the binding without affecting the @source
 * or @target, call [method@GObject.Object.unref] on the returned binding. It is
 * recommended to only call [method@GObject.Object.unref] when it is clear that
 * both @source and @target will outlive the binding.
 *
 * This function is not thread safe. A source can have multiple bindings.
 *
 * See also: [func@Adw.bind_property_to_css_class_full].
 *
 * Returns: (transfer none): the new binding
 *
 * Since: 1.10
 */
AdwCssClassBinding *
adw_bind_property_to_css_class (gpointer       source,
                                const char    *source_property,
                                GtkWidget     *target,
                                const char    *target_css_class,
                                GBindingFlags  flags)
{
  return adw_bind_property_to_css_class_full (source, source_property,
                                              target, target_css_class,
                                              flags,
                                              NULL, NULL,
                                              NULL, NULL);
}

/**
 * adw_bind_property_to_css_class_full: (skip)
 * @source: (type GObject): the source object
 * @source_property: the property on @source to bind
 * @target: the target widget
 * @target_css_class: the CSS class on @target to bind
 * @flags: flags to pass to @self
 * @map_to_class: (destroy notify) (closure user_data) (scope notified) (nullable):
 *   the mapping function from the @source_property to a boolean value, or
 *   `NULL` to use the default
 * @map_to_property: (destroy notify) (closure user_data) (scope notified) (nullable):
 *   the mapping function from the @target_css_class to a value, or `NULL` to
 *   use the default
 * @user_data: (nullable): custom data to be passed to @map_to_class and
 *   @map_to_property
 * @notify: (nullable): a function to call when disposing the binding, to free
 *   resources used by @map_to_class and @map_to_property
 *
 * Creates a binding between a property on @source and a CSS class on @target
 * with a custom mapping.
 *
 * Complete version of [func@Adw.bind_property_to_css_class].
 *
 * Whenever @source_property is changed the @target_css_class is toggled either
 * on or off on @target using the boolean value of @source_property.
 *
 * For instance:
 *
 * ```c
 * adw_bind_property_to_css_class (action, "active", widget, "active-widget", 0);
 * ```
 *
 * will result in the `active-widget` CSS class on the widget instance being
 * applied when `active` is `TRUE`, and `active-widget` being removed when
 * `active` is `FALSE`.
 *
 * The binding will automatically be removed when either the @source or @target
 * instances are finalized. To remove the binding without affecting the @source
 * or @target, call [method@GObject.Object.unref] on the returned binding. It is
 * recommended to only call [method@GObject.Object.unref] when it is clear that
 * both @source and @target will outlive the binding.
 *
 * This function is not thread safe. A source can have multiple bindings.
 *
 * Returns: (transfer none): the new binding
 *
 * Since: 1.10
 */
AdwCssClassBinding *
adw_bind_property_to_css_class_full (gpointer                             source,
                                     const char                          *source_property,
                                     GtkWidget                           *target,
                                     const char                          *target_css_class,
                                     GBindingFlags                        flags,
                                     AdwCssClassBindingMapToClassFunc     map_to_class,
                                     AdwCssClassBindingMapToPropertyFunc  map_to_property,
                                     gpointer                             user_data,
                                     GDestroyNotify                       notify)
{
  GParamSpec *pspec;
  AdwCssClassBinding *binding;

  g_return_val_if_fail (G_IS_OBJECT (source), NULL);
  g_return_val_if_fail (source_property != NULL, NULL);
  g_return_val_if_fail (g_param_spec_is_valid_name (source_property), NULL);
  g_return_val_if_fail (GTK_IS_WIDGET (target), NULL);
  g_return_val_if_fail (target_css_class != NULL, NULL);
  g_return_val_if_fail (target_css_class[0] != '\0', FALSE);
  g_return_val_if_fail (target_css_class[0] != '.', FALSE);

  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (source), source_property);
  if (pspec == NULL) {
    g_critical ("%s: The source object of type %s has no property called '%s'",
                G_STRLOC,
                G_OBJECT_TYPE_NAME (source),
                source_property);
    return NULL;
  }

  if (!(pspec->flags & G_PARAM_READABLE)) {
    g_critical ("%s: The source object of type %s has no readable property called '%s'",
                G_STRLOC,
                G_OBJECT_TYPE_NAME (source),
                source_property);
    return NULL;
  }
  
  if ((flags & G_BINDING_BIDIRECTIONAL) &&
      ((pspec->flags & G_PARAM_CONSTRUCT_ONLY) || !(pspec->flags & G_PARAM_WRITABLE))) {
    g_critical ("%s: The source object of type %s has no writable property called '%s'",
                G_STRLOC,
                G_OBJECT_TYPE_NAME (source),
                source_property);
    return NULL;
  }

  if ((flags & G_BINDING_INVERT_BOOLEAN) &&
      !(G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_BOOLEAN)) {
    g_critical ("%s: The G_BINDING_INVERT_BOOLEAN flag can only be used "
                "when binding a boolean property; the source property '%s' "
                "is of type '%s'",
                G_STRLOC,
                source_property,
                g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec)));
    return NULL;
  }

  if (!(G_PARAM_SPEC_VALUE_TYPE (pspec) == G_TYPE_BOOLEAN) &&
      (((flags & G_BINDING_BIDIRECTIONAL) && !map_to_property) ||
      !map_to_class)) {
    g_critical ("%s: The default mapping functions only support boolean properties, "
                "the source property '%s' is of type '%s'",
                G_STRLOC,
                source_property,
                g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec)));
    return NULL;
  }
  
  binding = g_object_new (ADW_TYPE_CSS_CLASS_BINDING,
                          "source", source,
                          "source-property", source_property,
                          "target", target,
                          "target-css-class", target_css_class,
                          "flags", flags,
                          NULL);

  binding->map_to_class = default_property_to_class_map;
  binding->map_to_property = default_class_to_property_map;

  /* Configure map function */
  if ((flags & G_BINDING_INVERT_BOOLEAN) && !map_to_class)
    binding->map_to_class = default_property_to_class_invert_map;
  else if (map_to_class)
    binding->map_to_class = map_to_class;

  if ((flags & G_BINDING_INVERT_BOOLEAN) && !map_to_property)
    binding->map_to_property = default_class_to_property_invert_map;
  else if (map_to_property)
    binding->map_to_property = map_to_property;

  binding->user_data = user_data;
  binding->destroy_notify = notify;

  if (flags & G_BINDING_SYNC_CREATE)
    on_source_notify_cb (source, binding->source_pspec, binding);

  return binding;
}

typedef struct _MappingClosureData
{
  GClosure *map_to_class_closure;
  GClosure *map_to_property_closure;
} MappingClosureData;

static gboolean
map_to_class_with_closures (AdwCssClassBinding *self,
                            const GValue       *property_value,
                            gpointer            data)
{
  MappingClosureData *map_data = data;
  GValue params[2] = { G_VALUE_INIT, G_VALUE_INIT };
  GValue retval = G_VALUE_INIT;
  gboolean ret;

  g_value_init (&params[0], ADW_TYPE_CSS_CLASS_BINDING);
  g_value_set_object (&params[0], self);

  g_value_init (&params[1], G_TYPE_VALUE);
  g_value_set_boxed (&params[1], property_value);

  g_value_init (&retval, G_TYPE_BOOLEAN);
  g_value_set_boolean (&retval, FALSE);

  g_closure_invoke (map_data->map_to_class_closure, &retval, 2, params, NULL);

  ret = g_value_get_boolean (&retval);

  g_value_unset (&params[0]);
  g_value_unset (&params[1]);
  g_value_unset (&retval);

  return ret;
}

static void
map_to_property_with_closures (AdwCssClassBinding *self,
                               gboolean            class_value,
                               GValue             *property_value,
                               gpointer            data)
{
  MappingClosureData *map_data = data;
  GValue params[3] = { G_VALUE_INIT, G_VALUE_INIT, G_VALUE_INIT };

  g_value_init (&params[0], ADW_TYPE_CSS_CLASS_BINDING);
  g_value_set_object (&params[0], self);

  g_value_init (&params[1], G_TYPE_BOOLEAN);
  g_value_set_boolean (&params[1], class_value);

  g_value_init (&params[2], G_TYPE_VALUE);
  g_value_set_boxed (&params[2], property_value);

  g_closure_invoke (map_data->map_to_property_closure, NULL, 3, params, NULL);

  const GValue *out_value = g_value_get_boxed (&params[2]);
  g_assert (out_value != NULL);
  g_value_copy (out_value, property_value);

  g_value_unset (&params[0]);
  g_value_unset (&params[1]);
  g_value_unset (&params[2]);
}

static void
mapping_closure_free_func (gpointer data)
{
  MappingClosureData *map_data = data;

  if (!map_data)
    return;

  if (map_data->map_to_class_closure) {
    g_closure_unref (map_data->map_to_class_closure);
    map_data->map_to_class_closure = NULL;
  }

  if (map_data->map_to_property_closure) {
    g_closure_unref (map_data->map_to_property_closure);
    map_data->map_to_property_closure = NULL;
  }

  g_free (map_data);
}

/**
 * adw_bind_property_to_css_class_with_closures: (rename-to adw_bind_property_to_css_class_full)
 * @source: (type GObject): the source object
 * @source_property: the property on @source to bind
 * @target: the target widget
 * @target_css_class: the CSS class on @target to bind
 * @flags: flags to pass to @self
 * @map_to_class: (nullable): a closure wrapping the @source_property to
 *   boolean value map, or `NULL` to use the default
 * @map_to_property: (nullable): a closure wrapping the @target_css_class to
 *   [struct@GObject.Value] map, or `NULL` to use the default
 *
 * Creates a binding between a property on @source and a CSS class on @target
 * with a custom mapping using closures.
 *
 * This function is the language bindings friendly version of
 * [func@Adw.bind_property_to_css_class_full].
 *
 * The binding will automatically be removed when either the @source or @target
 * instances are finalized. To remove the binding without affecting the @source
 * or @target, call [method@GObject.Object.unref] on the returned binding. It is
 * recommended to only call [method@GObject.Object.unref] when it is clear that
 * both @source and @target will outlive the binding.
 *
 * This function is not thread safe. A source can have multiple bindings.
 *
 * See also: [func@Adw.bind_property_to_css_class].
 *
 * Returns: (transfer none): the new binding
 *
 * Since: 1.10
 */
AdwCssClassBinding *
adw_bind_property_to_css_class_with_closures (gpointer       source,
                                              const char    *source_property,
                                              GtkWidget     *target,
                                              const char    *target_css_class,
                                              GBindingFlags  flags,
                                              GClosure      *map_to_class,
                                              GClosure      *map_to_property)
{
  MappingClosureData *data;
  data = g_new0 (MappingClosureData, 1);

  if (map_to_class) {
    if (G_CLOSURE_NEEDS_MARSHAL (map_to_class))
      g_closure_set_marshal (map_to_class, adw_marshal_BOOLEAN__BOXED);

    data->map_to_class_closure = g_closure_ref (map_to_class);
    g_closure_sink (data->map_to_class_closure);
  }

  if (map_to_property) {
    if (G_CLOSURE_NEEDS_MARSHAL (map_to_property))
      g_closure_set_marshal (map_to_property, adw_marshal_VOID__BOOLEAN_BOXED);
   
    data->map_to_property_closure = g_closure_ref (map_to_property);
    g_closure_sink (data->map_to_property_closure);
  }

  return adw_bind_property_to_css_class_full (source, source_property,
                                              target, target_css_class,
                                              flags,
                                              map_to_class != NULL ? map_to_class_with_closures : NULL,
                                              map_to_property != NULL ? map_to_property_with_closures : NULL,
                                              data,
                                              mapping_closure_free_func);
}
