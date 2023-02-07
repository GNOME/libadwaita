/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include <gio/gio.h>

#define PORTAL_BUS_NAME "org.freedesktop.portal.Desktop"
#define PORTAL_OBJECT_PATH "/org/freedesktop/portal/desktop"
#define PORTAL_SETTINGS_INTERFACE "org.freedesktop.portal.Settings"

#define PORTAL_ERROR_NOT_FOUND "org.freedesktop.portal.Error.NotFound"

struct _AdwSettingsImplPortal
{
  AdwSettingsImpl parent_instance;

  GDBusProxy *settings_portal;

  enum {
    COLOR_SCHEME_STATE_FDO,
    COLOR_SCHEME_STATE_GNOME,
    COLOR_SCHEME_STATE_NONE
  } color_scheme_portal_state;

  enum {
    HIGH_CONTRAST_STATE_GNOME,
    HIGH_CONTRAST_STATE_NONE
  } high_contrast_portal_state;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplPortal, adw_settings_impl_portal, ADW_TYPE_SETTINGS_IMPL)

static gboolean
get_disable_portal (void)
{
  const char *disable_portal = g_getenv ("ADW_DISABLE_PORTAL");

  return disable_portal && disable_portal[0] == '1';
}

static gboolean
read_setting (AdwSettingsImplPortal  *self,
              const char             *schema,
              const char             *name,
              const char             *type,
              GVariant              **out)
{
  GError *error = NULL;
  GVariant *ret;
  GVariant *child, *child2;
  GVariantType *out_type;
  gboolean result = FALSE;

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
    } else if (error->domain == G_DBUS_ERROR &&
               error->code == G_DBUS_ERROR_UNKNOWN_METHOD) {
      g_debug ("Portal doesn't provide settings: %s", error->message);
    } else if (g_dbus_error_is_remote_error (error)) {
      char *remote_error = g_dbus_error_get_remote_error (error);

      if (!g_strcmp0 (remote_error, PORTAL_ERROR_NOT_FOUND)) {
        g_debug ("Setting %s.%s of type %s not found", schema, name, type);
      }
      g_free (remote_error);
    } else {
      g_critical ("Couldn't read the %s setting: %s", name, error->message);
    }

    g_clear_error (&error);

    return FALSE;
  }

  g_variant_get (ret, "(v)", &child);
  g_variant_get (child, "v", &child2);

  out_type = g_variant_type_new (type);
  if (g_variant_type_equal (g_variant_get_type (child2), out_type)) {
    *out = child2;

    result = TRUE;
  } else {
    g_critical ("Invalid type for %s.%s: expected %s, got %s",
                schema, name, type, g_variant_get_type_string (child2));

    g_variant_unref (child2);
  }

  g_variant_type_free (out_type);
  g_variant_unref (child);
  g_variant_unref (ret);
  g_clear_error (&error);

  return result;
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
changed_cb (GDBusProxy            *proxy,
            const char            *sender_name,
            const char            *signal_name,
            GVariant              *parameters,
            AdwSettingsImplPortal *self)
{
  const char *namespace;
  const char *name;
  GVariant *value = NULL;

  if (g_strcmp0 (signal_name, "SettingChanged"))
    return;

  g_variant_get (parameters, "(&s&sv)", &namespace, &name, &value);

  if (!g_strcmp0 (namespace, "org.freedesktop.appearance") &&
      !g_strcmp0 (name, "color-scheme") &&
      self->color_scheme_portal_state == COLOR_SCHEME_STATE_FDO) {
    adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                        get_fdo_color_scheme (value));

    g_variant_unref (value);

    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.interface") &&
      !g_strcmp0 (name, "color-scheme") &&
      self->color_scheme_portal_state == COLOR_SCHEME_STATE_GNOME) {
    adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                        get_gnome_color_scheme (value));

    g_variant_unref (value);

    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.a11y.interface") &&
      !g_strcmp0 (name, "high-contrast") &&
      self->high_contrast_portal_state == HIGH_CONTRAST_STATE_GNOME) {
    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                         g_variant_get_boolean (value));

    g_variant_unref (value);

    return;
  }
}

static void
adw_settings_impl_portal_dispose (GObject *object)
{
  AdwSettingsImplPortal *self = ADW_SETTINGS_IMPL_PORTAL (object);

  g_clear_object (&self->settings_portal);

  G_OBJECT_CLASS (adw_settings_impl_portal_parent_class)->dispose (object);
}

static void
adw_settings_impl_portal_class_init (AdwSettingsImplPortalClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_settings_impl_portal_dispose;
}

static void
adw_settings_impl_portal_init (AdwSettingsImplPortal *self)
{
  self->high_contrast_portal_state = HIGH_CONTRAST_STATE_NONE;
  self->color_scheme_portal_state = COLOR_SCHEME_STATE_NONE;
}

AdwSettingsImpl *
adw_settings_impl_portal_new (gboolean has_color_scheme,
                              gboolean has_high_contrast)
{
  AdwSettingsImplPortal *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_PORTAL, NULL);
  GError *error = NULL;
  GVariant *variant;

  if (get_disable_portal ())
    return ADW_SETTINGS_IMPL (self);

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

    g_error_free (error);

    return ADW_SETTINGS_IMPL (self);
  }

  if (!has_color_scheme &&
      read_setting (self, "org.freedesktop.appearance",
                    "color-scheme", "u", &variant)) {
    has_color_scheme = TRUE;
    self->color_scheme_portal_state = COLOR_SCHEME_STATE_FDO;

    adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                        get_fdo_color_scheme (variant));

    g_variant_unref (variant);
  }

  if (!has_color_scheme &&
      read_setting (self, "org.gnome.desktop.interface",
                    "color-scheme", "s", &variant)) {
    has_color_scheme = TRUE;
    self->color_scheme_portal_state = COLOR_SCHEME_STATE_GNOME;

    adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                        get_gnome_color_scheme (variant));

    g_variant_unref (variant);
  }

  if (!has_high_contrast &&
      read_setting (self, "org.gnome.desktop.a11y.interface",
                    "high-contrast", "b", &variant)) {
    has_high_contrast = TRUE;
    self->high_contrast_portal_state = HIGH_CONTRAST_STATE_GNOME;

    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                         g_variant_get_boolean (variant));

    g_variant_unref (variant);
  }

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  has_color_scheme,
                                  has_high_contrast);

  if (has_color_scheme || has_high_contrast)
    g_signal_connect (self->settings_portal, "g-signal",
                      G_CALLBACK (changed_cb), self);

  return ADW_SETTINGS_IMPL (self);
}