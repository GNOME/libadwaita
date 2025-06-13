/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-settings-private.h"

#include "adw-settings-impl-private.h"

#include <gtk/gtk.h>

#define DEFAULT_DOCUMENT_FONT "Sans 10"
#define DEFAULT_MONOSPACE_FONT "Monospace 10"

struct _AdwSettings
{
  GObject parent_instance;

  AdwSettingsImpl *platform_impl;
  AdwSettingsImpl *gsettings_impl;
  AdwSettingsImpl *legacy_impl;

  AdwSystemColorScheme color_scheme;
  gboolean high_contrast;
  gboolean system_supports_color_schemes;
  AdwAccentColor accent_color;
  gboolean system_supports_accent_colors;
  char *document_font_name;
  char *monospace_font_name;

  gboolean override;
  gboolean system_supports_color_schemes_override;
  AdwSystemColorScheme color_scheme_override;
  gboolean high_contrast_override;
  gboolean system_supports_accent_colors_override;
  AdwAccentColor accent_color_override;
};

G_DEFINE_FINAL_TYPE (AdwSettings, adw_settings, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES,
  PROP_COLOR_SCHEME,
  PROP_HIGH_CONTRAST,
  PROP_SYSTEM_SUPPORTS_ACCENT_COLORS,
  PROP_ACCENT_COLOR,
  PROP_DOCUMENT_FONT_NAME,
  PROP_MONOSPACE_FONT_NAME,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

static AdwSettings *default_instance;

static void
set_color_scheme (AdwSettings          *self,
                  AdwSystemColorScheme  color_scheme)
{
  if (color_scheme == self->color_scheme)
    return;

  self->color_scheme = color_scheme;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
}

static void
set_high_contrast (AdwSettings *self,
                   gboolean     high_contrast)
{
  if (high_contrast == self->high_contrast)
    return;
  
  self->high_contrast = high_contrast;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

static void
set_accent_color (AdwSettings    *self,
                  AdwAccentColor  accent_color)
{
  if (accent_color == self->accent_color)
    return;

  self->accent_color = accent_color;

  if (!self->override)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCENT_COLOR]);
}

static void
set_document_font_name (AdwSettings *self,
                        const char  *document_font_name)
{
  if (!g_set_str (&self->document_font_name, document_font_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DOCUMENT_FONT_NAME]);
}

static void
set_monospace_font_name (AdwSettings *self,
                         const char  *monospace_font_name)
{
  if (!g_set_str (&self->monospace_font_name, monospace_font_name))
    return;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_MONOSPACE_FONT_NAME]);
}

static void
init_debug (AdwSettings *self,
            gboolean    *found_color_scheme,
            gboolean    *found_high_contrast,
            gboolean    *found_accent_colors)
{
  const char *env = g_getenv ("ADW_DEBUG_HIGH_CONTRAST");
  if (env && *env) {
    if (!g_strcmp0 (env, "1")) {
      *found_high_contrast = TRUE;
      self->high_contrast = TRUE;
    } else if (!g_strcmp0 (env, "0")) {
      *found_high_contrast = TRUE;
      self->high_contrast = FALSE;
    } else {
      g_warning ("Invalid value for ADW_DEBUG_HIGH_CONTRAST: %s (Expected 0 or 1)", env);
    }
  }

  env = g_getenv ("ADW_DEBUG_COLOR_SCHEME");
  if (env) {
    if (!g_strcmp0 (env, "default")) {
      *found_color_scheme = TRUE;
      self->color_scheme = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
    } else if (!g_strcmp0 (env, "prefer-dark")) {
      *found_color_scheme = TRUE;
      self->color_scheme = ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;
    } else if (!g_strcmp0 (env, "prefer-light")) {
      *found_color_scheme = TRUE;
      self->color_scheme = ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;
    } else {
      g_warning ("Invalid color scheme %s (Expected one of: default, prefer-dark, prefer-light)", env);
    }
  }

  env = g_getenv ("ADW_DEBUG_ACCENT_COLOR");
  if (env) {
    *found_accent_colors = TRUE;
    if (!g_strcmp0 (env, "blue")) {
      self->accent_color = ADW_ACCENT_COLOR_BLUE;
    } else if (!g_strcmp0 (env, "teal")) {
      self->accent_color = ADW_ACCENT_COLOR_TEAL;
    } else if (!g_strcmp0 (env, "green")) {
      self->accent_color = ADW_ACCENT_COLOR_GREEN;
    } else if (!g_strcmp0 (env, "yellow")) {
      self->accent_color = ADW_ACCENT_COLOR_YELLOW;
    } else if (!g_strcmp0 (env, "orange")) {
      self->accent_color = ADW_ACCENT_COLOR_ORANGE;
    } else if (!g_strcmp0 (env, "red")) {
      self->accent_color = ADW_ACCENT_COLOR_RED;
    } else if (!g_strcmp0 (env, "pink")) {
      self->accent_color = ADW_ACCENT_COLOR_PINK;
    } else if (!g_strcmp0 (env, "purple")) {
      self->accent_color = ADW_ACCENT_COLOR_PURPLE;
    } else if (!g_strcmp0 (env, "slate")) {
      self->accent_color = ADW_ACCENT_COLOR_SLATE;
    } else {
      g_warning ("Invalid accent color %s (Expected one of: blue, teal, green,"
                 "yellow, orange, red, pink, purple, slate)", env);
    }
  }
}

static void
register_impl (AdwSettings     *self,
               AdwSettingsImpl *impl,
               gboolean        *found_color_scheme,
               gboolean        *found_high_contrast,
               gboolean        *found_accent_colors,
               gboolean        *found_document_font_name,
               gboolean        *found_monospace_font_name)
{
  if (adw_settings_impl_get_has_color_scheme (impl)) {
    *found_color_scheme = TRUE;

    set_color_scheme (self, adw_settings_impl_get_color_scheme (impl));

    g_signal_connect_swapped (impl, "color-scheme-changed",
                              G_CALLBACK (set_color_scheme), self);
  }

  if (adw_settings_impl_get_has_high_contrast (impl)) {
    *found_high_contrast = TRUE;

    set_high_contrast (self, adw_settings_impl_get_high_contrast (impl));

    g_signal_connect_swapped (impl, "high-contrast-changed",
                              G_CALLBACK (set_high_contrast), self);
  }

  if (adw_settings_impl_get_has_accent_colors (impl)) {
    *found_accent_colors = TRUE;

    set_accent_color (self, adw_settings_impl_get_accent_color (impl));

    g_signal_connect_swapped (impl, "accent-color-changed",
                              G_CALLBACK (set_accent_color), self);
  }

  if (adw_settings_impl_get_has_document_font_name (impl)) {
    *found_document_font_name = TRUE;

    set_document_font_name (self, adw_settings_impl_get_document_font_name (impl));

    g_signal_connect_swapped (impl, "document-font-name-changed",
                              G_CALLBACK (set_document_font_name), self);
  }

  if (adw_settings_impl_get_has_monospace_font_name (impl)) {
    *found_monospace_font_name = TRUE;

    set_monospace_font_name (self, adw_settings_impl_get_monospace_font_name (impl));

    g_signal_connect_swapped (impl, "monospace-font-name-changed",
                              G_CALLBACK (set_monospace_font_name), self);
  }
}

static void
adw_settings_constructed (GObject *object)
{
  AdwSettings *self = ADW_SETTINGS (object);
  gboolean found_color_scheme = FALSE;
  gboolean found_high_contrast = FALSE;
  gboolean found_accent_colors = FALSE;
  gboolean found_document_font_name = FALSE;
  gboolean found_monospace_font_name = FALSE;

  G_OBJECT_CLASS (adw_settings_parent_class)->constructed (object);

  init_debug (self, &found_color_scheme, &found_high_contrast, &found_accent_colors);

#ifdef __APPLE__
  self->platform_impl = adw_settings_impl_macos_new (!found_color_scheme,
                                                     !found_high_contrast,
                                                     !found_accent_colors,
                                                     !found_document_font_name,
                                                     !found_monospace_font_name);
#elif defined(G_OS_WIN32)
  self->platform_impl = adw_settings_impl_win32_new (!found_color_scheme,
                                                     !found_high_contrast,
                                                     !found_accent_colors,
                                                     !found_document_font_name,
                                                     !found_monospace_font_name);
#else
  self->platform_impl = adw_settings_impl_portal_new (!found_color_scheme,
                                                      !found_high_contrast,
                                                      !found_accent_colors,
                                                      !found_document_font_name,
                                                      !found_monospace_font_name);
#endif

  register_impl (self, self->platform_impl, &found_color_scheme,
                 &found_high_contrast, &found_accent_colors,
                 &found_document_font_name, &found_monospace_font_name);

  if (!found_color_scheme ||
      !found_high_contrast ||
      !found_accent_colors ||
      !found_document_font_name ||
      !found_monospace_font_name) {
    self->gsettings_impl = adw_settings_impl_gsettings_new (!found_color_scheme,
                                                            !found_high_contrast,
                                                            !found_accent_colors,
                                                            !found_document_font_name,
                                                            !found_monospace_font_name);
    register_impl (self, self->gsettings_impl, &found_color_scheme,
                   &found_high_contrast, &found_accent_colors,
                   &found_document_font_name, &found_monospace_font_name);
  }

  if (!found_color_scheme || !found_high_contrast || !found_accent_colors) {
    self->legacy_impl = adw_settings_impl_legacy_new (!found_color_scheme,
                                                      !found_high_contrast,
                                                      !found_accent_colors,
                                                      !found_document_font_name,
                                                      !found_monospace_font_name);
    register_impl (self, self->legacy_impl, &found_color_scheme,
                   &found_high_contrast, &found_accent_colors,
                   &found_document_font_name, &found_monospace_font_name);
  }

  self->system_supports_color_schemes = found_color_scheme;
  self->system_supports_accent_colors = found_accent_colors;
}

static void
adw_settings_dispose (GObject *object)
{
  AdwSettings *self = ADW_SETTINGS (object);

  g_clear_object (&self->platform_impl);
  g_clear_object (&self->gsettings_impl);
  g_clear_object (&self->legacy_impl);
  g_clear_pointer (&self->document_font_name, g_free);
  g_clear_pointer (&self->monospace_font_name, g_free);

  G_OBJECT_CLASS (adw_settings_parent_class)->dispose (object);
}

static void
adw_settings_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwSettings *self = ADW_SETTINGS (object);

  switch (prop_id) {
  case PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES:
    g_value_set_boolean (value, adw_settings_get_system_supports_color_schemes (self));
    break;

  case PROP_COLOR_SCHEME:
    g_value_set_enum (value, adw_settings_get_color_scheme (self));
    break;

  case PROP_HIGH_CONTRAST:
    g_value_set_boolean (value, adw_settings_get_high_contrast (self));
    break;

  case PROP_SYSTEM_SUPPORTS_ACCENT_COLORS:
    g_value_set_boolean (value, adw_settings_get_system_supports_accent_colors (self));
    break;

  case PROP_ACCENT_COLOR:
    g_value_set_enum (value, adw_settings_get_accent_color (self));
    break;

  case PROP_DOCUMENT_FONT_NAME:
    g_value_set_string (value, adw_settings_get_document_font_name (self));
    break;

  case PROP_MONOSPACE_FONT_NAME:
    g_value_set_string (value, adw_settings_get_monospace_font_name (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_settings_class_init (AdwSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_settings_constructed;
  object_class->dispose = adw_settings_dispose;
  object_class->get_property = adw_settings_get_property;

  props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES] =
    g_param_spec_boolean ("system-supports-color-schemes", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_COLOR_SCHEME] =
    g_param_spec_enum ("color-scheme", NULL, NULL,
                       ADW_TYPE_SYSTEM_COLOR_SCHEME,
                       ADW_SYSTEM_COLOR_SCHEME_DEFAULT,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_HIGH_CONTRAST] =
    g_param_spec_boolean ("high-contrast", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_SYSTEM_SUPPORTS_ACCENT_COLORS] =
    g_param_spec_boolean ("system-supports-accent-colors", NULL, NULL,
                          FALSE,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_ACCENT_COLOR] =
    g_param_spec_enum ("accent-color", NULL, NULL,
                       ADW_TYPE_ACCENT_COLOR,
                       ADW_ACCENT_COLOR_BLUE,
                       G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_DOCUMENT_FONT_NAME] =
    g_param_spec_string ("document-font-name", NULL, NULL,
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  props[PROP_MONOSPACE_FONT_NAME] =
    g_param_spec_string ("monospace-font-name", NULL, NULL,
                         NULL,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
adw_settings_init (AdwSettings *self)
{
}

AdwSettings *
adw_settings_get_default (void)
{
  if (!default_instance)
    default_instance = g_object_new (ADW_TYPE_SETTINGS, NULL);

  return default_instance;
}

gboolean
adw_settings_get_system_supports_color_schemes (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->system_supports_color_schemes_override;

  return self->system_supports_color_schemes;
}

AdwSystemColorScheme
adw_settings_get_color_scheme (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), ADW_SYSTEM_COLOR_SCHEME_DEFAULT);

  if (self->override)
    return self->color_scheme_override;

  return self->color_scheme;
}

gboolean
adw_settings_get_high_contrast (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->high_contrast_override;

  return self->high_contrast;
}

gboolean
adw_settings_get_system_supports_accent_colors (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), FALSE);

  if (self->override)
    return self->system_supports_accent_colors_override;

  return self->system_supports_accent_colors;
}

AdwAccentColor
adw_settings_get_accent_color (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), ADW_ACCENT_COLOR_BLUE);

  if (self->override)
    return self->accent_color_override;

  return self->accent_color;
}

const char *
adw_settings_get_document_font_name (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), NULL);

  return self->document_font_name;
}

const char *
adw_settings_get_monospace_font_name (AdwSettings *self)
{
  g_return_val_if_fail (ADW_IS_SETTINGS (self), NULL);

  return self->monospace_font_name;
}

void
adw_settings_start_override (AdwSettings *self)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));

  if (self->override)
    return;

  self->override = TRUE;

  self->system_supports_color_schemes_override = self->system_supports_color_schemes;
  self->color_scheme_override = self->color_scheme;
  self->high_contrast_override = self->high_contrast;
  self->system_supports_accent_colors_override = self->system_supports_accent_colors;
  self->accent_color_override = self->accent_color;
}

void
adw_settings_end_override (AdwSettings *self)
{
  gboolean notify_system_supports_color_scheme, notify_color_scheme, notify_hc,
           notify_system_supports_accent_colors, notify_accent_color;

  g_return_if_fail (ADW_IS_SETTINGS (self));

  if (!self->override)
    return;

  notify_system_supports_color_scheme = self->system_supports_color_schemes_override != self->system_supports_color_schemes;
  notify_color_scheme = self->color_scheme_override != self->color_scheme;
  notify_hc = self->high_contrast_override != self->high_contrast;
  notify_system_supports_accent_colors = self->system_supports_accent_colors_override != self->system_supports_accent_colors;
  notify_accent_color= self->accent_color_override != self->accent_color;

  self->override = FALSE;
  self->system_supports_color_schemes_override = FALSE;
  self->color_scheme_override = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  self->high_contrast_override = FALSE;
  self->system_supports_accent_colors_override = FALSE;
  self->accent_color_override = ADW_ACCENT_COLOR_BLUE;

  if (notify_system_supports_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
  if (notify_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
  if (notify_hc)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
  if (notify_system_supports_accent_colors)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_ACCENT_COLORS]);
  if (notify_accent_color)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCENT_COLOR]);
}

void
adw_settings_override_system_supports_color_schemes (AdwSettings *self,
                                                     gboolean     system_supports_color_schemes)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  system_supports_color_schemes = !!system_supports_color_schemes;

  if (system_supports_color_schemes == self->system_supports_color_schemes_override)
    return;

  if (!system_supports_color_schemes)
    adw_settings_override_color_scheme (self, ADW_SYSTEM_COLOR_SCHEME_DEFAULT);

  self->system_supports_color_schemes_override = system_supports_color_schemes;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
}

void
adw_settings_override_color_scheme (AdwSettings          *self,
                                    AdwSystemColorScheme  color_scheme)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  if (color_scheme == self->color_scheme_override ||
      !self->system_supports_color_schemes_override)
    return;

  self->color_scheme_override = color_scheme;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
}

void
adw_settings_override_high_contrast (AdwSettings *self,
                                     gboolean     high_contrast)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  high_contrast = !!high_contrast;

  if (high_contrast == self->high_contrast_override)
    return;

  self->high_contrast_override = high_contrast;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
}

void
adw_settings_override_system_supports_accent_colors (AdwSettings *self,
                                                     gboolean     system_supports_accent_colors)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  system_supports_accent_colors = !!system_supports_accent_colors;

  if (system_supports_accent_colors == self->system_supports_accent_colors_override)
    return;

  if (!system_supports_accent_colors)
    adw_settings_override_accent_color (self, ADW_ACCENT_COLOR_BLUE);

  self->system_supports_accent_colors_override = system_supports_accent_colors;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_ACCENT_COLORS]);
}

void
adw_settings_override_accent_color (AdwSettings    *self,
                                    AdwAccentColor  accent_color)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));
  g_return_if_fail (self->override);

  if (accent_color == self->accent_color_override ||
      !self->system_supports_accent_colors_override)
    return;

  self->accent_color_override = accent_color;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACCENT_COLOR]);
}
