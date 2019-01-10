/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define HANDY_USE_UNSTABLE_API
#include <handy.h>

static void
test_hdy_string_utf8_truncate(void)
{
  GString *str;

  str = g_string_new("foo_bar");
  g_assert_cmpstr("foo_", ==, hdy_string_utf8_truncate(str, 4)->str);
  g_assert_cmpint(4 , ==, str->len);

  str = g_string_new("☃f☃o☃o");
  g_assert_cmpstr("☃f☃o", ==, hdy_string_utf8_truncate(str, 4)->str);
  g_assert_cmpint(8 , ==, str->len);

  str = g_string_new("☃f☃o☃o");
  g_assert_cmpstr("☃f☃o☃o", ==, hdy_string_utf8_truncate(str, 10)->str);
  g_assert_cmpint(12, ==, str->len);

  str = g_string_new("☃f☃o☃o");
  g_assert_cmpstr("", ==, hdy_string_utf8_truncate(str, 0)->str);
  g_assert_cmpint(0, ==, str->len);
}


static void
test_hdy_string_utf8_len(void)
{
  GString *str;

  str = g_string_new("foo_bar");
  g_assert_cmpint(7, ==, hdy_string_utf8_len(str));

  str = g_string_new("☃f☃o☃o");
  g_assert_cmpint(6, ==, hdy_string_utf8_len(str));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init (&argc, &argv);

  g_test_add_func("/Handy/StringUTF8/truncate", test_hdy_string_utf8_truncate);
  g_test_add_func("/Handy/StringUTF8/len", test_hdy_string_utf8_len);
  return g_test_run();
}
