/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "config.h"
#include "adw-main-private.h"
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

static int adw_initialized = FALSE;
static GtkCssProvider *provider;

static gboolean
is_high_contrast (void)
{
  GdkDisplay *display = gdk_display_get_default ();
  g_auto (GValue) value = G_VALUE_INIT;
  const char *theme_name;

  g_value_init (&value, G_TYPE_STRING);
  if (!gdk_display_get_setting (display, "gtk-theme-name", &value))
    return FALSE;

  theme_name = g_value_get_string (&value);

  /* TODO: HighContrast shouldn't be a theme */
  return !g_strcmp0 (theme_name, "HighContrast") ||
         !g_strcmp0 (theme_name, "HighContrastInverse");
}

static void
load_theme (gboolean dark,
            gboolean high_contrast)
{
  g_autofree char *path = NULL;
  const char *variant;

  if (high_contrast)
    variant = dark ? "hc-dark" : "hc";
  else
    variant = dark ? "dark" : "light";

  path = g_strdup_printf ("/org/gtk/libgtk/theme/Adwaita/Adwaita-%s.css", variant);

  gtk_css_provider_load_from_resource (provider, path);
}

static void
update_theme (void)
{
  GtkSettings *settings = gtk_settings_get_default ();
  gboolean prefer_dark_theme;

  g_object_get (settings,
                "gtk-application-prefer-dark-theme", &prefer_dark_theme,
                NULL);

  // Empty doesn't have a gtk-dark.css so we'll make our own instead
  g_object_set (settings, "gtk-theme-name", "Adwaita", NULL);

  load_theme (prefer_dark_theme, is_high_contrast ());
}

static void
setting_changed_cb (GdkDisplay *display,
                    const char *setting)
{
  if (!g_strcmp0 (setting, "gtk-theme-name"))
    update_theme ();
}

static void
adw_style_init (void)
{
  static gsize guard = 0;
  GtkSettings *settings;
  GdkDisplay *display;

  if (!g_once_init_enter (&guard))
    return;

  settings = gtk_settings_get_default ();

  if (!settings) {
    g_once_init_leave (&guard, 1);

    return;
  }

  display = gdk_display_get_default ();

  provider = gtk_css_provider_new ();
  gtk_style_context_add_provider_for_display (display,
                                              GTK_STYLE_PROVIDER (provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_THEME);

  g_signal_connect (settings,
                    "notify::gtk-application-prefer-dark-theme",
                    G_CALLBACK (update_theme),
                    NULL);

  /* If gtk_settings_get_default() has worked, GdkDisplay
   * exists, so we don't need to check that separately. */
  g_signal_connect (display,
                    "setting-changed",
                    G_CALLBACK (setting_changed_cb),
                    NULL);

  update_theme ();

  g_once_init_leave (&guard, 1);
}

static void
adw_icons_init (void)
{
  static gsize guard = 0;

  if (!g_once_init_enter (&guard))
    return;

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gdk_display_get_default ()),
                                    "/org/gnome/Adwaita/icons");

  g_once_init_leave (&guard, 1);
}

/**
 * adw_init:
 *
 * Initializes Libadwaita.
 *
 * Call this function just after initializing GTK, if you are using
 * [class@Gtk.Application] it means it must be called when the
 * `GApplication::startup` signal is emitted.
 *
 * If Libadwaita has already been initialized, the function will simply return.
 *
 * This makes sure translations, types, themes, and icons for the Adwaita
 * library are set up properly.
 *
 * Since: 1.0
 */
void
adw_init (void)
{
  if (adw_initialized)
    return;

  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  adw_init_public_types ();

  adw_style_init ();
  adw_icons_init ();

  adw_initialized = TRUE;
}
