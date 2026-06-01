/*
 * Copyright (C) 2026 Jamie Murphy <jmurphy@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined(_ADWAITA_INSIDE) && !defined(ADWAITA_COMPILATION)
#error "Only <adwaita.h> can be included directly."
#endif

#include "adw-version.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ADW_TYPE_CSS_CLASS_BINDING (adw_css_class_binding_get_type())

ADW_AVAILABLE_IN_1_10
G_DECLARE_FINAL_TYPE (AdwCssClassBinding, adw_css_class_binding, ADW, CSS_CLASS_BINDING, GObject)

/**
 * AdwCssClassBindingMapToClassFunc:
 * @self: The binding class
 * @property_value: the source property value
 * @user_data: (nullable): The user data provided when creating the binding
 *
 * Prototype for mapping properties to CSS classes for a [class@CssClassBinding].
 *
 * Returns: whether to apply the target CSS class
 *
 * Since: 1.10
 */
typedef gboolean (* AdwCssClassBindingMapToClassFunc) (AdwCssClassBinding *self,
                                                       const GValue       *property_value,
                                                       gpointer            user_data);

/**
 * AdwCssClassBindingMapToPropertyFunc:
 * @self: The binding class
 * @class_value: whether the target CSS class is applied
 * @property_value: The value to use for the source property
 * @user_data: (nullable): The user data provided when creating the binding
 *
 * Prototype for mapping CSS classes to properties for a [class@CssClassBinding].
 *
 * Since: 1.10
 */
typedef void (* AdwCssClassBindingMapToPropertyFunc) (AdwCssClassBinding *self,
                                                      gboolean            class_value,
                                                      GValue             *property_value,
                                                      gpointer            user_data);

ADW_AVAILABLE_IN_1_10
GBindingFlags adw_css_class_binding_get_flags (AdwCssClassBinding *self);

ADW_AVAILABLE_IN_1_10
GObject *adw_css_class_binding_get_source (AdwCssClassBinding *self);

ADW_AVAILABLE_IN_1_10
GtkWidget *adw_css_class_binding_get_target (AdwCssClassBinding *self);

ADW_AVAILABLE_IN_1_10
const char  *adw_css_class_binding_get_source_property (AdwCssClassBinding *self);

ADW_AVAILABLE_IN_1_10
const char  *adw_css_class_binding_get_target_css_class (AdwCssClassBinding *self);

ADW_AVAILABLE_IN_1_10
void adw_css_class_binding_unbind (AdwCssClassBinding *self);

ADW_AVAILABLE_IN_1_10
AdwCssClassBinding  *adw_bind_property_to_css_class (gpointer       source,
                                                     const char    *source_property,
                                                     GtkWidget     *target,
                                                     const char    *target_css_class,
                                                     GBindingFlags  flags);

ADW_AVAILABLE_IN_1_10
AdwCssClassBinding  *adw_bind_property_to_css_class_full (gpointer                             source,
                                                          const char                          *source_property,
                                                          GtkWidget                           *target,
                                                          const char                          *target_css_class,
                                                          GBindingFlags                        flags,
                                                          AdwCssClassBindingMapToClassFunc     map_to_class,
                                                          AdwCssClassBindingMapToPropertyFunc  map_to_property,
                                                          gpointer                             user_data,
                                                          GDestroyNotify                       notify);

ADW_AVAILABLE_IN_1_10
AdwCssClassBinding * adw_bind_property_to_css_class_with_closures (gpointer       source,
                                                                   const char    *source_property,
                                                                   GtkWidget     *target,
                                                                   const char    *target_css_class,
                                                                   GBindingFlags  flags,
                                                                   GClosure      *map_to_class,
                                                                   GClosure      *map_to_property);
G_END_DECLS
