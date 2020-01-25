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
#include "hdy-style-private.h"

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
