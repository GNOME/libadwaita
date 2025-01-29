/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include <gio/gio.h>

struct _AdwSettingsImplGSettings
{
  AdwSettingsImpl parent_instance;

  GSettings *interface_settings;
  GSettings *a11y_settings;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplGSettings, adw_settings_impl_gsettings, ADW_TYPE_SETTINGS_IMPL)

static gboolean
is_running_in_flatpak (void)
{
#ifndef G_OS_WIN32
  return g_file_test ("/.flatpak-info", G_FILE_TEST_EXISTS);
#else
  return FALSE;
#endif
}

static void
color_scheme_changed_cb (AdwSettingsImplGSettings *self)
{
  AdwSystemColorScheme color_scheme =
    g_settings_get_enum (self->interface_settings, "color-scheme");

  adw_settings_impl_set_color_scheme (ADW_SETTINGS_IMPL (self), color_scheme);
}

static void
high_contrast_changed_cb (AdwSettingsImplGSettings *self)
{
  gboolean high_contrast =
    g_settings_get_boolean (self->a11y_settings, "high-contrast");

  adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self), high_contrast);
}

static void
accent_color_changed_cb (AdwSettingsImplGSettings *self)
{
  AdwAccentColor accent_color =
    g_settings_get_enum (self->interface_settings, "accent-color");

  adw_settings_impl_set_accent_color (ADW_SETTINGS_IMPL (self), accent_color);
}

static void
document_font_name_changed_cb (AdwSettingsImplGSettings *self)
{
  char *font_name = g_settings_get_string (self->interface_settings, "document-font-name");

  adw_settings_impl_set_document_font_name (ADW_SETTINGS_IMPL (self), font_name);

  g_free (font_name);
}

static void
monospace_font_name_changed_cb (AdwSettingsImplGSettings *self)
{
  char *font_name = g_settings_get_string (self->interface_settings, "monospace-font-name");

  adw_settings_impl_set_monospace_font_name (ADW_SETTINGS_IMPL (self), font_name);

  g_free (font_name);
}

static void
adw_settings_impl_gsettings_dispose (GObject *object)
{
  AdwSettingsImplGSettings *self = ADW_SETTINGS_IMPL_GSETTINGS (object);

  g_clear_object (&self->interface_settings);
  g_clear_object (&self->a11y_settings);

  G_OBJECT_CLASS (adw_settings_impl_gsettings_parent_class)->dispose (object);
}

static void
adw_settings_impl_gsettings_class_init (AdwSettingsImplGSettingsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = adw_settings_impl_gsettings_dispose;
}

static void
adw_settings_impl_gsettings_init (AdwSettingsImplGSettings *self)
{
}

AdwSettingsImpl *
adw_settings_impl_gsettings_new (gboolean enable_color_scheme,
                                 gboolean enable_high_contrast,
                                 gboolean enable_accent_colors,
                                 gboolean enable_document_font_name,
                                 gboolean enable_monospace_font_name)
{
  AdwSettingsImplGSettings *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_GSETTINGS, NULL);
  GSettingsSchemaSource *source;
  gboolean found_color_scheme = FALSE;
  gboolean found_high_contrast = FALSE;
  gboolean found_accent_colors = FALSE;
  gboolean found_document_font_name = FALSE;
  gboolean found_monospace_font_name = FALSE;

  /* While we can access gsettings in flatpak, we can't do anything useful with
   * them as they aren't propagated from the system. */
  if (is_running_in_flatpak ())
    return ADW_SETTINGS_IMPL (self);

  source = g_settings_schema_source_get_default ();

  if (enable_color_scheme ||
      enable_accent_colors ||
      enable_document_font_name ||
      enable_monospace_font_name) {
    GSettingsSchema *schema =
      g_settings_schema_source_lookup (source, "org.gnome.desktop.interface", TRUE);

    if (schema) {
      self->interface_settings = g_settings_new ("org.gnome.desktop.interface");

      if (enable_color_scheme &&
          adw_get_disable_portal () &&
          g_settings_schema_has_key (schema, "color-scheme")) {
        found_color_scheme = TRUE;

        color_scheme_changed_cb (self);

        g_signal_connect_swapped (self->interface_settings,
                                  "changed::color-scheme",
                                  G_CALLBACK (color_scheme_changed_cb),
                                  self);
      }

      if (enable_accent_colors &&
          adw_get_disable_portal () &&
          g_settings_schema_has_key (schema, "accent-color")) {
        found_accent_colors = TRUE;

        accent_color_changed_cb (self);

        g_signal_connect_swapped (self->interface_settings,
                                  "changed::accent-color",
                                  G_CALLBACK (accent_color_changed_cb),
                                  self);
      }

      if (enable_document_font_name &&
          g_settings_schema_has_key (schema, "document-font-name")) {
        found_document_font_name = TRUE;

        document_font_name_changed_cb (self);

        g_signal_connect_swapped (self->interface_settings,
                                  "changed::document-font-name",
                                  G_CALLBACK (document_font_name_changed_cb),
                                  self);
      }

      if (enable_monospace_font_name &&
          g_settings_schema_has_key (schema, "monospace-font-name")) {
        found_monospace_font_name = TRUE;

        monospace_font_name_changed_cb (self);

        g_signal_connect_swapped (self->interface_settings,
                                  "changed::monospace-font-name",
                                  G_CALLBACK (monospace_font_name_changed_cb),
                                  self);
      }
    }

    g_clear_pointer (&schema, g_settings_schema_unref);
  }

  if (enable_high_contrast) {
    GSettingsSchema *schema =
      g_settings_schema_source_lookup (source, "org.gnome.desktop.a11y.interface", TRUE);

    if (schema && g_settings_schema_has_key (schema, "high-contrast")) {
      found_high_contrast = TRUE;
      self->a11y_settings = g_settings_new ("org.gnome.desktop.a11y.interface");

      high_contrast_changed_cb (self);

      g_signal_connect_swapped (self->a11y_settings,
                                "changed::high-contrast",
                                G_CALLBACK (high_contrast_changed_cb),
                                self);
    }

    g_clear_pointer (&schema, g_settings_schema_unref);
  }

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  found_color_scheme,
                                  found_high_contrast,
                                  found_accent_colors,
                                  found_document_font_name,
                                  found_monospace_font_name);

  return ADW_SETTINGS_IMPL (self);
}
