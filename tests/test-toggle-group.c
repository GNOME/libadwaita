/*
 * Copyright (C) 2023 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_toggle_group_add_remove (void)
{
  AdwToggleGroup *toggle_group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  const char *id = "some_id";

  g_assert_nonnull (toggle_group);

  adw_toggle_group_add_toggle (toggle_group, id, "label", NULL);
  g_assert_cmpstr (adw_toggle_group_get_active (toggle_group), ==, id);

  adw_toggle_group_remove_toggle (toggle_group, id);
  g_assert_cmpstr (adw_toggle_group_get_active (toggle_group), ==, "");

  g_assert_finalize_object (toggle_group);
}

static void
test_adw_toggle_group_use_underline (void)
{
  AdwToggleGroup *toggle_group = g_object_ref_sink (ADW_TOGGLE_GROUP (adw_toggle_group_new ()));
  const char *id = "some_id";

  g_assert_nonnull (toggle_group);

  adw_toggle_group_add_toggle (toggle_group, id, "_label", NULL);

  adw_toggle_group_set_use_underline (toggle_group, id, TRUE);
  g_assert (adw_toggle_group_get_use_underline (toggle_group, id));

  adw_toggle_group_set_use_underline (toggle_group, id, FALSE);
  g_assert (!adw_toggle_group_get_use_underline (toggle_group, id));

  g_assert_finalize_object (toggle_group);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/ToggleGroup/use_underline", test_adw_toggle_group_use_underline);
  g_test_add_func ("/Adwaita/ToggleGroup/add_remove", test_adw_toggle_group_add_remove);

  return g_test_run ();
}
