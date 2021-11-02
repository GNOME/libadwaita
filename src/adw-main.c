/*
 * Copyright (C) 2018-2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "config.h"

#include "adw-main-private.h"

#include "adw-style-manager-private.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

static int adw_initialized = FALSE;

/**
 * adw_init:
 *
 * Initializes Libadwaita.
 *
 * This function can be used instead of [func@Gtk.init] as it initializes GTK
 * implicitly.
 *
 * There's no need to call this function if you're using [class@Adw.Application].
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

  gtk_init ();

  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  adw_init_public_types ();

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_for_display (gdk_display_get_default ()),
                                    "/org/gnome/Adwaita/icons");

  adw_style_manager_ensure ();

  adw_initialized = TRUE;
}

/**
 * adw_is_initialized:
 *
 * Use this function to check if libadwaita has been initialized with
 * adw_init().
 *
 * Returns: the initialization status
 */
gboolean
adw_is_initialized (void)
{
  return adw_initialized;
}
