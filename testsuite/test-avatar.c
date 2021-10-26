/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

#define TEST_ICON_NAME "avatar-default-symbolic"
#define TEST_STRING "Mario Rossi"
#define TEST_SIZE 128


static void
test_adw_avatar_icon_name (void)
{
  AdwAvatar *avatar = g_object_ref_sink (ADW_AVATAR (adw_avatar_new (128, NULL, TRUE)));

  g_assert_null (adw_avatar_get_icon_name (avatar));
  adw_avatar_set_icon_name (avatar, TEST_ICON_NAME);
  g_assert_cmpstr (adw_avatar_get_icon_name (avatar), ==, TEST_ICON_NAME);

  g_assert_finalize_object (avatar);
}

static void
test_adw_avatar_text (void)
{
  AdwAvatar *avatar = g_object_ref_sink (ADW_AVATAR (adw_avatar_new (128, NULL, TRUE)));

  g_assert_cmpstr (adw_avatar_get_text (avatar), ==, "");
  adw_avatar_set_text (avatar, TEST_STRING);
  g_assert_cmpstr (adw_avatar_get_text (avatar), ==, TEST_STRING);

  g_assert_finalize_object (avatar);
}

static void
test_adw_avatar_size (void)
{
  AdwAvatar *avatar = g_object_ref_sink (ADW_AVATAR (adw_avatar_new (TEST_SIZE, NULL, TRUE)));

  g_assert_cmpint (adw_avatar_get_size (avatar), ==, TEST_SIZE);
  adw_avatar_set_size (avatar, TEST_SIZE / 2);
  g_assert_cmpint (adw_avatar_get_size (avatar), ==, TEST_SIZE / 2);

  g_assert_finalize_object (avatar);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/Avatar/icon_name", test_adw_avatar_icon_name);
  g_test_add_func ("/Adwaita/Avatar/text", test_adw_avatar_text);
  g_test_add_func ("/Adwaita/Avatar/size", test_adw_avatar_size);

  return g_test_run ();
}
