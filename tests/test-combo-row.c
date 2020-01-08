/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>


static void
test_hdy_combo_row_set_for_enum (void)
{
  g_autoptr (HdyComboRow) row = NULL;
  GListModel *model;
  HdyEnumValueObject *value;

  row = g_object_ref_sink (HDY_COMBO_ROW (hdy_combo_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (hdy_combo_row_get_model (row));

  hdy_combo_row_set_for_enum (row, GTK_TYPE_ORIENTATION, hdy_enum_value_row_name, NULL, NULL);
  model = hdy_combo_row_get_model (row);
  g_assert_true (G_IS_LIST_MODEL (model));

  g_assert_cmpuint (g_list_model_get_n_items (model), ==, 2);

  value = g_list_model_get_item (model, 0);
  g_assert_true (HDY_IS_ENUM_VALUE_OBJECT (value));
  g_assert_cmpstr (hdy_enum_value_object_get_nick (value), ==, "horizontal");

  value = g_list_model_get_item (model, 1);
  g_assert_true (HDY_IS_ENUM_VALUE_OBJECT (value));
  g_assert_cmpstr (hdy_enum_value_object_get_nick (value), ==, "vertical");
}


static void
test_hdy_combo_row_use_subtitle (void)
{
  g_autoptr (HdyComboRow) row = NULL;

  row = g_object_ref_sink (HDY_COMBO_ROW (hdy_combo_row_new ()));
  g_assert_nonnull (row);

  g_assert_false (hdy_combo_row_get_use_subtitle (row));

  hdy_combo_row_set_use_subtitle (row, TRUE);
  g_assert_true (hdy_combo_row_get_use_subtitle (row));

  hdy_combo_row_set_use_subtitle (row, FALSE);
  g_assert_false (hdy_combo_row_get_use_subtitle (row));
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/ComboRow/set_for_enum", test_hdy_combo_row_set_for_enum);
  g_test_add_func("/Handy/ComboRow/use_subtitle", test_hdy_combo_row_use_subtitle);

  return g_test_run();
}
