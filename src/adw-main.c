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

/**
 * SECTION:adw-main
 * @short_description: Library initialization.
 * @Title: adw-main
 *
 * Before using the Adwaita library you should initialize it by calling the
 * adw_init() function.
 * This makes sure translations, types, themes, and icons for the Adwaita
 * library are set up properly.
 *
 * Since: 1.0
 */

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
update_theme (void)
{
  GtkSettings *settings = gtk_settings_get_default ();
  g_autofree char *new_theme_name = NULL;
  gboolean prefer_dark_theme;
  const char *variant;

  g_object_get (settings,
                "gtk-application-prefer-dark-theme", &prefer_dark_theme,
                NULL);

  if (is_high_contrast ())
    variant = prefer_dark_theme ? "hc-dark" : "hc";
  else
    variant = prefer_dark_theme ? "dark" : "light";

  new_theme_name = g_strdup_printf ("Adwaita-%s", variant);

  g_object_set (settings, "gtk-theme-name", new_theme_name, NULL);
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

  if (!g_once_init_enter (&guard))
    return;

  settings = gtk_settings_get_default ();

  if (!settings) {
    g_once_init_leave (&guard, 1);

    return;
  }

  g_signal_connect (settings,
                    "notify::gtk-application-prefer-dark-theme",
                    G_CALLBACK (update_theme),
                    NULL);

  /* If gtk_settings_get_default() has worked, GdkDisplay
   * exists, so we don't need to check that separately. */
  g_signal_connect (gdk_display_get_default (),
                    "setting-changed",
                    G_CALLBACK (setting_changed_cb),
                    NULL);

  update_theme ();

  g_object_set (settings,
                "gtk-icon-theme-name", "hicolor",
                NULL);

  g_once_init_leave (&guard, 1);
}

/**
 * adw_icons_init:
 *
 * Initializes the embedded icons. This must be called once GTK has been
 * initialized.
 *
 * Since: 1.0
 */
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
 * Call this function just after initializing GTK, if you are using
 * #GtkApplication it means it must be called when the #GApplication::startup
 * signal is emitted. If libadwaita has already been initialized, the function
 * will simply return.
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
