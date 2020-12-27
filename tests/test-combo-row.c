/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <handy.h>

gint notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

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
test_hdy_combo_row_selected_index (void)
{
  g_autoptr (HdyComboRow) row = NULL;
  gint selected_index = 0;

  row = HDY_COMBO_ROW (g_object_ref_sink (hdy_combo_row_new ()));
  g_assert_nonnull (row);

  notified = 0;
  g_signal_connect (row, "notify::selected-index", G_CALLBACK (notify_cb), NULL);

  g_object_get (row, "selected-index", &selected_index, NULL);
  g_assert_cmpint (selected_index, ==, -1);

  hdy_combo_row_set_selected_index (row, -1);
  g_assert_cmpint (notified, ==, 0);

  hdy_combo_row_set_for_enum (row, GTK_TYPE_SELECTION_MODE, hdy_enum_value_row_name, NULL, NULL);
  g_assert_cmpint (hdy_combo_row_get_selected_index (row), ==, 0);
  g_assert_cmpint (notified, ==, 1);

  hdy_combo_row_set_selected_index (row, 3);
  g_assert_cmpint (hdy_combo_row_get_selected_index (row), ==, 3);
  g_assert_cmpint (notified, ==, 2);

  g_object_set (row, "selected-index", 1, NULL);
  g_assert_cmpint (hdy_combo_row_get_selected_index (row), ==, 1);
  g_assert_cmpint (notified, ==, 3);
}

static void
test_hdy_combo_row_use_subtitle (void)
{
  g_autoptr (HdyComboRow) row = NULL;
  gboolean use_subtitle = FALSE;

  row = g_object_ref_sink (HDY_COMBO_ROW (hdy_combo_row_new ()));
  g_assert_nonnull (row);

  notified = 0;
  g_signal_connect (row, "notify::use-subtitle", G_CALLBACK (notify_cb), NULL);

  g_assert_false (hdy_combo_row_get_use_subtitle (row));

  hdy_combo_row_set_use_subtitle (row, FALSE);
  g_assert_cmpint (notified, ==, 0);

  hdy_combo_row_set_use_subtitle (row, TRUE);
  g_assert_true (hdy_combo_row_get_use_subtitle (row));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (row, "use-subtitle", FALSE, NULL);
  g_object_get (row, "use-subtitle", &use_subtitle, NULL);
  g_assert_false (use_subtitle);
  g_assert_cmpint (notified, ==, 2);
}


gint
main (gint argc,
      gchar *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  hdy_init ();

  g_test_add_func("/Handy/ComboRow/set_for_enum", test_hdy_combo_row_set_for_enum);
  g_test_add_func("/Handy/ComboRow/selected_index", test_hdy_combo_row_selected_index);
  g_test_add_func("/Handy/ComboRow/use_subtitle", test_hdy_combo_row_use_subtitle);

  return g_test_run();
}
