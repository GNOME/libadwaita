/*
 * Copyright (C) 2021 Nahuel Gomez Castro
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"
#include "adw-application.h"
#include "adw-main-private.h"

/**
 * AdwApplication:
 *
 * A base class for Adwaita applications.
 *
 * `AdwApplication` handles library initialization by calling [func@Adw.init] in
 * the default [signal@Gio.Application::startup] signal handler, in turn
 * chaining up as required by [class@Gtk.Application]. Therefore, any subclass
 * of `AdwApplication` should always chain up its `startup` handler before using
 * any Adwaita or GTK API.
 *
 * ## Automatic Resources
 *
 * `AdwApplication` will automatically load stylesheets located in the
 * application's resource base path (see
 * [method@Gio.Application.set_resource_base_path], if they're present.
 *
 * They can be used to add custom styles to the application, as follows:
 *
 * - `style.css` contains styles that are always present.
 *
 * - `style-dark.css` contains styles only used when
 *   [property@Gtk.Settings:gtk-application-prefer-dark-theme] is set to `TRUE`.
 *
 * - `style-hc.css` contains styles used when the system high contrast
 *   preference is enabled.
 *
 * - `style-hc-dark.css` contains styles used when the system high contrast
 *   preference is enabled and
 *   [property@Gtk.Settings:gtk-application-prefer-dark-theme] is set to `TRUE`.
 *
 * Since: 1.0
 */

typedef struct
{
  GtkStyleProvider *base_style_provider;
  GtkStyleProvider *dark_style_provider;
  GtkStyleProvider *hc_style_provider;
  GtkStyleProvider *hc_dark_style_provider;
} AdwApplicationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (AdwApplication, adw_application, GTK_TYPE_APPLICATION)

static inline void
style_provider_set_enabled (GtkStyleProvider *provider,
                            gboolean          enabled)
{
  if (enabled)
    gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                                provider,
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  else
    gtk_style_context_remove_provider_for_display (gdk_display_get_default (),
                                                   provider);
}

static void
update_stylesheet (AdwApplication *self)
{
  AdwApplicationPrivate *priv = adw_application_get_instance_private (self);
  gboolean is_dark, is_hc, is_hc_dark;
  g_autofree gchar *theme_name;

  g_object_get (gtk_settings_get_default (),
                "gtk-theme-name", &theme_name,
                NULL);

  is_dark = !g_strcmp0 (theme_name, "Adwaita-dark");
  is_hc = !g_strcmp0 (theme_name, "Adwaita-hc");
  is_hc_dark = !g_strcmp0 (theme_name, "Adwaita-hc-dark");

  if (priv->dark_style_provider)
    style_provider_set_enabled (priv->dark_style_provider, is_dark || is_hc_dark);

  if (priv->hc_style_provider)
    style_provider_set_enabled (priv->hc_style_provider, is_hc || is_hc_dark);

  if (priv->hc_dark_style_provider)
    style_provider_set_enabled (priv->hc_dark_style_provider, is_hc_dark);
}

static void
init_provider_from_file (GtkStyleProvider **provider,
                         GFile             *file)
{
  if (!g_file_query_exists (file, NULL)) {
    g_clear_object (&file);
    return;
  }

  *provider = GTK_STYLE_PROVIDER (gtk_css_provider_new ());
  gtk_css_provider_load_from_file (GTK_CSS_PROVIDER (*provider), file);

  g_clear_object (&file);
}

static void
init_providers (AdwApplication *self)
{
  AdwApplicationPrivate *priv = adw_application_get_instance_private (self);

  const char *base_path = NULL;
  g_autofree char *base_uri = NULL;
  g_autoptr (GFile) base_file = NULL;

  base_path = g_application_get_resource_base_path (G_APPLICATION (self));

  if (base_path == NULL)
    return;

  base_uri = g_strconcat ("resource://", base_path, NULL);
  base_file = g_file_new_for_uri (base_uri);

  init_provider_from_file (&priv->base_style_provider,
                           g_file_get_child (base_file, "style.css"));
  init_provider_from_file (&priv->dark_style_provider,
                           g_file_get_child (base_file, "style-dark.css"));
  init_provider_from_file (&priv->hc_style_provider,
                           g_file_get_child (base_file, "style-hc.css"));
  init_provider_from_file (&priv->hc_dark_style_provider,
                           g_file_get_child (base_file, "style-hc-dark.css"));
}

static void
init_styling (AdwApplication *self)
{
  AdwApplicationPrivate *priv = adw_application_get_instance_private (self);

  GdkDisplay *display = gdk_display_get_default ();

  if (display == NULL)
    return;

  if (priv->base_style_provider != NULL)
    gtk_style_context_add_provider_for_display (display,
                                                priv->base_style_provider,
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  /* If gdk_display_get_default() worked, it means that
   * gtk_settings get_default() won't return NULL, so we don't
   * need to check it separately */
  g_signal_connect_object (gtk_settings_get_default (),
                           "notify::gtk-theme-name",
                           G_CALLBACK (update_stylesheet),
                           self,
                           G_CONNECT_SWAPPED);

  update_stylesheet (self);
}

static void
adw_application_startup (GApplication *application)
{
  AdwApplication *self = ADW_APPLICATION (application);

  G_APPLICATION_CLASS (adw_application_parent_class)->startup (application);

  adw_init ();

  init_providers (self);
  init_styling (self);
}

static void
adw_application_dispose (GObject *object)
{
  AdwApplication *self = ADW_APPLICATION (object);
  AdwApplicationPrivate *priv = adw_application_get_instance_private (self);

  g_clear_object (&priv->base_style_provider);
  g_clear_object (&priv->dark_style_provider);
  g_clear_object (&priv->hc_style_provider);
  g_clear_object (&priv->hc_dark_style_provider);

  G_OBJECT_CLASS (adw_application_parent_class)->dispose (object);
}

static void
adw_application_class_init (AdwApplicationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GApplicationClass *application_class = G_APPLICATION_CLASS (klass);

  object_class->dispose = adw_application_dispose;

  application_class->startup = adw_application_startup;
}

static void
adw_application_init (AdwApplication *self)
{
}

/**
 * adw_application_new:
 * @application_id: (nullable): The application ID
 * @flags: The application flags
 *
 * Creates a new `AdwApplication`.
 *
 * If `application_id` is not `NULL`, then it must be valid. See
 * [func@Gio.Application.id_is_valid].
 *
 * If no application ID is given then some features (most notably application
 * uniqueness) will be disabled.
 *
 * Returns: the newly created `AdwApplication`
 *
 * Since: 1.0
 */
AdwApplication *
adw_application_new (const char        *application_id,
                     GApplicationFlags  flags)
{
  return g_object_new (ADW_TYPE_APPLICATION,
                       "application-id", application_id,
                       "flags", flags,
                       NULL);
}
