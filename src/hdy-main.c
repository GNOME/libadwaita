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

  gtk_css_provider_load_from_resource (css_fallback_provider, "/sm/puri/handy/style/handy-fallback.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (css_fallback_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);

  css_override_provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_resource (css_override_provider, "/sm/puri/handy/style/handy-override.css");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (css_override_provider),
                                             HDY_STYLE_PROVIDER_PRIORITY_OVERRIDE);

  g_once_init_leave (&guard, 1);
}

static gboolean
init_style_cb (void)
{
  hdy_style_init ();

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

 /* Initializes the style when the main loop starts, which should be before any
  * window shows up but after GTK is initialized.
  */
  g_idle_add (G_SOURCE_FUNC (init_style_cb), NULL);
}

#else
# error Your platform/compiler is missing constructor support
#endif
