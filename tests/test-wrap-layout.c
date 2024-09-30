/*
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alicem@gnome.org>
 */

#include <adwaita.h>

static void
increment (int *data)
{
  (*data)++;
}

static void
test_adw_wrap_layout_child_spacing (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  int child_spacing;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::child-spacing", G_CALLBACK (increment), &notified);

  g_object_get (layout, "child-spacing", &child_spacing, NULL);
  g_assert_cmpint (child_spacing, ==, 0);

  adw_wrap_layout_set_child_spacing (layout, 0);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_child_spacing (layout, 6);
  g_assert_cmpint (adw_wrap_layout_get_child_spacing (layout), ==, 6);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "child-spacing", 12, NULL);
  g_assert_cmpint (adw_wrap_layout_get_child_spacing (layout), ==, 12);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_child_spacing_unit (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  AdwLengthUnit unit;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::child-spacing-unit", G_CALLBACK (increment), &notified);

  g_object_get (layout, "child-spacing-unit", &unit, NULL);
  g_assert_cmpint (unit, ==, ADW_LENGTH_UNIT_PX);

  adw_wrap_layout_set_child_spacing_unit (layout, ADW_LENGTH_UNIT_PX);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_child_spacing_unit (layout, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (adw_wrap_layout_get_child_spacing_unit (layout), ==, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "child-spacing-unit", ADW_LENGTH_UNIT_PT, NULL);
  g_assert_cmpint (adw_wrap_layout_get_child_spacing_unit (layout), ==, ADW_LENGTH_UNIT_PT);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_pack_direction (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  AdwPackDirection pack_direction;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::pack-direction", G_CALLBACK (increment), &notified);

  g_object_get (layout, "pack-direction", &pack_direction, NULL);
  g_assert_cmpint (pack_direction, ==, ADW_PACK_START_TO_END);

  adw_wrap_layout_set_pack_direction (layout, ADW_PACK_START_TO_END);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_pack_direction (layout, ADW_PACK_END_TO_START);
  g_assert_cmpint (adw_wrap_layout_get_pack_direction (layout), ==, ADW_PACK_END_TO_START);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "pack-direction", ADW_PACK_START_TO_END, NULL);
  g_assert_cmpint (adw_wrap_layout_get_pack_direction (layout), ==, ADW_PACK_START_TO_END);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_align (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  float align;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::align", G_CALLBACK (increment), &notified);

  g_object_get (layout, "align", &align, NULL);
  g_assert_cmpfloat_with_epsilon (align, 0.0, 0.005);

  adw_wrap_layout_set_align (layout, 0);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_align (layout, 0.5);
  g_assert_cmpfloat_with_epsilon (adw_wrap_layout_get_align (layout), 0.5, 0.005);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "align", 1.0f, NULL);
  g_assert_cmpfloat_with_epsilon (adw_wrap_layout_get_align (layout), 1, 0.005);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_justify (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  AdwJustifyMode justify;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::justify", G_CALLBACK (increment), &notified);

  g_object_get (layout, "justify", &justify, NULL);
  g_assert_cmpint (justify, ==, ADW_JUSTIFY_NONE);

  adw_wrap_layout_set_justify (layout, ADW_JUSTIFY_NONE);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_justify (layout, ADW_JUSTIFY_FILL);
  g_assert_cmpint (adw_wrap_layout_get_justify (layout), ==, ADW_JUSTIFY_FILL);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "justify", ADW_JUSTIFY_SPREAD, NULL);
  g_assert_cmpint (adw_wrap_layout_get_justify (layout), ==, ADW_JUSTIFY_SPREAD);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_justify_last_line (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  gboolean justify_last_line;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::justify-last-line", G_CALLBACK (increment), &notified);

  g_object_get (layout, "justify-last-line", &justify_last_line, NULL);
  g_assert_false (justify_last_line);

  adw_wrap_layout_set_justify_last_line (layout, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_justify_last_line (layout, TRUE);
  g_assert_true (adw_wrap_layout_get_justify_last_line (layout));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "justify-last-line", FALSE, NULL);
  g_assert_false (adw_wrap_layout_get_justify_last_line (layout));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_line_spacing (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  int line_spacing;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::line-spacing", G_CALLBACK (increment), &notified);

  g_object_get (layout, "line-spacing", &line_spacing, NULL);
  g_assert_cmpint (line_spacing, ==, 0);

  adw_wrap_layout_set_line_spacing (layout, 0);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_line_spacing (layout, 6);
  g_assert_cmpint (adw_wrap_layout_get_line_spacing (layout), ==, 6);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "line-spacing", 12, NULL);
  g_assert_cmpint (adw_wrap_layout_get_line_spacing (layout), ==, 12);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_line_spacing_unit (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  AdwLengthUnit unit;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::line-spacing-unit", G_CALLBACK (increment), &notified);

  g_object_get (layout, "line-spacing-unit", &unit, NULL);
  g_assert_cmpint (unit, ==, ADW_LENGTH_UNIT_PX);

  adw_wrap_layout_set_line_spacing_unit (layout, ADW_LENGTH_UNIT_PX);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_line_spacing_unit (layout, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (adw_wrap_layout_get_line_spacing_unit (layout), ==, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "line-spacing-unit", ADW_LENGTH_UNIT_PT, NULL);
  g_assert_cmpint (adw_wrap_layout_get_line_spacing_unit (layout), ==, ADW_LENGTH_UNIT_PT);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_line_homogeneous (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  gboolean line_homogeneous;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::line-homogeneous", G_CALLBACK (increment), &notified);

  g_object_get (layout, "line-homogeneous", &line_homogeneous, NULL);
  g_assert_false (line_homogeneous);

  adw_wrap_layout_set_line_homogeneous (layout, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_line_homogeneous (layout, TRUE);
  g_assert_true (adw_wrap_layout_get_line_homogeneous (layout));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "line-homogeneous", FALSE, NULL);
  g_assert_false (adw_wrap_layout_get_line_homogeneous (layout));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_natural_line_length (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  int natural_line_length;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::natural-line-length", G_CALLBACK (increment), &notified);

  g_object_get (layout, "natural-line-length", &natural_line_length, NULL);
  g_assert_cmpint (natural_line_length, ==, -1);

  adw_wrap_layout_set_natural_line_length (layout, -1);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_natural_line_length (layout, 200);
  g_assert_cmpint (adw_wrap_layout_get_natural_line_length (layout), ==, 200);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "natural-line-length", 300, NULL);
  g_assert_cmpint (adw_wrap_layout_get_natural_line_length (layout), ==, 300);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_natural_line_length_unit (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  AdwLengthUnit unit;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::natural-line-length-unit", G_CALLBACK (increment), &notified);

  g_object_get (layout, "natural-line-length-unit", &unit, NULL);
  g_assert_cmpint (unit, ==, ADW_LENGTH_UNIT_PX);

  adw_wrap_layout_set_natural_line_length_unit (layout, ADW_LENGTH_UNIT_PX);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_natural_line_length_unit (layout, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (adw_wrap_layout_get_natural_line_length_unit (layout), ==, ADW_LENGTH_UNIT_SP);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "natural-line-length-unit", ADW_LENGTH_UNIT_PT, NULL);
  g_assert_cmpint (adw_wrap_layout_get_natural_line_length_unit (layout), ==, ADW_LENGTH_UNIT_PT);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_wrap_reverse (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  gboolean wrap_reverse;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::wrap-reverse", G_CALLBACK (increment), &notified);

  g_object_get (layout, "wrap-reverse", &wrap_reverse, NULL);
  g_assert_false (wrap_reverse);

  adw_wrap_layout_set_wrap_reverse (layout, FALSE);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_wrap_reverse (layout, TRUE);
  g_assert_true (adw_wrap_layout_get_wrap_reverse (layout));
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "wrap-reverse", FALSE, NULL);
  g_assert_false (adw_wrap_layout_get_wrap_reverse (layout));
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

static void
test_adw_wrap_layout_wrap_policy (void)
{
  AdwWrapLayout *layout = ADW_WRAP_LAYOUT (adw_wrap_layout_new ());
  AdwWrapPolicy wrap_policy;
  int notified = 0;

  g_assert_nonnull (layout);

  g_signal_connect_swapped (layout, "notify::wrap-policy", G_CALLBACK (increment), &notified);

  g_object_get (layout, "wrap-policy", &wrap_policy, NULL);
  g_assert_cmpint (wrap_policy, ==, ADW_WRAP_NATURAL);

  adw_wrap_layout_set_wrap_policy (layout, ADW_WRAP_NATURAL);
  g_assert_cmpint (notified, ==, 0);

  adw_wrap_layout_set_wrap_policy (layout, ADW_WRAP_MINIMUM);
  g_assert_cmpint (adw_wrap_layout_get_wrap_policy (layout), ==, ADW_WRAP_MINIMUM);
  g_assert_cmpint (notified, ==, 1);

  g_object_set (layout, "wrap-policy", ADW_WRAP_NATURAL, NULL);
  g_assert_cmpint (adw_wrap_layout_get_wrap_policy (layout), ==, ADW_WRAP_NATURAL);
  g_assert_cmpint (notified, ==, 2);

  g_assert_finalize_object (layout);
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func("/Adwaita/WrapLayout/child_spacing", test_adw_wrap_layout_child_spacing);
  g_test_add_func("/Adwaita/WrapLayout/child_spacing_unit", test_adw_wrap_layout_child_spacing_unit);
  g_test_add_func("/Adwaita/WrapLayout/pack_direction", test_adw_wrap_layout_pack_direction);
  g_test_add_func("/Adwaita/WrapLayout/align", test_adw_wrap_layout_align);
  g_test_add_func("/Adwaita/WrapLayout/justify", test_adw_wrap_layout_justify);
  g_test_add_func("/Adwaita/WrapLayout/justify_last_line", test_adw_wrap_layout_justify_last_line);
  g_test_add_func("/Adwaita/WrapLayout/line_spacing", test_adw_wrap_layout_line_spacing);
  g_test_add_func("/Adwaita/WrapLayout/line_spacing_unit", test_adw_wrap_layout_line_spacing_unit);
  g_test_add_func("/Adwaita/WrapLayout/line_homogeneous", test_adw_wrap_layout_line_homogeneous);
  g_test_add_func("/Adwaita/WrapLayout/natural_line_length", test_adw_wrap_layout_natural_line_length);
  g_test_add_func("/Adwaita/WrapLayout/natural_line_length_unit", test_adw_wrap_layout_natural_line_length_unit);
  g_test_add_func("/Adwaita/WrapLayout/wrap_reverse", test_adw_wrap_layout_wrap_reverse);
  g_test_add_func("/Adwaita/WrapLayout/wrap_policy", test_adw_wrap_layout_wrap_policy);

  return g_test_run();
}
