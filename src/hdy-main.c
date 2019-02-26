/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */
#include "config.h"
#include "hdy-main-private.h"
#include <gio/gio.h>
#include <glib/gi18n-lib.h>

static gint hdy_initialized = FALSE;

/**
 * SECTION:hdy-main
 * @Short_description: Library initialization
 * @Title: hdy-main
 *
 * Before using the Handy libarary you should initialize it. This makes
 * sure translations for the Handy library are set up properly.
 */

GResource *hdy_get_resource (void);

/**
 * hdy_init:
 * @argc: (inout) (optional): Address of the <parameter>argc</parameter>
 *     parameter of your main() function (or 0 if @argv is %NULL). This will be
 *     changed if any arguments were handled.
 * @argv: (array length=argc) (inout) (nullable) (optional) (transfer none):
 *     Address of the <parameter>argv</parameter> parameter of main(), or %NULL.
 *     Any options understood by Handy are stripped before return.
 *
 * Call this function before using any other Handy functions in your
 * GUI applications. If libhandy has already been initialized, the function will
 * simply return without processing the new arguments.
 *
 * Returns: %TRUE if initialization was successful, %FALSE otherwise.
 */
gboolean
hdy_init (int *argc, char ***argv)
{
  if (hdy_initialized)
    return TRUE;

  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  g_resources_register (hdy_get_resource ());
  hdy_init_public_types ();

  hdy_initialized = TRUE;

  return TRUE;
}
