/*
 * Copyright (C) 2018-2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "config.h"
#include "hdy-main-private.h"
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include "gconstructorprivate.h"

/**
 * PRIVATE:hdy-main
 * @short_description: Library initialization.
 * @Title: hdy-main
 * @stability: Private
 *
 * Before using the Handy libarary you should initialize it. This makes
 * sure translations for the Handy library are set up properly.
 */

#if defined (G_HAS_CONSTRUCTORS)

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(hdy_constructor)
#endif
G_DEFINE_CONSTRUCTOR(hdy_constructor)

/* A stupidly high priority used to load the styles before anything else
 * happens.
 *
 * See https://source.puri.sm/Librem5/libhandy/issues/214.
 */
#define HDY_PRIORITY_STYLE -1000

/* The style provider priority to use for libhandy widgets custom styling. It is
 * higher than themes and settings, allowing to override theme defaults, but
 * lower than applications and user provided styles, so application developers
 * can nonetheless apply custom styling on top of it. */
#define HDY_STYLE_PROVIDER_PRIORITY_OVERRIDE (GTK_STYLE_PROVIDER_PRIORITY_SETTINGS + 1)

/**
 * hdy_style_init:
 *
 * Initializes the style classes. This must be called once GTK has been
 * initialized.
 *
 * Since: 1.0
 */
static void
hdy_style_init (void)
{
  static volatile gsize guard = 0;
  g_autoptr (GtkCssProvider) css_override_provider = NULL;
  g_autoptr (GtkCssProvider) css_fallback_provider = NULL;

  if (!g_once_init_enter (&guard))
    return;

  css_fallback_provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (css_fallback_provider, "/sm/puri/handy/themes/shared.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (css_fallback_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);

  css_override_provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (css_override_provider, "/sm/puri/handy/themes/Adwaita.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (css_override_provider),
                                             HDY_STYLE_PROVIDER_PRIORITY_OVERRIDE);

  g_once_init_leave (&guard, 1);
}

/**
 * hdy_icons_init:
 *
 * Initializes the embedded icons. This must be called once GTK has been
 * initialized.
 *
 * Since: 1.0
 */
static void
hdy_icons_init (void)
{
  static volatile gsize guard = 0;

  if (!g_once_init_enter (&guard))
    return;

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (),
                                    "/sm/puri/handy/icons");

  g_once_init_leave (&guard, 1);
}

        /* var theme = Gtk.IconTheme.get_default (); */
        /* theme.add_resource_path ("/org/gnome/clocks/icons"); */

static gboolean
init_theme_cb (void)
{
  hdy_style_init ();
  hdy_icons_init ();

  return G_SOURCE_REMOVE;
}

/**
 * hdy_constructor:
 *
 * Automatically initializes libhandy.
 */
static void
hdy_constructor (void)
{
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  hdy_init_public_types ();

 /* Initializes the style and icons when the main loop starts, which should be
  * before any window shows up but after GTK is initialized.
  */
  g_idle_add_full (HDY_PRIORITY_STYLE, G_SOURCE_FUNC (init_theme_cb), NULL, NULL);
}

#else
# error Your platform/compiler is missing constructor support
#endif
