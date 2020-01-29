/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-style-private.h"

#include <gtk/gtk.h>

/**
 * PRIVATE:hdy-style
 * @short_description: Style helpers
 * @title: Style
 * @stability: Private
 *
 * Helpers for styles.
 *
 * Since: 1.0
 */

/**
 * hdy_style_init:
 *
 * Initializes the style classes. This must be called once GTK has been
 * initialized.
 *
 * Since: 1.0
 */
void
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
                                             HDY_STYLE_PROVIDER_PRIORITY);

  g_once_init_leave (&guard, 1);
}
