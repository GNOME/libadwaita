/*
 * Copyright (C) 2020 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>

#define TEST_ICON_NAME "avatar-default-symbolic"
#define TEST_STRING "Mario Rossi"
#define TEST_SIZE 128


static void
test_hdy_avatar_icon_name (void)
{
  HdyAvatar *avatar = HDY_AVATAR (hdy_avatar_new (128, NULL, TRUE));

  g_assert_null (hdy_avatar_get_icon_name (avatar));
  hdy_avatar_set_icon_name (avatar, TEST_ICON_NAME);
  g_assert_cmpstr (hdy_avatar_get_icon_name (avatar), ==, TEST_ICON_NAME);
}

static void
test_hdy_avatar_text (void)
{
  HdyAvatar *avatar = HDY_AVATAR (hdy_avatar_new (128, NULL, TRUE));

  g_assert_null (hdy_avatar_get_text (avatar));
  hdy_avatar_set_text (avatar, TEST_STRING);
  g_assert_cmpstr (hdy_avatar_get_text (avatar), ==, TEST_STRING);
}

static void
test_hdy_avatar_size (void)
{
  HdyAvatar *avatar = HDY_AVATAR (hdy_avatar_new (TEST_SIZE, NULL, TRUE));

  g_assert_cmpint (hdy_avatar_get_size (avatar), ==, TEST_SIZE);
  hdy_avatar_set_size (avatar, TEST_SIZE / 2);
  g_assert_cmpint (hdy_avatar_get_size (avatar), ==, TEST_SIZE / 2);
}

static void
test_hdy_avatar_draw_to_pixbuf (void)
{
  g_autoptr (HdyAvatar) avatar = NULL;
  g_autoptr (GdkPixbuf) pixbuf = NULL;

  avatar = g_object_ref_sink (HDY_AVATAR (hdy_avatar_new (TEST_SIZE, NULL, TRUE)));

  pixbuf = hdy_avatar_draw_to_pixbuf (avatar, TEST_SIZE * 2, 1);

  g_assert_cmpint (gdk_pixbuf_get_width (pixbuf), ==, TEST_SIZE * 2);
  g_assert_cmpint (gdk_pixbuf_get_height (pixbuf), ==, TEST_SIZE * 2);
}

gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func ("/Handy/Avatar/icon_name", test_hdy_avatar_icon_name);
  g_test_add_func ("/Handy/Avatar/text", test_hdy_avatar_text);
  g_test_add_func ("/Handy/Avatar/size", test_hdy_avatar_size);
  g_test_add_func ("/Handy/Avatar/draw_to_pixbuf", test_hdy_avatar_draw_to_pixbuf);

  return g_test_run ();
}
