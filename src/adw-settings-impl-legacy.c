/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-settings-impl-private.h"

#include <gtk/gtk.h>

struct _AdwSettingsImplLegacy
{
  AdwSettingsImpl parent_instance;
};

G_DEFINE_FINAL_TYPE (AdwSettingsImplLegacy, adw_settings_impl_legacy, ADW_TYPE_SETTINGS_IMPL)

static gboolean
is_theme_high_contrast (GdkDisplay *display)
{
  GValue value = G_VALUE_INIT;
  const char *theme_name;
  gboolean ret;

  g_value_init (&value, G_TYPE_STRING);
  if (!gdk_display_get_setting (display, "gtk-theme-name", &value))
    return FALSE;

  theme_name = g_value_get_string (&value);

  ret = !g_strcmp0 (theme_name, "HighContrast") ||
        !g_strcmp0 (theme_name, "HighContrastInverse");

  g_value_unset (&value);

  return ret;
}

static void
display_setting_changed_cb (AdwSettingsImplLegacy *self,
                            const char            *setting,
                            GdkDisplay            *display)
{
  if (!g_strcmp0 (setting, "gtk-theme-name"))
    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                         is_theme_high_contrast (display));
}

static void
adw_settings_impl_legacy_class_init (AdwSettingsImplLegacyClass *klass)
{
}

static void
adw_settings_impl_legacy_init (AdwSettingsImplLegacy *self)
{
}

AdwSettingsImpl *
adw_settings_impl_legacy_new (gboolean enable_color_scheme,
                              gboolean enable_high_contrast,
                              gboolean enable_accent_colors,
                              gboolean enable_document_font_name,
                              gboolean enable_monospace_font_name)
{
  AdwSettingsImplLegacy *self = g_object_new (ADW_TYPE_SETTINGS_IMPL_LEGACY, NULL);

  if (enable_high_contrast) {
    GdkDisplay *display = gdk_display_get_default ();

    g_signal_connect_swapped (display,
                              "setting-changed",
                              G_CALLBACK (display_setting_changed_cb),
                              self);

    adw_settings_impl_set_high_contrast (ADW_SETTINGS_IMPL (self),
                                         is_theme_high_contrast (display));
  }

  adw_settings_impl_set_features (ADW_SETTINGS_IMPL (self),
                                  /* has_color_scheme */ FALSE,
                                  enable_high_contrast,
                                  /* has_accent_colors */ FALSE,
                                  /* has_document_font_name */ FALSE,
                                  /* has_monospace_font_name */ FALSE);

  return ADW_SETTINGS_IMPL (self);
}
