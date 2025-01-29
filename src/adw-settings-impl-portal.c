/*
 * Copyright (C) 2021 Purism SPC
 * Copyright (C) 2024 GNOME Foundation, Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
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

  gboolean found_color_scheme;

  enum {
    HIGH_CONTRAST_STATE_NONE = 0,
    HIGH_CONTRAST_STATE_FDO,
    HIGH_CONTRAST_STATE_GNOME,
  } high_contrast_portal_state;

  gboolean found_accent_colors;

  gboolean found_document_font_name;
  gboolean found_monospace_font_name;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplPortal, adw_settings_impl_portal, ADW_TYPE_SETTINGS_IMPL)

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

static AdwAccentColor
get_fdo_accent_color (GVariant *variant)
{
  double r = -1, g = -1, b = -1;
  GdkRGBA rgba;

  g_variant_get (variant, "(ddd)", &r, &g, &b);

  if (r < 0 || g < 0 || b < 0 || r > 1 || g > 1 || b > 1)
    return ADW_ACCENT_COLOR_BLUE;

  rgba.red = r;
  rgba.green = g;
  rgba.blue = b;
  rgba.alpha = 1.0;

  return adw_accent_color_nearest_from_rgba (&rgba);
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

  if (!g_strcmp0 (namespace, "org.freedesktop.appearance")) {
    if (!g_strcmp0 (name, "color-scheme") && self->found_color_scheme) {
      adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                          get_fdo_color_scheme (value));

      g_variant_unref (value);

      return;
    }

    if (!g_strcmp0 (name, "contrast") &&
        self->high_contrast_portal_state == HIGH_CONTRAST_STATE_FDO) {
      adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                           g_variant_get_uint32 (value) == 1);

      g_variant_unref (value);

      return;
    }

    if (!g_strcmp0 (name, "accent-color") && self->found_accent_colors) {
      adw_settings_impl_set_accent_color (ADW_SETTINGS_IMPL (self),
                                          get_fdo_accent_color (value));

      g_variant_unref (value);

      return;
    }
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.a11y.interface") &&
      !g_strcmp0 (name, "high-contrast") &&
      self->high_contrast_portal_state == HIGH_CONTRAST_STATE_GNOME) {
    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                         g_variant_get_boolean (value));

    g_variant_unref (value);
    return;
  }

  if (!g_strcmp0 (namespace, "org.gnome.desktop.interface")) {
    if (!g_strcmp0 (name, "document-font-name")) {
      adw_settings_impl_set_document_font_name (ADW_SETTINGS_IMPL (self),
                                                g_variant_get_string (value, NULL));
      g_variant_unref (value);
      return;
    }

    if (!g_strcmp0 (name, "monospace-font-name")) {
      adw_settings_impl_set_monospace_font_name (ADW_SETTINGS_IMPL (self),
                                                 g_variant_get_string (value, NULL));
      g_variant_unref (value);
      return;
    }
  }

  g_variant_unref (value);
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
}

static gboolean
is_running_in_flatpak (void)
{
#ifndef G_OS_WIN32
  return g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS);
#else
  return FALSE;
#endif
}

AdwSettingsImpl *
adw_settings_impl_portal_new (gboolean enable_color_scheme,
                              gboolean enable_high_contrast,
                              gboolean enable_accent_colors,
                              gboolean enable_document_font_name,
                              gboolean enable_monospace_font_name)
{
  AdwSettingsImplPortal *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_PORTAL, NULL);
  GError *error = NULL;
  GVariant *variant;

  if (adw_get_disable_portal ())
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

  if (enable_color_scheme &&
      read_setting (self, "org.freedesktop.appearance",
                    "color-scheme", "u", &variant)) {
    self->found_color_scheme = TRUE;

    adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self),
                                        get_fdo_color_scheme (variant));

    g_variant_unref (variant);
  }

  if (enable_high_contrast) {
    if (read_setting (self, "org.freedesktop.appearance",
                      "contrast", "u", &variant)) {
      self->high_contrast_portal_state = HIGH_CONTRAST_STATE_FDO;

      adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                           g_variant_get_uint32 (variant) == 1);

      g_variant_unref (variant);
    } else if (read_setting (self, "org.gnome.desktop.a11y.interface",
               "high-contrast", "b", &variant)) {
      self->high_contrast_portal_state = HIGH_CONTRAST_STATE_GNOME;

      adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                           g_variant_get_boolean (variant));
      g_variant_unref (variant);
    }
  }

  if (enable_accent_colors &&
      read_setting (self, "org.freedesktop.appearance",
                    "accent-color", "(ddd)", &variant)) {
    self->found_accent_colors = TRUE;

    adw_settings_impl_set_accent_color (ADW_SETTINGS_IMPL (self),
                                        get_fdo_accent_color (variant));

    g_variant_unref (variant);
  }

  if (is_running_in_flatpak ()) {
    if (enable_document_font_name &&
        read_setting (self, "org.gnome.desktop.interface",
                      "document-font-name", "s", &variant)) {
      self->found_document_font_name = TRUE;

      adw_settings_impl_set_document_font_name (ADW_SETTINGS_IMPL (self),
                                                g_variant_get_string (variant, NULL));

      g_variant_unref (variant);
    }

    if (enable_monospace_font_name &&
        read_setting (self, "org.gnome.desktop.interface",
                      "monospace-font-name", "s", &variant)) {
      self->found_monospace_font_name = TRUE;

      adw_settings_impl_set_monospace_font_name (ADW_SETTINGS_IMPL (self),
                                                 g_variant_get_string (variant, NULL));

      g_variant_unref (variant);
    }
  }

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  self->found_color_scheme,
                                  self->high_contrast_portal_state != HIGH_CONTRAST_STATE_NONE,
                                  self->found_accent_colors,
                                  self->found_document_font_name,
                                  self->found_monospace_font_name);

  if (self->found_color_scheme ||
      self->high_contrast_portal_state != HIGH_CONTRAST_STATE_NONE ||
      self->found_accent_colors ||
      self->found_document_font_name ||
      self->found_monospace_font_name) {
    g_signal_connect (self->settings_portal, "g-signal",
                      G_CALLBACK (changed_cb), self);
  }

  return ADW_SETTINGS_IMPL (self);
}
