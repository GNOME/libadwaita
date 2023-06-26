/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static void
test_easing_ease (gconstpointer data)
{
  AdwEasing easing = GPOINTER_TO_INT (data);

  g_assert_cmpfloat_with_epsilon (adw_easing_ease (easing, 0), 0, 0.005);
  g_assert_cmpfloat_with_epsilon (adw_easing_ease (easing, 1), 1, 0.005);
}

int
main (int   argc,
      char *argv[])
{
  GEnumClass *enum_class;
  guint i;

  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  enum_class = g_type_class_ref (ADW_TYPE_EASING);

  for (i = 0; i < enum_class->n_values; i++) {
    GEnumValue *value = &enum_class->values[i];
    char *path = g_strdup_printf ("/Adwaita/Easing/%s", value->value_nick);

    g_test_add_data_func (path, GINT_TO_POINTER (value->value), test_easing_ease);

    g_free (path);
  }

  g_type_class_unref (enum_class);

  return g_test_run();
}
