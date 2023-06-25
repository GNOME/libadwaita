/*
 * Copyright (C) 2022 Christopher Davis <christopherdavis@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <adwaita.h>

static void
test_adw_spin_row_new_with_range (void)
{
  AdwSpinRow *row = g_object_ref_sink (ADW_SPIN_ROW (adw_spin_row_new_with_range (0, 100, 1)));
  GtkAdjustment *adjustment;

  g_assert_nonnull (row);

  adjustment = adw_spin_row_get_adjustment (row);

  g_assert_nonnull (adjustment);

  g_assert_true (G_APPROX_VALUE (gtk_adjustment_get_lower (adjustment), 0, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (gtk_adjustment_get_upper (adjustment), 100, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (gtk_adjustment_get_step_increment (adjustment), 1, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (gtk_adjustment_get_page_increment (adjustment), 10, DBL_EPSILON));

  g_assert_finalize_object (row);
}

static void
test_adw_spin_row_configure (void)
{
  AdwSpinRow *row = g_object_ref_sink (ADW_SPIN_ROW (adw_spin_row_new_with_range (0, 1, 1)));
  GtkAdjustment *adjustment = gtk_adjustment_new (50, 0, 100, 1, 10, 0);

  g_assert_nonnull (row);
  g_assert_nonnull (adjustment);

  adw_spin_row_configure (row, g_object_ref (adjustment), 2, 2);

  g_assert_true (G_APPROX_VALUE (adw_spin_row_get_value (row), 50, DBL_EPSILON));
  g_assert_true (G_APPROX_VALUE (adw_spin_row_get_climb_rate (row), 2, DBL_EPSILON));
  g_assert_cmpint (adw_spin_row_get_digits (row), ==, 2);
  g_assert_true (adw_spin_row_get_adjustment (row) == adjustment);

  g_assert_finalize_object (row);
  g_assert_finalize_object (adjustment);
}

static void
test_adw_spin_row_set_range (void)
{
  AdwSpinRow *row = g_object_ref_sink (ADW_SPIN_ROW (adw_spin_row_new_with_range (1, 2, 1)));

  g_assert_true (G_APPROX_VALUE (adw_spin_row_get_value (row), 1, DBL_EPSILON));

  adw_spin_row_set_range (row, 2, 3);

  g_assert_true (G_APPROX_VALUE (adw_spin_row_get_value (row), 2, DBL_EPSILON));
  g_assert_finalize_object (row);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/SpinRow/new_with_range", test_adw_spin_row_new_with_range);
  g_test_add_func("/Adwaita/SpinRow/configure", test_adw_spin_row_configure);
  g_test_add_func("/Adwaita/SpinRow/set_range", test_adw_spin_row_set_range);

  return g_test_run();
}
