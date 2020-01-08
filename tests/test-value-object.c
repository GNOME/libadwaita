/*
 * Copyright (C) 2019 Red Hat Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_value_object_init (void)
{
  HdyValueObject *obj;
  GValue value = G_VALUE_INIT;
  gchar *str;

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, "asdfasdf");
  obj = hdy_value_object_new (&value);
  g_assert_cmpstr (hdy_value_object_get_string (obj), ==, "asdfasdf");
  g_clear_object (&obj);

  obj = hdy_value_object_new_string ("asdfasdf");
  g_assert_cmpstr (hdy_value_object_get_string (obj), ==, "asdfasdf");
  g_clear_object (&obj);

  obj = hdy_value_object_new_take_string (g_strdup ("asdfasdf"));
  g_assert_cmpstr (hdy_value_object_get_string (obj), ==, "asdfasdf");
  g_clear_object (&obj);

  obj = hdy_value_object_new_collect (G_TYPE_STRING, "asdfasdf");
  g_assert_cmpstr (hdy_value_object_get_string (obj), ==, "asdfasdf");

  /* And check that _dup_string works too */
  str = hdy_value_object_dup_string (obj);
  g_assert_cmpstr (str, ==, "asdfasdf");
  g_clear_pointer (&str, g_free);
  g_clear_object (&obj);

  g_value_unset (&value);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/ValueObject/init", test_hdy_value_object_init);

  return g_test_run();
}
