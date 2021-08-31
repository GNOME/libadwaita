/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

int notified;

static void
notify_cb (GtkWidget *widget, gpointer data)
{
  notified++;
}

static void
test_adw_combo_row_set_for_enum (void)
{
  g_autoptr (AdwComboRow) row = NULL;
  GtkExpression *expr = NULL;
  GListModel *model;
  AdwEnumListItem *item;

  row = g_object_ref_sink (ADW_COMBO_ROW (adw_combo_row_new ()));
  g_assert_nonnull (row);

  g_assert_null (adw_combo_row_get_model (row));

  expr = gtk_property_expression_new (ADW_TYPE_ENUM_LIST_ITEM, NULL, "nick");
  adw_combo_row_set_expression (row, expr);
  gtk_expression_unref (expr);

  model = G_LIST_MODEL (adw_enum_list_model_new (GTK_TYPE_ORIENTATION));
  adw_combo_row_set_model (row, model);
  g_object_unref (model);

  model = adw_combo_row_get_model (row);
  g_assert_true (G_IS_LIST_MODEL (model));

  g_assert_cmpuint (g_list_model_get_n_items (model), ==, 2);

  item = g_list_model_get_item (model, 0);
  g_assert_true (ADW_IS_ENUM_LIST_ITEM (item));
  g_assert_cmpstr (adw_enum_list_item_get_nick (item), ==, "horizontal");

  item = g_list_model_get_item (model, 1);
  g_assert_true (ADW_IS_ENUM_LIST_ITEM (item));
  g_assert_cmpstr (adw_enum_list_item_get_nick (item), ==, "vertical");
}

static void
test_adw_combo_row_selected (void)
{
  g_autoptr (AdwComboRow) row = NULL;
  g_autoptr (GListModel) model = NULL;
  int selected = 0;

  row = ADW_COMBO_ROW (g_object_ref_sink (adw_combo_row_new ()));
  g_assert_nonnull (row);

  notified = 0;
  g_signal_connect (row, "notify::selected", G_CALLBACK (notify_cb), NULL);

  g_object_get (row, "selected", &selected, NULL);
  g_assert_cmpint (selected, ==, -1);

  adw_combo_row_set_selected (row, -1);
  g_assert_cmpint (notified, ==, 0);

  model = G_LIST_MODEL (adw_enum_list_model_new (GTK_TYPE_SELECTION_MODE));

  adw_combo_row_set_model (row, model);

  g_assert_cmpint (adw_combo_row_get_selected (row), ==, 0);
  g_assert_cmpint (notified, ==, 1);

  adw_combo_row_set_selected (row, 3);
  g_assert_cmpint (adw_combo_row_get_selected (row), ==, 3);
  g_assert_cmpint (notified, ==, 2);

  g_object_set (row, "selected", 1, NULL);
  g_assert_cmpint (adw_combo_row_get_selected (row), ==, 1);
  g_assert_cmpint (notified, ==, 3);
}

static void
test_adw_combo_row_use_subtitle (void)
{
  g_autoptr (AdwComboRow) row = NULL;
  gboolean use_subtitle = FALSE;

  row = g_object_ref_sink (ADW_COMBO_ROW (adw_combo_row_new ()));
  g_assert_nonnull (row);

  notified = 0;
  g_signal_connect (row, "notify::use-subtitle", G_CALLBACK (notify_cb), NULL);

  g_assert_false (adw_combo_row_get_use_subtitle (row));

  adw_combo_row_set_use_subtitle (row, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_combo_row_set_use_subtitle (row, TRUE);
  g_assert_true (adw_combo_row_get_use_subtitle (row));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (row, "use-subtitle", FALSE, NULL);
  g_object_get (row, "use-subtitle", &use_subtitle, NULL);
  g_assert_false (use_subtitle);
  g_assert_cmpint (notified, ==, 2);
}


int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/ComboRow/set_for_enum", test_adw_combo_row_set_for_enum);
  g_test_add_func("/Adwaita/ComboRow/selected", test_adw_combo_row_selected);
  g_test_add_func("/Adwaita/ComboRow/use_subtitle", test_adw_combo_row_use_subtitle);

  return g_test_run();
}
