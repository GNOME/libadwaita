/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-settings-private.h"

#include "adw-macros-private.h"

#include <gio/gio.h>
#include <gtk/gtk.h>

#define PORTAL_BUS_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_SETTINGS_INTERFACE "org.freedesktop.portal.Settings"

#define PORTAL_ERROR_NOT_FOUND "org.freedesktop.portal.Error.NotFound"

struct _AdwSettings
{
  GObject parent_instance;

  GDBusProxy *settings_portal;
  GSettings *interface_settings;
  GSettings *a11y_settings;

  AdwSystemColorScheme color_scheme;
  gboolean high_contrast;

  gboolean has_high_contrast;
  gboolean has_color_scheme;
  gboolean color_scheme_use_fdo_setting;

  gboolean override;
  gboolean system_supports_color_schemes_override;
  AdwSystemColorScheme color_scheme_override;
  gboolean high_contrast_override;
};

G_DEFINE_FINAL_TYPE (AdwSettings, adw_settings, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES,
  PROP_COLOR_SCHEME,
  PROP_HIGH_CONTRAST,
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

/* Settings portal */

#ifndef G_OS_WIN32
static gboolean
get_disable_portal (void)
{
  const char *disable_portal = g_getenv ("ADW_DISABLE_PORTAL");

  return disable_portal && disable_portal[0] == '1';
}

static gboolean
read_portal_setting (AdwSettings  *self,
                     const char   *schema,
                     const char   *name,
                     const char   *type,
                     GVariant    **out)
{
  g_autoptr (GError) error = NULL;
  g_autoptr (GVariant) ret = NULL;
  g_autoptr (GVariant) child = NULL;
  g_autoptr (GVariant) child2 = NULL;
  g_autoptr (GVariantType) out_type = NULL;

  ret = g_dbus_proxy_call_sync (self->settings_portal,
                                "Read",
                                g_variant_new ("(ss)", schema, name),
                                G_DBUS_CALL_FLAGS_NONE,
                                G_MAXINT,
                                NULL,
                                &error);
  if (error) {
    if (error->domain == G_DBUS_ERROR &&
        error->code == G_DBUS_ERROR_SERVICE_UNKNOWN) {
      g_debug ("Portal not found: %s", error->message);

      return FALSE;
    }

    if (error->domain == G_DBUS_ERROR &&
        error->code == G_DBUS_ERROR_UNKNOWN_METHOD) {
      g_debug ("Portal doesn't provide settings: %s", error->message);

      return FALSE;
    }

    if (g_dbus_error_is_remote_error (error)) {
      g_autofree char *remote_error = g_dbus_error_get_remote_error (error);

      if (!g_strcmp0 (remote_error, PORTAL_ERROR_NOT_FOUND)) {
        g_debug ("Setting %s.%s of type %s not found", schema, name, type);

        return FALSE;
      }
    }

    g_critical ("Couldn't read the %s setting: %s", name, error->message);

    return FALSE;
  }

  g_variant_get (ret, "(v)", &child);
  g_variant_get (child, "v", &child2);

  out_type = g_variant_type_new (type);
  if (!g_variant_type_equal (g_variant_get_type (child2), out_type)) {
    g_critical ("Invalid type for %s.%s: expected %s, got %s",
                schema, name, type, g_variant_get_type_string (child2));

    return FALSE;
  }

  *out = g_steal_pointer (&child2);

  return TRUE;
}

static AdwSystemColorScheme
get_fdo_color_scheme (GVariant *variant)
{
  guint32 color_scheme = g_variant_get_uint32 (variant);

  if (color_scheme > ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT) {
    g_warning ("Invalid color scheme: %u", color_scheme);

    color_scheme = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  }

  return color_scheme;
}

static AdwSystemColorScheme
get_gnome_color_scheme (GVariant *variant)
{
  const char *str = g_variant_get_string (variant, NULL);

  if (!g_strcmp0 (str, "default"))
    return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;

  if (!g_strcmp0 (str, "prefer-dark"))
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_DARK;

  if (!g_strcmp0 (str, "prefer-light"))
    return ADW_SYSTEM_COLOR_SCHEME_PREFER_LIGHT;

  g_warning ("Invalid color scheme: %s", str);

  return ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
}

static void
settings_portal_changed_cb (GDBusProxy  *proxy,
                            const char  *sender_name,
                            const char  *signal_name,
                            GVariant    *parameters,
                            AdwSettings *self)
{
  const char *namespace;
  const char *name;
  g_autoptr (GVariant) value = NULL;

  if (g_strcmp0 (signal_name, "SettingChanged"))
    return;

  g_variant_get (parameters, "(&s&sv)", &namespace, &name, &value);

  if (!g_strcmp0 (namespace, "org.freedesktop.appearance") &&
      !g_strcmp0 (name, "color-scheme") &&
      self->color_scheme_use_fdo_setting) {
    set_color_scheme (self, get_fdo_color_scheme (value));

    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.interface") &&
      !g_strcmp0 (name, "color-scheme") &&
      !self->color_scheme_use_fdo_setting) {
    set_color_scheme (self, get_gnome_color_scheme (value));

    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.interface.a11y") &&
      !g_strcmp0 (name, "high-contrast")) {
    set_high_contrast (self, g_variant_get_boolean (value));

    return;
  }
}

static void
init_portal (AdwSettings *self)
{
  g_autoptr (GError) error = NULL;
  g_autoptr (GVariant) color_scheme_variant = NULL;
  g_autoptr (GVariant) high_contrast_variant = NULL;

  if (get_disable_portal ())
    return;

  self->settings_portal = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                                         G_DBUS_PROXY_FLAGS_NONE,
                                                         NULL,
                                                         PORTAL_BUS_NAME,
                                                         PORTAL_OBJECT_PATH,
                                                         PORTAL_SETTINGS_INTERFACE,
                                                         NULL,
                                                         &error);
  if (error) {
    g_debug ("Settings portal not found: %s", error->message);

    return;
  }

  if (read_portal_setting (self, "org.freedesktop.appearance",
                           "color-scheme", "u", &color_scheme_variant)) {
    self->has_color_scheme = TRUE;
    self->color_scheme_use_fdo_setting = TRUE;
    self->color_scheme = get_fdo_color_scheme (color_scheme_variant);
  }

  if (!self->has_color_scheme &&
      read_portal_setting (self, "org.gnome.desktop.interface",
                           "color-scheme", "s", &color_scheme_variant)) {
    self->has_color_scheme = TRUE;
    self->color_scheme = get_gnome_color_scheme (color_scheme_variant);
  }

  if (read_portal_setting (self, "org.gnome.desktop.interface.a11y",
                           "high-contrast", "b", &high_contrast_variant)) {
    self->has_high_contrast = TRUE;
    self->high_contrast = g_variant_get_boolean (high_contrast_variant);
  }

  if (!self->has_color_scheme && !self->has_high_contrast)
    return;

  g_signal_connect (self->settings_portal, "g-signal",
                    G_CALLBACK (settings_portal_changed_cb), self);
}
#endif

/* GSettings */

static gboolean
is_running_in_flatpak (void)
{
  return g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS);
}

static void
gsettings_color_scheme_changed_cb (AdwSettings *self)
{
  set_color_scheme (self, g_settings_get_enum (self->interface_settings, "color-scheme"));
}

static void
gsettings_high_contrast_changed_cb (AdwSettings *self)
{
  set_high_contrast (self, g_settings_get_boolean (self->a11y_settings, "high-contrast"));
}

static void
init_gsettings (AdwSettings *self)
{
  GSettingsSchemaSource *source;
  g_autoptr (GSettingsSchema) schema = NULL;
  g_autoptr (GSettingsSchema) a11y_schema = NULL;

  /* While we can access gsettings in flatpak, we can't do anything useful with
   * them as they aren't propagated from the system. */
  if (is_running_in_flatpak ())
    return;

  source = g_settings_schema_source_get_default ();

  schema = g_settings_schema_source_lookup (source, "org.gnome.desktop.interface", TRUE);
  if (schema &&
      !self->has_color_scheme &&
      g_settings_schema_has_key (schema, "color-scheme")) {
    self->has_color_scheme = TRUE;
    self->interface_settings = g_settings_new ("org.gnome.desktop.interface");
    self->color_scheme = g_settings_get_enum (self->interface_settings, "color-scheme");

    g_signal_connect_swapped (self->interface_settings,
                              "changed::color-scheme",
                              G_CALLBACK (gsettings_color_scheme_changed_cb),
                              self);
  }

  a11y_schema = g_settings_schema_source_lookup (source, "org.gnome.desktop.a11y.interface", TRUE);
  if (a11y_schema &&
      !self->has_high_contrast &&
      g_settings_schema_has_key (a11y_schema, "high-contrast")) {
    self->has_high_contrast = TRUE;
    self->a11y_settings = g_settings_new ("org.gnome.desktop.a11y.interface");
    self->high_contrast = g_settings_get_boolean (self->a11y_settings, "high-contrast");

    g_signal_connect_swapped (self->a11y_settings,
                              "changed::high-contrast",
                              G_CALLBACK (gsettings_high_contrast_changed_cb),
                              self);
  }
}

/* Legacy */

static gboolean
is_theme_high_contrast (GdkDisplay *display)
{
  g_auto (GValue) value = G_VALUE_INIT;
  const char *theme_name;

  g_value_init (&value, G_TYPE_STRING);
  if (!gdk_display_get_setting (display, "gtk-theme-name", &value))
    return FALSE;

  theme_name = g_value_get_string (&value);

  return !g_strcmp0 (theme_name, "HighContrast") ||
         !g_strcmp0 (theme_name, "HighContrastInverse");
}

static void
display_setting_changed_cb (AdwSettings *self,
                            const char  *setting,
                            GdkDisplay  *display)
{
  if (!g_strcmp0 (setting, "gtk-theme-name"))
    set_high_contrast (self, is_theme_high_contrast (display));
}

static void
init_legacy (AdwSettings *self)
{
  GdkDisplay *display = gdk_display_get_default ();

  if (!display)
    return;

  self->has_high_contrast = TRUE;
  self->high_contrast = is_theme_high_contrast (display);

  g_signal_connect_swapped (display,
                            "setting-changed",
                            G_CALLBACK (display_setting_changed_cb),
                            self);
}

static void
adw_settings_constructed (GObject *object)
{
  AdwSettings *self = ADW_SETTINGS (object);

  G_OBJECT_CLASS (adw_settings_parent_class)->constructed (object);

#ifndef G_OS_WIN32
  init_portal (self);
#endif

  if (!self->has_color_scheme || !self->has_high_contrast)
    init_gsettings (self);

  if (!self->has_high_contrast)
    init_legacy (self);
}

static void
adw_settings_dispose (GObject *object)
{
  AdwSettings *self = ADW_SETTINGS (object);

  g_clear_object (&self->settings_portal);
  g_clear_object (&self->interface_settings);
  g_clear_object (&self->a11y_settings);

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
    g_param_spec_boolean ("system-supports-color-schemes",
                          "System supports color schemes",
                          "Whether the system supports color schemes",
                          FALSE,
                          G_PARAM_READABLE);

  props[PROP_COLOR_SCHEME] =
    g_param_spec_enum ("color-scheme",
                       "Color Scheme",
                       "Color Scheme",
                       ADW_TYPE_SYSTEM_COLOR_SCHEME,
                       ADW_SYSTEM_COLOR_SCHEME_DEFAULT,
                       G_PARAM_READABLE);

  props[PROP_HIGH_CONTRAST] =
    g_param_spec_boolean ("high-contrast",
                          "High Contrast",
                          "High Contrast",
                          FALSE,
                          G_PARAM_READABLE);

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

  return self->has_color_scheme;
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

void
adw_settings_start_override (AdwSettings *self)
{
  g_return_if_fail (ADW_IS_SETTINGS (self));

  if (self->override)
    return;

  self->override = TRUE;

  self->system_supports_color_schemes_override = self->has_color_scheme;
  self->color_scheme_override = self->color_scheme;
  self->high_contrast_override = self->high_contrast;
}

void
adw_settings_end_override (AdwSettings *self)
{
  gboolean notify_system_supports_color_scheme, notify_color_scheme, notify_hc;

  g_return_if_fail (ADW_IS_SETTINGS (self));

  if (!self->override)
    return;

  notify_system_supports_color_scheme = self->system_supports_color_schemes_override != self->has_color_scheme;
  notify_color_scheme = self->color_scheme_override != self->color_scheme;
  notify_hc = self->high_contrast_override != self->high_contrast;

  self->override = FALSE;
  self->system_supports_color_schemes_override = FALSE;
  self->color_scheme_override = ADW_SYSTEM_COLOR_SCHEME_DEFAULT;
  self->high_contrast_override = FALSE;

  if (notify_system_supports_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SYSTEM_SUPPORTS_COLOR_SCHEMES]);
  if (notify_color_scheme)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COLOR_SCHEME]);
  if (notify_hc)
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HIGH_CONTRAST]);
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
