/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alexander Mikhaylenko <alexander.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

#include "adw-inspector-page-private.h"

static gboolean
get_force_inspector (void)
{
  const char *disable_portal = g_getenv ("ADW_FORCE_INSPECTOR");

  return disable_portal && disable_portal[0] == '1';
}


void
g_io_module_load (GIOModule *module)
{
  if (!adw_is_initialized ()) {
    if (!get_force_inspector ())
      return;

    adw_init ();
  }

  g_type_module_use (G_TYPE_MODULE (module));

  g_io_extension_point_implement ("gtk-inspector-page",
                                  ADW_TYPE_INSPECTOR_PAGE,
                                  "libadwaita",
                                  10);
}

void
g_io_module_unload (GIOModule *module)
{
}
